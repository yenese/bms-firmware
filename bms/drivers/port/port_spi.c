/** @file port_spi.c
 *
 * @brief SPI erişimi, HAL üzerinden.
 */

#include "port_spi.h"

static bms_status_t hal_to_status(HAL_StatusTypeDef hal)
{
    switch (hal)
    {
    case HAL_OK:      return BMS_OK;
    case HAL_TIMEOUT: return BMS_ERR_TIMEOUT;
    case HAL_BUSY:    return BMS_ERR_BUSY;
    default:          return BMS_ERR_COMM;
    }
}

bms_status_t port_spi_write(const port_spi_dev_t *self, const uint8_t *cmd, uint16_t cmd_len,
                            const uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef hal;

    if ((NULL == self) || (NULL == self->hspi) || (NULL == self->cs_port) || (NULL == cmd) || (0U == cmd_len))
    {
        return BMS_ERR_PARAM;
    }

    if ((len > 0U) && (NULL == data))
    {
        return BMS_ERR_PARAM;
    }

    HAL_GPIO_WritePin(self->cs_port, self->cs_pin, GPIO_PIN_RESET);

    hal = HAL_SPI_Transmit(self->hspi, (uint8_t *)cmd, cmd_len, self->timeout_ms);

    if ((HAL_OK == hal) && (len > 0U))
    {
        hal = HAL_SPI_Transmit(self->hspi, (uint8_t *)data, len, self->timeout_ms);
    }

    HAL_GPIO_WritePin(self->cs_port, self->cs_pin, GPIO_PIN_SET);

    return hal_to_status(hal);
}

bms_status_t port_spi_read(const port_spi_dev_t *self, const uint8_t *cmd, uint16_t cmd_len,
                           uint8_t *rx_buf, uint16_t len)
{
    HAL_StatusTypeDef hal;

    if ((NULL == self) || (NULL == self->hspi) || (NULL == self->cs_port) || (NULL == cmd) || (0U == cmd_len))
    {
        return BMS_ERR_PARAM;
    }

    if ((len > 0U) && (NULL == rx_buf))
    {
        return BMS_ERR_PARAM;
    }

    HAL_GPIO_WritePin(self->cs_port, self->cs_pin, GPIO_PIN_RESET);

    hal = HAL_SPI_Transmit(self->hspi, (uint8_t *)cmd, cmd_len, self->timeout_ms);

    if ((HAL_OK == hal) && (len > 0U))
    {
        hal = HAL_SPI_Receive(self->hspi, rx_buf, len, self->timeout_ms);
    }

    HAL_GPIO_WritePin(self->cs_port, self->cs_pin, GPIO_PIN_SET);

    return hal_to_status(hal);
}
