/*
 * room.c - game logic for one room. See room.h.
 *
 * Rules (docs/room-b-handoff.md section 7):
 *  1. Crates cannot be pushed, in any direction.
 *  2. Tank has no gravity; he moves freely in four directions.
 *  3. Tank moves one tile per input, in four directions, no diagonals.
 *  4. Entering dirt removes it.
 *  5. A crate falls straight down while the tile below it is open.
 *  6. Tank standing directly beneath a resting crate holds it up.
 *     It starts to fall when he steps away.
 *  7. A crate that is already falling kills Tank if it reaches his tile.
 *  8. The exit opens when the disk is collected.
 */
#include "room.h"

struct room room;

static const signed char dir_dc[4] = { 0, 0, -1, 1 };
static const signed char dir_dr[4] = { -1, 1, 0, 0 };

void room_load(const char* const* map)
{
    unsigned char r, c, t;

    room.disks_total = 0;
    room.disks_got = 0;
    room.exit_open = 0;
    room.status = STATUS_PLAYING;
    room.moves = 0;
    room.dug = 0;
    room.fall_count = 0;

    for (r = 0; r < ROOM_ROWS; ++r) {
        for (c = 0; c < ROOM_COLS; ++c) {
            switch (map[r][c]) {
                case '#': t = TILE_WALL;  break;
                case '%': t = TILE_DIRT;  break;
                case 'C': t = TILE_CRATE; break;
                case 'D': t = TILE_DISK;  ++room.disks_total; break;
                case 'E':
                    t = TILE_EXIT;
                    room.exit_col = c;
                    room.exit_row = r;
                    break;
                case 'T':
                    t = TILE_OPEN;
                    room.tank_col = c;
                    room.tank_row = r;
                    break;
                default:  t = TILE_OPEN;  break;
            }
            room.tiles[r][c] = t;
        }
    }

    /* Crate list, lowest row first (see room.h). */
    room.crate_count = 0;
    for (r = ROOM_ROWS; r-- > 0; ) {
        for (c = 0; c < ROOM_COLS; ++c) {
            if (room.tiles[r][c] == TILE_CRATE && room.crate_count < MAX_CRATES) {
                room.crate_col[room.crate_count] = c;
                room.crate_row[room.crate_count] = r;
                ++room.crate_count;
            }
        }
    }
}

static unsigned char tank_at(unsigned char c, unsigned char r)
{
    return room.tank_col == c && room.tank_row == r;
}

/*
 * Settle every crate. Crates are visited lowest first, so a crate resting
 * on another crate falls after the one beneath it has moved.
 */
static unsigned char settle_crates(void)
{
    unsigned char res = 0;
    unsigned char i, c, r, row;

    room.fall_count = 0;
    for (i = 0; i < room.crate_count; ++i) {
        c = room.crate_col[i];
        r = room.crate_row[i];
        if (r + 1 >= ROOM_ROWS) {
            continue;
        }
        /* Rule 6: a resting crate does not start to fall while Tank
           is directly beneath it. */
        if (room.tiles[r + 1][c] != TILE_OPEN || tank_at(c, r + 1)) {
            continue;
        }
        /* Rule 5: fall while the tile below is open. */
        row = r;
        room.tiles[row][c] = TILE_OPEN;
        while (row + 1 < ROOM_ROWS && room.tiles[row + 1][c] == TILE_OPEN) {
            ++row;
            if (tank_at(c, row)) {
                /* Rule 7: an already-falling crate crushes Tank. */
                room.status = STATUS_DEAD;
                res |= RES_CRUSHED;
                break;
            }
        }
        room.tiles[row][c] = TILE_CRATE;
        room.crate_row[i] = row;
        res |= RES_FELL;
        if (room.fall_count < MAX_FALLS) {
            room.falls[room.fall_count].col = c;
            room.falls[room.fall_count].from_row = r;
            room.falls[room.fall_count].to_row = row;
            ++room.fall_count;
        }
    }
    return res;
}

unsigned char room_move(unsigned char dir)
{
    unsigned char res = RES_MOVED;
    unsigned char c, r;

    room.fall_count = 0;
    if (room.status != STATUS_PLAYING || dir > DIR_RIGHT) {
        return 0;
    }

    c = room.tank_col + dir_dc[dir];
    r = room.tank_row + dir_dr[dir];
    if (c >= ROOM_COLS || r >= ROOM_ROWS) {
        return 0;               /* off the grid (wraps below zero too) */
    }

    switch (room.tiles[r][c]) {
        case TILE_WALL:
        case TILE_CRATE:        /* rule 1: crates cannot be pushed */
            return 0;
        case TILE_EXIT:
            if (!room.exit_open) {
                return 0;       /* shut until the disk is collected */
            }
            room.status = STATUS_WON;
            res |= RES_WON;
            break;
        case TILE_DIRT:         /* rule 4 */
            room.tiles[r][c] = TILE_OPEN;
            ++room.dug;
            res |= RES_DUG;
            break;
        case TILE_DISK:
            room.tiles[r][c] = TILE_OPEN;
            ++room.disks_got;
            res |= RES_DISK;
            if (room.disks_got >= room.disks_total && !room.exit_open) {
                room.exit_open = 1;     /* rule 8 */
                res |= RES_EXIT_OPEN;
            }
            break;
        default:
            break;
    }

    room.tank_col = c;
    room.tank_row = r;
    ++room.moves;

    if (room.status == STATUS_PLAYING) {
        res |= settle_crates();
    }
    return res;
}
