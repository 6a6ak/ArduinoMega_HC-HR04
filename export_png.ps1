# export_png.ps1 - convert wiring.svg to wiring.png using ImageMagick or Inkscape
# Usage: .\export_png.ps1

$svg = Join-Path -Path $PSScriptRoot -ChildPath 'wiring.svg'
$png = Join-Path -Path $PSScriptRoot -ChildPath 'wiring.png'

if (-not (Test-Path $svg)) {
    Write-Host "wiring.svg not found in $PSScriptRoot" -ForegroundColor Red
    exit 1
}

function Run-Magick($infile, $outfile) {
    & magick convert $infile $outfile
}
function Run-Inkscape($infile, $outfile) {
    # inkscape CLI varies by version; try common options
    & inkscape $infile --export-type=png --export-filename=$outfile
}

# Try ImageMagick (magick)
try {
    $magick = Get-Command magick -ErrorAction SilentlyContinue
    if ($magick) {
        Write-Host "Using ImageMagick (magick) to convert SVG -> PNG"
        Run-Magick $svg $png
        Write-Host "Generated: $png"
        exit 0
    }
} catch { }

# Try inkscape
try {
    $ink = Get-Command inkscape -ErrorAction SilentlyContinue
    if ($ink) {
        Write-Host "Using Inkscape to convert SVG -> PNG"
        Run-Inkscape $svg $png
        Write-Host "Generated: $png"
        exit 0
    }
} catch { }

Write-Host "No ImageMagick or Inkscape found on PATH." -ForegroundColor Yellow
Write-Host "Install one of these and re-run the script. Examples:" -ForegroundColor Yellow
Write-Host "- ImageMagick: https://imagemagick.org/script/download.php" -ForegroundColor Cyan
Write-Host "- Inkscape: https://inkscape.org/release/" -ForegroundColor Cyan
exit 1
