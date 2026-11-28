# Tank's Disk Rescue

A Commodore 64 game written in C with [cc65](https://cc65.github.io/), starring Tank, a red Pembroke Welsh Corgi who rescues floppy disks.

## Current state

One playable room: **Room B, "Long Way Home"**. The level spec is [`docs/room-b-handoff.md`](docs/room-b-handoff.md), and [`docs/room-b-reference.png`](docs/room-b-reference.png) is a reference render of the room.

The exit is two tiles from where Tank starts, but behind a wall. Tank digs his way round the whole room to fetch the disk. Then he has to drop the crate out of the only shaft that leads to the exit, without standing underneath it when it falls.

### Controls

| Input | Action |
| --- | --- |
| Joystick in port 2, or `W` `A` `S` `D` | Move one tile up, left, down or right. Hold to keep moving. |
| `R` | Restart the room |
| Fire | Restart after Tank is crushed or the room is cleared |

### Rules

- Tank moves one tile per step in four directions. He has no gravity, so he can climb shafts.
- Moving into dirt digs it away.
- Crates can't be pushed. A crate falls while the tile below it is open.
- If Tank stands directly under a resting crate, he holds it up. It falls when he steps away.
- A falling crate that reaches Tank crushes him.
- Collecting the disk opens the exit. Entering the open exit clears the room.

The game is turn-based. Each move is one step, and crates finish falling before the next move is taken. On the C64, the fall is animated, and input waits until the crate lands.

## Requirements

- [cc65](https://cc65.github.io/) provides `cl65` and `sim65`, which must be on your `PATH`.
- [VICE](https://vice-emu.sourceforge.io/) is optional. `build.ps1 -Run` uses its `x64sc` emulator.

## Building

With PowerShell:

```powershell
.\build.ps1         # build tank.prg
.\build.ps1 -Run    # build, then launch it in VICE
.\build.ps1 -Test   # build, then run the logic test under sim65
```

Or call the compiler directly:

```sh
cl65 -t c64 -O -Wl -D,__HIMEM__=0xC000 -o tank.prg main.c gfx.c room.c rooms.c
```

`__HIMEM__=0xC000` keeps the program below `$C000`, because the screen, sprites and character set live at `$C000-$CFFF`. If the program grows into that area, the linker stops with an error.

## Testing

The game logic has no C64 dependencies. `tests/test_room.c` runs it on the PC under `sim65`, the 6502 simulator that comes with cc65. It checks the following, taken from section 8 of the handoff:

- The 86-move intended route. The disk is collected on move 61, the crate ends in the pit at (4,5), and Tank reaches the exit, having dug 18 dirt tiles.
- Every tile next to the exit is a wall except the sealed pocket at (4,1). Tank can't reach that pocket until the crate has dropped.
- The crate can't be approached from above before it drops.
- From (4,4), stepping down into the pit gets Tank crushed.
- Dropping the crate before fetching the disk still leaves every tile reachable, and the room can still be won.

The test is plain C89, so it also builds with a native compiler:

```sh
gcc -I. -o test_room tests/test_room.c room.c rooms.c && ./test_room
```

## Running

Load `tank.prg` in any C64 emulator, or on real hardware. With VICE:

```sh
x64sc -autostart tank.prg
```

## Project layout

| File | Purpose |
| ---- | ------- |
| `main.c` | Frame loop and input (joystick port 2, keyboard) |
| `room.h`, `room.c` | Game logic: movement, digging, crates, disk, exit. No C64 code. |
| `rooms.h`, `rooms.c` | Room data. The Room B map is copied verbatim from section 4 of the handoff. |
| `gfx.h`, `gfx.c` | C64 drawing: VIC-II setup, multicolour tile charset, sprites, HUD, crate fall animation |
| `tests/test_room.c` | PC-side logic test, run with `.\build.ps1 -Test` |
| `build.ps1` | Build script, with `-Run` (launch VICE) and `-Test` (run the logic test) |
| `docs/` | Room B level spec and reference render |
| `tank.prg`, `test_room.prg` | Build output (git-ignored) |

### Graphics

The VIC-II uses memory bank 3:

| Address | Contents |
| --- | --- |
| `$C000` | Screen |
| `$C400` | Sprites |
| `$C800` | Character set: the ROM font for the HUD, with tile glyphs from character 64 up |

Each 16×16 tile is 2×2 multicolour characters, drawn from the art in section 10 of the handoff. Black, brown and orange are shared by every tile, and each tile adds one colour of its own: blue for walls, yellow for the crate, green for the exit. Tank and the disk are multicolour sprites. White and black are shared between them, and each has its own colour: red for Tank, cyan for the disk.

## Roadmap

These come from the old prototype roadmap. None are designed for Room B yet.

- More rooms
- SID sound effects
- Title and game-over screens
- Lives, and what happens after Tank is crushed (for now the room just restarts)
- Tank's animation and facing
