# Room B handoff: "Long Way Home"

Level spec for the first playable room of Tank's Disk Rescue (cc65, `main.c`).
Room B was selected by Asaf from two candidates on 2026-11-28. Room A is not part of this handoff.

This file specifies the room. It does not prescribe how to restructure `main.c`.
Section 9 lists every detail that was decided by the designer rather than by Asaf. Treat those as open until he confirms them, and ask before building on any that affect architecture (items 1 to 4).

Reference render of the room: `room-b-reference.png` (same folder, 640 x 384, 2x scale).

## 1. Identity

| Field | Value |
| --- | --- |
| Room letter | B |
| Room name | Long Way Home |
| Idea | The exit is two tiles from the start, behind a wall. Tank winds through the whole room to fetch the disk, then has to drop a crate out of the only shaft that leads to the exit. |
| Crates | 1 |
| Disks | 1 |

## 2. Grid

- 20 columns x 12 rows of tiles.
- One tile is 16 x 16 pixels, which is 2 x 2 character cells.
- Playfield is 320 x 192 pixels, which is 40 x 24 character cells: screen rows 0 to 23.
- Screen row 24 is free for the HUD.
- Coordinates in this file are `(col, row)`, zero-based, origin at the top-left tile. Columns run 0 to 19, rows 0 to 11.
- Tile `(c, r)` covers character cells `(2c, 2r)` to `(2c+1, 2r+1)`.

## 3. Palette

Nine of the sixteen C64 colours. The index is authoritative. Hex values are only the mockup's approximation.

| Index | cc65 constant | Name | Used for | Mockup hex |
| --- | --- | --- | --- | --- |
| 0 | `COLOR_BLACK` | Black | Open space, screen background, Tank's eyes and nose, disk slot | #000000 |
| 1 | `COLOR_WHITE` | White | Tank's bib and blaze, disk label | #FFFFFF |
| 2 | `COLOR_RED` | Red | Tank's coat | #813338 |
| 3 | `COLOR_CYAN` | Cyan | Disk body, HUD text | #75CEC8 |
| 5 | `COLOR_GREEN` | Green | Exit frame | #56AC4D |
| 6 | `COLOR_BLUE` | Blue | Wall bricks | #2E2C9B |
| 7 | `COLOR_YELLOW` | Yellow | Crate frame, HUD counter | #EDF171 |
| 8 | `COLOR_ORANGE` | Orange | Dirt flecks, exit arrow | #8E5029 |
| 9 | `COLOR_BROWN` | Brown | Dirt, crate planks | #553800 |

## 4. Text map

Exactly 12 lines of exactly 20 characters. This block is the source of truth for the room.

```
####################
#T#E.#%%%%%%%%%%%%%#
#.##.#%%%%%%%%%D%%%#
#.##C#############%#
#.##%..............#
#.##.#############.#
#.####.............#
#.####.#############
#.####...%%%%%%....#
#.################.#
#....%%%%.....%%%..#
####################
```

The same map with rulers and the key tiles called out:

```
col     0         1
        01234567890123456789
row  0  ####################
row  1  #T#E.#%%%%%%%%%%%%%#   T = Tank start (1,1)    E = exit (3,1)
row  2  #.##.#%%%%%%%%%D%%%#   D = disk (15,2), embedded in the dirt field
row  3  #.##C#############%#   C = crate (4,3)
row  4  #.##%..............#   % at (4,4) = the crate's support (dirt plug)
row  5  #.##.#############.#   . at (4,5) = pit the crate lands in
row  6  #.####.............#   # at (4,6) = pit floor
row  7  #.####.#############
row  8  #.####...%%%%%%....#
row  9  #.################.#
row 10  #....%%%%.....%%%..#
row 11  ####################
```

## 5. Tile legend

| Char | Tile | Behaviour |
| --- | --- | --- |
| `#` | Wall | Solid. Blocks Tank and crates. Never changes. |
| `%` | Dirt | Diggable. Becomes open space when Tank enters it. Holds a crate up. |
| `.` | Open space | Walkable. A crate falls through it. |
| `C` | Crate | Falls while the tile below it is open. Cannot be pushed. |
| `D` | Disk | Collectible. Collected when Tank enters the tile. Opens the exit. |
| `T` | Tank | Tank's start tile. Open space once he moves off it. |
| `E` | Exit | Shut until the disk is collected. Entering it then ends the room. |

## 6. Key positions

| What | Tile | Notes |
| --- | --- | --- |
| Tank start | (1,1) | Top-left corner. |
| Exit | (3,1) | Two tiles east of the start, with wall (2,1) between them. |
| Exit pocket | (4,1), (4,2) | Open tiles between the exit and the crate. Sealed from the rest of the room by walls. |
| Crate | (4,3) | Plugs the shaft below the exit pocket. |
| Crate's support | (4,4) | One dirt tile. Removing it is the only way to move the crate. |
| Pit | (4,5) | Open tile under the support. The crate comes to rest here. Enclosed by walls (3,5), (5,5), (4,6). |
| Alcove | (5,4) | West end of the row 4 lane. The safe tile to step back to. |
| Disk | (15,2) | Inside the dirt field at the top right. Reached from (18,2). |

## 7. Rules the room depends on

Confirmed by Asaf:

1. Crates cannot be pushed, in any direction.
2. Tank has no gravity. He moves freely in four directions, including up shafts.

Decided by the designer, see section 9:

3. Tank moves one tile per input, in four directions, no diagonals.
4. Entering dirt removes it.
5. A crate falls straight down while the tile below it is open.
6. Tank standing directly beneath a resting crate holds it up. It starts to fall when he steps away.
7. A crate that is already falling kills Tank if it reaches his tile.
8. The exit opens when the disk is collected.

## 8. Intended route

Moves are `U D L R`. A number after a letter is a repeat count.

| Step | Moves | Ends at | Total moves | What happens |
| --- | --- | --- | --- | --- |
| 1 | D9 | (1,10) | 9 | Down the west shaft. |
| 2 | R17 | (18,10) | 26 | East along row 10. Digs 4 + 3 dirt tiles. |
| 3 | U2 | (18,8) | 28 | Up the east link. |
| 4 | L12 | (6,8) | 40 | West along row 8. Digs 6 dirt tiles. |
| 5 | U2 | (6,6) | 42 | Up the west link. |
| 6 | R12 | (18,6) | 54 | East along row 6. |
| 7 | U4 | (18,2) | 58 | Up past the row 4 lane into the dirt field. Digs (18,3) and (18,2). |
| 8 | L3 | (15,2) | 61 | Digs (17,2) and (16,2), collects the disk. Exit opens. |
| 9 | R3 D2 | (18,4) | 66 | Back out of the dirt field to the row 4 lane. |
| 10 | L13 | (5,4) | 79 | West along row 4 to the alcove. |
| 11 | L1 | (4,4) | 80 | Digs the support. Tank is now directly under the crate and holds it up. |
| 12 | R1 | (5,4) | 81 | Steps back. The crate falls from (4,3) through (4,4) into the pit at (4,5). |
| 13 | L1 U3 | (4,1) | 85 | Through the cleared shaft into the exit pocket. |
| 14 | L1 | (3,1) | 86 | Exit. |

As one string, for an automated test:

```
DDDDDDDDDRRRRRRRRRRRRRRRRRUULLLLLLLLLLLLUURRRRRRRRRRRRUUUULLLRRRDDLLLLLLLLLLLLLLRLUUUL
```

That is 86 moves and 18 dirt tiles dug, if a falling crate comes to rest before Tank's next move is processed.
If instead a crate falls one tile per game step, Tank has to wait one step after step 12, before `L`. Without the wait, the crate is still in (4,4) and blocks him. Simulated both ways: 86 inputs with instant falls, 87 with one wait.

Route trace. This is not a level map. `a` = approach only, `r` = return only, `b` = used both ways. The crate tile (4,3) shows as `r` because Tank passes through it after the crate has dropped.

```
####################
#T#Er#%%%%%%%%%%%%%#
#a##r#%%%%%%%%%Dbbb#
#a##r#############b#
#a##rrrrrrrrrrrrrrb#
#a##.#############a#
#a####aaaaaaaaaaaaa#
#a####a#############
#a####aaaaaaaaaaaaa#
#a################a#
#aaaaaaaaaaaaaaaaaa#
####################
```

Things that must also hold:

- From the start, the exit is unreachable without the disk-side detour. The only tiles next to the exit are wall (2,1), wall (3,2) and the pocket (4,1).
- The crate cannot be approached from above before it has dropped. The pocket is sealed.
- From (4,4), stepping down into the pit at (4,5) instead of back east to (5,4) lets the crate fall onto Tank. This is the room's one lethal mistake.
- The room cannot be made unwinnable. Dropping the crate before fetching the disk leaves every tile still reachable.

## 9. Details the designer decided, for Asaf to confirm

Settled by Asaf on 2026-11-28: Tank's coat is red (index 2, `COLOR_RED`), as `README.md` and `main.c` already have it. The first draft drew him orange.

Items 1 and 2 conflict with what is already in this repository.

1. **Disk colour.** The design draws the disk in cyan with a white label. `main.c` draws disks in dark grey (`COLOR_GRAY1`) with a white label. Dark grey would be hard to see on the black background this room uses.
2. **Screen colours.** The design assumes a black background, since open space is black. `main.c` sets a light blue background and a blue border.
3. **How tiles are drawn.** The art in section 10 is 8 x 8 chunky pixels per tile. `main.c` currently draws with reversed-space cells through conio, one colour per character cell, which gives only 2 x 2 coloured cells per tile and cannot show that art. Showing it needs a custom character set, hardware sprites, or both. The README roadmap already lists sprites. This is the biggest build decision.
4. **Time model.** The route was checked as turn-based: one input, one tile, then crates settle. The README roadmap asks for a real-time loop. In real time, the crate's fall speed decides whether step 12 needs a wait beat, and how forgiving the pit mistake is.
5. **Crate held by Tank.** A resting crate does not fall onto Tank while he is directly beneath it. Step 11 depends on this.
6. **Falling crate is lethal.** What happens after Tank is crushed (restart the room, lose a life) is not designed.
7. **Exit states.** The exit is shut until the disk is collected. Only one exit tile was drawn, so the shut and open looks are not distinguished yet.
8. **Scoring and HUD.** The mockup HUD reads `ROOM B  LONG WAY HOME` and `DISK 0/1` on screen row 24. The current prototype scores 10 points per disk and shows the score at the top. How the two fit together is not designed.
9. **Room name.** "Long Way Home" is the designer's working title.
10. **Input.** One tile per input in four directions. Keyboard or joystick, and which port, is not specified.
11. **Decorative dirt.** The dirt on rows 8 and 10 has no puzzle function. It only makes the lanes something to dig through. The dirt at (18,3), the top-right field and the support at (4,4) are required.
12. **Crate at rest.** Once in the pit the crate stays there for the rest of the room.
13. **Tile art.** All six tile drawings in section 10 are first drafts for readability at 16 x 16, not approved art.

## 10. Tile art drafts

Each tile is 8 x 8 chunky pixels. One chunky pixel is 2 x 2 screen pixels.
Letters: `K` black, `W` white, `C` cyan, `G` green, `B` blue, `Y` yellow, `O` orange, `N` brown, `R` red. `.` is background (black) for tiles and transparent for Tank and the disk.

```
Wall      Dirt      Crate     Disk      Exit      Tank
BBBB.BBB  NNONNNNN  YYYYYYYY  CCCCCCC.  GGGGGGGG  R......R
BBBB.BBB  NNNNNONN  YYNNNNYY  CCWWKWCC  G......G  RR....RR
BBBB.BBB  ONNN.NNN  YNYNNYNY  CCWWKWCC  G..OO..G  RRRRRRRR
........  NNNONNNO  YNNYYNNY  CCCCCCCC  G.OOOO.G  RKRWWRKR
B.BBBBBB  NNNNNNNN  YNNYYNNY  CWWWWWWC  G..OO..G  RRWWWWRR
B.BBBBBB  NONNNONN  YNYNNYNY  CWKKKKWC  G..OO..G  .WWKKWW.
B.BBBBBB  NN.NNNNO  YYNNNNYY  CWWWWWWC  G......G  .RWWWWR.
........  NNNNONNN  YYYYYYYY  CCCCCCCC  G......G  .RR..RR.
```

Colour budget the drafts were drawn to: wall, dirt, crate and exit each use black, brown, orange and at most one more colour (blue, yellow or green). Tank and the disk each use white, black and one own colour (red for Tank, cyan for the disk).

## 11. Not designed

Sound, title and game-over screens, lives, timers, more than one room, and Tank's animation or facing. Room A exists as an alternate on the design canvas but is not part of this handoff.
