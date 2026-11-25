# bms/ — BMS uygulama kodu

CubeMX'in ürettiği `Core/` ve `Drivers/` dışında kalan, projeye ait bütün kod burada (K-01).

| Klasör | İçerik |
|---|---|
| `app/measurement/` | Ölçüm: AFE'den oku, CRC ve makullük kontrolü |
| `app/protection/` | Katman 1 koruma: eşikler, I²t, sıcaklık → şarj/deşarj izni |
| `app/estimation/` | Kestirim: EKF ile SoC ve SoH |
| `app/state_machine/` | Durum makinesi: mod ve anahtar kararı (durumu yalnızca bu modül değiştirir) |
| `app/hw_control/` | Donanım-kontrol: Katman 2 denetimi, ön şarj, anahtar pinleri, Share Bus (taslak) |
| `app/comms/` | CAN: telemetri gönderme, komut alma |
| `app/storage/` | FRAM: olay kaydı ve batarya pasaportu |
| `app/supervisor/` | Görev gözetmeni: kritik görevler çalışıyorsa watchdog'u besler |
| `drivers/` | Çiplere özel kod: BQ76952, FRAM, CAN, ADC, GPIO |
| `common/` | Ortak tipler, birimler, hata kodları, eşikler |
| `system/` | Açılış, görevlerin oluşturulması, kesmeler |

Kural: `app/` içindeki modüller donanıma doğrudan dokunmaz, her zaman `drivers/` üzerinden konuşur.
Ayrıntılı kararlar: `Bitirme1/PROJE_DURUMU.md` Bölüm 0.
