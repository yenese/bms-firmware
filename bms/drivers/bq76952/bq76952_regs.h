/** @file bq76952_regs.h
 *
 * @brief BQ76952 direct command, subcommand ve veri belleği adresleri (SLUUBY2B).
 */

#ifndef BQ76952_REGS_H
#define BQ76952_REGS_H

#define BQ76952_I2C_ADDR_7B                (0x08U)
#define BQ76952_XFER_BUF_SIZE              (32U)

#define BQ76952_CMD_BATTERY_STATUS         (0x12U)
#define BQ76952_CMD_CELL1_VOLTAGE          (0x14U)
#define BQ76952_CMD_STACK_VOLTAGE          (0x34U)
#define BQ76952_CMD_PACK_VOLTAGE           (0x36U)
#define BQ76952_CMD_LD_VOLTAGE             (0x38U)
#define BQ76952_CMD_CC2_CURRENT            (0x3AU)
#define BQ76952_CMD_SUBCMD                 (0x3EU)
#define BQ76952_CMD_XFER_BUF               (0x40U)
#define BQ76952_CMD_XFER_CHECKSUM          (0x60U)
#define BQ76952_CMD_XFER_LEN               (0x61U)
#define BQ76952_CMD_INT_TEMP               (0x68U)
#define BQ76952_CMD_TS1_TEMP               (0x70U)
#define BQ76952_CMD_TS2_TEMP               (0x72U)
#define BQ76952_CMD_TS3_TEMP               (0x74U)

#define BQ76952_SUBCMD_DEVICE_NUMBER       (0x0001U)
#define BQ76952_SUBCMD_DASTATUS5           (0x0075U)
#define BQ76952_SUBCMD_DASTATUS6           (0x0076U)
#define BQ76952_SUBCMD_RESET_PASSQ         (0x0082U)
#define BQ76952_SUBCMD_SET_CFGUPDATE       (0x0090U)
#define BQ76952_SUBCMD_EXIT_CFGUPDATE      (0x0092U)
#define BQ76952_SUBCMD_SWAP_COMM_MODE      (0x29BCU)

#define BQ76952_DM_COMM_TYPE               (0x9239U)
#define BQ76952_DM_DA_CONFIG               (0x9303U)

#define BQ76952_DEVICE_NUMBER_ID           (0x7695U)
#define BQ76952_BATTERY_STATUS_CFGUPDATE   (0x01U)
#define BQ76952_COMM_TYPE_I2C_FAST_CRC     (0x12U)
#define BQ76952_DA_USER_AMPS_MASK          (0x03U)
#define BQ76952_DA_USER_AMPS_10MA          (0x02U)
#define BQ76952_DA_USER_VOLTS_CV           (0x04U)

#endif /* BQ76952_REGS_H */
