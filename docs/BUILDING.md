# Building KOF 2003

## Prerequisites

- Git, CMake 3.21+.
- **Windows:** Visual Studio 2022/2026 with "Desktop development with C++"
  (MSVC x64).  SDL2 development package (2.30+; 2.32.10 was used here).
- **Linux:** `gcc`, `g++`, `make`, CMake, `libsdl2-dev`.
- Your own unzipped `kof2003` and `neogeo` sets.  Nothing derived from them is
  committed.

## Windows

```
cmake -S . -B build-windows -G "Visual Studio 18 2026" -A x64 ^
  -DKOF2003_ROM_DIR=C:/path/to/unzipped/sets ^
  -DSDL2_DIR=C:/path/to/SDL2-2.32.10/cmake
cmake --build build-windows --config Release --parallel
```

Outputs:

```
build-windows/Release/KOF 2003.exe
build-windows/Release/kof2003_decrypt.exe
build-windows/generated/*.c          (ROM-derived, not committed)
build-windows/roms/                  (prepared set)
```

The generated C is large.  `CMakeLists.txt` compiles those files with
`/bigobj;/O1`; MSVC's `/O2` is superlinear on them.  Leave that alone.

Without `KOF2003_ROM_DIR` the player builds interpreter-only, which is useful
for checking the board model before recompilation.

## Linux

```
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release \
  -DKOF2003_ROM_DIR=/path/to/unzipped/sets
cmake --build build-linux --parallel
```

The Linux build uses the vendored `neogeorecomp/` and finds SDL2 through
`pkg-config`/CMake.  If several builds are running in parallel on a small VM,
use `--parallel 1` for the generated C; the files are memory-hungry.

Output: `build-linux/KOF 2003`.

## Profile pass

```
cmake --build build-windows --config Release --target profile
cmake --build build-windows --config Release
```

`profile` runs a 6,000-frame attract pass and two scripted sessions, writes
the runtime-only entry points to `build-windows/entries.txt`, then the next
build regenerates the dispatch table.  The recorded result was 18,439 entry
points and 100.00% native on the 9,000-frame coin/start session.

For Linux, copy the same entry list before rebuilding:

```
cp build-windows/entries.txt build-linux/entries.txt
touch build-linux/entries.txt
cmake --build build-linux --parallel 1
```

## Packaging

Windows:

```
powershell -ExecutionPolicy Bypass -File packaging\windows\package-windows.ps1
```

This writes `dist/windows/KOF 2003-Windows-x64/` and its zip, copying the
player, decrypt helper, SDL2, VC runtime DLLs, README, license texts and a
SHA-256 manifest.  It never copies ROMs.

Steam Deck:

```
./packaging/appimage/build-appimage.sh /path/to/unzipped/sets
```

This builds `build-linux` if needed and writes
`dist/steamdeck/KOF 2003-SteamDeck-x86_64.AppImage`.  It bundles the SDL2 the
binary linked against and the license texts, plus an `AppRun` that finds or
prepares a ROM directory and adds `--fullscreen` for Gaming Mode.  It does not
bundle glibc, libstdc++ or ROMs.

## Toolkit provenance

The public source repository omits `neogeorecomp/` and the CMake files.  For a
local rebuild, obtain the MIT toolkit from
<https://github.com/sp00nznet/neogeorecomp> and use the local `CMakeLists.txt`
from the full source tree.  The published snapshot in this repository is the
project glue, decryption tool, tests, packaging and documentation.

`neogeorecomp/` is the v0.2.0 source (upstream commit
`af96b2b4b39b42b392b17bcaa6a74a5f27088202`) with generic changes needed by
this and the sibling ports:

- NEO-PVC protection RAM and bank selector (`NG_PROT_PVC`);
- the KOF 2000/2003 fixed-layer bank selector and NEO-SMA support already
  present in the other projects;
- the M1 bank-base repair and `audio_sync()` pull from the audio repair;
- the MAME-accurate NeoGeo YM2610 stereo mix;
- the SDL software-renderer fallback.

Upstream MIT terms and notices are preserved.  No KOF 2003 addresses or
patches were added to the runtime.
