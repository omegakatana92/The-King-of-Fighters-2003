#!/usr/bin/env bash
# Build the Linux x86_64 player and package it as a Steam Deck AppImage.
#
# Prerequisites: gcc, g++, cmake 3.21+, make, libsdl2-dev, wget (and FUSE,
# or appimagetool's --appimage-extract-and-run path used below).
# The user's ROMs are never bundled: the AppImage contains the recompiled
# player and the build-time preparation tool only.
#
# Usage:
#   ./packaging/appimage/build-appimage.sh [ROM_DIR]
# Default ROM_DIR=$ROOT/roms-src, output dist/steamdeck/...
#
# Override the build directory when the source tree is on a slow shared
# filesystem:
#   KOF2003_BUILD_DIR=$HOME/kof2003-build ./packaging/appimage/build-appimage.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ROMS="${1:-$ROOT/roms-src}"
BUILD="${KOF2003_BUILD_DIR:-$ROOT/build-linux}"
APPDIR="$BUILD/AppDir"
OUT="$ROOT/dist/steamdeck/KOF 2003-SteamDeck-x86_64.AppImage"

echo "== configure/build (Release) =="
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release "-DKOF2003_ROM_DIR=$ROMS"
cmake --build "$BUILD" --parallel "${KOF2003_JOBS:-2}"

echo "== assemble AppDir =="
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" \
         "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
         "$APPDIR/usr/share/doc/kof2003"
cp "$BUILD/KOF 2003" "$APPDIR/usr/bin/KOF2003"
cp "$BUILD/kof2003_decrypt" "$APPDIR/usr/bin/kof2003_decrypt"
strip --strip-unneeded "$APPDIR/usr/bin/KOF2003" 2>/dev/null || true
strip --strip-unneeded "$APPDIR/usr/bin/kof2003_decrypt" 2>/dev/null || true

# Bundle the SDL2 the build linked against; SteamOS's system copy may differ.
SDL=$(ldd "$BUILD/KOF 2003" | awk '/libSDL2-2\.0\.so\.0/{print $3; exit}')
if [ -n "$SDL" ]; then cp "$SDL" "$APPDIR/usr/lib/"; fi

cp "$ROOT/packaging/appimage/kof2003.png" "$APPDIR/kof2003.png"
cp "$ROOT/packaging/appimage/kof2003.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/kof2003.png"
cp "$ROOT/packaging/appimage/kof2003.desktop" "$APPDIR/kof2003.desktop"
cp "$ROOT/packaging/appimage/kof2003.desktop" "$APPDIR/usr/share/applications/kof2003.desktop"
cp "$ROOT/packaging/appimage/README.txt" "$APPDIR/usr/share/doc/kof2003/README.txt"
cp "$ROOT/LICENSE" "$APPDIR/usr/share/doc/kof2003/LICENSE.txt"
cp "$ROOT/LICENSE-STATUS.md" "$APPDIR/usr/share/doc/kof2003/LICENSE-STATUS.txt"
cp "$ROOT/THIRD_PARTY_NOTICES.md" "$APPDIR/usr/share/doc/kof2003/THIRD_PARTY_NOTICES.txt"
mkdir -p "$APPDIR/usr/share/doc/kof2003/LICENSES"
cp "$ROOT/neogeorecomp/LICENSE" "$APPDIR/usr/share/doc/kof2003/LICENSES/neogeorecomp.txt"
cp "$ROOT/neogeorecomp/NOTICE" "$APPDIR/usr/share/doc/kof2003/LICENSES/neogeorecomp-NOTICE.txt"
cp "$ROOT/neogeorecomp/LICENSES/"*.txt "$APPDIR/usr/share/doc/kof2003/LICENSES/"
cp "$ROOT/packaging/windows/SDL2.txt" "$APPDIR/usr/share/doc/kof2003/LICENSES/SDL2.txt"

# Compatibility, testing and ROM-validation reports, plus the long README.
mkdir -p "$APPDIR/usr/share/doc/kof2003/docs"
cp "$ROOT/docs/"*.md "$APPDIR/usr/share/doc/kof2003/docs/"
cp "$ROOT/README.md" "$APPDIR/usr/share/doc/kof2003/README.md"

cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# Steam Input / Gaming Mode may deliver a virtual controller while the
# overlay has focus; let SDL keep reading it.
export SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS=1
export SDL_GAMECONTROLLER_USE_BUTTON_LABELS=0

# An explicit --rom-path always wins.
has_rom=0
for a in "$@"; do
    [ "$a" = "--rom-path" ] && has_rom=1
done

if [ "$has_rom" = 0 ]; then
    cand=""
    if [ -n "${KOF2003_ROM_DIR:-}" ]; then
        cand="$KOF2003_ROM_DIR"
    elif [ -n "${APPIMAGE:-}" ]; then
        for d in "$(dirname "$APPIMAGE")/roms" \
                 "$(dirname "$APPIMAGE")/roms/kof2003" \
                 "$HOME/.local/share/kof2003/roms" \
                 "$HOME/Emulation/roms/kof2003" \
                 "$HOME/roms/kof2003" "$HOME/roms"; do
            if [ -d "$d" ]; then cand="$d"; break; fi
        done
    fi
    if [ -n "$cand" ]; then
        if [ -f "$cand/271-s1.s1" ]; then
            # Already prepared (the extracted S ROM is the marker).
            set -- --rom-path "$cand" "$@"
        elif [ -f "$cand/271-p1c.p1" ] && [ -f "$cand/271-m1c.m1" ]; then
            # The user pointed us at the original encrypted set: prepare a
            # per-user copy once, then launch from it.
            CACHE="$HOME/.local/share/kof2003/roms"
            if [ ! -f "$CACHE/271-s1.s1" ]; then
                mkdir -p "$CACHE"
                "$HERE/usr/bin/kof2003_decrypt" --rom-path "$cand" --out "$CACHE" || exit 1
            fi
            set -- --rom-path "$CACHE" "$@"
        else
            set -- --rom-path "$cand" "$@"
        fi
    fi
fi

# Steam Deck / Gaming Mode wants a fullscreen game.  Keep --headless and an
# explicit --scale/--fullscreen from being overridden.
add_fs=1
for a in "$@"; do
    case "$a" in --headless|--fullscreen|--scale) add_fs=0;; esac
done
[ "$add_fs" = 1 ] && set -- --fullscreen "$@"

exec "$HERE/usr/bin/KOF2003" "$@"
EOF
chmod +x "$APPDIR/AppRun"

echo "== appimagetool =="
mkdir -p "$ROOT/dist/steamdeck"
cp "$ROOT/packaging/appimage/README.txt" "$ROOT/dist/steamdeck/README.txt"
mkdir -p "$ROOT/dist/steamdeck/LICENSES"
cp "$ROOT/LICENSE" "$ROOT/dist/steamdeck/LICENSES/LICENSE.txt"
cp "$ROOT/LICENSE-STATUS.md" "$ROOT/dist/steamdeck/LICENSES/LICENSE-STATUS.txt"
cp "$ROOT/THIRD_PARTY_NOTICES.md" "$ROOT/dist/steamdeck/LICENSES/THIRD_PARTY_NOTICES.txt"
cp "$ROOT/neogeorecomp/LICENSE" "$ROOT/dist/steamdeck/LICENSES/neogeorecomp.txt"
cp "$ROOT/neogeorecomp/NOTICE" "$ROOT/dist/steamdeck/LICENSES/neogeorecomp-NOTICE.txt"
cp "$ROOT/neogeorecomp/LICENSES/"*.txt "$ROOT/dist/steamdeck/LICENSES/"
cp "$ROOT/packaging/windows/SDL2.txt" "$ROOT/dist/steamdeck/LICENSES/SDL2.txt"
TOOL="${APPIMAGETOOL:-$HOME/appimagetool-x86_64.AppImage}"
if [ ! -x "$TOOL" ]; then
    TOOL="$BUILD/appimagetool.AppImage"
    wget -q -O "$TOOL" \
      https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x "$TOOL"
fi
ARCH=x86_64 "$TOOL" --appimage-extract-and-run --no-appstream "$APPDIR" "$OUT"
chmod +x "$OUT"
echo "wrote $OUT"

