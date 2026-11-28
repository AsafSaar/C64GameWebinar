/*
 * Tank's Disk Rescue - Room B, "Long Way Home".
 *
 * Tank (a red Pembroke Welsh Corgi) digs through the room to fetch the disk,
 * then drops the crate out of the shaft that leads to the exit.
 *
 * main.c is the frame loop and input only. The rules live in room.c,
 * the room map in rooms.c and all drawing in gfx.c.
 *
 * Controls: joystick in port 2, or W A S D. Hold a direction to keep moving.
 *           R restarts the room. Fire also restarts after a crush or a win.
 *
 * Build: .\build.ps1   (see README.md)
 */
#include <c64.h>
#include "room.h"
#include "rooms.h"
#include "gfx.h"

/* KERNAL keyboard matrix code of the key currently held ($CB). */
#define KEY_HELD  (*(volatile unsigned char*)0xCB)
#define KEY_W     9
#define KEY_A     10
#define KEY_S     13
#define KEY_D     18
#define KEY_R     17

#define JOY_UP    0x01
#define JOY_DOWN  0x02
#define JOY_LEFT  0x04
#define JOY_RIGHT 0x08
#define JOY_FIRE  0x10

#define NO_DIR    0xFF

/* Frames before a held direction repeats, then between repeats. */
#define REPEAT_FIRST 14
#define REPEAT_NEXT  8

static unsigned char held_dir = NO_DIR;
static unsigned char repeat_timer;
static unsigned char prev_buttons;

static void start_room(void)
{
    room_load(room_b_map);
    gfx_draw_room();
}

static void do_move(unsigned char dir)
{
    unsigned char res = room_move(dir);

    if (!res) {
        return;                             /* blocked */
    }
    gfx_draw_tile(room.tank_col, room.tank_row);    /* dug dirt, taken disk */
    gfx_place_tank();
    if (res & RES_DISK) {
        gfx_update_disk();
        gfx_draw_hud();
    }
    if (res & RES_EXIT_OPEN) {
        gfx_draw_tile(room.exit_col, room.exit_row);
    }
    if (res & RES_FELL) {
        gfx_animate_falls();
    }
    if (res & (RES_CRUSHED | RES_WON)) {
        gfx_update_tank();
        gfx_draw_hud();
    }
}

void main(void)
{
    unsigned char joy, key, dir, buttons, pressed;

    gfx_init();
    start_room();

    for (;;) {
        gfx_wait_frame();

        joy = (unsigned char)~CIA1.pra & 0x1F;  /* port 2, active low */
        key = KEY_HELD;

        /* Restart buttons react to the press, not to holding them. */
        buttons = (key == KEY_R ? 1 : 0) | (joy & JOY_FIRE ? 2 : 0);
        pressed = buttons & ~prev_buttons;
        prev_buttons = buttons;
        if ((pressed & 1) || ((pressed & 2) && room.status != STATUS_PLAYING)) {
            held_dir = NO_DIR;
            start_room();
            continue;
        }
        if (room.status != STATUS_PLAYING) {
            continue;
        }

        if      ((joy & JOY_UP)    || key == KEY_W) dir = DIR_UP;
        else if ((joy & JOY_DOWN)  || key == KEY_S) dir = DIR_DOWN;
        else if ((joy & JOY_LEFT)  || key == KEY_A) dir = DIR_LEFT;
        else if ((joy & JOY_RIGHT) || key == KEY_D) dir = DIR_RIGHT;
        else                                        dir = NO_DIR;

        if (dir == NO_DIR) {
            held_dir = NO_DIR;
        } else if (dir != held_dir) {
            held_dir = dir;
            repeat_timer = REPEAT_FIRST;
            do_move(dir);
        } else if (--repeat_timer == 0) {
            repeat_timer = REPEAT_NEXT;
            do_move(dir);
        }
    }
}
