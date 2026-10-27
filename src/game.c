/*
 * game.c - The King of Fighters 2003 (MAME set "kof2003", NGM-2710)
 * ROM layout shared by the player and the recompiler.
 *
 * The retail MVS cartridge is the NEO-MVS PROGBK3S / CHAFIO board:
 *   - 9 MB program: 1 MB fixed bank + 8 MB banked image, encrypted by the
 *     NEO-PVC chip
 *   - 64 MB NEO-CMC 7050 encrypted sprite data, with the 512 KB fixed layer
 *     stored at the end of the C data
 *   - CMC50-encrypted 512 KB M1, two NEO-PCM2-scrambled 8 MB V ROMs
 *
 * tools/decrypt.c performs all of that at build time and verifies the
 * result against the layout MAME produces.  This structure names only the
 * runtime-ready files it writes, so the player and the recompiler load the
 * program identically.
 *
 * Banking is NEO-PVC, not the plain 1 MB selector: the game writes a byte
 * address to $2FFFF0/$2FFFF2 inside the chip's 8 KB RAM at
 * $2FE000-$2FFFFF, and the PVC adds the 1 MB fixed-bank base.  The runtime
 * models that chip (NG_PROT_PVC); there is no ng_bank_map.
 */
#include "game.h"

const ng_game_t k_kof2003 = {
    .name = "kof2003",
    .title = "The King of Fighters 2003",
    .prom_size = 0x900000,           /* 1 MB fixed + 8 MB banked */
    .prom = {
        {"271-pd.bin", 0x000000, 0x000000, 0x900000, 0},  /* decrypted image */
    },
    .crom_size = 0x4000000,          /* 64 MB after pairing */
    .crom_pairs = {
        {"271-c1c.c1", "271-c2c.c2"},
        {"271-c3c.c3", "271-c4c.c4"},
        {"271-c5c.c5", "271-c6c.c6"},
        {"271-c7c.c7", "271-c8c.c8"},
    },
    .srom = "271-s1.s1",             /* 512 KB, extracted from the C data */
    .m1 = "271-m1c.m1",              /* CMC50-decrypted Z80 program */
    .vrom = {"271-v1c.v1", "271-v2c.v2"},
    .fix_bank_type = NG_FIX_BANK_KOF2000,  /* 512 KB S ROM, KOF2000 VRAM bits */
    .bank_map = NULL,
    .bank_write_addr = 0,
    .prot_type = NG_PROT_PVC,
};
