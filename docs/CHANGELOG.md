# Project changelog

## 0.1.0 - initial KOF 2003 port

- Added the verified NGH-2710 ROM layout (`src/game.c`): 9 MB program
  (1 MB fixed + 8 MB banked), 64 MB C-ROM sprite region, 512 KB embedded
  S-ROM, CMC50 512 KB M1 and two 8 MB PCM2 V-ROMs.
- Added `tools/decrypt.c`:
  - MAME-exact P-ROM loading (`p1`/`p2` 32-bit-word interleave, `p3`
    16-bit word swap) and `kof2003_decrypt_68k()` PVC decryption;
  - CMC 7050 graphics decryption with key `0x9d` and 512 KB S-ROM extraction;
  - CMC50 M1 decryption (checksum key) for 512 KB;
  - NEO-PCM2 V descrambling (swap value 5).
  Input and output CRCs/SHA-256s are recorded in `docs/ROM-VALIDATION.md`;
  the decrypted regions were compared with MAME 0.242 and match byte-for-byte.
- Added the Windows and Linux CMake builds, the 24-file generated-C layout,
  the `profile` target and the AppImage packaging script.
- Added generic NEO-PVC support to the vendored runtime: 8 KB cart RAM at
  `$2FE000-$2FFFFF`, the `$2FFFF0/$2FFFF2` byte-address bank selector,
  colour pack/unpack registers, and a `NG_PROT_PVC` board type.  No title
  addresses were added to the toolkit.
- Verified the BIOS PVC self-test, boot, coin/start, team select, order select,
  stage rendering and several minutes of a match.
- Measured 18,439 entry points, 100.00% native on the 9,000-frame session,
  0 `--verify` mismatches, 2,669 small cycle-count differences.
- Built and smoke-tested the Windows package and the Linux AppImage (headless
  and under Xvfb at 1280x800).

## Generic audio fixes carried into this port

The YM2610 mix previously summed FM and SSG at full scale.  MAME's NeoGeo
stereo routing is SSG `0.84` to both speakers and FM left/right at `0.98`.
The runtime now uses those gains, which removes the 32767 clipping seen in
KOF 2003 and Garou captures.

The SDL window also opens the audio device with rate/format/channel
conversion and feeds it through `SDL_AudioStream`.  The old path requested
exactly 55,555 Hz and padded/dropped its queue, so devices that reject that
rate played nothing and devices that accept it could click under scheduling
jitter.  This was the remaining real-time sound defect in the older ports;
KOF 2003 now uses the repaired path.

Both are generic hardware/front-end fixes, shared with the sibling ports.

## Known open items

- Physical Steam Deck, Steam Input and controller hardware were not tested.
- Full-game content (all characters, endings, survival/continue) is not
  exhaustively covered.
- Two blocks still report small cycle-count differences under `--verify`.

