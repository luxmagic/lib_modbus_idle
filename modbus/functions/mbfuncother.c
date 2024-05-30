/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"


#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0

/* ----------------------- Static variables ---------------------------------*/
static uchar ucMBSlaveID[MB_FUNC_OTHER_REP_SLAVEID_BUF];
static ushort usMBSlaveIDLen;

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode eMBSetSlaveID(uchar ucSlaveID, bool xIsRunning, uchar const *pucAdditional, ushort usAdditionalLen)
{
    eMBErrorCode eStatus = MB_ENOERR;

    /* the first byte and second byte in the buffer is reserved for
     * the parameter ucSlaveID and the running flag. The rest of
     * the buffer is available for additional data. */
    if (usAdditionalLen + 2 < MB_FUNC_OTHER_REP_SLAVEID_BUF)
    {
        usMBSlaveIDLen = 0;
        ucMBSlaveID[usMBSlaveIDLen++] = ucSlaveID;
        ucMBSlaveID[usMBSlaveIDLen++] = (uchar)(xIsRunning ? 0xFF : 0x00);
        if (usAdditionalLen > 0)
        {
            memcpy(&ucMBSlaveID[usMBSlaveIDLen], pucAdditional, (size_t)usAdditionalLen);
            usMBSlaveIDLen += usAdditionalLen;
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }
    return eStatus;
}

eMBException eMBFuncReportSlaveID(uchar *pucFrame, ushort volatile *usLen, uchar *pucFrameCur)
{
    memcpy(&pucFrame[MB_PDU_DATA_OFF], &ucMBSlaveID[0], (size_t)usMBSlaveIDLen);
    *usLen = (ushort)(MB_PDU_DATA_OFF + usMBSlaveIDLen);
    return MB_EX_NONE;
}

#endif
