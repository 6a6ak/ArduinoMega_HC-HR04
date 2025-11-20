# build_diag.ps1 - compile diag_7seg.c to diag_7seg.hex for ATmega2560
$ErrorActionPreference = 'Stop'

function Find-Tool($name, $defaultPaths) {
    $tool = Get-Command $name -ErrorAction SilentlyContinue
    if ($tool) { return $tool.Path }
    foreach ($p in $defaultPaths) {
        if (Test-Path "$p\$name.exe") { return "$p\$name.exe" }
    }
    return $null
}

$defaultAvrPaths = @('C:\avr\bin', 'C:\Program Files (x86)\Atmel\avr8-gnu-toolchain\bin')
$gcc = Find-Tool 'avr-gcc' $defaultAvrPaths
$objcopy = Find-Tool 'avr-objcopy' $defaultAvrPaths

if (-not $gcc -or -not $objcopy) {
    Write-Host "avr-gcc or avr-objcopy not found. Please install AVR toolchain and ensure tools are on PATH." -ForegroundColor Yellow
    exit 1
}

Write-Host "Using avr-gcc: $gcc"
Write-Host "Using avr-objcopy: $objcopy"

$mcU = 'atmega2560'
$f_cpu = '16000000UL'
$src = 'diag_7seg.c'
$elf = 'diag_7seg.elf'
$hex = 'diag_7seg.hex'
$obj = 'diag_7seg.o'

$compileArgs = @("-mmcu=$mcU", "-DF_CPU=$f_cpu", "-Os", "-Wall", "-c", $src, "-o", $obj)
Write-Host "Compiling: $gcc $($compileArgs -join ' ')"
& $gcc @compileArgs
if ($LASTEXITCODE -ne 0) { throw "Compilation failed (avr-gcc)" }

$linkArgs = @("-mmcu=$mcU", $obj, "-o", $elf)
Write-Host "Linking: $gcc $($linkArgs -join ' ')"
& $gcc @linkArgs
if ($LASTEXITCODE -ne 0) { throw "Link failed (avr-gcc)" }

$objcopyArgs = @("-O", "ihex", "-R", ".eeprom", $elf, $hex)
Write-Host "Objcopy: $objcopy $($objcopyArgs -join ' ')"
& $objcopy @objcopyArgs
if ($LASTEXITCODE -ne 0) { throw "objcopy failed (avr-objcopy)" }

Write-Host "Build succeeded. Generated: $hex" -ForegroundColor Green
Get-ChildItem -Path . -Filter $hex | Format-List Name,Length,LastWriteTime
