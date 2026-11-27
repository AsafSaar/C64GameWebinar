# Builds tank.prg for the Commodore 64. Pass -Run to launch it in VICE afterwards.
param([switch]$Run)

cl65 -t c64 -O -o "$PSScriptRoot\tank.prg" "$PSScriptRoot\main.c"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Built tank.prg"

if ($Run) { x64sc -autostart "$PSScriptRoot\tank.prg" }
