# Builds tank.prg for the Commodore 64.
#   .\build.ps1         build tank.prg
#   .\build.ps1 -Run    build, then launch it in VICE
#   .\build.ps1 -Test   build and run the PC-side logic test under sim65
param([switch]$Run, [switch]$Test)

$src = "main.c", "gfx.c", "room.c", "rooms.c" | ForEach-Object { Join-Path $PSScriptRoot $_ }

# __HIMEM__=$C000 keeps the program out of $C000-$CFFF, which gfx.c uses for
# the screen, sprites and character set. The linker fails if code grows into it.
cl65 -t c64 -O -I "$PSScriptRoot" -Wl "-D,__HIMEM__=0xC000" -o "$PSScriptRoot\tank.prg" @src
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Built tank.prg"

if ($Test) {
    $testSrc = "tests\test_room.c", "room.c", "rooms.c" | ForEach-Object { Join-Path $PSScriptRoot $_ }
    cl65 -t sim6502 -O -I "$PSScriptRoot" -o "$PSScriptRoot\test_room.prg" @testSrc
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    sim65 "$PSScriptRoot\test_room.prg"
    if ($LASTEXITCODE -ne 0) { Write-Host "Tests FAILED"; exit $LASTEXITCODE }
}

if ($Run) { x64sc -autostart "$PSScriptRoot\tank.prg" }
