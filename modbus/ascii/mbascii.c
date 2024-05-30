/*
 *  Nedomolkin KO-35
 *  NIITM
 *  30.01.2024
 */

#include "mbascii.h"
#include "mb.h"

/*----------------------- Prototypes ---------------------------------*/
static uchar prvucMBLRC(uchar *pucFrame, ushort usLen);
static uchar prvucMBCHAR2BIN(uchar ucCharacter);
static uchar prvucMBBIN2CHAR(uchar ucByte);
/*--------------------------------------------------------------------*/

/*-------------------------- Defines ---------------------------------*/
static u8 ucASCIIBuf[MB_SER_PDU_SIZE_MAX] = {
    0,
};
static u8 ucASCIIBufWR[MB_SER_PDU_SIZE_MAX] = {
    0,
};
static u8 frLen = 0; // ������ �������� ������ � ASCII
static u8 dtLen = 0; // ������ ��������� ������ � HEX
/*--------------------------------------------------------------------*/

void eMBASCIISend(void)
{
    uchar usLRC;

    if (func == MB_FUNC_WRITE_REGISTER || func == MB_FUNC_WRITE_MULTIPLE_REGISTERS ||
        func == MB_FUNC_READWRITE_MULTIPLE_REGISTERS)
    {
        memset(txbuff, 0, frLen);
        memcpy(txbuff, ucASCIIBuf, framelen);
        datalen = framelen;
        xMBASCIITransmitFSM();
    }
    else
    {
        usLRC = prvucMBLRC((uchar *)txbuff, datalen);
        txbuff[datalen++] = usLRC;
        xMBASCIITransmitFSM();
    }
}

void xMBASCIITransmitFSM(void)
{
    /*����� ��������� HEX ������ � ������ ��(1 ����)*/
    dtLen = datalen;

    /*3 - ��-�� ����� �������� ':', '\r', '\n' */
    datalen = datalen * 2 + 2;

    u8 count = 1;

    /*��������� ������� ����� �������� */
    memset(ucASCIIBuf, 0, framelen);

    /*���� ���������� HEX ����� �� ��������� � ������������� �� ��� ��������������� ������ ��� �������� �� ASCII*/
    for (u8 i = 0; i < dtLen; i++)
    {
        ucASCIIBuf[count++] = (txbuff[i] >> 4) & 0x0f;
        ucASCIIBuf[count++] = txbuff[i] & 0x0f;
    }

    /*��������� HEX ������ �.�. ������ ������ ��� ��������*/
    memset(txbuff, 0, dtLen);

    /*���� �������� ������ �� HEX � ASCII ������*/
    for (u8 i = 1; i < datalen; i++)
    {
        txbuff[i] = prvucMBBIN2CHAR(ucASCIIBuf[i]);
    }

    /*���������� �������� ������*/
    txbuff[0] = MB_ASCII_DEFAULT_ST;
    txbuff[datalen - 1] = MB_ASCII_DEFAULT_CR;
    txbuff[datalen] = MB_ASCII_DEFAULT_LF;

    /*���� ��������� ��� ������ ���������*/
    datalen += 1;
}

eMBErrorCode eMBASCIIReceive(void)
{
    eMBErrorCode errorStatus = MB_ENOERR;
    frLen = framelen;

    xMBASCIIReceiveFSM();

    memset(rxbuff, 0, frLen);
    memcpy(rxbuff, ucASCIIBuf, framelen);

    if ((framelen >= MB_SER_PDU_SIZE_MIN) && (prvucMBLRC((uchar *)rxbuff, framelen) == 0))
    {
        slvid = rxbuff[MB_SER_PDU_ADDR_OFF];
        datalen = (u16)(framelen - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_LRC);
        errorStatus = MB_ENOERR;
    }
    else
    {
        errorStatus = MB_EIO;
        clearbuff();
    }
    return errorStatus;
}

void xMBASCIIReceiveFSM(void)
{
    u8 count = 0;
    u8 LRC = 0;

    /*����� �������� '\r' � '\n' �.�. ����� �� �����*/
    framelen -= 2;

    /*���� �������� �������� �� ASCII � HEX (� 1 �.�. �� ����������� ������ ':')*/
    for (u8 i = 1; i < framelen; i++)
    {
        ucASCIIBuf[i - 1] = prvucMBCHAR2BIN(rxbuff[i]);
    }
    /*����� �� �.�. ����� ��������*/
    framelen -= 2;

    /*���� ������������ �������� ������*/
    for (u8 i = 0; i < framelen; i++)
    {
        if (i % 2 != 0)
            ucASCIIBuf[count++] = ucASCIIBuf[i];
    }
    /*���������� �������� �� � ���� ����*/
    LRC |= ucASCIIBuf[framelen - 1] << 4;
    LRC |= ucASCIIBuf[framelen];
    ucASCIIBuf[count++] = LRC;

    /*���������� ����� ������ �.�. ���������� �� ������ �����*/
    framelen /= 2;

    /*���� �� �.�. �� ����� �� ��������*/
    framelen += 1;
}

static uchar prvucMBCHAR2BIN(uchar ucCharacter)
{
    if ((ucCharacter >= '0') && (ucCharacter <= '9'))
    {
        return (uchar)(ucCharacter - '0');
    }
    else if ((ucCharacter >= 'A') && (ucCharacter <= 'F'))
    {
        return (uchar)(ucCharacter - 'A' + 0x0A);
    }
    else
    {
        return 0xFF;
    }
}

static uchar prvucMBBIN2CHAR(uchar ucByte)
{
    if (ucByte <= 0x09)
    {
        return (uchar)('0' + ucByte);
    }
    else if ((ucByte >= 0x0A) && (ucByte <= 0x0F))
    {
        return (uchar)(ucByte - 0x0A + 'A');
    }
    else
    {
        /* Programming error. */
        assert(0);
    }
    return '0';
}

uchar prvucMBLRC(uchar *pucFrame, u16 usLen)
{
    uchar ucLRC = 0; /* LRC char initialized */

    while (usLen--)
    {
        ucLRC += *pucFrame++; /* Add buffer byte without carry */
    }

    /* Return twos complement */
    ucLRC = (uchar)(-((char)ucLRC));
    return ucLRC;
}
