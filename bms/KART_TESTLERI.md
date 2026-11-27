# Kart gelince test edilecekler

Kart olmadan yazılan kodda belgeden doğrulanamayan ya da belgede çelişkili olan noktalar.
Her madde: ne bilinmiyor, nasıl bakılır, sonuca göre kodda ne değişir.
Sonucu "Sonuç" sütununa tarihle yaz.

## BQ76952 (`drivers/bq76952/`)

| # | Ne test edilecek | Neden bilinmiyor | Nasıl bakılır | Sonuca göre ne yapılacak | Sonuç |
|---|---|---|---|---|---|
| 1 | **CRC'ye ne zaman geçiyor?** EXIT_CFGUPDATE'te mi, SWAP_COMM_MODE ile mi? | TRM çelişkili: s. 73 "CONFIG_UPDATE'ten çıkınca", s. 138 "reset'te ya da SWAP_COMM_MODE ile" | `bq76952_init` içinde `wait_crc_mode`'un ilk çağrısının sonucuna bak (breakpoint). Başarılıysa EXIT ile geçiyor; başarısızsa SWAP yolu çalıştı. | Kullanılmayan yolu sil, init'i sadeleştir. K-10'u güncelle. | |
| 2 | **CRC'ye geçiş kaç denemede oluyor?** | Geçiş süresi belgede yok | `wait_crc_mode` ve `wait_cfgupdate` döngülerinde kaçıncı denemede çıktığını say | 50 deneme sınırını gerçek değere göre ayarla (pay bırakarak) | |
| 3 | **DEVICE_NUMBER = 0x7695 mi?** | Değer TRM'de ve datasheet'te yok, genel bilgi | `bq76952_read_device_number` ile oku | Farklıysa `BQ76952_DEVICE_NUMBER_ID`'yi düzelt | |
| 4 | **Çip açılışta FULLACCESS modunda mı?** | CONFIG_UPDATE yalnızca FULLACCESS'te çalışır (TRM s. 71); açılış modu belgede açıkça yazmıyor | SET_CFGUPDATE sonrası Battery Status[CFGUPDATE] = 1 oluyor mu; Battery Status[SEC1:SEC0] = [0,1] mi | Değilse init'e erişim anahtarı adımı eklenmeli (ayrı karar) | |
| 5 | **Veri belleğine yazdıktan sonra beklemek gerekiyor mu?** | TRM'de süre yok; TI örnek kodu ~2 ms bekliyor, bizim kodumuz beklemiyor | Init'ten sonra Comm Type (0x9239) ve DA Configuration'ı (0x9303) geri oku: 0x12 ve beklenen değer mi? | Yazma kayboluyorsa `subcmd_write` sonrasına bekleme ekle | |
| 6 | **CRC hesabı doğru mu?** | TRM'de CRC-8 için sayısal örnek yok | Lojik analizörle bir okuma ve bir yazma çerçevesine bak; CRC'li okumalar `BMS_ERR_CRC` vermeden geçiyor mu | Hata varsa `crc8` ve çerçeve düzenini düzelt | |
| 7 | **CRC'li çipten CRC'siz 1 bayt okuma doğru veriyor mu?** | Init'te Battery Status bu şekilde okunuyor; belgede açıkça yazmıyor | EXIT sonrası `wait_cfgupdate` doğru sonuç veriyor mu | Vermiyorsa bekleme yöntemini değiştir | |
| 8 | **Akımın işareti:** şarj + mı? | SRP/SRN bağlantı yönüne bağlı (TRM §4.3 s. 20) | Bilinen yönde akım geçir, `read_current_ma` işaretine bak | Ters ise donanım düzeltilir ya da işaret driver'da çevrilir (karar) | |
| 9 | **Hücre giriş listesi doğru mu?** | Hangi 8 girişin kullanılacağı donanım ekibinde (1, 2, 16 zorunlu) | `read_all_cells_mv` ile 8 hücreyi oku; değerler makul mü; toplamları ≈ `read_stack_mv` mi | `board.h`'deki listeyi düzelt | |
| 10 | **MCU reset, BQ açık kalıyor** (BQ zaten CRC modunda) | Kart olmadan denenemedi | Init bittikten sonra MCU'yu resetle, init tekrar başarılı mı | Başarısızsa `detect_mode` akışını düzelt | |
| 11 | **MCU, CONFIG_UPDATE içindeyken resetleniyor** | Kart olmadan denenemedi | SET_CFGUPDATE'ten sonra breakpoint'te resetle; sonraki init takılı modu temizliyor mu | Temizlemiyorsa init'in 2. adımını düzelt | |
| 12 | **Passed charge açılışta sıfırlanıyor mu?** | Belgede açılış davranışı yazmıyor | Açılışta `read_passed_charge` oku | Sıfırlanmıyorsa init'e `reset_passed_charge` eklenmesi düşünülür | |
| 13 | **Passed charge kesir ölçeği** (isteğe bağlı) | Bayt 4–7'nin ölçeği belgede yok (muhtemelen 1/2³²) | Bilinen akımla bir süre biriktir, tam ve kesir kısımlarını karşılaştır | Ölçek bulunursa çözünürlük 10 mAh'ten iyileştirilebilir | |
| 14 | **CC3 ortalaması ne sıklıkla güncelleniyor?** | Belgede yok; ~240 ms türetildi (80 örnek × 3 ms) | `read_current_avg_ma` değerinin değişme aralığını ölç | Afe periyodu seçilirken hesaba kat | |
| 15 | **Sıcaklıklar:** INT ve TS1 makul mü, TS2/TS3 ne okuyor? | TS2/TS3 fabrika ayarında kapalı (TRM §13.3.2.13–14) | `read_temp` ile dördünü de oku | TS pin ayarı kararı (AS-04) | |
| 16 | **PACK ve LD anlamlı değer veriyor mu?** | Pinlerin bağlantısı donanıma bağlı | FET açıkken PACK ≈ stack mı | Kullanılmıyorsa app'te okunmaz | |
