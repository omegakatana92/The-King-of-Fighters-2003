# The King of Fighters 2003 - native static recompilation

A native recompilation of **The King of Fighters 2003** (SNK Playmore,
NGM-2710 / NGH-2710) for the SNK Neo Geo MVS/AES, built on
[neogeorecomp](https://github.com/sp00nznet/neogeorecomp) v0.2.0.

The game's Motorola 68000 program is translated to C ahead of time and runs
natively against a model of the MVS board.  The Z80 sound driver executes the
real cartridge M1 program, and the YM2610 is synthesized by ymfm.  This is not
an emulator front end: the 68000 code is statically recompiled, with Musashi
kept only as the fallback and the `--verify` oracle.

> **No game data here.**  `kof2003.zip`, `neogeo.zip`, and anything generated
> from them (decrypted ROMs, recompiled C) are not distributed.  The user
> supplies their own legally obtained dumps; the build prepares them locally.

## Deliverables

| Target | Artifact |
|---|---|
| Windows x64 | `dist/windows/KOF 2003-Windows-x64/KOF 2003.exe` and `.zip` |
| Steam Deck / Linux x86_64 | `dist/steamdeck/KOF 2003-SteamDeck-x86_64.AppImage` |
| Source | this directory (see `SOURCE-NOTES.md`; the MIT toolkit is a separate dependency) |
| License texts | `LICENSE`, `LICENSE-STATUS.md`, `THIRD_PARTY_NOTICES.md` |

## Status

Verified in this environment (Windows 11 + MSVC 2022/2026 toolchain, and
Ubuntu 22.04 under WSL2).  "Verified" means the commands below were run and
their output was checked; it is not a claim about every character, stage or
ending.

| Area | State |
|---|---|
| Boot | BIOS eye-catcher -> insert coin -> title -> character select -> order select -> match |
| Program | NEO-PVC encrypted 9 MB P ROM decrypted byte-exact against MAME 0.242 |
| Graphics | NEO-CMC 7050 64 MB sprite set + 512 KB fixed layer; stage, sprites, HUD and palettes render |
| Audio | Real CMC50 512 KB M1 + PCM2 16 MB V ROMs; continuous BGM/SFX through a match; mix follows MAME's NeoGeo route |
| Native coverage | **100.00%** on the 9,000-frame coin/start session after one profile pass (0 interpreted, 0 uncovered) |
| `--verify` | 3,583,410 blocks, **0 mismatches**, 2,669 cycle-count differences (see Known limitations) |
| Windows build | Release built and launched from the packaged folder |
| Steam Deck AppImage | Built; runs headless and under Xvfb at 1280x800; not tested on physical Deck hardware |
| Cross-platform | The same frame from Windows and Linux is byte-identical (SHA-256 of the PNG) |

Coverage figures from the recorded sessions:

```
[neogeorecomp] kof2003: 18439 recompiled entry points
[exec] native blocks 6825769, native instructions 174458026,
       interpreted instructions 0 (100.00% native), 0 uncovered entry points
[verify] 3583410 blocks checked, 0 mismatches, 2669 cycle-count differences
```

The 2,669 cycle differences are small (+/-2 to +/-4 cycles) in two blocks:
BIOS block `$000D5A` (an absolute-long `tst.b` path) and cart block
`$01AE3A` (a `movem.l` register-save path).  Registers, flags and bus writes
match; only the reported cycle total differs.  That is accurate enough for the
frame loop and all tested gameplay, but it is reported rather than hidden.

## Hardware / ROM layout

KOF 2003 is the NEO-MVS PROGBK3S (2003.10.1) + NEO-MVS CHAFIO (2003.7.24)
cartridge, MAME set `kof2003`:

| File | Size | Function |
|---|---|---|
| `271-p1c.p1` | 4 MB | first half of the 9 MB program, 32-bit-word-scrambled |
| `271-p2c.p2` | 4 MB | second half, 32-bit-word-scrambled |
| `271-p3c.p3` | 1 MB | last program bank, 16-bit-word-swapped |
| `271-c1c.c1` .. `271-c8c.c8` | 8 MB each | NEO-CMC 7050 encrypted sprite planes |
| `271-m1c.m1` | 512 KB | CMC50-encrypted Z80 program |
| `271-v1c.v1`, `271-v2c.v2` | 8 MB each | NEO-PCM2-scrambled ADPCM-A/B samples |
| BIOS: `sp-s2.sp1`, `sfix.sfix`, `sm1.sm1`, `000-lo.lo` | | MVS system ROMs |

The prepared output is `271-pd.bin` (9 MB: 1 MB fixed + 8 MB banked),
`271-c1c..c8c`, `271-s1.s1` (512 KB fixed layer extracted from the C data),
`271-m1c.m1` (decrypted) and `271-v1c/v2c` (descrambled).

### What the port had to model

* **NEO-PVC program protection.**  The 9 MB program is decrypted with MAME's
  `pvc_prot_device::kof2003_decrypt_68k()`; the runtime also models the PVC's
  8 KB RAM at `$2FE000-$2FFFFF` and its `$2FFFF0/$2FFFF2` byte-address bank
  selector.  The self-test the BIOS runs over that RAM passes.
* **KOF 2000-style fixed-layer banking.**  The 512 KB S ROM needs two extra
  tile-code bits from VRAM; `fix_bank_type = NG_FIX_BANK_KOF2000` selects that
  generic path.
* **CMC50 M1 and NEO-PCM2 V decryption.**  The build tool's output was
  compared with MAME's decrypted `cslot1:maincpu`, `cslot1:fixed`,
  `cslot1:audiocpu` and `cslot1:ymsnd:adpcma` regions; the P, S, M1 and V
  hashes match exactly.

## Getting the ROMs ready

You need the MAME `kof2003` set and the `neogeo` BIOS set, legally obtained.
Both are copyrighted; they are not included.

The easy path on Windows, from the packaged folder:

```
powershell -ExecutionPolicy Bypass -File .\Prepare-ROMs.ps1
```

The script asks for the two zip files, extracts only the needed members and
runs `kof2003_decrypt.exe`.  It writes a `roms` folder next to the player.
Then:

```
"KOF 2003.exe" --rom-path .\roms
```

Manual preparation:

```
kof2003_decrypt --rom-path C:\path\to\unzipped\sets --out C:\path\to\roms
```

The original zips/files are never modified.  Full checksums are in
`docs/ROM-VALIDATION.md`.

On Linux / Steam Deck the AppImage's `AppRun` searches next to the AppImage
(`roms/`, `roms/kof2003/`) and in the usual Deck locations.  If it finds the
original encrypted set it prepares a per-user copy once in
`~/.local/share/kof2003/roms`.

## Controls

Keyboard: arrows; **Z X C V** = A B C D; **5** = coin; **1**/**2** = start
(P1/P2); **3** = select; **9** = service; **F2** = test; **F11** = fullscreen;
**Esc** = quit.

Gamepad (SDL2 GameController, Xbox layout): D-pad/left stick = joystick;
**A B X Y** = Neo Geo A B C D; **Start** = start; **Back/View** = coin;
**left shoulder** = select.

On an MVS, insert a coin before Start does anything.

## Building

### Windows x64 (MSVC)

```
cmake -S . -B build-windows -G "Visual Studio 18 2026" -A x64 ^
  -DKOF2003_ROM_DIR=C:/path/to/unzipped/sets ^
  -DSDL2_DIR=C:/path/to/SDL2-2.32.10/cmake
cmake --build build-windows --config Release --parallel
```

The build runs `kof2003_decrypt`, then `kof2003_recomp`, then compiles the
24 generated C files and links `build-windows/Release/KOF 2003.exe`.
Without `KOF2003_ROM_DIR` the player still builds interpreter-only.

The generated routines are large dispatch functions.  MSVC is deliberately
built with `/O1` for them (`/bigobj;/O1` in `CMakeLists.txt`); `/O2` is
superlinear on these files.  Do not "optimize" that back without measuring.

### Linux x86_64

```
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release \
  -DKOF2003_ROM_DIR=/path/to/unzipped/sets
cmake --build build-linux --parallel
```

Requires `gcc`/`g++`, CMake 3.21+, and `libsdl2-dev`.  The Linux binary is
`build-linux/KOF 2003`.

### Profile pass

```
cmake --build build-windows --config Release --target profile
cmake --build build-windows --config Release
```

`profile` runs attract and a scripted match, writes runtime-only entry points
into `build-windows/entries.txt`, and the next build regenerates with them.
Copy that file to `build-linux/entries.txt` before rebuilding Linux if you
want the same coverage there.

### Packaging

```
powershell -ExecutionPolicy Bypass -File packaging\windows\package-windows.ps1
./packaging/appimage/build-appimage.sh /path/to/unzipped/sets
```

Both read the build tree; neither copies ROMs into the distribution.

## Testing

```
build-windows\Release\"KOF 2003.exe" --headless --rom-path build-windows\roms \
  --frames 9000 --input tests\coin_start.txt \
  --screenshot 5400:shots\match.png --wav shots\audio.wav

build-windows\Release\"KOF 2003.exe" --headless --verify --rom-path build-windows\roms \
  --frames 6000 --input tests\play.txt
```

The AppImage was also run under Xvfb at 1280x800:

```
xvfb-run -a -s "-screen 0 1280x800x24" \
  ./"KOF 2003-SteamDeck-x86_64.AppImage" --rom-path build-linux/roms \
  --frames 900 --screenshot 899:window.png
```

See `docs/TESTING.md` for the full results and the exact commands.

## Known limitations

* **Physical Steam Deck hardware was not available.**  The AppImage was
  executed on Linux x86_64 under WSL2, headless and under Xvfb; the 1280x800
  fullscreen path works, but Gaming Mode, suspend/resume, Steam Input and the
  Deck's audio device were not tested on a real unit.
* **Physical controller hardware was not available.**  SDL2 gamepad code is
  shared with the other ports; the mapping is documented but not exercised
  with a real pad here.
* **The full game was not exhaustively tested.**  Boot, coin, title, team
  select, order select, stage rendering and several minutes of a match were
  verified.  Every character, ending, survival/continue path and menu was not.
* **The audio mix was not auditioned by a human.**  Continuity, levels and
  the absence of clipping are measured; subjective quality is not.
* **Cycle-count differences** remain in two blocks (see Status).  They do not
  affect registers, flags, bus writes or tested gameplay.
* **CPU-heavy 68000 code** on a very slow host can miss the 59.19 Hz frame
  budget; the headless build has no frame cap.  On a Steam Deck the native
  path is comfortably fast in the tested sessions, but no on-device profile
  was captured.

## Licensing

The original project code (the KOF 2003 ROM layout, preparation tool, input
scripts, packaging and documentation) is licensed under **PolyForm
Noncommercial 1.0.0**.  Upstream `neogeorecomp/` remains MIT, Musashi and z80
are MIT, ymfm and the MAME Neo Geo driver references are BSD-3-Clause, and
SDL2 is zlib.  See `LICENSE-STATUS.md` and `THIRD_PARTY_NOTICES.md`.

The game, BIOS and recompiled code generated from them are SNK Playmore/SNK
property and are not licensed by this project.  Do not redistribute game data.
