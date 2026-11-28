/*
 * test_room.c - PC-side test of the Room B logic.
 *
 * Checks the intended route and the "things that must also hold" from
 * docs/room-b-handoff.md section 8.
 *
 * Built for sim65 by .\build.ps1 -Test:
 *   cl65 -t sim6502 -O -o test_room.prg tests/test_room.c room.c rooms.c
 *   sim65 test_room.prg
 * It is plain C89 and also builds with a native compiler:
 *   gcc -I. -o test_room tests/test_room.c room.c rooms.c
 */
#include <stdio.h>
#include <string.h>
#include "room.h"
#include "rooms.h"

/* Section 8, "As one string, for an automated test". */
static const char route[] =
    "DDDDDDDDDRRRRRRRRRRRRRRRRRUULLLLLLLLLLLLUURRRRRRRRRRRRUUUULLLRRRDDLLLLLLLLLLLLLLRLUUUL";

/* The route up to step 11: Tank stands at (4,4), holding the crate up. */
#define MOVES_TO_SUPPORT 80

/* Alternative order: drop the crate first, then fetch the disk. */
static const char crate_first[] =
    "DDDDDDDDD" "RRRRRRRRRRRRRRRRR" "UU" "LLLLLLLLLLLL" "UU" "RRRRRRRRRRRR"
    "UU"                    /* (18,4), the row 4 lane */
    "LLLLLLLLLLLLL" "L" "R" /* to the alcove, dig the support, step back */
    "RRRRRRRRRRRRR" "UU" "LLL" /* back east, into the dirt field, disk */
    "RRR" "DD" "LLLLLLLLLLLLL" "L" "UUU" "L";

static unsigned int failures;
static unsigned int checks;

static void check(int ok, const char* what)
{
    ++checks;
    if (!ok) {
        ++failures;
        printf("FAIL: %s\n", what);
    }
}

static unsigned char dir_of(char ch)
{
    switch (ch) {
        case 'U': return DIR_UP;
        case 'D': return DIR_DOWN;
        case 'L': return DIR_LEFT;
        default:  return DIR_RIGHT;
    }
}

/* Play moves[0..count). Returns the 1-based number of the move that collected
   the disk (0 if none). Fails if any move is blocked. */
static unsigned int play(const char* moves, unsigned int count)
{
    unsigned int i, disk_move = 0;
    unsigned char res;

    for (i = 0; i < count; ++i) {
        res = room_move(dir_of(moves[i]));
        if (!(res & RES_MOVED)) {
            printf("  move %u (%c) blocked at (%u,%u)\n", i + 1, moves[i],
                   room.tank_col, room.tank_row);
            check(0, "every move in the sequence is accepted");
            return disk_move;
        }
        if (res & RES_DISK) {
            disk_move = i + 1;
        }
    }
    return disk_move;
}

/* Flood fill from Tank over tiles he can walk or dig through without
   collecting or pushing anything special: open, dirt and disk. Walls,
   crates and the exit stop the fill. */
static unsigned char seen[ROOM_ROWS][ROOM_COLS];

static void flood_from_tank(void)
{
    static unsigned char stack[ROOM_ROWS * ROOM_COLS];
    unsigned int top = 0;
    unsigned char c, r, t, pos, nc, nr, d;
    static const signed char dc[4] = { 0, 0, -1, 1 };
    static const signed char dr[4] = { -1, 1, 0, 0 };

    memset(seen, 0, sizeof seen);
    seen[room.tank_row][room.tank_col] = 1;
    stack[top++] = room.tank_row * ROOM_COLS + room.tank_col;
    while (top > 0) {
        pos = stack[--top];
        r = pos / ROOM_COLS;
        c = pos % ROOM_COLS;
        for (d = 0; d < 4; ++d) {
            nc = c + dc[d];
            nr = r + dr[d];
            if (nc >= ROOM_COLS || nr >= ROOM_ROWS || seen[nr][nc]) {
                continue;
            }
            t = room.tiles[nr][nc];
            if (t == TILE_OPEN || t == TILE_DIRT || t == TILE_DISK) {
                seen[nr][nc] = 1;
                stack[top++] = nr * ROOM_COLS + nc;
            }
        }
    }
}

static void test_map_shape(void)
{
    unsigned char r;

    printf("map shape\n");
    for (r = 0; r < ROOM_ROWS; ++r) {
        check(strlen(room_b_map[r]) == ROOM_COLS, "every map row is 20 characters");
    }
    room_load(room_b_map);
    check(room.tank_col == 1 && room.tank_row == 1, "Tank starts at (1,1)");
    check(room.exit_col == 3 && room.exit_row == 1, "exit is at (3,1)");
    check(room.tiles[3][4] == TILE_CRATE, "crate is at (4,3)");
    check(room.tiles[2][15] == TILE_DISK, "disk is at (15,2)");
    check(room.disks_total == 1, "one disk");
}

static void test_intended_route(void)
{
    unsigned int n = sizeof route - 1;
    unsigned int disk_move, i;
    unsigned char res;

    printf("intended route (%u moves)\n", n);
    check(n == 86, "route string is 86 moves");

    room_load(room_b_map);

    /* Up to step 11 the crate must not move: Tank holds it at move 80. */
    disk_move = play(route, MOVES_TO_SUPPORT);
    check(disk_move == 61, "disk collected on move 61");
    check(room.exit_open, "exit open after the disk");
    check(room.tank_col == 4 && room.tank_row == 4, "after move 80 Tank is at (4,4)");
    check(room.tiles[3][4] == TILE_CRATE, "crate still at (4,3) while Tank holds it up");
    check(room.status == STATUS_PLAYING, "Tank alive under the held crate");

    /* Step 12: step back east; the crate drops into the pit. */
    res = room_move(DIR_RIGHT);
    check((res & RES_FELL) != 0, "move 81 drops the crate");
    check(room.fall_count == 1 && room.falls[0].col == 4 &&
          room.falls[0].from_row == 3 && room.falls[0].to_row == 5,
          "crate falls from (4,3) to (4,5)");
    check(!(res & RES_CRUSHED), "stepping back to (5,4) is safe");

    for (i = MOVES_TO_SUPPORT + 1; i < n; ++i) {
        res = room_move(dir_of(route[i]));
        check((res & RES_MOVED) != 0, "moves 82-86 are accepted");
    }

    check(room.tiles[5][4] == TILE_CRATE, "crate ends at (4,5)");
    check(room.tiles[3][4] == TILE_OPEN, "crate's start tile (4,3) is open");
    check(room.status == STATUS_WON, "Tank reaches the exit");
    check(room.tank_col == 3 && room.tank_row == 1, "Tank is on the exit tile (3,1)");
    check(room.moves == 86, "86 moves taken");
    check(room.dug == 18, "18 dirt tiles dug");
}

/* Must hold 1: the exit is unreachable without the disk-side detour. */
static void test_exit_needs_detour(void)
{
    printf("exit needs the detour\n");
    room_load(room_b_map);

    check(room.tiles[1][2] == TILE_WALL, "(2,1) next to the exit is wall");
    check(room.tiles[2][3] == TILE_WALL, "(3,2) next to the exit is wall");
    check(room.tiles[0][3] == TILE_WALL, "(3,0) next to the exit is wall");
    check(room.tiles[1][4] == TILE_OPEN, "(4,1) next to the exit is the pocket");

    check(room_move(DIR_RIGHT) == 0, "Tank cannot step east from the start");

    flood_from_tank();
    check(!seen[1][4], "the pocket (4,1), the exit's only open neighbour, is unreachable");
    check(seen[2][15], "the disk is reachable");
}

/* Must hold 2: the crate cannot be approached from above before it drops. */
static void test_pocket_sealed(void)
{
    printf("pocket sealed\n");
    room_load(room_b_map);
    flood_from_tank();
    check(!seen[1][4] && !seen[2][4], "pocket tiles (4,1) and (4,2) are unreachable");
    check(room.tiles[1][5] == TILE_WALL && room.tiles[2][5] == TILE_WALL &&
          room.tiles[2][3] == TILE_WALL && room.tiles[0][4] == TILE_WALL,
          "the pocket is walled on every side but the exit and the crate");
}

/* Must hold 3: from (4,4), stepping down into the pit is lethal. */
static void test_pit_is_lethal(void)
{
    unsigned char res;

    printf("pit is lethal\n");
    room_load(room_b_map);
    play(route, MOVES_TO_SUPPORT);
    res = room_move(DIR_DOWN);
    check((res & RES_MOVED) != 0, "Tank can step down into the pit");
    check((res & RES_CRUSHED) != 0, "the crate falls onto Tank");
    check(room.status == STATUS_DEAD, "Tank is dead");
    check(room_move(DIR_UP) == 0, "no moves are accepted after death");
}

/* Must hold 4: dropping the crate before the disk leaves the room winnable. */
static void test_crate_first(void)
{
    unsigned int n = sizeof crate_first - 1;
    unsigned char r, c, ok;

    printf("crate dropped first (%u moves)\n", n);
    room_load(room_b_map);

    /* Up to and including stepping back from the support (move 71). */
    play(crate_first, 71);
    check(room.tank_col == 5 && room.tank_row == 4, "Tank is back in the alcove (5,4)");
    check(room.disks_got == 0, "the disk is not yet collected");
    check(room.tiles[5][4] == TILE_CRATE, "the crate is already in the pit");

    flood_from_tank();
    ok = 1;
    for (r = 0; r < ROOM_ROWS; ++r) {
        for (c = 0; c < ROOM_COLS; ++c) {
            switch (room.tiles[r][c]) {
                case TILE_OPEN: case TILE_DIRT: case TILE_DISK:
                    if (!seen[r][c]) {
                        printf("  (%u,%u) unreachable\n", c, r);
                        ok = 0;
                    }
                    break;
            }
        }
    }
    check(ok, "every open, dirt and disk tile is still reachable");
    check(seen[1][4], "the pocket next to the exit is reachable");

    room_load(room_b_map);
    play(crate_first, n);
    check(room.disks_got == 1, "disk collected");
    check(room.status == STATUS_WON, "Tank still reaches the exit");
}

int main(void)
{
    test_map_shape();
    test_intended_route();
    test_exit_needs_detour();
    test_pocket_sealed();
    test_pit_is_lethal();
    test_crate_first();

    if (failures) {
        printf("%u of %u checks FAILED\n", failures, checks);
        return 1;
    }
    printf("all %u checks passed\n", checks);
    return 0;
}
