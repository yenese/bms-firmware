# common — Ortak

Bütün modüllerin kullandığı tipler, birimler ve hata kodu (K-01, K-05). Donanımdan bağımsızdır:
HAL veya FreeRTOS başlığı içermez, SIL için bilgisayarda da derlenir.

## Dosyalar

| Dosya | İçerik | REQ |
|---|---|---|
| `bms_types.h` | Fiziksel büyüklük tipleri (aşağıdaki tablo) ve `BMS_CELLS_IN_SERIES` (8, 8S1P) | REQ-SW-012, REQ-SW-013, REQ-SW-026, REQ-COM-008, REQ-SYS-001 |
| `bms_status.h` | Tek hata tipi `bms_status_t` | REQ-SW-010, REQ-SW-003, REQ-SW-017, REQ-SW-025, REQ-PRT-012 |

Eşikler (OV/UV/OT vb.) henüz burada değil; çoğu TBD (AS-01).

## Birimler (K-05)

| Büyüklük | Tip | Taban tip | Birim | Not |
|---|---|---|---|---|
| Hücre gerilimi | `bms_cell_mv_t` | `uint16_t` | mV | BQ76952 birimiyle aynı |
| Akım | `bms_ma_t` | `int32_t` | mA | şarj (+), deşarj (−); 120 A 16 bite sığmaz |
| Sıcaklık | `bms_deci_degc_t` | `int16_t` | 0,1 °C | BQ76952 0,1 K verir; dönüşüm BQ76952 driver'ında |
| Zaman damgası | `bms_time_ms_t` | `uint32_t` | ms, açılıştan beri | ~49,7 günde sıfıra döner; süre = `yeni - eski` (işaretsiz) |
| SoC | `bms_soc_t` | `float` | 0–1 | yalnızca kestirim |
| SoH | `bms_soh_t` | `float` | 0–1 | yalnızca kestirim |

Koruma kararları tamsayı tiplerle, kestirim float ile yapılır.

## Hata kodları (`bms_status_t`)

| Değer | Ad | Anlamı |
|---|---|---|
| 0 | `BMS_OK` | Başarılı |
| 1 | `BMS_ERR_PARAM` | Geçersiz parametre / NULL işaretçi (REQ-SW-010) |
| 2 | `BMS_ERR_TIMEOUT` | Sınırlı bekleme doldu (REQ-SW-003) |
| 3 | `BMS_ERR_CRC` | CRC uyuşmadı (REQ-SW-017) |
| 4 | `BMS_ERR_COMM` | Yanıt yok / hat hatası |
| 5 | `BMS_ERR_RANGE` | Makul olmayan değer (REQ-SW-025, REQ-PRT-012) |
| 6 | `BMS_ERR_BUSY` | Kaynak meşgul |
| 7 | `BMS_ERR_STATE` | Bu durumda izin verilmeyen işlem |

Her fonksiyon `bms_status_t` döndürür, çağıran dönüş değerini her zaman kontrol eder.

## Derleme

`bms/common` include yoluna eklenmelidir (CubeIDE projesinde `bms/` henüz kaynak/include yolu değil).
Bu klasörde yalnızca başlık dosyaları var.
