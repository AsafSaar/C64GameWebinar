/*
 * Tank's Disk Rescue - experiment 1
 *
 * Tank (a red Pembroke Welsh Corgi) stands beside two floppy disks.
 *   D  collects the nearest remaining disk: +10 each, so 0 -> 10 -> 20.
 *      Once both are gone, D does nothing.
 *   R  resets the scene.
 *
 * Build: cl65 -t c64 -O -o tank.prg main.c
 */

#include <conio.h>
#include <c64.h>

#define TANK_X   4
#define TANK_Y   9
#define DISK_COUNT 2
#define DISK_Y  10
#define DISK_POINTS 10
#define SCREEN_COLS 40
#define SCORE_Y      1

/* Block art: each letter is one coloured character cell, '.' is empty. */
/* o = red coat, w = white, k = black, g = dark grey                    */
static const char* const tank_art[] = {
    "......o..o..",
    "......oooo..",
    "......okoowk",
    "oooooooowww.",
    "ooooooooww..",
    "ooooooooo...",
    "w.w...w.w..."
};
#define TANK_ROWS (sizeof tank_art / sizeof tank_art[0])

static const char* const disk_art[] = {
    "gggggg",
    "gwwwwg",
    "gwwwwg",
    "gggggg",
    "ggkkgg",
    "gggggg"
};
#define DISK_ROWS (sizeof disk_art / sizeof disk_art[0])

static unsigned char score;
/* Column of each disk, nearest to Tank first. */
static const unsigned char disk_x[DISK_COUNT] = { 20, 29 };

static unsigned char collected;     /* how many disks Tank has grabbed */

static unsigned char art_color(char c)
{
    switch (c) {
        case 'o': return COLOR_RED;
        case 'w': return COLOR_WHITE;
        case 'k': return COLOR_BLACK;
        default:  return COLOR_GRAY1;
    }
}

/* Draw block art, or erase it (blank every cell) when erase is non-zero. */
static void draw_art(unsigned char x, unsigned char y,
                     const char* const* rows, unsigned char count,
                     unsigned char erase)
{
    unsigned char r, i;
    const char* p;

    for (r = 0; r < count; ++r) {
        p = rows[r];
        for (i = 0; p[i] != '\0'; ++i) {
            if (p[i] == '.') {
                continue;
            }
            gotoxy(x + i, y + r);
            if (erase) {
                revers(0);
            } else {
                revers(1);
                textcolor(art_color(p[i]));
            }
            cputc(' ');
        }
    }
    revers(0);
}

static void draw_score(void)
{
    /* "SCORE: " is 7 characters, plus one or two digits. */
    unsigned char len = (score >= 10) ? 9 : 8;

    textcolor(COLOR_WHITE);
    cclearxy(0, SCORE_Y, SCREEN_COLS);      /* wipe the old, differently centred text */
    gotoxy((SCREEN_COLS - len) / 2, SCORE_Y);
    cprintf("SCORE: %u", score);
}

static void reset_scene(void)
{
    unsigned char i;

    score = 0;
    collected = 0;

    clrscr();
    draw_score();

    textcolor(COLOR_YELLOW);
    cputsxy(1, 3, "TANK'S DISK RESCUE");

    draw_art(TANK_X, TANK_Y, tank_art, TANK_ROWS, 0);
    for (i = 0; i < DISK_COUNT; ++i) {
        draw_art(disk_x[i], DISK_Y, disk_art, DISK_ROWS, 0);
    }

    textcolor(COLOR_WHITE);
    cputsxy(1, 22, "D = COLLECT DISK    R = RESET");
}

static void collect_disk(void)
{
    if (collected >= DISK_COUNT) {
        return;             /* all disks are gone: D does nothing more */
    }
    draw_art(disk_x[collected], DISK_Y, disk_art, DISK_ROWS, 1);
    ++collected;
    score += DISK_POINTS;
    draw_score();
}

void main(void)
{
    char key;

    bordercolor(COLOR_BLUE);
    bgcolor(COLOR_LIGHTBLUE);
    cursor(0);
    reset_scene();

    for (;;) {
        key = cgetc();
        if (key == 'd' || key == 'D') {
            collect_disk();
        } else if (key == 'r' || key == 'R') {
            reset_scene();
        }
    }
}
