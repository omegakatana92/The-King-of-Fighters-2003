# Steam Deck / SteamOS notes

## Install

1. Copy `KOF 2003-SteamDeck-x86_64.AppImage` to the Deck, for example
   `~/Applications/`.
2. `chmod +x "KOF 2003-SteamDeck-x86_64.AppImage"`.
3. Put a prepared ROM directory beside it (`roms/271-s1.s1`, ...) or in
   `~/.local/share/kof2003/roms`, `~/Emulation/roms/kof2003`, `~/roms/kof2003`
   or `~/roms`.  If the folder holds the original encrypted files, AppRun
   prepares a per-user copy once with the bundled `kof2003_decrypt`.
4. Add the AppImage as a non-Steam game in Desktop Mode, or run it directly.

AppRun adds `--fullscreen` when no `--headless`, `--scale` or `--fullscreen`
is present.  The game keeps the Neo Geo 4:3 picture and pillarboxes it on the
1280x800 16:10 panel; it does not stretch.

## Controls

Steam Input should expose a standard SDL game controller.  The default
"Gamepad with Mouse Trackpad" layout is enough; map:

| Deck control | Neo Geo |
|---|---|
| D-pad / left stick | joystick |
| A / B / X / Y | A / B / C / D |
| Start (☰) | start |
| Back/View | coin |
| Left shoulder (L1) | select |
| Steam button | Steam overlay |

If a pad is not recognised, SDL's `SDL_GAMECONTROLLERCONFIG` mapping format can
describe it.  The game does not include a remapping UI.

## Presentation and performance

- 1280x800 fullscreen, 4:3 pillarboxed, integer-friendly default scale.
- 59.19 Hz frame pacing from the emulated 12 MHz/264-line clock.
- The native path is the same C used on Windows; the Linux binary was
  measured at 100% native coverage on the test session.
- No on-device frame-time profile was captured.  If a demanding stage drops
  frames, try `--scale 2` in windowed mode to isolate the SDL blit from the
  emulation cost.

## Audio

- SDL2 default audio device; no custom backend.
- The YM2610 mix follows MAME's NeoGeo route (SSG 0.84, FM 0.98).
- The Deck's built-in speaker/headphone routing was not tested on hardware;
  the AppImage was run with `SDL_AUDIODRIVER=dummy` under Xvfb, and headless
  WAV output was inspected.

## Testing status

Tested here: AppImage startup, ELF/executable bit, SDL2 resolution, BIOS and
ROM loading, headless rendering and audio capture, `--verify`, ROM discovery,
and a 1280x800 Xvfb window/fullscreen smoke test.

Not tested here: physical Deck LCD/OLED, Gaming Mode, suspend/resume, Steam
Input, Deck audio device, Bluetooth/USB controllers.
