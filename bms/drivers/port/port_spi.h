/** @file port_spi.h
 *
 * @brief SPI erişimi: CS yönetimli, bloklayan, zaman aşımlı komut + veri aktarımı.
 */

#ifndef PORT_SPI_H
#define PORT_SPI_H

#include <stdint.h>

#include "bms_status.h"
#include "board.h"

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
    uint32_t           timeout_ms;
} port_spi_dev_t;

bms_status_t port_spi_write(const port_spi_dev_t *self, const uint8_t *cmd, uint16_t cmd_len,
                            const uint8_t *data, uint16_t len);
bms_status_t port_spi_read(const port_spi_dev_t *self, const uint8_t *cmd, uint16_t cmd_len,
                           uint8_t *rx_buf, uint16_t len);

#endif /* PORT_SPI_H */
