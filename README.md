# Lock Key Indicator

Caps Lock, Num Lock ve Scroll Lock tuşlarının durumunu ekranda anlık olarak gösteren, C++ ile yazılmış şık ve pratik bir Windows uygulamasıdır.

## ⚠️ Antivirüs Yanlış Alarmı (False Positive) Hakkında Önemli Bilgi
LockKeyIndicator arka planda klavye tuşlarına basıldığını algılamak için Windows'un yasal bir özelliği olan **`SetWindowsHookEx(WH_KEYBOARD_LL)`** API'sini kullanmaktadır. Bu kod, programın sadece Caps/Num/Scroll Lock tuşlarına basıldığında ekranda belirmesini sağlar. Ancak bu yöntem genellikle "keylogger" tarzı zararlı yazılımlar tarafından da kullanıldığı için, ayrıca programın doğrudan internetten indirilmiş imzasız bir (.exe) dosyası olması sebebiyle **Windows Defender ve bazı antivirüs programları uygulamayı yanlışlıkla virüs (Trojan:Win32/Sabsik.TE.A!ml vb.) olarak algılayabilir.**

Bu tamamen **yanlış bir alarmdır (False Positive)**. Uygulama hiçbir şekilde tuş vuruşlarınızı kaydetmez veya internete bir veri göndermez. Kodlar tamamen açık kaynaklıdır; şeffaf bir şekilde inceleyebilir veya kendiniz derleyebilirsiniz.

**Çözüm:** Antivirüsünüz uyarı verdiğinde "Cihazda İzin Ver" seçeneğini seçebilir veya `LockKeyIndicator.exe` dosyasını tarama istisnalarına ekleyebilirsiniz.

## Özellikler
- **Anlık Bildirim:** Lock tuşlarına bastığınızda ekranda şeffaf bir arayüzle durumlarını gösterir.
- **Düşük Kaynak Tüketimi:** Tamamen C++ ve Windows API kullanılarak yazılmıştır.
- **Kişiselleştirilebilir:** Gecikme süresi ve şeffaflık ayarlarını değiştirebilirsiniz.
- **Taşınabilir:** Kurulum gerektirmez, doğrudan çalışır.

## Geliştirici
Ali HABER (ogcizimci@gmail.com)
