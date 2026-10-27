# Hardware compatibility

*The King of Fighters 2003* (NGM-2710 export / NGH-2710 AES) is a late Neo
Geo cartridge.  MAME's `kof2003` set describes the board as:

- **NEO-MVS PROGBK3S (2003.10.1)** with **NEO-PCM2** and **NEO-PVC**
- **NEO-MVS CHAFIO (2003.7.24)** with **NEO-CMC 7050**

The port models each generic piece in the runtime; only the ROM layout lives
in `src/game.c`.

## 68000 program and PVC

| Item | Result |
|---|---|
| P ROM loading | `271-p1c.p1` at word 0, `271-p2c.p2` at word 2 (MAME `ROM_LOAD32_WORD_SWAP`), `271-p3c.p3` at `$800000` (`ROM_LOAD16_WORD_SWAP`) |
| Image size | `$900000`: 1 MB fixed + 8 MB banked |
| Decryption | `pvc_prot_device::kof2003_decrypt_68k()` port, then a 16-bit byte swap to 68k order |
| Verification | Prepared `271-pd.bin` is byte-identical to MAME 0.242's `cslot1:maincpu` region (MD5 `5754bbff9d51c5bed3e41c9109dc4e36`) |
| Banking | PVC 8 KB RAM at `$2FE000-$2FFFFF`; writes at `$2FFFF0/$2FFFF2` form a 24-bit byte address; the window base is address + `$100000` |
| Self-test | The BIOS RAM test over `$2FE000-$2FFFFF` passes; boot reaches the game |

The PVC colour pack/unpack registers at offsets `$1FE0`/`$1FE8` are also
modelled.  There is no title-specific address patch in the toolkit: the
`NG_PROT_PVC` board type is a generic chip model.

## Graphics

| Item | Result |
|---|---|
| C ROMs | 8 x 8 MB, CMC 7050 key `0x9d` |
| Fixed layer | 512 KB extracted from the end of the C data (`271-s1.s1`) |
| Fixed banking | `NG_FIX_BANK_KOF2000` (two tile-code bits from VRAM `$7500`) |
| Prepared C data | SHA-256 of each file in `ROM-VALIDATION.md`; matches MAME's decrypted sprite/fixed regions |
| Rendering | Sprites, stage, HUD, portraits, palettes, scanline effects and the fix layer render in the tested match |

## Audio

| Item | Result |
|---|---|
| M1 | 512 KB CMC50-encrypted, checksum-derived key `$dc74`; output matches MAME `cslot1:audiocpu[0x10000:]` (MD5 `794a54f9d82a37e7c671882bb7ab9f12`) |
| V ROMs | 16 MB NEO-PCM2 swap value 5; concatenated output matches MAME `cslot1:ymsnd:adpcma` (MD5 `c3ecc216f6a5c604a9d76d962b264778`) |
| Z80 | Real M1 driver, NMI command latch, reply latch, 2 KB RAM, banked M1 windows |
| YM2610 | FM, SSG and ADPCM-A/B through ymfm; timers/status drive interrupts |
| Mix | MAME's NeoGeo stereo route: SSG `0.84` to both speakers, FM L/R `0.98`; `SDL_AudioStream` converts 55,555 Hz to the device rate |
| Continuity | No silent gaps outside the gaps also present in MAME's own attract sequence; match BGM/SFX audible across the tested session |

The mixer gain is a generic accuracy fix, not a KOF 2003 patch.  Earlier
ports summed the stems at full scale, which clipped loud tracks (peaks at
32767).  The corrected route removes that clipping.

## BIOS

The build uses `sp-s2.sp1` (MVS Asia/Europe v2) plus `sfix.sfix`, `sm1.sm1`
and `000-lo.lo`.  AES ROMs and PAL timing are not tested.  The BIOS runs from
its real reset vector; there is no high-level BIOS emulation.

## What is not covered

- Physical MVS/AES hardware comparison (only MAME 0.242 as the oracle).
- AES system ROMs, PAL timing, memory cards.
- Two-player link / versus cabinet wiring.
- Every character, move, ending, survival and continue path.
- Physical Steam Deck, Steam Input and a physical gamepad.

These are reported as untested rather than assumed working.

