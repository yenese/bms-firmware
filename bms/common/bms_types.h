/** @file bms_types.h
 *
 * @brief Ortak fiziksel büyüklük tipleri ve paket sabitleri.
 *
 * Bütün modüller ölçüm, kestirim ve zaman damgası değerlerini bu tiplerle
 * taşır. Birimler K-05 kararıyla sabittir (PROJE_DURUMU.md Bölüm 0):
 *
 * | Tip               | Taban tip  | Birim / aralık                          |
 * |-------------------|------------|-----------------------------------------|
 * | bms_cell_mv_t     | uint16_t   | hücre gerilimi, mV                      |
 * | bms_ma_t          | int32_t    | akım, mA; şarj (+), deşarj (-)          |
 * | bms_deci_degc_t   | int16_t    | sıcaklık, 0,1 °C                        |
 * | bms_time_ms_t     | uint32_t   | zaman damgası, açılıştan beri ms        |
 * | bms_soc_t         | float      | SoC, 0-1 (yalnızca kestirim)            |
 * | bms_soh_t         | float      | SoH, 0-1 (yalnızca kestirim)            |
 *
 */

#ifndef BMS_TYPES_H
#define BMS_TYPES_H

#include <stdint.h>

#define BMS_CELLS_IN_SERIES  (8u)

typedef uint16_t bms_cell_mv_t;
typedef int32_t bms_ma_t;

typedef int16_t bms_deci_degc_t;

typedef uint32_t bms_time_ms_t;

typedef float bms_soc_t;
typedef float bms_soh_t;

#endif /* BMS_TYPES_H */
