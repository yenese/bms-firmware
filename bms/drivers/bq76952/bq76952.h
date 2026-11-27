/** @file bq76952.h
 *
 * @brief BQ76952 AFE sürücüsü: I2C + CRC haberleşme, subcommand, ölçüm.
 *
 * Gerilimler mV, akım mA (şarj +), sıcaklık 0,1 °C. Hücre girişi 1..16; blok okumada
 * okunacak girişler çağıranın verdiği listeyle seçilir.
 */

#ifndef BQ76952_H
#define BQ76952_H

#include <stdbool.h>
#include <stdint.h>

#include "bms_status.h"
#include "bms_types.h"
#include "port_i2c.h"

typedef struct
{
    port_i2c_dev_t i2c;
    bool           crc_on;
} bq76952_dev_t;

typedef enum
{
    BQ76952_TEMP_INT = 0,
    BQ76952_TEMP_TS1 = 1,
    BQ76952_TEMP_TS2 = 2,
    BQ76952_TEMP_TS3 = 3
} bq76952_temp_t;

bms_status_t bq76952_init(bq76952_dev_t *self);
bms_status_t bq76952_read_device_number(const bq76952_dev_t *self, uint16_t *id);

bms_status_t bq76952_cmd_read(const bq76952_dev_t *self, uint8_t cmd, uint8_t *buf, uint8_t len);
bms_status_t bq76952_cmd_write(const bq76952_dev_t *self, uint8_t cmd, const uint8_t *data, uint8_t len);
bms_status_t bq76952_subcmd(const bq76952_dev_t *self, uint16_t subcmd);
bms_status_t bq76952_subcmd_read(const bq76952_dev_t *self, uint16_t subcmd, uint8_t *buf, uint8_t len);
bms_status_t bq76952_subcmd_write(const bq76952_dev_t *self, uint16_t subcmd, const uint8_t *data, uint8_t len);

bms_status_t bq76952_read_cell_mv(const bq76952_dev_t *self, uint8_t cell, bms_cell_mv_t *mv);
bms_status_t bq76952_read_all_cells_mv(const bq76952_dev_t *self, const uint8_t *inputs, bms_cell_mv_t *mv,
                                       uint8_t count);
bms_status_t bq76952_read_stack_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv);
bms_status_t bq76952_read_pack_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv);
bms_status_t bq76952_read_ld_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv);
bms_status_t bq76952_read_current_ma(const bq76952_dev_t *self, bms_ma_t *ma);
bms_status_t bq76952_read_current_avg_ma(const bq76952_dev_t *self, bms_ma_t *ma);
bms_status_t bq76952_read_passed_charge(const bq76952_dev_t *self, int32_t *charge_mah, uint32_t *time_s);
bms_status_t bq76952_reset_passed_charge(const bq76952_dev_t *self);
bms_status_t bq76952_read_temp(const bq76952_dev_t *self, bq76952_temp_t sensor, bms_deci_degc_t *t);

#endif /* BQ76952_H */
