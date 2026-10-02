# 地面の画像(コンクリート風グリッド)を生成するスクリプト
# タイル張り(繰り返し)しても、継ぎ目が目立たないように作る
# 出力先: Asset/Texture/Ground.png

Add-Type -AssemblyName System.Drawing

$size = 512
$cell = 64   # グリッドのマス目の大きさ(px)

$bitmap = New-Object System.Drawing.Bitmap $size, $size
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::None

# ---- ベースの色(コンクリートのような、明るいグレー) ----
$baseColor = [System.Drawing.Color]::FromArgb(255, 150, 150, 155)
$baseBrush = New-Object System.Drawing.SolidBrush $baseColor
$graphics.FillRectangle($baseBrush, 0, 0, $size, $size)

# ---- 少しだけ色をランダムにむらをつける(単調になりすぎないように) ----
$random = New-Object System.Random 42
$noiseCount = 2500
for ($i = 0; $i -lt $noiseCount; $i++) {
    $x = $random.Next(0, $size)
    $y = $random.Next(0, $size)
    $shade = $random.Next(-10, 10)
    $c = [System.Drawing.Color]::FromArgb(255,
        [Math]::Max(0, [Math]::Min(255, 150 + $shade)),
        [Math]::Max(0, [Math]::Min(255, 150 + $shade)),
        [Math]::Max(0, [Math]::Min(255, 155 + $shade)))
    $bitmap.SetPixel($x, $y, $c)
}

# ---- グリッドの線(タイルの境目。継ぎ目が揃うように、0 から cell 刻みで描く) ----
$lineColor = [System.Drawing.Color]::FromArgb(255, 120, 120, 125)
$linePen = New-Object System.Drawing.Pen $lineColor, 2

for ($x = 0; $x -lt $size; $x += $cell) {
    $graphics.DrawLine($linePen, $x, 0, $x, $size)
}
for ($y = 0; $y -lt $size; $y += $cell) {
    $graphics.DrawLine($linePen, 0, $y, $size, $y)
}

# ---- 保存 ----
$outDir = Join-Path $PSScriptRoot "..\Asset\Texture"
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }
$outPath = Join-Path $outDir "Ground.png"

$bitmap.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)

$graphics.Dispose()
$bitmap.Dispose()
$baseBrush.Dispose()
$linePen.Dispose()

Write-Output "Saved: $outPath"
