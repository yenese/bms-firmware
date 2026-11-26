/** @file port_i2c.h
 *
 * @brief I2C erişimi: bloklayan, zaman aşımlı okuma/yazma.
 *
 * CRC ve tekrar deneme yoktur; çip sürücüsü yapar.
 */

#ifndef PORT_I2C_H
#define PORT_I2C_H

#include <stdint.h>

#include "bms_status.h"
#include "board.h"

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint8_t            dev_addr_7b;
    uint32_t           timeout_ms;
} port_i2c_dev_t;

bms_status_t port_i2c_write(const port_i2c_dev_t *self, const uint8_t *tx_buf, uint16_t len);
bms_status_t port_i2c_read(const port_i2c_dev_t *self, uint8_t *rx_buf, uint16_t len);
bms_status_t port_i2c_read_reg(const port_i2c_dev_t *self, uint8_t reg, uint8_t *rx_buf, uint16_t len);
bms_status_t port_i2c_write_reg(const port_i2c_dev_t *self, uint8_t reg, const uint8_t *tx_buf, uint16_t len);

#endif /* PORT_I2C_H */
