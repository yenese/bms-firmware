/** @file port_i2c.c
 *
 * @brief I2C erişimi, HAL üzerinden.
 */

#include "port_i2c.h"

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

bms_status_t port_i2c_write(const port_i2c_dev_t *self, const uint8_t *tx_buf, uint16_t len)
{
    if ((NULL == self) || (NULL == self->hi2c) || (NULL == tx_buf) || (0U == len))
    {
        return BMS_ERR_PARAM;
    }

    return hal_to_status(HAL_I2C_Master_Transmit(self->hi2c,
                                                 (uint16_t)((uint16_t)self->dev_addr_7b << 1),
                                                 (uint8_t *)tx_buf, len, self->timeout_ms));
}

bms_status_t port_i2c_read(const port_i2c_dev_t *self, uint8_t *rx_buf, uint16_t len)
{
    if ((NULL == self) || (NULL == self->hi2c) || (NULL == rx_buf) || (0U == len))
    {
        return BMS_ERR_PARAM;
    }

    return hal_to_status(HAL_I2C_Master_Receive(self->hi2c,
                                                (uint16_t)((uint16_t)self->dev_addr_7b << 1),
                                                rx_buf, len, self->timeout_ms));
}

bms_status_t port_i2c_read_reg(const port_i2c_dev_t *self, uint8_t reg, uint8_t *rx_buf, uint16_t len)
{
    if ((NULL == self) || (NULL == self->hi2c) || (NULL == rx_buf) || (0U == len))
    {
        return BMS_ERR_PARAM;
    }

    return hal_to_status(HAL_I2C_Mem_Read(self->hi2c,
                                          (uint16_t)((uint16_t)self->dev_addr_7b << 1),
                                          reg, I2C_MEMADD_SIZE_8BIT, rx_buf, len, self->timeout_ms));
}

bms_status_t port_i2c_write_reg(const port_i2c_dev_t *self, uint8_t reg, const uint8_t *tx_buf, uint16_t len)
{
    if ((NULL == self) || (NULL == self->hi2c) || (NULL == tx_buf) || (0U == len))
    {
        return BMS_ERR_PARAM;
    }

    return hal_to_status(HAL_I2C_Mem_Write(self->hi2c,
                                           (uint16_t)((uint16_t)self->dev_addr_7b << 1),
                                           reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)tx_buf, len, self->timeout_ms));
}
