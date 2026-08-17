# Değişim Günlüğü (Changelog)

## [V1.0.0.3] - 2026-08-17
### Eklenenler/Düzeltilenler
- README.md dosyası eklendi ve antivirüs programlarının uyarı verme nedeni (SetWindowsHookEx sebebiyle oluşan False Positive) açıklandı.
- Tüm mesaj kutularındaki ASCII (İngilizce karakter) metinler %100 Türkçe Unicode karakterlerle (MessageBoxW kullanılarak) güncellendi.
- Sürüm numaralandırması ve yedekleme sistemi Changelog dosyası üzerinden takip edilecek şekilde revize edildi.

## [V1.0.0.2] - 2026-08-17
### Eklenenler/Düzeltilenler
- Proje C++ Static Library (/MT) desteği ile güncellendi, böylece programın çalışması için son kullanıcılarda VCRedist kurulmasına gerek kalmadı.
- Otomatik GitHub CLI publish scripti (`github_publish.ps1`) eklendi.
- Script ve derleme notlarındaki mojibake hataları giderildi.
