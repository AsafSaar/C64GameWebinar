/*
 * room.h - game logic for one room of Tank's Disk Rescue.
 *
 * Pure logic: no C64 headers, no drawing, no input. It compiles with cc65
 * for the C64 and for sim65 (the PC-side test), and with any C89 compiler.
 *
 * Time model: turn-based. Each call to room_move() is one input. After Tank
 * moves, every crate settles completely before the call returns. The falls
 * are recorded in room.falls so the C64 side can animate them.
 */
#ifndef ROOM_H
#define ROOM_H

#define ROOM_COLS 20
#define ROOM_ROWS 12

/* Tile types stored in the grid. Tank is not stored in the grid. */
#define TILE_OPEN  0
#define TILE_WALL  1
#define TILE_DIRT  2
#define TILE_CRATE 3
#define TILE_DISK  4
#define TILE_EXIT  5

/* Directions for room_move(). */
#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

/* Room status. */
#define STATUS_PLAYING 0
#define STATUS_DEAD    1    /* crushed by a falling crate */
#define STATUS_WON     2    /* entered the open exit */

/* Result flags returned by room_move(). */
#define RES_MOVED     0x01  /* Tank changed tile */
#define RES_DUG       0x02  /* Tank dug a dirt tile */
#define RES_DISK      0x04  /* Tank collected a disk */
#define RES_EXIT_OPEN 0x08  /* the exit opened on this move */
#define RES_FELL      0x10  /* at least one crate fell (see room.falls) */
#define RES_CRUSHED   0x20  /* a falling crate landed on Tank */
#define RES_WON       0x40  /* Tank entered the exit */

#define MAX_FALLS  4
#define MAX_CRATES 8

/* One crate fall: the crate moved from (col, from_row) to (col, to_row). */
struct fall {
    unsigned char col;
    unsigned char from_row;
    unsigned char to_row;
};

struct room {
    unsigned char tiles[ROOM_ROWS][ROOM_COLS];
    unsigned char tank_col, tank_row;
    unsigned char exit_col, exit_row;
    unsigned char disks_total;
    unsigned char disks_got;
    unsigned char exit_open;
    unsigned char status;
    unsigned int  moves;      /* accepted moves, i.e. Tank changed tile */
    unsigned char dug;        /* dirt tiles removed */
    unsigned char fall_count; /* falls recorded by the last room_move() */
    struct fall   falls[MAX_FALLS];
    /* Crate positions, lowest row first. Crates only move straight down,
       so this order stays valid within every column. The grid also holds
       TILE_CRATE at each position. */
    unsigned char crate_count;
    unsigned char crate_col[MAX_CRATES];
    unsigned char crate_row[MAX_CRATES];
};

extern struct room room;

/*
 * Load a room from its text map: ROOM_ROWS strings of ROOM_COLS characters,
 * using the legend in docs/room-b-handoff.md section 5.
 */
void room_load(const char* const* map);

/* Try to move Tank one tile. Returns RES_* flags; 0 means blocked or ignored. */
unsigned char room_move(unsigned char dir);

#endif
