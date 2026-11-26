/** @file bms_status.h
 *
 * @brief Bütün fonksiyonların döndürdüğü ortak hata tipi.
 *
 * Değerlerin sayıları sabittir; kayıtta ve telemetride bir kod her zaman
 * aynı anlama gelir. Çağıran taraf dönüş değerini her zaman kontrol eder.
 */

#ifndef BMS_STATUS_H
#define BMS_STATUS_H

/** Ortak dönüş/hata kodu. */
typedef enum
{
    BMS_OK          = 0, /**< İşlem başarılı. */
    BMS_ERR_PARAM   = 1, /**< Geçersiz parametre veya NULL işaretçi. */
    BMS_ERR_TIMEOUT = 2, /**< Bekleme süresi doldu. */
    BMS_ERR_CRC     = 3, /**< CRC uyuşmadı. */
    BMS_ERR_COMM    = 4, /**< Yanıt yok veya hat hatası. */
    BMS_ERR_RANGE   = 5, /**< Değer fiziksel/makul aralığın dışında. */
    BMS_ERR_BUSY    = 6, /**< Kaynak meşgul, işlem şu an yapılamıyor. */
    BMS_ERR_STATE   = 7  /**< Bu durumda izin verilmeyen işlem. */
} bms_status_t;

#endif /* BMS_STATUS_H */

/*** end of file ***/
