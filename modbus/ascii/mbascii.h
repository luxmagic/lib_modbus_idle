/*
 *  Nedomolkin KO-35
 *  NIITM
 *  19.01.2024
 */

#ifndef _MB_ASCII_H
#define _MB_ASCII_H

#include "mb.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MB_ASCII_DEFAULT_CR '\r'    /*!< Default CR character for Modbus ASCII. */
#define MB_ASCII_DEFAULT_LF '\n'    /*!< Default LF character for Modbus ASCII. */
#define MB_ASCII_DEFAULT_ST ':'     /*!< Default : character for Modbus ASCII. */
#define MB_SER_PDU_SIZE_MIN_ASCII 3 /*!< Minimum size of a Modbus ASCII frame. */
#define MB_SER_PDU_SIZE_MAX 256     /*!< Maximum size of a Modbus ASCII frame. */
#define MB_SER_PDU_SIZE_LRC 1       /*!< Size of LRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF 0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF 1        /*!< Offset of Modbus-PDU in Ser-PDU. */

    /* ----------------------- Prototypes ---------------------------------*/
    void eMBASCIISend(void);
    void xMBASCIITransmitFSM(void);

    eMBErrorCode eMBASCIIReceive(void);
    void xMBASCIIReceiveFSM(void);

#ifdef __cplusplus
}
#endif
#endif
