/** @file board.h
 *
 * @brief Mantıksal hat -> HAL tutamacı eşlemesi.
 */

#ifndef BOARD_H
#define BOARD_H

#include "i2c.h"
#include "spi.h"

#define PORT_I2C_BUS_AFE_HANDLE (&hi2c1)
#define PORT_SPI_FRAM_HANDLE    (&hspi1)

#endif /* BOARD_H */
