# Tank's Disk Rescue

A Commodore 64 game written in C with [cc65](https://cc65.github.io/), starring Tank, a red Pembroke Welsh Corgi who rescues floppy disks.

This is an early prototype ("experiment 1"): a single static scene with a score counter, not yet a full game.

## Current state

The scene shows Tank standing beside two floppy disks, drawn as coloured block art with reversed-space characters.

| Key | Action |
| --- | ------ |
| `D` | Collect the nearest remaining disk (+10 points). Does nothing once both are gone. |
| `R` | Reset the scene and the score. |

The score goes 0 → 10 → 20 and is shown centred at the top of the screen.

## Requirements

- [cc65](https://cc65.github.io/) — provides the `cl65` compiler/linker, which must be on your `PATH`
- [VICE](https://vice-emu.sourceforge.io/) — optional, provides the `x64sc` emulator used by `build.ps1 -Run`

## Building

With PowerShell:

```powershell
.\build.ps1         # build tank.prg
.\build.ps1 -Run    # build, then launch it in VICE
```

Or call the compiler directly:

```sh
cl65 -t c64 -O -o tank.prg main.c
```

## Running

Load `tank.prg` in any C64 emulator, or on real hardware. With VICE:

```sh
x64sc -autostart tank.prg
```

## Project layout

| File | Purpose |
| ---- | ------- |
| `main.c` | The whole game: block art, drawing, score and input loop |
| `build.ps1` | Build script, with an optional `-Run` switch to launch VICE |
| `tank.prg` | Build output (git-ignored) |

## Roadmap

- Real-time game loop with non-blocking joystick and keyboard input
- Tank moves, and collects disks by touching them
- Hardware sprites instead of character-cell block art
- Spawning disks, hazards or a timer
- SID sound effects
- Title and game-over screens
