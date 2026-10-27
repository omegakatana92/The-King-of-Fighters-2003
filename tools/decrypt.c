/*
 * decrypt.c - The King of Fighters 2003 (MAME set "kof2003") ROM
 * preparation tool.
 *
 * The retail MVS release (NGM-2710, PROGBK3S / CHAFIO) stores every data
 * ROM encrypted:
 *   - the 9 MB program is NEO-PVC scrambled (PVC chip, MAME
 *     pvc_prot_device::kof2003_decrypt_68k)
 *   - the 64 MB sprite data and the 512 KB M1 are NEO-CMC 7050 encrypted
 *     (key 0x9d); the 512 KB fixed layer lives at the end of the C data
 *   - the two 8 MB V ROMs are NEO-PCM2 scrambled (swap value 5)
 *
 * The tool writes a self-contained, runtime-ready directory.  Input files
 * are never modified.  Algorithms and tables are ported from MAME's Neo Geo
 * driver (BSD-3-Clause; S. Smith, D. Haywood, F. Priuli); see
 * tools/prot_cmc.cpp / tools/prot_pvc references in THIRD_PARTY_NOTICES.md.
 *
 * The P-ROM loading follows MAME exactly: p1 and p2 are
 * ROM_LOAD32_WORD_SWAP at word offsets 0 and 2 (two 4 MB halves of the
 * first 8 MB), p3 is ROM_LOAD16_WORD_SWAP at 0x800000.  MAME's 16-bit
 * big-endian region is then decrypted and stored in 68k byte order, which
 * is what the recompiler and the runtime consume.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tables.h"

/* ---- small helpers ---- */

static uint32_t crc_table[256];
static void crc32_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        crc_table[i] = c;
    }
}
static uint32_t crc32_buf(const uint8_t *p, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) c = crc_table[(c ^ p[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

static uint8_t *read_file(const char *dir, const char *name, size_t *size) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "decrypt: cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) {
        fclose(f); free(buf); return NULL;
    }
    fclose(f);
    *size = (size_t)n;
    return buf;
}

static int write_file(const char *dir, const char *name, const uint8_t *data, size_t size) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "decrypt: cannot write %s\n", path); return -1; }
    if (fwrite(data, 1, size, f) != size) { fclose(f); return -1; }
    fclose(f);
    return 0;
}

static int copy_file(const char *dir, const char *outdir, const char *name) {
    size_t size;
    uint8_t *d = read_file(dir, name, &size);
    if (!d) return -1;
    int rc = write_file(outdir, name, d, size);
    free(d);
    return rc;
}

static uint32_t bitswap32(uint32_t val, const uint8_t *bits, int n) {
    uint32_t r = 0;
    for (int i = 0; i < n; i++)
        r |= ((val >> bits[i]) & 1u) << (n - 1 - i);
    return r;
}

static void swap16(uint8_t *p, size_t n) {
    for (size_t i = 0; i + 1 < n; i += 2) {
        uint8_t t = p[i]; p[i] = p[i + 1]; p[i + 1] = t;
    }
}

/* ---- program ROM: NEO-PVC (kof2003) ---- */

static int decrypt_program(const char *dir, const char *outdir) {
    static const uint8_t xor1[0x20] = {
        0x3b,0x6a,0xf7,0xb7,0xe8,0xa9,0x20,0x99,0x9f,0x39,0x34,0x0c,0xc3,0x9a,0xa5,0xc8,
        0xb8,0x18,0xce,0x56,0x94,0x44,0xe3,0x7a,0xf7,0xdd,0x42,0xf0,0x18,0x60,0x92,0x9f };
    static const uint8_t xor2[0x20] = {
        0x2f,0x02,0x60,0xbb,0x77,0x01,0x30,0x08,0xd8,0x01,0xa0,0xdf,0x37,0x0a,0xf0,0x65,
        0x28,0x03,0xd0,0x23,0xd3,0x03,0x70,0x42,0xbb,0x06,0xf0,0x28,0xba,0x0f,0xf0,0x7a };
    static const uint8_t b16[16] = {15,14,13,12,5,4,7,6,9,8,11,10,3,2,1,0};
    static const uint8_t b8a[8]  = {7,6,5,4,0,1,2,3};
    static const uint8_t b8b[8]  = {4,5,6,7,1,0,3,2};

    size_t n1, n2, n3;
    uint8_t *p1 = read_file(dir, "271-p1c.p1", &n1);
    uint8_t *p2 = read_file(dir, "271-p2c.p2", &n2);
    uint8_t *p3 = read_file(dir, "271-p3c.p3", &n3);
    if (!p1 || !p2 || !p3) return -1;
    if (n1 != 0x400000 || n2 != 0x400000 || n3 != 0x100000) {
        fprintf(stderr, "decrypt: unexpected P-ROM sizes %zx/%zx/%zx\n", n1, n2, n3);
        return -1;
    }
    uint8_t *rom = (uint8_t *)calloc(1, 0x900000);
    if (!rom) return -1;

    /* MAME ROM_LOAD32_WORD_SWAP: each file supplies alternating 16-bit
     * words of 32-bit groups, p1 at byte 0 and p2 at byte 2.  The words
     * are stored in file order here; MAME's 16-bit big-endian region
     * post-processing happens before the PVC sees them, so this is the
     * byte order the decryption algorithm expects.  p3 is a plain
     * ROM_LOAD16_WORD_SWAP at 0x800000. */
    for (size_t j = 0; j < 0x200000; j++) {
        rom[4 * j]     = p1[2 * j];
        rom[4 * j + 1] = p1[2 * j + 1];
        rom[4 * j + 2] = p2[2 * j];
        rom[4 * j + 3] = p2[2 * j + 1];
    }
    memcpy(rom + 0x800000, p3, 0x100000);

    /* pvc_prot_device::kof2003_decrypt_68k() */
    for (int i = 0; i < 0x100000; i++) rom[0x800000 + i] ^= rom[0x100002 | i];
    for (int i = 0; i < 0x100000; i++) rom[i] ^= xor1[i & 0x1F];
    for (int i = 0x100000; i < 0x800000; i++) rom[i] ^= xor2[i & 0x1F];
    for (int i = 0x100000; i < 0x800000; i += 4) {
        uint16_t v = (uint16_t)(rom[i + 1] | (rom[i + 2] << 8));
        v = (uint16_t)bitswap32(v, b16, 16);
        rom[i + 1] = (uint8_t)v;
        rom[i + 2] = (uint8_t)(v >> 8);
    }
    uint8_t *buf = (uint8_t *)malloc(0x900000);
    if (!buf) return -1;
    for (int i = 0; i < 16; i++) {
        int ofst = (i & 0xF0) + (int)bitswap32((uint32_t)(i & 0x0F), b8a, 8);
        memcpy(buf + i * 0x10000, rom + ofst * 0x10000, 0x10000);
    }
    for (int i = 0x100000; i < 0x900000; i += 0x100) {
        int ofst = (i & 0xF000FF) + ((i & 0x000F00) ^ 0x00800) +
                   ((int)bitswap32((uint32_t)((i & 0x0FF000) >> 12), b8b, 8) << 12);
        memcpy(buf + i, rom + ofst, 0x100);
    }
    memcpy(rom, buf, 0x100000);
    memcpy(rom + 0x100000, buf + 0x800000, 0x100000);
    memcpy(rom + 0x200000, buf + 0x100000, 0x700000);

    /* The recompiler and the runtime hold P ROMs in 68k byte order. */
    swap16(rom, 0x900000);

    int rc = write_file(outdir, "271-pd.bin", rom, 0x900000);
    if (!rc) fprintf(stderr, "decrypt: P ROM -> 271-pd.bin (0x900000 bytes, crc %08x)\n",
                     crc32_buf(rom, 0x900000));
    free(buf); free(rom); free(p1); free(p2); free(p3);
    return rc;
}

/* ---- CMC50 graphics decryption (key 0x9d for KOF2003) ---- */

static void gfx_byte(uint8_t *r0, uint8_t *r1, uint8_t c0, uint8_t c1,
                     const uint8_t *t0hi, const uint8_t *t0lo, const uint8_t *t1,
                     uint32_t base, int invert) {
    uint8_t tmp = t1[(base & 0xff) ^ kof2000_address_0_7_xor[(base >> 8) & 0xff]];
    uint8_t xor0 = (uint8_t)((t0hi[(base >> 8) & 0xff] & 0xfe) | (tmp & 0x01));
    uint8_t xor1 = (uint8_t)((tmp & 0xfe) | (t0lo[(base >> 8) & 0xff] & 0x01));
    if (invert) { *r0 = c1 ^ xor0; *r1 = c0 ^ xor1; }
    else        { *r0 = c0 ^ xor0; *r1 = c1 ^ xor1; }
}

static void gfx_decrypt(uint8_t *rom, uint32_t rom_size, uint8_t key) {
    uint8_t *buf = (uint8_t *)malloc(rom_size);
    const uint8_t *a07  = kof2000_address_0_7_xor;
    const uint8_t *a811 = kof2000_address_8_15_xor1;
    const uint8_t *a812 = kof2000_address_8_15_xor2;
    const uint8_t *a161 = kof2000_address_16_23_xor1;
    const uint8_t *a162 = kof2000_address_16_23_xor2;

    for (uint32_t rpos = 0; rpos < rom_size / 4; rpos++) {
        gfx_byte(&buf[4*rpos], &buf[4*rpos+3], rom[4*rpos], rom[4*rpos+3],
                 kof2000_type0_t03, kof2000_type0_t12, kof2000_type1_t03, rpos,
                 (int)((rpos >> 8) & 1));
        gfx_byte(&buf[4*rpos+1], &buf[4*rpos+2], rom[4*rpos+1], rom[4*rpos+2],
                 kof2000_type0_t12, kof2000_type0_t03, kof2000_type1_t12, rpos,
                 (int)(((rpos >> 16) ^ a162[(rpos >> 8) & 0xff]) & 1));
    }
    for (uint32_t rpos = 0; rpos < rom_size / 4; rpos++) {
        uint32_t baser = rpos;
        baser ^= key;
        baser ^= (uint32_t)a811[(baser >> 16) & 0xff] << 8;
        baser ^= (uint32_t)a812[baser & 0xff] << 8;
        baser ^= (uint32_t)a161[baser & 0xff] << 16;
        baser ^= (uint32_t)a162[(baser >> 8) & 0xff] << 16;
        baser ^= a07[(baser >> 8) & 0xff];
        baser &= (rom_size / 4) - 1;
        rom[4*rpos]   = buf[4*baser];
        rom[4*rpos+1] = buf[4*baser+1];
        rom[4*rpos+2] = buf[4*baser+2];
        rom[4*rpos+3] = buf[4*baser+3];
    }
    free(buf);
}

static void sfix_extract(const uint8_t *rom, uint32_t rom_size, uint8_t *fixed, uint32_t fixed_size) {
    const uint8_t *src = rom + rom_size - fixed_size;
    for (uint32_t i = 0; i < fixed_size; i++)
        fixed[i] = src[(i & ~0x1fu) + ((i & 7) << 2) + ((~i & 8) >> 2) + ((i & 0x10) >> 4)];
}

static int decrypt_crom(const char *dir, const char *outdir) {
    static const char *pairs[4][2] = {
        {"271-c1c.c1", "271-c2c.c2"}, {"271-c3c.c3", "271-c4c.c4"},
        {"271-c5c.c5", "271-c6c.c6"}, {"271-c7c.c7", "271-c8c.c8"},
    };
    size_t csize = 0;
    uint8_t *even[4], *odd[4];
    for (int i = 0; i < 4; i++) {
        even[i] = read_file(dir, pairs[i][0], &csize);
        odd[i]  = read_file(dir, pairs[i][1], &csize);
        if (!even[i] || !odd[i]) return -1;
    }
    if (csize != 0x800000) {
        fprintf(stderr, "decrypt: unexpected C-ROM size %zx\n", csize);
        return -1;
    }
    uint32_t total = (uint32_t)csize * 2 * 4;  /* 64 MB */
    uint8_t *crom = (uint8_t *)malloc(total);
    for (int i = 0; i < 4; i++) {
        uint32_t base = (uint32_t)csize * 2 * i;
        for (uint32_t k = 0; k < csize; k++) {
            crom[base + 2*k]     = even[i][k];
            crom[base + 2*k + 1] = odd[i][k];
        }
    }
    gfx_decrypt(crom, total, 0x9d);

    /* Fixed layer: 512 KB at the end of the decrypted C data. */
    uint8_t *srom = (uint8_t *)malloc(0x80000);
    sfix_extract(crom, total, srom, 0x80000);
    if (write_file(outdir, "271-s1.s1", srom, 0x80000)) return -1;
    fprintf(stderr, "decrypt: S ROM -> 271-s1.s1 (0x80000 bytes, crc %08x)\n",
            crc32_buf(srom, 0x80000));
    free(srom);

    for (int i = 0; i < 4; i++) {
        uint32_t base = (uint32_t)csize * 2 * i;
        for (uint32_t k = 0; k < csize; k++) {
            even[i][k] = crom[base + 2*k];
            odd[i][k]  = crom[base + 2*k + 1];
        }
        if (write_file(outdir, pairs[i][0], even[i], csize)) return -1;
        if (write_file(outdir, pairs[i][1], odd[i],  csize)) return -1;
        free(even[i]); free(odd[i]);
    }
    free(crom);
    fprintf(stderr, "decrypt: C ROMs -> 271-c1c..271-c8c (%zu bytes each)\n", csize);
    return 0;
}

/* ---- CMC50 M1 decryption (512 KB) ---- */

static uint16_t sum16(const uint8_t *data, uint32_t length) {
    uint16_t sum = 0;
    for (uint32_t i = 0; i < length; i++) sum = (uint16_t)(sum + data[i]);
    return sum;
}

static uint32_t m1_scramble(uint32_t address, uint16_t key) {
    static const uint8_t k1[16] = {12,0,2,4,8,15,7,13,10,1,3,6,11,9,14,5};
    static const uint8_t k3[16] = {7,15,14,6,5,13,12,4,11,3,10,2,9,1,8,0};
    uint32_t block = (address >> 16) & 7;
    uint32_t aux = address & 0xffff;
    uint8_t p[16];
    for (int i = 0; i < 16; i++) p[i] = (uint8_t)p1[block][15 - i];
    aux ^= bitswap32(key, k1, 16);
    aux = bitswap32((uint16_t)aux, p, 16);
    aux ^= m1_address_0_7_xor[(aux >> 8) & 0xff];
    aux ^= (uint32_t)m1_address_8_15_xor[aux & 0xff] << 8;
    aux = bitswap32((uint16_t)aux, k3, 16);
    return (block << 16) | aux;
}

static int decrypt_m1(const char *dir, const char *outdir) {
    size_t size;
    uint8_t *m1 = read_file(dir, "271-m1c.m1", &size);
    if (!m1) return -1;
    if (size != 0x80000) {
        fprintf(stderr, "decrypt: 271-m1c.m1 is %zu bytes, expected 0x80000\n", size);
        free(m1); return -1;
    }
    uint16_t key = sum16(m1, 0x10000);
    uint8_t *buf = (uint8_t *)malloc(0x80000);
    for (uint32_t i = 0; i < 0x80000; i++) buf[i] = m1[m1_scramble(i, key)];
    int rc = write_file(outdir, "271-m1c.m1", buf, 0x80000);
    fprintf(stderr, "decrypt: M1 key=%04x -> 271-m1c.m1 (0x80000 bytes, crc %08x)\n",
            key, crc32_buf(buf, 0x80000));
    free(buf); free(m1);
    return rc;
}

/* ---- NEO-PCM2 V-ROM descrambling (swap value 5) ---- */

static int decrypt_vrom(const char *dir, const char *outdir) {
    static const uint8_t bits[24] = {23,22,21,20,19,18,17,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,16};
    static const uint8_t xordata[8] = {0x4b,0xa4,0x63,0x46,0xf0,0x91,0xea,0x62};
    size_t s1, s2;
    uint8_t *v1 = read_file(dir, "271-v1c.v1", &s1);
    uint8_t *v2 = read_file(dir, "271-v2c.v2", &s2);
    if (!v1 || !v2) return -1;
    if (s1 != 0x800000 || s2 != 0x800000) return -1;
    uint8_t *src = (uint8_t *)malloc(0x1000000);
    uint8_t *dst = (uint8_t *)malloc(0x1000000);
    memcpy(src, v1, 0x800000);
    memcpy(src + 0x800000, v2, 0x800000);
    for (uint32_t i = 0; i < 0x1000000; i++) {
        uint32_t j = bitswap32(i, bits, 24) ^ 0xa7001u;
        uint32_t d = (i + 0xff14eau) & 0xffffffu;
        dst[j] = (uint8_t)(src[d] ^ xordata[j & 7]);
    }
    int rc = write_file(outdir, "271-v1c.v1", dst, 0x800000);
    if (!rc) rc = write_file(outdir, "271-v2c.v2", dst + 0x800000, 0x800000);
    fprintf(stderr, "decrypt: V ROMs -> 271-v1c.v1/v2 (PCM2 swap 5, crc %08x)\n",
            crc32_buf(dst, 0x1000000));
    free(src); free(dst); free(v1); free(v2);
    return rc;
}

int main(int argc, char **argv) {
    const char *romdir = "roms-src", *outdir = "roms";
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--rom-path") && i + 1 < argc) romdir = argv[++i];
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) outdir = argv[++i];
        else if (!strcmp(argv[i], "--help")) {
            fprintf(stderr, "usage: kof2003_decrypt [--rom-path DIR] [--out DIR]\n");
            return 0;
        }
    }
    crc32_init();
    if (decrypt_program(romdir, outdir)) return 1;
    if (decrypt_crom(romdir, outdir)) return 1;
    if (decrypt_m1(romdir, outdir)) return 1;
    if (decrypt_vrom(romdir, outdir)) return 1;

    const char *copy[] = {
        "sp-s2.sp1", "sfix.sfix", "sm1.sm1", "000-lo.lo", NULL
    };
    for (int i = 0; copy[i]; i++)
        if (copy_file(romdir, outdir, copy[i])) return 1;
    fprintf(stderr, "decrypt: wrote a complete ROM set to %s\n", outdir);
    return 0;
}
