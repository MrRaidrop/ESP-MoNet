# Fix include path inside monet_hal to match renamed component
# From: components/monet_hal/include/my_hal/...
# To:   components/monet_hal/include/monet_hal/...

$oldPath = "components\monet_hal\include\my_hal"
$newPath = "components\monet_hal\include\monet_hal"

if (Test-Path $oldPath) {
    Rename-Item -Path $oldPath -NewName "monet_hal"
    Write-Host "✅ Renamed $oldPath -> $newPath"
} else {
    Write-Host "❌ Path not found: $oldPath"
}

# Recursively fix all #include references in .c and .h files
Get-ChildItem -Path . -Recurse -Include *.c, *.h | ForEach-Object {
    (Get-Content $_.FullName) `
    -replace 'my_hal/', 'monet_hal/' | Set-Content $_.FullName
}

Write-Host "✅ Include path references updated to 'monet_hal/'"
