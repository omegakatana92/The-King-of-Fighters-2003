THE KING OF FIGHTERS 2003 - Windows x64
=======================================

A native static recompilation of KOF 2003 for the SNK Neo Geo MVS/AES,
built with neogeorecomp v0.2.0.  The game's Motorola 68000 program is
translated to C ahead of time and runs natively; the Z80 sound driver and
YM2610 are modelled.

CONTENTS
--------
  KOF 2003.exe          the player
  kof2003_decrypt.exe   build-time ROM preparation (runs on your dumps)
  Prepare-ROMs.ps1      helper that extracts and prepares a ROM set
  SDL2.dll              window/audio/gamepad library
  vcruntime140*.dll     MSVC runtime
  LICENSES\             upstream and third-party license texts
  README.txt            this file

REQUIREMENTS
------------
  * Windows 10/11 x64.
  * Your own legally obtained MAME sets:
      kof2003.zip   the game (271-p1c.p1, p2c, p3c, c1c-c8c, m1c,
                    v1c, v2c)
      neogeo.zip    the system ROMs (the build uses sp-s2.sp1, sfix.sfix,
                    sm1.sm1 and 000-lo.lo)
    The game and BIOS are not included and are not downloaded.
  * About 400 MB free for the prepared ROM directory.

FIRST RUN
---------
The easy path:

  powershell -ExecutionPolicy Bypass -File .\Prepare-ROMs.ps1

It asks for the two zip files, extracts them next to the tool and prepares
a `roms` folder.  Then run:

  "KOF 2003.exe" --rom-path ".\roms"

Or prepare manually:

  kof2003_decrypt.exe --rom-path C:\path\to\unzipped-sets --out C:\path\to\roms

The original zip files are never modified.  The prepared folder is the only
thing the player reads.

CONTROLS
--------
  Keyboard              Neo Geo
  ------------------------------------------------
  Arrow keys            joystick
  Z X C V               A B C D
  5                     coin (P1)
  1 / 2                 start (P1 / P2)
  3                     select
  9                     service
  F2                    test switch
  Esc                   quit
  F11                   toggle fullscreen

  Gamepad (Xbox layout, SDL2 GameController)
  ------------------------------------------------
  D-pad / left stick    joystick
  A B X Y               A B C D
  Start                 start
  Back / View           coin
  Left shoulder         select

OPTIONS
-------
  --rom-path DIR        prepared ROM directory (default: roms)
  --bios-path DIR       directory for the system ROMs (default: ROM path)
  --bios FILE           system ROM (default: sp-s2.sp1)
  --headless            no window; run as fast as possible
  --frames N            stop after N frames
  --screenshot N:FILE   save a frame as PNG (repeatable)
  --input FILE          scripted input
  --interp              run the whole machine in the interpreter
  --verify              replay every native block and compare
  --dump-misses FILE    record entry points that only ran in the interpreter
  --scale N             window scale (default 3)
  --fullscreen          fullscreen, 4:3 pillarboxed
  --mute                no sound
  --wav FILE            write the audio to a WAV file

The MVS BIOS asks for a coin before Start does anything.

AUDIO
-----
Music and effects are produced by the real Z80 driver and the YM2610.  The
mixer follows MAME's NeoGeo stereo route (SSG 0.84, FM left/right 0.98);
earlier ports summed the stems at full scale and clipped on loud tracks.
Continuity is measured with tests; the mix was not auditioned by a human.

KNOWN LIMITATIONS
-----------------
  * Native coverage is measured, not assumed.  See README.md in the project
    source for the figures from the recorded session.
  * The full game (every character, stage, ending and menu path) has not been
    exhaustively tested.  The verified session covers boot, coin/start,
    team select, order select and several minutes of a match.
  * Physical Steam Deck / controller hardware was not available in this build
    environment; the Linux AppImage was tested under WSL2.
  * The x64 build needs an SSE2-capable CPU, like Windows 10 itself.

LICENSE
-------
The original project code is licensed under PolyForm Noncommercial 1.0.0
(see LICENSES\LICENSE-STATUS.txt).
neogeorecomp is MIT; Musashi and z80 are MIT; ymfm and the MAME Neo Geo
driver references are BSD-3-Clause; SDL2 is zlib.  See
LICENSES\THIRD_PARTY_NOTICES.txt.  The game ROMs and BIOS are SNK/Playmore
property and are not licensed by this project.

The application and its bundled libraries are provided as-is, without
warranty.  Do not redistribute the game data.
