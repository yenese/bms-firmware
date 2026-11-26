/** @file bms_status.h
 *
 * @brief Bütün fonksiyonların döndürdüğü ortak hata tipi.
 *
 * Değerlerin sayıları sabittir; kayıtta ve telemetride bir kod her zaman
 * aynı anlama gelir. Çağıran taraf dönüş değerini her zaman kontrol eder.
 */

#ifndef BMS_STATUS_H
#define BMS_STATUS_H

typedef enum
{
    BMS_OK          = 0,
    BMS_ERR_PARAM   = 1,
    BMS_ERR_TIMEOUT = 2,
    BMS_ERR_CRC     = 3,
    BMS_ERR_COMM    = 4,
    BMS_ERR_RANGE   = 5,
    BMS_ERR_BUSY    = 6,
    BMS_ERR_STATE   = 7
} bms_status_t;

#endif /* BMS_STATUS_H */
