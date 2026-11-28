/*
 * gfx.c - C64 drawing for Tank's Disk Rescue.
 *
 * Memory: the VIC-II uses bank 3. The program is linked with
 * __HIMEM__=$C000 (see build.ps1), so $C000-$CFFF belongs to graphics:
 *   $C000  screen RAM (1000 bytes) + sprite pointers at $C3F8
 *   $C400  sprite 0: Tank       (pointer 16)
 *   $C440  sprite 1: the disk   (pointer 17)
 *   $C800  character set (2K): ROM font copied in, tile glyphs at 64+
 *
 * Tiles are 2x2 multicolour characters. Multicolour colours:
 *   %00 background $D021 = black
 *   %01 $D022           = brown
 *   %10 $D023           = orange
 *   %11 colour RAM      = blue (wall), yellow (crate), green (exit)
 * HUD text uses colour RAM values below 8, so it stays hi-res.
 *
 * Sprites are multicolour: %01 = white ($D025), %10 = own colour
 * (red Tank, cyan disk), %11 = black ($D026).
 */
#include <string.h>
#include <c64.h>
#include "room.h"
#include "gfx.h"

#define SCREEN      ((unsigned char*)0xC000)
#define SPRITE_PTRS ((unsigned char*)0xC3F8)
#define SPRITE_MEM  ((unsigned char*)0xC400)
#define CHARSET     ((unsigned char*)0xC800)

#define SPR_TANK 0
#define SPR_DISK 1

#define HUD_ROW 24

/* First character code of each tile's 2x2 block: TL, TR, BL, BR. */
#define CH_WALL       64
#define CH_DIRT       68
#define CH_CRATE      72
#define CH_EXIT_SHUT  76
#define CH_EXIT_OPEN  80
#define CH_BLANK      32

/* Colour RAM value per tile; bit 3 selects multicolour for the cell. */
#define MC 8
#define CR_WALL  (MC | COLOR_BLUE)
#define CR_DIRT  (MC | COLOR_BLACK)     /* %11 is unused by the dirt art */
#define CR_CRATE (MC | COLOR_YELLOW)
#define CR_EXIT  (MC | COLOR_GREEN)
#define CR_OPEN  COLOR_BLACK

#define HUD_TEXT    COLOR_CYAN
#define HUD_COUNTER COLOR_YELLOW

#define FALL_FRAMES 5   /* frames per tile of crate fall */
#define FRAME_LINE  250 /* raster line that marks a new frame, below the playfield */

/*
 * Tile art, copied from docs/room-b-handoff.md section 10.
 * K black, W white, C cyan, G green, B blue, Y yellow, O orange, N brown,
 * R red, '.' background (black) for tiles, transparent for sprites.
 */
static const char* const art_wall[8] = {
    "BBBB.BBB",
    "BBBB.BBB",
    "BBBB.BBB",
    "........",
    "B.BBBBBB",
    "B.BBBBBB",
    "B.BBBBBB",
    "........"
};
static const char* const art_dirt[8] = {
    "NNONNNNN",
    "NNNNNONN",
    "ONNN.NNN",
    "NNNONNNO",
    "NNNNNNNN",
    "NONNNONN",
    "NN.NNNNO",
    "NNNNONNN"
};
static const char* const art_crate[8] = {
    "YYYYYYYY",
    "YYNNNNYY",
    "YNYNNYNY",
    "YNNYYNNY",
    "YNNYYNNY",
    "YNYNNYNY",
    "YYNNNNYY",
    "YYYYYYYY"
};
static const char* const art_exit_open[8] = {
    "GGGGGGGG",
    "G......G",
    "G..OO..G",
    "G.OOOO.G",
    "G..OO..G",
    "G..OO..G",
    "G......G",
    "G......G"
};
/* Not in the handoff: the shut exit is the open art without the arrow. */
static const char* const art_exit_shut[8] = {
    "GGGGGGGG",
    "G......G",
    "G......G",
    "G......G",
    "G......G",
    "G......G",
    "G......G",
    "G......G"
};
static const char* const art_disk[8] = {
    "CCCCCCC.",
    "CCWWKWCC",
    "CCWWKWCC",
    "CCCCCCCC",
    "CWWWWWWC",
    "CWKKKKWC",
    "CWWWWWWC",
    "CCCCCCCC"
};
static const char* const art_tank[8] = {
    "R......R",
    "RR....RR",
    "RRRRRRRR",
    "RKRWWRKR",
    "RRWWWWRR",
    ".WWKKWW.",
    ".RWWWWR.",
    ".RR..RR."
};

static unsigned char disk_col, disk_row;

/* --- building the character set and sprites ---------------------------- */

static unsigned char tile_bits(char ch)
{
    switch (ch) {
        case 'N': return 1;                         /* brown,  $D022 */
        case 'O': return 2;                         /* orange, $D023 */
        case 'B': case 'Y': case 'G': return 3;     /* colour RAM    */
        default:  return 0;                         /* black         */
    }
}

static unsigned char sprite_bits(char ch)
{
    switch (ch) {
        case 'W': return 1;                         /* white, $D025   */
        case 'R': case 'C': return 2;               /* sprite colour  */
        case 'K': return 3;                         /* black, $D026   */
        default:  return 0;                         /* transparent    */
    }
}

/* One chunky pixel is one multicolour pixel wide and two lines tall. */
static void build_tile(unsigned char code, const char* const* art)
{
    unsigned char quad, line, x, b;
    const char* src;
    unsigned char* dst;

    for (quad = 0; quad < 4; ++quad) {
        dst = CHARSET + (unsigned int)(code + quad) * 8;
        for (line = 0; line < 8; ++line) {
            src = art[(quad >> 1) * 4 + (line >> 1)] + (quad & 1) * 4;
            b = 0;
            for (x = 0; x < 4; ++x) {
                b = (b << 2) | tile_bits(src[x]);
            }
            dst[line] = b;
        }
    }
}

static void build_sprite(unsigned char n, const char* const* art)
{
    unsigned char line, x, b0, b1;
    const char* src;
    unsigned char* dst = SPRITE_MEM + n * 64;

    memset(dst, 0, 64);
    for (line = 0; line < 16; ++line) {
        src = art[line >> 1];
        b0 = b1 = 0;
        for (x = 0; x < 4; ++x) {
            b0 = (b0 << 2) | sprite_bits(src[x]);
            b1 = (b1 << 2) | sprite_bits(src[x + 4]);
        }
        dst[line * 3] = b0;
        dst[line * 3 + 1] = b1;
    }
    SPRITE_PTRS[n] = (unsigned char)(((unsigned int)dst - (unsigned int)SCREEN) / 64);
}

static void copy_rom_font(void)
{
    /* Restore the caller's banking afterwards: cc65 runs with BASIC ROM
       switched out, and the C stack lives in that RAM just below $C000. */
    unsigned char port = *(unsigned char*)0x01;

    __asm__("sei");
    *(unsigned char*)0x01 = port & ~0x04;   /* character ROM in at $D000 */
    memcpy(CHARSET, (void*)0xD000, 0x800);
    *(unsigned char*)0x01 = port;
    __asm__("cli");
}

void gfx_init(void)
{
    VIC.ctrl1 &= ~0x10;                     /* blank the screen during setup */

    copy_rom_font();
    build_tile(CH_WALL, art_wall);
    build_tile(CH_DIRT, art_dirt);
    build_tile(CH_CRATE, art_crate);
    build_tile(CH_EXIT_SHUT, art_exit_shut);
    build_tile(CH_EXIT_OPEN, art_exit_open);
    build_sprite(SPR_TANK, art_tank);
    build_sprite(SPR_DISK, art_disk);

    CIA2.pra &= 0xFC;                       /* VIC bank 3: $C000-$FFFF */
    VIC.addr = 0x02;                        /* screen $C000, charset $C800 */
    *(unsigned char*)0x0288 = 0xC0;         /* tell the KERNAL where the screen is */

    VIC.bordercolor = COLOR_BLACK;
    VIC.bgcolor0 = COLOR_BLACK;
    VIC.bgcolor1 = COLOR_BROWN;
    VIC.bgcolor2 = COLOR_ORANGE;
    VIC.ctrl2 |= 0x10;                      /* multicolour text mode */

    VIC.spr_mcolor = (1 << SPR_TANK) | (1 << SPR_DISK);
    VIC.spr_mcolor0 = COLOR_WHITE;
    VIC.spr_mcolor1 = COLOR_BLACK;
    VIC.spr_color[SPR_TANK] = COLOR_RED;
    VIC.spr_color[SPR_DISK] = COLOR_CYAN;
    VIC.spr_ena = 0;

    VIC.rasterline = FRAME_LINE;            /* raster compare, for gfx_wait_frame */
    VIC.ctrl1 = (VIC.ctrl1 & 0x7F) | 0x10;  /* compare bit 8 clear, screen on */
    VIC.irr = 0x01;
}

/* --- drawing ------------------------------------------------------------ */

/* Wait for the VIC's raster-compare latch ($D019 bit 0), set when the beam
   reaches line FRAME_LINE (below the playfield). The latch is set whether or
   not the raster IRQ is enabled (it is not), so unlike polling $D012 it
   cannot be missed while the KERNAL IRQ is running. */
void gfx_wait_frame(void)
{
    while (!(VIC.irr & 0x01)) { }
    VIC.irr = 0x01;                         /* acknowledge */
}

static void put_tile(unsigned char col, unsigned char row,
                     unsigned char ch, unsigned char color)
{
    unsigned int off = (unsigned int)row * 80 + col * 2;
    unsigned char* s = SCREEN + off;
    unsigned char* c = COLOR_RAM + off;

    if (ch == CH_BLANK) {
        s[0] = s[1] = s[40] = s[41] = CH_BLANK;
    } else {
        s[0] = ch;
        s[1] = ch + 1;
        s[40] = ch + 2;
        s[41] = ch + 3;
    }
    c[0] = c[1] = c[40] = c[41] = color;
}

void gfx_draw_tile(unsigned char col, unsigned char row)
{
    switch (room.tiles[row][col]) {
        case TILE_WALL:  put_tile(col, row, CH_WALL, CR_WALL);   break;
        case TILE_DIRT:  put_tile(col, row, CH_DIRT, CR_DIRT);   break;
        case TILE_CRATE: put_tile(col, row, CH_CRATE, CR_CRATE); break;
        case TILE_EXIT:
            put_tile(col, row, room.exit_open ? CH_EXIT_OPEN : CH_EXIT_SHUT, CR_EXIT);
            break;
        default:        /* open; the disk is a sprite over an open tile */
            put_tile(col, row, CH_BLANK, CR_OPEN);
            break;
    }
}

static void place_sprite(unsigned char n, unsigned char col, unsigned char row)
{
    unsigned int x = 24 + (unsigned int)col * 16;

    VIC.spr_pos[n].x = (unsigned char)x;
    VIC.spr_pos[n].y = 50 + row * 16;
    if (x > 255) {
        VIC.spr_hi_x |= (1 << n);
    } else {
        VIC.spr_hi_x &= ~(1 << n);
    }
}

void gfx_place_tank(void)
{
    place_sprite(SPR_TANK, room.tank_col, room.tank_row);
    VIC.spr_ena |= (1 << SPR_TANK);
}

void gfx_update_tank(void)
{
    if (room.status == STATUS_DEAD) {
        VIC.spr_ena &= ~(1 << SPR_TANK);
    } else {
        gfx_place_tank();
    }
}

void gfx_update_disk(void)
{
    if (room.disks_got > 0) {
        VIC.spr_ena &= ~(1 << SPR_DISK);
    } else {
        place_sprite(SPR_DISK, disk_col, disk_row);
        VIC.spr_ena |= (1 << SPR_DISK);
    }
}

/* Write PETSCII text as screen codes. Both letter ranges map to the
   upper-case letters of the ROM font. */
static void put_text(unsigned char col, const char* s, unsigned char color)
{
    unsigned int off = HUD_ROW * 40 + col;
    unsigned char ch;

    for (; *s; ++s, ++off) {
        ch = (unsigned char)*s;
        if (ch >= 0xC0 && ch < 0xE0) {
            ch -= 0xC0;
        } else if (ch >= 0x40 && ch < 0x60) {
            ch -= 0x40;
        }
        SCREEN[off] = ch;
        COLOR_RAM[off] = color;
    }
}

void gfx_draw_hud(void)
{
    static char count[4] = "0/1";

    memset(SCREEN + HUD_ROW * 40, CH_BLANK, 40);
    switch (room.status) {
        case STATUS_DEAD: put_text(0, "CRUSHED! PRESS R", HUD_COUNTER);    break;
        case STATUS_WON:  put_text(0, "ROOM CLEAR! PRESS R", HUD_COUNTER); break;
        default:          put_text(0, "ROOM B  LONG WAY HOME", HUD_TEXT);  break;
    }
    count[0] = '0' + room.disks_got;
    count[2] = '0' + room.disks_total;
    put_text(32, "DISK", HUD_TEXT);
    put_text(37, count, HUD_COUNTER);
}

void gfx_draw_room(void)
{
    unsigned char r, c;

    VIC.spr_ena = 0;
    for (r = 0; r < ROOM_ROWS; ++r) {
        for (c = 0; c < ROOM_COLS; ++c) {
            if (room.tiles[r][c] == TILE_DISK) {
                disk_col = c;
                disk_row = r;
            }
            gfx_draw_tile(c, r);
        }
    }
    gfx_update_disk();
    gfx_update_tank();
    gfx_draw_hud();
}

void gfx_animate_falls(void)
{
    unsigned char i, row, f;
    const struct fall* fl;

    for (i = 0; i < room.fall_count; ++i) {
        fl = &room.falls[i];
        for (row = fl->from_row; row < fl->to_row; ++row) {
            for (f = 0; f < FALL_FRAMES; ++f) {
                gfx_wait_frame();
            }
            put_tile(fl->col, row, CH_BLANK, CR_OPEN);
            put_tile(fl->col, row + 1, CH_CRATE, CR_CRATE);
        }
        for (row = fl->from_row; row <= fl->to_row; ++row) {
            gfx_draw_tile(fl->col, row);
        }
    }
}
