# drivers — Çip sürücüleri

Donanıma özel kod: BQ76952, FRAM, CAN, ADC, GPIO. Uygulama modülleri donanıma yalnızca buradan erişir.

## port/

Çip katmanının altındaki ince HAL katmanı (K-04, K-08). Çip sürücüleri HAL'i görmez, yalnızca buradaki başlıkları çağırır.

- `port_i2c.h/.c`: bloklayan, zaman aşımlı I2C. Cihaz `port_i2c_dev_t` (hat, 7 bit adres, zaman aşımı); `port_i2c_write()`, `port_i2c_read()`, `port_i2c_read_reg()`. CRC ve tekrar deneme yoktur.
- `board.h`: mantıksal hat -> HAL tutamacı (`PORT_I2C_BUS_AFE` -> `hi2c1`).

Karşılanan gereksinimler: REQ-SW-003, REQ-SW-010, REQ-SW-017.
