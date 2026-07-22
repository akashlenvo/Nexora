param([string]$RepositoryRoot = (Resolve-Path "$PSScriptRoot\..\.."))

Add-Type -AssemblyName System.Drawing

function New-NexoraBitmap([int]$Size, [bool]$Round = $false) {
    $bitmap = New-Object Drawing.Bitmap $Size, $Size, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.Clear([Drawing.Color]::Transparent)
    $radius = [single]($Size * 0.22)
    $path = New-Object Drawing.Drawing2D.GraphicsPath
    if ($Round) { $path.AddEllipse(0, 0, $Size, $Size) }
    else {
        $diameter = $radius * 2
        $path.AddArc(0, 0, $diameter, $diameter, 180, 90)
        $path.AddArc($Size-$diameter, 0, $diameter, $diameter, 270, 90)
        $path.AddArc($Size-$diameter, $Size-$diameter, $diameter, $diameter, 0, 90)
        $path.AddArc(0, $Size-$diameter, $diameter, $diameter, 90, 90)
        $path.CloseFigure()
    }
    $graphics.FillPath((New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(8,13,24))), $path)
    $violet = New-Object Drawing.Pen ([Drawing.Color]::FromArgb(108,99,255)), ([single]($Size * 0.115))
    $violet.StartCap = $violet.EndCap = [Drawing.Drawing2D.LineCap]::Round
    $graphics.DrawLines($violet, [Drawing.PointF[]]@(
        [Drawing.PointF]::new($Size*.27,$Size*.73), [Drawing.PointF]::new($Size*.27,$Size*.27),
        [Drawing.PointF]::new($Size*.72,$Size*.73), [Drawing.PointF]::new($Size*.72,$Size*.27)))
    $violet.Dispose(); $path.Dispose(); $graphics.Dispose()
    return $bitmap
}

$densities = @{ 'mipmap-mdpi'=48; 'mipmap-hdpi'=72; 'mipmap-xhdpi'=96; 'mipmap-xxhdpi'=144; 'mipmap-xxxhdpi'=192 }
foreach ($entry in $densities.GetEnumerator()) {
    $dir = Join-Path $RepositoryRoot "android\app\src\main\res\$($entry.Key)"
    New-Item -ItemType Directory -Force $dir | Out-Null
    $square = New-NexoraBitmap $entry.Value $false
    $round = New-NexoraBitmap $entry.Value $true
    $square.Save((Join-Path $dir 'ic_launcher.png'), [Drawing.Imaging.ImageFormat]::Png)
    $square.Save((Join-Path $dir 'ic_launcher_foreground.png'), [Drawing.Imaging.ImageFormat]::Png)
    $round.Save((Join-Path $dir 'ic_launcher_round.png'), [Drawing.Imaging.ImageFormat]::Png)
    $square.Dispose(); $round.Dispose()
}

$play = New-NexoraBitmap 512 $false
$play.Save((Join-Path $RepositoryRoot 'android\app\src\main\ic_launcher-playstore.png'), [Drawing.Imaging.ImageFormat]::Png)
$play.Dispose()

$iconBitmap = New-NexoraBitmap 256 $false
$handle = $iconBitmap.GetHicon()
$icon = [Drawing.Icon]::FromHandle($handle)
$iconPath = Join-Path $RepositoryRoot 'windows\res\nexora.ico'
$stream = [IO.File]::Open($iconPath, [IO.FileMode]::Create)
$icon.Save($stream)
$stream.Dispose(); $icon.Dispose(); $iconBitmap.Dispose()

Write-Host "Nexora assets generated."
