# build.ps1 - compile main.c to main.hex for ATmega2560
# Usage: Open PowerShell, cd to repository and run `.uild.ps1`

$ErrorActionPreference = 'Stop'

# Check for avr-gcc
function Find-Tool($name, $defaultPaths) {
    $tool = Get-Command $name -ErrorAction SilentlyContinue
    if ($tool) { return $tool.Path }
    foreach ($p in $defaultPaths) {
        if (Test-Path "$p\$name.exe") { return "$p\$name.exe" }
    }
    return $null
}

$defaultAvrPaths = @('C:\avr\bin', 'C:\Program Files (x86)\Atmel\avr8-gnu-toolchain\bin', 'C:\Program Files (x86)\Microchip\xc8\v2.36\bin')
$gcc = Find-Tool 'avr-gcc' $defaultAvrPaths
$objcopy = Find-Tool 'avr-objcopy' $defaultAvrPaths

if (-not $gcc -or -not $objcopy) {
    Write-Host "avr-gcc or avr-objcopy not found.\nPlease install the AVR toolchain and ensure 'avr-gcc' and 'avr-objcopy' are on your PATH or located in one of: $($defaultAvrPaths -join ', ')" -ForegroundColor Yellow
    exit 1
}

Write-Host "Using avr-gcc: $gcc"
Write-Host "Using avr-objcopy: $objcopy"

$mcU = 'atmega2560'
$f_cpu = '16000000UL'
$src = 'main.c'
$elf = 'main.elf'
$hex = 'main.hex'
$obj = 'main.o'

# compile (use argument arrays to ensure proper variable expansion)
$compileArgs = @("-mmcu=$mcU", "-DF_CPU=$f_cpu", "-Os", "-Wall", "-c", $src, "-o", $obj)
Write-Host "Compiling: $gcc $($compileArgs -join ' ')"
& $gcc @compileArgs
if ($LASTEXITCODE -ne 0) { throw "Compilation failed (avr-gcc)" }

# link
$linkArgs = @("-mmcu=$mcU", $obj, "-o", $elf)
Write-Host "Linking: $gcc $($linkArgs -join ' ')"
& $gcc @linkArgs
if ($LASTEXITCODE -ne 0) { throw "Link failed (avr-gcc)" }

# create hex
$objcopyArgs = @("-O", "ihex", "-R", ".eeprom", $elf, $hex)
Write-Host "Objcopy: $objcopy $($objcopyArgs -join ' ')"
& $objcopy @objcopyArgs
if ($LASTEXITCODE -ne 0) { throw "objcopy failed (avr-objcopy)" }

Write-Host "Build succeeded. Generated: $hex" -ForegroundColor Green
Get-ChildItem -Path . -Filter $hex | Format-List Name,Length,LastWriteTime
