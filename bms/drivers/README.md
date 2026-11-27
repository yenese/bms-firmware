# drivers — Çip sürücüleri

Donanıma özel kod: BQ76952, FRAM, CAN, ADC, GPIO. Uygulama modülleri donanıma yalnızca buradan erişir.

## port/

Çip katmanının altındaki ince HAL katmanı (K-04, K-08). Çip sürücüleri HAL'i görmez, yalnızca buradaki başlıkları çağırır.

- `port_i2c.h/.c`: bloklayan, zaman aşımlı I2C. Cihaz `port_i2c_dev_t` (hat, 7 bit adres, zaman aşımı); `port_i2c_write()`, `port_i2c_read()`, `port_i2c_read_reg()`. CRC ve tekrar deneme yoktur.
- `board.h`: mantıksal hat -> HAL tutamacı (`PORT_I2C_BUS_AFE` -> `hi2c1`).

Karşılanan gereksinimler: REQ-SW-003, REQ-SW-010, REQ-SW-017.

## bq76952/

BQ76952 AFE çip sürücüsü (K-10). Yalnızca `port_i2c` üzerinden haberleşir, HAL'i görmez.

- `bq76952_regs.h`: kullanılan direct command, subcommand ve veri belleği adresleri (SLUUBY2B).
- `bq76952.h/.c`: `bq76952_dev_t` (I2C cihazı + `crc_on`). I2C CRC-8 çerçeveleme; çekirdek `cmd_read/cmd_write`,
  `subcmd`, `subcmd_read/subcmd_write` (0x3E/0x40/0x60/0x61 checksum); `bq76952_init()` (CRC kipi yalnızca
  Battery Status'u CRC'li okuyarak algılanır, CONFIG_UPDATE içinde Comm Type = 0x12 ve DA Configuration:
  10 mA / 10 mV, gerekirse SWAP_COMM_MODE); ölçüm: hücre (tek giriş ya da `read_all_cells_mv`: 16 giriş tek seferde
  okunur, çağıranın verdiği giriş listesi 1..16 seçilir), stack/PACK/LD gerilimi (mV), CC2 akımı (mA, şarj +),
  iç ve TS1-3 sıcaklığı (0,1 °C). Akım grubu: `read_current_avg_ma` (CC3 ortalama akımı, DASTATUS5, mA),
  `read_passed_charge` (biriken yük, DASTATUS6 tam kısmı, mAh, 10 mAh çözünürlük; süre s),
  `reset_passed_charge` (RESET_PASSQ).

Karşılanan gereksinimler: REQ-SW-003, REQ-SW-005, REQ-SW-010, REQ-SYS-003 (Katman 1 AFE ölçümü).
