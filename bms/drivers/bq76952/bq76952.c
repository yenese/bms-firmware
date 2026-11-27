/** @file bq76952.c
 *
 * @brief BQ76952 AFE sürücüsü, port_i2c üzerinden.
 */

#include <stddef.h>

#include "bq76952.h"
#include "bq76952_regs.h"

#define BQ76952_TIMEOUT_MS      (10U)
#define BQ76952_WAIT_TRIES      (50U)
#define BQ76952_CELL_COUNT      (16U)
#define BQ76952_XFER_OVERHEAD   (4U)
#define BQ76952_REG_MAX         (BQ76952_XFER_BUF_SIZE + 2U)
#define BQ76952_USERV_TO_MV     (10)
#define BQ76952_USERA_TO_MA     (10)
#define BQ76952_DECI_K_OFFSET   (2731)
#define BQ76952_USERAH_TO_MAH   (10)
#define BQ76952_DASTATUS5_LEN   (22U)
#define BQ76952_CC3_OFFSET      (20U)
#define BQ76952_DASTATUS6_LEN   (12U)
#define BQ76952_PASSQ_OFFSET    (0U)
#define BQ76952_PASSQ_T_OFFSET  (8U)

static uint8_t crc8(uint8_t init, const uint8_t *data, uint8_t len)
{
    uint8_t crc = init;
    uint8_t i;
    uint8_t bit;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if (0U != (crc & 0x80U))
            {
                crc = (uint8_t)((uint8_t)(crc << 1) ^ 0x07U);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}

static uint8_t addr_write(const bq76952_dev_t *self)
{
    return (uint8_t)(self->i2c.dev_addr_7b << 1);
}

static int32_t to_i16(const uint8_t *b)
{
    uint16_t u = (uint16_t)((uint16_t)b[0] | (uint16_t)((uint16_t)b[1] << 8));

    if (u >= 0x8000U)
    {
        return (int32_t)u - 65536;
    }

    return (int32_t)u;
}

static uint32_t to_u32(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static int32_t to_i32(const uint8_t *b)
{
    uint32_t u = to_u32(b);

    if (u <= (uint32_t)INT32_MAX)
    {
        return (int32_t)u;
    }

    return (int32_t)(u - 0x80000000UL) + INT32_MIN;
}

static uint8_t xfer_checksum(uint16_t subcmd, const uint8_t *data, uint8_t len)
{
    uint8_t sum = (uint8_t)((uint8_t)(subcmd & 0xFFU) + (uint8_t)(subcmd >> 8));
    uint8_t i;

    for (i = 0U; i < len; i++)
    {
        sum = (uint8_t)(sum + data[i]);
    }

    return (uint8_t)(~sum);
}

static bms_status_t reg_write(const bq76952_dev_t *self, uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t raw[2U * BQ76952_REG_MAX];
    uint8_t head[2];
    uint8_t crc;
    uint8_t i;

    if ((NULL == self) || (NULL == data) || (0U == len) || (len > BQ76952_REG_MAX))
    {
        return BMS_ERR_PARAM;
    }

    if (!self->crc_on)
    {
        return port_i2c_write_reg(&self->i2c, reg, data, len);
    }

    head[0] = addr_write(self);
    head[1] = reg;
    crc = crc8(0U, head, 2U);

    for (i = 0U; i < len; i++)
    {
        crc = crc8(crc, &data[i], 1U);
        raw[2U * i] = data[i];
        raw[(2U * i) + 1U] = crc;
        crc = 0U;
    }

    return port_i2c_write_reg(&self->i2c, reg, raw, (uint16_t)(2U * len));
}

static bms_status_t wait_echo(const bq76952_dev_t *self, uint16_t subcmd)
{
    uint8_t echo[2];
    uint8_t i;
    bms_status_t st;

    for (i = 0U; i < BQ76952_WAIT_TRIES; i++)
    {
        st = bq76952_cmd_read(self, BQ76952_CMD_SUBCMD, echo, 2U);
        if (BMS_OK != st)
        {
            return st;
        }
        if ((echo[0] == (uint8_t)(subcmd & 0xFFU)) && (echo[1] == (uint8_t)(subcmd >> 8)))
        {
            return BMS_OK;
        }
    }

    return BMS_ERR_TIMEOUT;
}

static bms_status_t read_cfgupdate(const bq76952_dev_t *self, bool *on)
{
    uint8_t status;
    bms_status_t st;

    st = bq76952_cmd_read(self, BQ76952_CMD_BATTERY_STATUS, &status, 1U);
    if (BMS_OK != st)
    {
        return st;
    }

    *on = (0U != (status & BQ76952_BATTERY_STATUS_CFGUPDATE));
    return BMS_OK;
}

static bms_status_t wait_cfgupdate(const bq76952_dev_t *self, bool want)
{
    bool on = !want;
    uint8_t i;
    bms_status_t st;

    for (i = 0U; i < BQ76952_WAIT_TRIES; i++)
    {
        st = read_cfgupdate(self, &on);
        if (BMS_OK != st)
        {
            return st;
        }
        if (on == want)
        {
            return BMS_OK;
        }
    }

    return BMS_ERR_TIMEOUT;
}

static bms_status_t exit_cfgupdate(const bq76952_dev_t *self)
{
    bms_status_t st;

    st = bq76952_subcmd(self, BQ76952_SUBCMD_EXIT_CFGUPDATE);
    if (BMS_OK != st)
    {
        return st;
    }

    return wait_cfgupdate(self, false);
}

static bms_status_t probe_crc(bq76952_dev_t *self)
{
    uint8_t status[2];
    bms_status_t st;

    self->crc_on = true;
    st = bq76952_cmd_read(self, BQ76952_CMD_BATTERY_STATUS, status, 2U);
    if (BMS_ERR_CRC == st)
    {
        self->crc_on = false;
    }

    return st;
}

static bms_status_t check_device(const bq76952_dev_t *self)
{
    uint16_t id = 0U;
    bms_status_t st;

    st = bq76952_read_device_number(self, &id);
    if (BMS_OK != st)
    {
        return st;
    }
    if (BQ76952_DEVICE_NUMBER_ID != id)
    {
        return BMS_ERR_STATE;
    }

    return BMS_OK;
}

static bms_status_t detect_mode(bq76952_dev_t *self)
{
    bms_status_t st;

    st = probe_crc(self);
    if ((BMS_OK != st) && (BMS_ERR_CRC != st))
    {
        return st;
    }

    return check_device(self);
}

static bms_status_t wait_crc_mode(bq76952_dev_t *self)
{
    uint8_t i;

    for (i = 0U; i < BQ76952_WAIT_TRIES; i++)
    {
        if (BMS_OK == probe_crc(self))
        {
            return BMS_OK;
        }
    }

    return BMS_ERR_COMM;
}

static bms_status_t read_userv_mv(const bq76952_dev_t *self, uint8_t cmd, bms_pack_mv_t *mv)
{
    uint8_t raw[2];
    int32_t v;
    bms_status_t st;

    if (NULL == mv)
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_cmd_read(self, cmd, raw, 2U);
    if (BMS_OK != st)
    {
        return st;
    }

    v = to_i16(raw);
    if (v < 0)
    {
        return BMS_ERR_RANGE;
    }
    v *= BQ76952_USERV_TO_MV;
    if (v > (int32_t)UINT16_MAX)
    {
        return BMS_ERR_RANGE;
    }

    *mv = (bms_pack_mv_t)v;
    return BMS_OK;
}

bms_status_t bq76952_cmd_read(const bq76952_dev_t *self, uint8_t cmd, uint8_t *buf, uint8_t len)
{
    uint8_t raw[2U * BQ76952_XFER_BUF_SIZE];
    uint8_t head[3];
    uint8_t crc;
    uint8_t i;
    bms_status_t st;

    if ((NULL == self) || (NULL == buf) || (0U == len) || (len > BQ76952_XFER_BUF_SIZE))
    {
        return BMS_ERR_PARAM;
    }

    if (!self->crc_on)
    {
        return port_i2c_read_reg(&self->i2c, cmd, buf, len);
    }

    st = port_i2c_read_reg(&self->i2c, cmd, raw, (uint16_t)(2U * len));
    if (BMS_OK != st)
    {
        return st;
    }

    head[0] = addr_write(self);
    head[1] = cmd;
    head[2] = (uint8_t)(head[0] | 0x01U);
    crc = crc8(0U, head, 3U);

    for (i = 0U; i < len; i++)
    {
        crc = crc8(crc, &raw[2U * i], 1U);
        if (crc != raw[(2U * i) + 1U])
        {
            return BMS_ERR_CRC;
        }
        crc = 0U;
    }

    for (i = 0U; i < len; i++)
    {
        buf[i] = raw[2U * i];
    }

    return BMS_OK;
}

bms_status_t bq76952_cmd_write(const bq76952_dev_t *self, uint8_t cmd, const uint8_t *data, uint8_t len)
{
    if (len > BQ76952_XFER_BUF_SIZE)
    {
        return BMS_ERR_PARAM;
    }

    return reg_write(self, cmd, data, len);
}

bms_status_t bq76952_subcmd(const bq76952_dev_t *self, uint16_t subcmd)
{
    uint8_t tx[2];

    tx[0] = (uint8_t)(subcmd & 0xFFU);
    tx[1] = (uint8_t)(subcmd >> 8);

    return reg_write(self, BQ76952_CMD_SUBCMD, tx, 2U);
}

bms_status_t bq76952_subcmd_read(const bq76952_dev_t *self, uint16_t subcmd, uint8_t *buf, uint8_t len)
{
    uint8_t blk[BQ76952_XFER_BUF_SIZE];
    uint8_t xfer_len;
    uint8_t blk_len;
    uint8_t checksum;
    uint8_t i;
    bms_status_t st;

    if ((NULL == self) || (NULL == buf) || (0U == len) || (len > BQ76952_XFER_BUF_SIZE))
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_subcmd(self, subcmd);
    if (BMS_OK != st)
    {
        return st;
    }

    st = wait_echo(self, subcmd);
    if (BMS_OK != st)
    {
        return st;
    }

    st = bq76952_cmd_read(self, BQ76952_CMD_XFER_LEN, &xfer_len, 1U);
    if (BMS_OK != st)
    {
        return st;
    }
    if (xfer_len > (BQ76952_XFER_BUF_SIZE + BQ76952_XFER_OVERHEAD))
    {
        return BMS_ERR_COMM;
    }
    if (xfer_len < (len + BQ76952_XFER_OVERHEAD))
    {
        return BMS_ERR_PARAM;
    }
    blk_len = (uint8_t)(xfer_len - BQ76952_XFER_OVERHEAD);

    st = bq76952_cmd_read(self, BQ76952_CMD_XFER_BUF, blk, blk_len);
    if (BMS_OK != st)
    {
        return st;
    }

    st = bq76952_cmd_read(self, BQ76952_CMD_XFER_CHECKSUM, &checksum, 1U);
    if (BMS_OK != st)
    {
        return st;
    }
    if (xfer_checksum(subcmd, blk, blk_len) != checksum)
    {
        return BMS_ERR_CRC;
    }

    for (i = 0U; i < len; i++)
    {
        buf[i] = blk[i];
    }

    return BMS_OK;
}

bms_status_t bq76952_subcmd_write(const bq76952_dev_t *self, uint16_t subcmd, const uint8_t *data, uint8_t len)
{
    uint8_t tx[BQ76952_REG_MAX];
    uint8_t tail[2];
    uint8_t i;
    bms_status_t st;

    if ((NULL == self) || (NULL == data) || (0U == len) || (len > BQ76952_XFER_BUF_SIZE))
    {
        return BMS_ERR_PARAM;
    }

    tx[0] = (uint8_t)(subcmd & 0xFFU);
    tx[1] = (uint8_t)(subcmd >> 8);
    for (i = 0U; i < len; i++)
    {
        tx[2U + i] = data[i];
    }

    st = reg_write(self, BQ76952_CMD_SUBCMD, tx, (uint8_t)(len + 2U));
    if (BMS_OK != st)
    {
        return st;
    }

    tail[0] = xfer_checksum(subcmd, data, len);
    tail[1] = (uint8_t)(len + BQ76952_XFER_OVERHEAD);

    return reg_write(self, BQ76952_CMD_XFER_CHECKSUM, tail, 2U);
}

bms_status_t bq76952_read_device_number(const bq76952_dev_t *self, uint16_t *id)
{
    uint8_t raw[2];
    bms_status_t st;

    if (NULL == id)
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_subcmd_read(self, BQ76952_SUBCMD_DEVICE_NUMBER, raw, 2U);
    if (BMS_OK != st)
    {
        return st;
    }

    *id = (uint16_t)((uint16_t)raw[0] | (uint16_t)((uint16_t)raw[1] << 8));
    return BMS_OK;
}

bms_status_t bq76952_init(bq76952_dev_t *self)
{
    uint8_t value;
    bool in_cfgupdate = false;
    bms_status_t st;

    if (NULL == self)
    {
        return BMS_ERR_PARAM;
    }

    self->i2c.hi2c = PORT_I2C_BUS_AFE_HANDLE;
    self->i2c.dev_addr_7b = BQ76952_I2C_ADDR_7B;
    self->i2c.timeout_ms = BQ76952_TIMEOUT_MS;
    self->crc_on = false;

    st = detect_mode(self);
    if (BMS_OK != st)
    {
        return st;
    }

    st = read_cfgupdate(self, &in_cfgupdate);
    if (BMS_OK != st)
    {
        return st;
    }
    if (in_cfgupdate)
    {
        st = exit_cfgupdate(self);
        if (BMS_OK != st)
        {
            return st;
        }
        st = detect_mode(self);
        if (BMS_OK != st)
        {
            return st;
        }
    }

    st = bq76952_subcmd(self, BQ76952_SUBCMD_SET_CFGUPDATE);
    if (BMS_OK != st)
    {
        return st;
    }
    st = wait_cfgupdate(self, true);
    if (BMS_OK != st)
    {
        return st;
    }

    value = BQ76952_COMM_TYPE_I2C_FAST_CRC;
    st = bq76952_subcmd_write(self, BQ76952_DM_COMM_TYPE, &value, 1U);
    if (BMS_OK != st)
    {
        return st;
    }

    st = bq76952_subcmd_read(self, BQ76952_DM_DA_CONFIG, &value, 1U);
    if (BMS_OK != st)
    {
        return st;
    }
    value = (uint8_t)((value & (uint8_t)(~BQ76952_DA_USER_AMPS_MASK)) | BQ76952_DA_USER_AMPS_10MA
                      | BQ76952_DA_USER_VOLTS_CV);
    st = bq76952_subcmd_write(self, BQ76952_DM_DA_CONFIG, &value, 1U);
    if (BMS_OK != st)
    {
        return st;
    }

    st = exit_cfgupdate(self);
    if (BMS_OK != st)
    {
        return st;
    }

    st = wait_crc_mode(self);
    if (BMS_OK != st)
    {
        self->crc_on = false;
        st = bq76952_subcmd(self, BQ76952_SUBCMD_SWAP_COMM_MODE);
        if (BMS_OK != st)
        {
            return st;
        }
        st = wait_crc_mode(self);
        if (BMS_OK != st)
        {
            return st;
        }
    }

    if (BMS_OK != check_device(self))
    {
        return BMS_ERR_COMM;
    }

    return BMS_OK;
}

bms_status_t bq76952_read_cell_mv(const bq76952_dev_t *self, uint8_t cell, bms_cell_mv_t *mv)
{
    uint8_t raw[2];
    int32_t v;
    bms_status_t st;

    if ((NULL == mv) || (cell < 1U) || (cell > BQ76952_CELL_COUNT))
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_cmd_read(self, (uint8_t)(BQ76952_CMD_CELL1_VOLTAGE + (2U * (cell - 1U))), raw, 2U);
    if (BMS_OK != st)
    {
        return st;
    }

    v = to_i16(raw);
    if (v < 0)
    {
        return BMS_ERR_RANGE;
    }

    *mv = (bms_cell_mv_t)v;
    return BMS_OK;
}

bms_status_t bq76952_read_all_cells_mv(const bq76952_dev_t *self, const uint8_t *inputs, bms_cell_mv_t *mv,
                                       uint8_t count)
{
    uint8_t raw[2U * BQ76952_CELL_COUNT];
    int32_t v;
    uint8_t i;
    bms_status_t st;

    if ((NULL == inputs) || (NULL == mv) || (0U == count) || (count > BQ76952_CELL_COUNT))
    {
        return BMS_ERR_PARAM;
    }

    for (i = 0U; i < count; i++)
    {
        if ((inputs[i] < 1U) || (inputs[i] > BQ76952_CELL_COUNT))
        {
            return BMS_ERR_PARAM;
        }
    }

    st = bq76952_cmd_read(self, BQ76952_CMD_CELL1_VOLTAGE, raw, (uint8_t)(2U * BQ76952_CELL_COUNT));
    if (BMS_OK != st)
    {
        return st;
    }

    for (i = 0U; i < count; i++)
    {
        v = to_i16(&raw[2U * (inputs[i] - 1U)]);
        if (v < 0)
        {
            return BMS_ERR_RANGE;
        }
        mv[i] = (bms_cell_mv_t)v;
    }

    return BMS_OK;
}

bms_status_t bq76952_read_stack_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv)
{
    return read_userv_mv(self, BQ76952_CMD_STACK_VOLTAGE, mv);
}

bms_status_t bq76952_read_pack_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv)
{
    return read_userv_mv(self, BQ76952_CMD_PACK_VOLTAGE, mv);
}

bms_status_t bq76952_read_ld_mv(const bq76952_dev_t *self, bms_pack_mv_t *mv)
{
    return read_userv_mv(self, BQ76952_CMD_LD_VOLTAGE, mv);
}

bms_status_t bq76952_read_current_ma(const bq76952_dev_t *self, bms_ma_t *ma)
{
    uint8_t raw[2];
    bms_status_t st;

    if (NULL == ma)
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_cmd_read(self, BQ76952_CMD_CC2_CURRENT, raw, 2U);
    if (BMS_OK != st)
    {
        return st;
    }

    *ma = (bms_ma_t)(to_i16(raw) * BQ76952_USERA_TO_MA);
    return BMS_OK;
}

bms_status_t bq76952_read_current_avg_ma(const bq76952_dev_t *self, bms_ma_t *ma)
{
    uint8_t raw[BQ76952_DASTATUS5_LEN];
    bms_status_t st;

    if (NULL == ma)
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_subcmd_read(self, BQ76952_SUBCMD_DASTATUS5, raw, BQ76952_DASTATUS5_LEN);
    if (BMS_OK != st)
    {
        return st;
    }

    *ma = (bms_ma_t)(to_i16(&raw[BQ76952_CC3_OFFSET]) * BQ76952_USERA_TO_MA);
    return BMS_OK;
}

bms_status_t bq76952_read_passed_charge(const bq76952_dev_t *self, int32_t *charge_mah, uint32_t *time_s)
{
    uint8_t raw[BQ76952_DASTATUS6_LEN];
    int32_t q;
    bms_status_t st;

    if ((NULL == charge_mah) || (NULL == time_s))
    {
        return BMS_ERR_PARAM;
    }

    st = bq76952_subcmd_read(self, BQ76952_SUBCMD_DASTATUS6, raw, BQ76952_DASTATUS6_LEN);
    if (BMS_OK != st)
    {
        return st;
    }

    q = to_i32(&raw[BQ76952_PASSQ_OFFSET]);
    if ((q > (INT32_MAX / BQ76952_USERAH_TO_MAH)) || (q < (INT32_MIN / BQ76952_USERAH_TO_MAH)))
    {
        return BMS_ERR_RANGE;
    }

    *charge_mah = q * BQ76952_USERAH_TO_MAH;
    *time_s = to_u32(&raw[BQ76952_PASSQ_T_OFFSET]);
    return BMS_OK;
}

bms_status_t bq76952_reset_passed_charge(const bq76952_dev_t *self)
{
    return bq76952_subcmd(self, BQ76952_SUBCMD_RESET_PASSQ);
}

bms_status_t bq76952_read_temp(const bq76952_dev_t *self, bq76952_temp_t sensor, bms_deci_degc_t *t)
{
    uint8_t raw[2];
    uint8_t cmd;
    int32_t v;
    bms_status_t st;

    if (NULL == t)
    {
        return BMS_ERR_PARAM;
    }

    switch (sensor)
    {
    case BQ76952_TEMP_INT: cmd = BQ76952_CMD_INT_TEMP; break;
    case BQ76952_TEMP_TS1: cmd = BQ76952_CMD_TS1_TEMP; break;
    case BQ76952_TEMP_TS2: cmd = BQ76952_CMD_TS2_TEMP; break;
    case BQ76952_TEMP_TS3: cmd = BQ76952_CMD_TS3_TEMP; break;
    default:               return BMS_ERR_PARAM;
    }

    st = bq76952_cmd_read(self, cmd, raw, 2U);
    if (BMS_OK != st)
    {
        return st;
    }

    v = to_i16(raw) - BQ76952_DECI_K_OFFSET;
    if (v < (int32_t)INT16_MIN)
    {
        return BMS_ERR_RANGE;
    }

    *t = (bms_deci_degc_t)v;
    return BMS_OK;
}
