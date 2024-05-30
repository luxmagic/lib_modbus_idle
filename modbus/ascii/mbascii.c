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
static u8 frLen = 0; // размер входного фрейма в ASCII
static u8 dtLen = 0; // размер выходного фрейма в HEX
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
    /*длина исходного HEX фрейма с учетом КС(1 байт)*/
    dtLen = datalen;

    /*3 - из-за учета символов ':', '\r', '\n' */
    datalen = datalen * 2 + 2;

    u8 count = 1;

    /*обнуление массива после парсинга */
    memset(ucASCIIBuf, 0, framelen);

    /*цикл разделения HEX чисел на полубайты и представление их как самостоятельных байтов для передачи по ASCII*/
    for (u8 i = 0; i < dtLen; i++)
    {
        ucASCIIBuf[count++] = (txbuff[i] >> 4) & 0x0f;
        ucASCIIBuf[count++] = txbuff[i] & 0x0f;
    }

    /*обнуление HEX фрейма т.к. данные оттуда уже получены*/
    memset(txbuff, 0, dtLen);

    /*цикл перевода фрейма из HEX в ASCII формат*/
    for (u8 i = 1; i < datalen; i++)
    {
        txbuff[i] = prvucMBBIN2CHAR(ucASCIIBuf[i]);
    }

    /*добавление сигнатур фрейма*/
    txbuff[0] = MB_ASCII_DEFAULT_ST;
    txbuff[datalen - 1] = MB_ASCII_DEFAULT_CR;
    txbuff[datalen] = MB_ASCII_DEFAULT_LF;

    /*учет вычитания для записи сигнатуры*/
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

    /*вычет символов '\r' и '\n' т.к. нахуй не нужны*/
    framelen -= 2;

    /*цикл перевода значений из ASCII в HEX (с 1 т.к. не учитывается символ ':')*/
    for (u8 i = 1; i < framelen; i++)
    {
        ucASCIIBuf[i - 1] = prvucMBCHAR2BIN(rxbuff[i]);
    }
    /*вычет КС т.к. будет мешаться*/
    framelen -= 2;

    /*цикл укорачивания основных данных*/
    for (u8 i = 0; i < framelen; i++)
    {
        if (i % 2 != 0)
            ucASCIIBuf[count++] = ucASCIIBuf[i];
    }
    /*склеивание значений КС в один байт*/
    LRC |= ucASCIIBuf[framelen - 1] << 4;
    LRC |= ucASCIIBuf[framelen];
    ucASCIIBuf[count++] = LRC;

    /*уменьшение длины фрейма т.к. избавились от лишних нулей*/
    framelen /= 2;

    /*учет КС т.к. до этого ее вычитали*/
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
