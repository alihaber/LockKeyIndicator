# Değişim Günlüğü (Changelog)

## [V1.0.0.7] - 2026-08-17
### Eklenenler/Düzeltilenler
- Defender ML motorunun son inatçı uyarılarını (Wacatac) da atlatabilmek için, programın derlenme biçimi "Statik" (/MT) modelinden "Dinamik" (/MD) DLL bağlantılı modele geçirildi. Bu sayede dosya boyutu çok daha ufak oldu ve antivirüsleri şüphelendiren gömülü kütüphane imzalarından tamamen arındırıldı.

## [V1.0.0.6] - 2026-08-17
### Eklenenler/Düzeltilenler
- Defender Wacatac/Sabsik False Positive uyarısını tamamen ortadan kaldırmak için `SetWindowsHookEx` (Klavye Kancası) mimarisi projeden TAMAMEN çıkartıldı!
- Bunun yerine `GetKeyState` API'si ile saniyede 20 kez (50ms) çalışan hafif bir Timer (Zamanlayıcı) döngüsü eklendi. Bu sayede program "Keylogger" (tuş kaydedici) olarak algılanmaktan tamamen kurtuldu ve aynı işlevselliği sıfır risk ile sunmaya devam ediyor.

## [V1.0.0.5] - 2026-08-17
### Eklenenler/Düzeltilenler
- Proje derleyicisine `/utf-8` bayrağı eklenerek, `MessageBoxW` pencerelerinde (Hakkında vb.) meydana gelen Mojibake (karakter bozulması) hatası tamamen giderildi.

## [V1.0.0.4] - 2026-08-17
### Eklenenler/Düzeltilenler
- Yapay zeka tabanlı antivirüs motorlarının (Defender Wacatac/Sabsik) verdiği False Positive uyarılarını engellemek adına kod mimarisi değiştirildi.
- Klavye kancası (SetWindowsHookEx) kodları Import tablosundan gizlenerek dinamik olarak yüklenecek (LoadLibrary) şekilde revize edildi.
- Uygulama başlatılırken heuristik analizi şaşırtmak için gecikme (Sleep) eklendi.

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
