$OutputEncoding = [System.Text.Encoding]::UTF8
[console]::InputEncoding = [System.Text.Encoding]::UTF8
[console]::OutputEncoding = [System.Text.Encoding]::UTF8
$gh = ".\gh_cli\bin\gh.exe"

# GitHub hesabınıza bağlı olup olmadığını kontrol et
& $gh auth status 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Lütfen tarayıcı penceresinden GitHub hesabınıza giriş yapın ve cihazı doğrulayın..." -ForegroundColor Yellow
    & $gh auth login -w -p https
}

Write-Host "`nGitHub üzerinde 'LockKeyIndicator' reposu oluşturuluyor ve kodlar yükleniyor..." -ForegroundColor Cyan
& $gh repo create LockKeyIndicator --public --description "Caps Lock / NumLock / ScrollLock On-off indicator for Windows" --source=. --push

Write-Host "`nV1.0.0.2 adıyla Release oluşturuluyor ve LockKeyIndicator.exe yükleniyor..." -ForegroundColor Cyan
& $gh release create v1.0.0.2 ".\LockKeyIndicator.exe" --title "V1.0.0.2" --notes "Yayınlanabilir güncel sürüm"

Write-Host "`nTüm işlemler başarıyla tamamlandı! Tarayıcınızda açılıyor..." -ForegroundColor Green
& $gh repo view --web
