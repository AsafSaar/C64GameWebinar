/*
 * rooms.c - room data.
 *
 * The map is copied character for character from the text map in
 * docs/room-b-handoff.md section 4, which is the source of truth.
 * Legend: # wall, % dirt, . open, C crate, D disk, T Tank start, E exit.
 */
#include "rooms.h"

const char* const room_b_map[ROOM_ROWS] = {
    /*        01234567890123456789 */
    /*  0 */ "####################",
    /*  1 */ "#T#E.#%%%%%%%%%%%%%#",
    /*  2 */ "#.##.#%%%%%%%%%D%%%#",
    /*  3 */ "#.##C#############%#",
    /*  4 */ "#.##%..............#",
    /*  5 */ "#.##.#############.#",
    /*  6 */ "#.####.............#",
    /*  7 */ "#.####.#############",
    /*  8 */ "#.####...%%%%%%....#",
    /*  9 */ "#.################.#",
    /* 10 */ "#....%%%%.....%%%..#",
    /* 11 */ "####################"
};
