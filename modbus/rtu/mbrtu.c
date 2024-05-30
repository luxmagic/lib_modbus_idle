#include "mbrtu.h"
#include "mb.h"

void eMBRTUSend(void)
{
    if (func == MB_FUNC_WRITE_REGISTER || func == MB_FUNC_WRITE_MULTIPLE_REGISTERS ||
        func == MB_FUNC_READWRITE_MULTIPLE_REGISTERS)
    {
        memset(txbuff, 0, framelen);
        memcpy(txbuff, rxbuff, framelen);
        datalen = framelen;
    }
    else
    {
        crcval = usMBCRC16((uchar *)txbuff, datalen);
        txbuff[datalen++] = (uchar)(crcval & 0xff);
        txbuff[datalen++] = (uchar)(crcval >> 8);
    }
}

eMBErrorCode eMBRTUReceive(void)
{
    eMBErrorCode errorStatus = MB_ENOERR;

    if ((framelen >= MB_SER_PDU_SIZE_MIN) && (usMBCRC16((uchar *)rxbuff, framelen) == 0))
    {
        slvid = rxbuff[MB_SER_PDU_ADDR_OFF];
        datalen = (u16)(framelen - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);
        errorStatus = MB_ENOERR;
    }
    else
    {
        errorStatus = MB_EIO;
        clearbuff();
    }

    return errorStatus;
}
