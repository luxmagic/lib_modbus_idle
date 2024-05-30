/*
 *  Nedomolkin KO-35
 *  NIITM
 *  28.01.2024
 */

#ifndef _MB_RTU_H
#define _MB_RTU_H

#include "mb.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------- Defines -----------------------------------*/
#define MB_SER_PDU_SIZE_MIN_RTU 4 /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX 256   /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC 2     /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF 0     /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF 1      /*!< Offset of Modbus-PDU in Ser-PDU. */

    /*----------------------- Prototypes ---------------------------------*/
    void eMBRTUSend(void);
    eMBErrorCode eMBRTUReceive(void);

#ifdef __cplusplus
}
#endif
#endif
