# 背景画像(空のグラデーション + ビルのシルエット)を生成するスクリプト
# 出力先: Asset/Texture/Background.png

Add-Type -AssemblyName System.Drawing

$width = 1920
$height = 1080

$bitmap = New-Object System.Drawing.Bitmap $width, $height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

# ---- 空のグラデーション(上:濃い空色 → 下:淡い水色) ----
$skyTop = [System.Drawing.Color]::FromArgb(255, 120, 180, 235)
$skyBottom = [System.Drawing.Color]::FromArgb(255, 210, 235, 248)

$skyRect = New-Object System.Drawing.Rectangle 0, 0, $width, $height
$skyBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
    $skyRect, $skyTop, $skyBottom, [System.Drawing.Drawing2D.LinearGradientMode]::Vertical)
$graphics.FillRectangle($skyBrush, $skyRect)

# ---- ビルのシルエット(奥:薄い色、手前:濃い色の2レイヤー) ----
function Draw-Skyline {
    param($graphics, $color, $baseY, $minHeight, $maxHeight, $minWidth, $maxWidth, $seed)

    $random = New-Object System.Random $seed
    $brush = New-Object System.Drawing.SolidBrush $color
    $x = -50

    while ($x -lt $width + 50) {
        $buildingWidth = $random.Next($minWidth, $maxWidth)
        $buildingHeight = $random.Next($minHeight, $maxHeight)
        $y = $baseY - $buildingHeight

        $graphics.FillRectangle($brush, $x, $y, $buildingWidth, $buildingHeight)

        # 屋上の小さい突起(アンテナ・貯水槽っぽいもの)
        if ($random.Next(0, 3) -eq 0) {
            $topWidth = [int]($buildingWidth * 0.3)
            $topHeight = $random.Next(10, 30)
            $topX = $x + [int](($buildingWidth - $topWidth) / 2)
            $graphics.FillRectangle($brush, $topX, $y - $topHeight, $topWidth, $topHeight)
        }

        $x += $buildingWidth + $random.Next(5, 25)
    }
    $brush.Dispose()
}

$baseline = [int]($height * 0.72)

# 奥のビル(薄い色、低め)
$farColor = [System.Drawing.Color]::FromArgb(255, 165, 195, 220)
Draw-Skyline -graphics $graphics -color $farColor -baseY $baseline -minHeight 80 -maxHeight 220 -minWidth 60 -maxWidth 140 -seed 1

# 手前のビル(濃い色、高め)
$nearColor = [System.Drawing.Color]::FromArgb(255, 130, 160, 190)
Draw-Skyline -graphics $graphics -color $nearColor -baseY ($baseline + 20) -minHeight 140 -maxHeight 340 -minWidth 80 -maxWidth 180 -seed 2

# ---- 保存 ----
$outDir = Join-Path $PSScriptRoot "..\Asset\Texture"
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }
$outPath = Join-Path $outDir "Background.png"

$bitmap.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)

$graphics.Dispose()
$bitmap.Dispose()

Write-Output "Saved: $outPath"
