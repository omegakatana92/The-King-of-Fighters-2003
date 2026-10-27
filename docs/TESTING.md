# Testing report

Environment: Windows 11 x64, MSVC 19.51 (Visual Studio 18 2026), CMake 4.3;
Ubuntu 22.04 under WSL2, GCC 11.4, CMake 4.4, SDL2 2.0.20.  MAME 0.242 was
used only as the ROM/decryption oracle.  No physical Steam Deck or gamepad was
available.

## 1. ROM validation

All input CRCs/SHA-1s match MAME's `kof2003` set.  The decrypted program, fixed
layer, M1 and V regions match MAME 0.242 byte-for-byte.  See
`ROM-VALIDATION.md` for the tables and hashes.

## 2. Interpreted boot

```
"KOF 2003.exe" --headless --interp --rom-path build-windows\roms ^
  --frames 1500 --screenshot 600:bios.png ^
  --screenshot 1200:attract.png --screenshot 1499:intro.png
```

Observed: BIOS "INSERT COIN" at frame 600, "WE WELCOME ALL ONCOMERS!" at 1200,
the animated intro at 1500.  This exercises the board model without relying on
recompiled code.

## 3. Native build and coverage

```
cmake --build build-windows --config Release --target profile
cmake --build build-windows --config Release
"KOF 2003.exe" --headless --rom-path build-windows\roms --frames 9000 ^
  --input tests\coin_start.txt --screenshot 5400:match.png --wav audio.wav
```

Observed:

```
[neogeorecomp] kof2003: 18439 recompiled entry points
[exec] native blocks 6825769, native instructions 174458026,
       interpreted instructions 0 (100.00% native), 0 uncovered entry points
```

The 9,000-frame run inserts coins, starts a 1P game, selects a leader and two
members, confirms the order and plays the opening match.  Frames 5400 and
9000 show a live match (Iori vs Whip, then Terry vs Whip) with stage,
sprites, HUD, timer, health bars and the fix layer.

## 4. CPU verification

```
"KOF 2003.exe" --headless --verify --rom-path build-windows\roms ^
  --frames 6000 --input tests\play.txt
```

Observed:

```
[verify] 3583410 blocks checked, 0 mismatches, 2669 cycle-count differences
[exec] native blocks 3583410, native instructions 118359693,
       interpreted instructions 0 (100.00% native), 0 uncovered entry points
```

Mismatches compare registers, flags and every bus write.  The 2,669 reported
items are cycle-total differences of +/-2 to +/-4 cycles in two blocks:
`$000D5A` (BIOS absolute-long `tst.b` path) and `$01AE3A` (cart `movem.l`
save path).  They are not correctness mismatches and are reported separately.

Native vs interpreter: frame 1499 of a no-input run was captured from both
paths and the PNGs are byte-identical (SHA-256
`6B689A3856DACDFDC063CD19C3D6410910A39C36080C6F6F3E28043349E4C4CD`).

## 5. Audio

WAV captures at 55,555 Hz stereo.  The YM2610 mixer follows MAME's NeoGeo
stereo route, and `window_sdl.c` converts 55,555 Hz to whatever the device
accepts with `SDL_AudioStream`; the old direct-rate queue was silent on
devices that reject that rate and could click on devices that accept it.
Measurements:

- **Attract, no input, 4,000 frames (67.6 s):** non-silent from 4 s to 64 s;
  one 2 s gap at 65-66 s (attract transition); peak 0.732, no 32767 clipping.
- **Coin/start/team-select/match, 9,000 frames (152 s):** menu transitions
  contain the expected silent stretches (character-select/how-to-play screens
  do not all carry BGM); from the opening match onward the 1-second RMS never
  falls below 0.001.  Peak 0.965, no constant clipping.
- The real M1 driver is running (commands/replies and bank changes in
  `NG_AUDIO_TRACE` output), and the V ROM data matches MAME.

This is a signal-level verification.  The mix was not auditioned by a human.

## 6. Windows package

```
powershell -ExecutionPolicy Bypass -File packaging\windows\package-windows.ps1
dist\windows\KOF 2003-Windows-x64\"KOF 2003.exe" --headless ^
  --rom-path build-windows\roms --frames 3000 --input tests\coin_start.txt --verify
```

Observed: the packaged exe loads the bundled SDL2 and VC runtime, reports
18,439 entry points, 0 mismatches and 100.00% native.  The folder contains the
player, decrypt helper, `SDL2.dll`, `vcruntime140*.dll`, README, licenses and
`SHA256SUMS.txt`.

## 7. Linux binary and AppImage

```
cmake --build build-linux --parallel 1
./"KOF 2003" --headless --rom-path build-linux/roms --frames 9000 \
  --input tests/coin_start.txt --screenshot 8999:linux.png
```

Observed: 18,439 entry points, 100.00% native, 0 uncovered.  The frame-8999
PNG is **byte-identical** to the Windows build's frame (SHA-256
`A618488CD9CC0A8A24E2B5308731AD499F5862868A96075C81CE207217C6046C`).

AppImage:

```
./packaging/appimage/build-appimage.sh roms-src
APPIMAGE_EXTRACT_AND_RUN=1 ./"KOF 2003-SteamDeck-x86_64.AppImage" \
  --headless --rom-path build-linux/roms --frames 600 --verify
```

Observed: runs, 100.00% native, 0 mismatches.  `--appimage-version` reports
the type-2 runtime; the file is executable and ELF x86-64.

Windowed/fullscreen smoke test under a 1280x800 Xvfb:

```
SDL_AUDIODRIVER=dummy xvfb-run -a -s "-screen 0 1280x800x24" \
  ./"KOF 2003-SteamDeck-x86_64.AppImage" --rom-path build-linux/roms \
  --frames 900 --screenshot 899:window.png
```

Observed: SDL window created, 900 frames rendered, screenshot written, 100.00%
native.  AppRun adds `--fullscreen` when no `--headless/--scale/--fullscreen`
is given.

ROM discovery: a copy of the AppImage in a directory with a `roms` subfolder
launched headless without `--rom-path` and found the prepared set.

## 8. Not tested

- Physical Steam Deck (LCD/OLED), Gaming Mode, suspend/resume, Deck audio.
- Physical Xbox/Steam controller; only SDL's compiled-in mapping was used.
- Every character, stage, ending, survival and continue path.
- AES system ROMs, PAL timing, memory cards, two-player link.
- Subjective audio quality.

These are limitations of the available environment, not claims of success.

