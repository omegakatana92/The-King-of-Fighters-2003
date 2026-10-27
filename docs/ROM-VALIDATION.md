# ROM validation

The supplied archive was inspected without modifying it.  It is the MAME
`kof2003` set (NGM-2710 export).  The files below were compared with MAME
0.242's Neo Geo driver definitions.

## Input files

| File | Size | CRC32 | SHA-1 |
|---|---|---|---|
| `271-p1c.p1` | 4,194,304 | `530ecc14` | `812cf7e9902af3f5e9e330b7c05c2171b139ad2b` |
| `271-p2c.p2` | 4,194,304 | `fd568da9` | `46364906a1e81dc251117e91a1a7b43af1373ada` |
| `271-p3c.p3` | 1,048,576 | `aec5b4a9` | `74087f785590eda5898ce146029818f86ced42b6` |
| `271-c1c.c1` | 8,388,608 | `b1dc25d0` | `50adc3c60d5b4b3abd10a49db2267306c6dbd772` |
| `271-c2c.c2` | 8,388,608 | `d5362437` | `66db36522dc09106388c707252df9fe1c88b4856` |
| `271-c3c.c3` | 8,388,608 | `0a1fbeab` | `9fe30d36ba98d00fda010832ff2f27783dd577c1` |
| `271-c4c.c4` | 8,388,608 | `87b19a0c` | `b72a8e7d9124ce859b5149bb4381ba481c161ea5` |
| `271-c5c.c5` | 8,388,608 | `704ea371` | `e75b80422f0d72eac826f8ffadf79efeccaab124` |
| `271-c6c.c6` | 8,388,608 | `20a1164c` | `c9843b37612a16fc95f6851793b1cfb5d49d811d` |
| `271-c7c.c7` | 8,388,608 | `189aba7f` | `7152195a57ad36b28290810fe87ed8c206262ba9` |
| `271-c8c.c8` | 8,388,608 | `20ec4fdc` | `deb5f7ec5a090e419b9d1a6a74877bee081198e2` |
| `271-m1c.m1` | 524,288 | `f5515629` | `7516bf1b0207a3c8d41dc30c478f8d8b1f71304b` |
| `271-v1c.v1` | 8,388,608 | `ffa3f8c7` | `7cf4a933973ca23b7f87c81151d8659e6ec4bd20` |
| `271-v2c.v2` | 8,388,608 | `5382c7d1` | `1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76` |

System ROMs used: `sp-s2.sp1`, `sfix.sfix`, `sm1.sm1`, `000-lo.lo`.

There is no S ROM in the original set; the 512 KB fixed layer is stored at the
end of the C data, as MAME's `sfix_decrypt()` expects.

## Prepared output

The build writes these files into `build-*/roms`.  SHA-256 values are from the
verified Windows build; the Linux build produces the same bytes.

| File | Size | SHA-256 |
|---|---|---|
| `271-pd.bin` | 9,437,184 | `f6ad2e2d7496eb094b92108b85aca6c71cf6fb397c690e49a04048da27143215` |
| `271-c1c.c1` | 8,388,608 | `bb20d1827ed8d66870622b72f13c7fe612bd8fde86f750634700718e7b260d7d` |
| `271-c2c.c2` | 8,388,608 | `788e5558c97c86a97a303efeeb3bfa32e1ec87590f845a6e039f8e1041d3cc5e` |
| `271-c3c.c3` | 8,388,608 | `00945e8f85e603d0c00787074551a49db965ce7668c941412a15b8e553e764d9` |
| `271-c4c.c4` | 8,388,608 | `ba9540235460c2f1674024a4247abfccde838a3f51b665e8ea18af6b373c54ed` |
| `271-c5c.c5` | 8,388,608 | `417939b4d6f6505875d24dda05ab0fa49a5dc799044fca11c1ace709ea7a5f87` |
| `271-c6c.c6` | 8,388,608 | `2277fed8dd0bdd70749ac1d9dab3a91f53a76d89e77719dedb2b768b04fc5564` |
| `271-c7c.c7` | 8,388,608 | `318e4746e97f289fb8e054ebb1fda12e254161dfb2d876a80d9c660de2b12aac` |
| `271-c8c.c8` | 8,388,608 | `8f456f4ff0939bf08c9ca1ff69a73cdfd7ed2edce79cba16fce7c3ee34399146` |
| `271-s1.s1` | 524,288 | `d6327c711ac102ef01abbadd2e03127384cf65ffc0a6213c121129da6839eebc` |
| `271-m1c.m1` | 524,288 | `13e6ed399292eff412d5c7ddb2285111afda8342efb06ba6d2d865775abfadc6` |
| `271-v1c.v1` | 8,388,608 | `d62cfd0bbe4974c6e2d4a0968ea8f38ae9a2b06220da11dbb6b3ed912bda0f28` |
| `271-v2c.v2` | 8,388,608 | `e10f0698963c5d17012558c44340ae58543f192d9fae51c7539dfbe2cfdd726a` |

## Oracle comparison

MAME 0.242 was run with `-video none -sound none` and a Lua script that
dumps the decrypted memory regions.  The port's output matches:

| Region | Port | MAME 0.242 | Result |
|---|---|---|---|
| `cslot1:maincpu` (9 MB) | `271-pd.bin` | MD5 `5754bbff9d51c5bed3e41c9109dc4e36` | identical |
| `cslot1:fixed` (512 KB) | `271-s1.s1` | MD5 `35dc1def9d4707f0854f30aae32ffb0c` | identical |
| `cslot1:audiocpu[0x10000..]` (512 KB) | `271-m1c.m1` | MD5 `794a54f9d82a37e7c671882bb7ab9f12` | identical |
| `cslot1:ymsnd:adpcma` (16 MB) | v1 + v2 | MD5 `c3ecc216f6a5c604a9d76d962b264778` | identical |

The sprite region is not compared byte-for-byte because MAME post-processes
it into an optimized blitter layout; the fixed-layer extraction and the
on-screen result were checked instead.

## Byte order notes

- MAME's P region is `ROMREGION_BE|ROMREGION_16BIT`.  The loader's word-swap
  and region post-processing cancel at load time; the PVC decryption therefore
  sees file-order 16-bit words.  The port reproduces that and swaps each word
  once more after decryption, which is the 68k byte order the runtime and the
  recompiler consume.
- C ROMs are paired as `c1/c2`, `c3/c4`, ...; the runtime interleaves them
  even/odd, the same layout MAME's `ROM_LOAD16_BYTE` produces.
- V ROMs are concatenated in MAME's 16 MB `adpcma` region before the PCM2
  swap and split back into two 8 MB files.
