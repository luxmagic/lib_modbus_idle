/*
 *  Nedomolkin KO-35
 *  NIITM
 *  26.01.2024
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Static variables ---------------------------------*/
#if MB_RTU_ENABLED == 1
#include "mbrtu.h"
#endif
#if MB_ASCII_ENABLED == 1
#include "mbascii.h"
#endif
#if MB_TCP_ENABLED == 1
#include "mbtcp.h"
#endif

volatile u16 MB_SER_PDU_SIZE_MIN = 0;

eMBErrorCode volatile errflag = MB_ENOERR;
eMBEventType volatile stsflag = EV_READY;
eMBException volatile eException = MB_EX_NONE;

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

u8 rxbuff[MB_SER_PDU_SIZE_MAX] = {
    0,
};
uchar txbuff[MB_SER_PDU_SIZE_MAX] = {
    0,
};
u8 buffcmp[MB_SER_PDU_SIZE_MAX] = {
    0,
};
uchar *ptrbuff = &txbuff[0];

u16 volatile framelen = 0; // for rxbuff
u16 volatile datalen = 0;  // for txbuff

u16 volatile numreg = 0;
u16 volatile crcval = 0;

u8 volatile slvid = 0;
u8 volatile func = 0;

uchar ucMBAddress = 1;

static xMBFunctionHandler xFuncHandlers[12] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBFuncReadInputRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBFuncReadHoldingRegister},
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBFuncWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBFuncWriteHoldingRegister},
#endif
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBFuncReadWriteMultipleHoldingRegister},
#endif
};

/* ----------------------- Start implementation -----------------------------*/

#if MB_TCP_ENABLED > 0
eMBErrorCode eMBTCPInit(USHORT ucTCPPort)
{
    eMBErrorCode eStatus = MB_ENOERR;

    if ((eStatus = eMBTCPDoInit(ucTCPPort)) != MB_ENOERR)
    {
        eMBState = STATE_DISABLED;
    }
    else if (!xMBPortEventInit())
    {
        /* Port dependent event module initalization failed. */
        eStatus = MB_EPORTERR;
    }
    else
    {
        pvMBFrameStartCur = eMBTCPStart;
        pvMBFrameStopCur = eMBTCPStop;
        peMBFrameReceiveCur = eMBTCPReceive;
        peMBFrameSendCur = eMBTCPSend;
        pvMBFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBTCPPortClose : NULL;
        ucMBAddress = MB_TCP_PSEUDO_ADDRESS;
        eMBCurrentMode = MB_TCP;
        eMBState = STATE_DISABLED;
    }
    return eStatus;
}
#endif

void mbpoll(void)
{
    switch (stsflag)
    {
    case EV_FRAME_RECEIVED:
        /* проверка пакета на валидность (адрес и контрольная сумма),
         * при успехе переход в секцию EV_EXECUTE
         */
        perifswch(false, false);
        if (eMBCurrentMode == MB_RTU)
        {
            errflag = eMBRTUReceive();
        }
        else if (eMBCurrentMode == MB_ASCII)
        {
            errflag = eMBASCIIReceive();
        }

        if (errflag == MB_ENOERR)
        {
            if ((slvid == ucMBAddress) || (slvid == MB_ADDRESS_BROADCAST))
            {
                stsflag = EV_EXECUTE;
            }
        }
        break;

    case EV_FRAME_SENT:
        /*очистка данных после отправки подготовленного ответа*/
        clearbuff();
        break;
    case EV_EXECUTE:
        /* обработка полученного пакета и подготовка ответа EV_FRAME_SENT*/
        __disable_irq();
        func = rxbuff[MB_SER_PDU_PDU_OFF];
        eException = MB_EX_ILLEGAL_FUNCTION;
        for (u8 i = 0; i < MB_FUNC_HANDLERS_MAX; i++)
        {
            if (xFuncHandlers[i].ucFunctionCode == 0)
            {
                stsflag = EV_FRAME_SENT;
                break;
            }
            else if (xFuncHandlers[i].ucFunctionCode == func)
            {
                eException = xFuncHandlers[i].pxHandler(rxbuff, &datalen, ptrbuff);
                break;
            }
        }
        if (slvid != MB_ADDRESS_BROADCAST)
        {
            if (eException != MB_EX_NONE)
            {
                datalen = 0;
                txbuff[datalen++] = (uchar)(func | MB_FUNC_ERROR);
                txbuff[datalen++] = eException;
            }
            else
            {
                if (eMBCurrentMode == MB_RTU)
                {
                    eMBRTUSend();
                }
                else if (eMBCurrentMode == MB_ASCII)
                {
                    eMBASCIISend();
                }
            }
            perifswch(true, false);
            stsflag = EV_FRAME_SENT;
        }
        __enable_irq();
        break;
    case EV_READY:
        /* начальное состояние, в котором ожидается прием пакета.
         * при получении пакета секция переходит в EV_FRAME_RECEIVED
         */
        perifswch(false, true);
        break;
    }
}

void clearbuff(void)
{
    memset(rxbuff, 0, datalen);
    memset(txbuff, 0, datalen);
    slvid = 0;
    func = 0;
    datalen = 0;
    framelen = 0;
    ptrbuff = &txbuff[0];
    errflag = MB_ENOERR;
    eException = MB_EX_NONE;
}

void perifswch(bool txstate, bool rxstate)
{
    if (!txstate && rxstate)
    {
        HAL_UART_AbortTransmit_IT(&huart2);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxbuff, 256);
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    }
    if (txstate && !rxstate)
    {
        HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)txbuff, (uint32_t)buffcmp, (uint32_t)datalen);
        HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, 1);

        HAL_UART_AbortReceive_IT(&huart2);
        HAL_UART_Transmit_DMA(&huart2, buffcmp, datalen);
    }
    if (!txstate && !rxstate)
    {
        HAL_UART_AbortReceive_IT(&huart2);
        HAL_UART_AbortTransmit_IT(&huart2);
    }
}

void modeswch(void)
{
    if (eMBCurrentMode == MB_RTU)
    {
        MB_SER_PDU_SIZE_MIN = MB_SER_PDU_SIZE_MIN_RTU;
    }
    else if (eMBCurrentMode == MB_ASCII)
    {
        MB_SER_PDU_SIZE_MIN = MB_SER_PDU_SIZE_MIN_ASCII;
    }
}

eMBException errorCodeException(eMBErrorCode eErrorCode)
{
    eMBException eStatus;

    switch (eErrorCode)
    {
    case MB_ENOERR:
        eStatus = MB_EX_NONE;
        break;

    case MB_ENOREG:
        eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
        break;

    case MB_ETIMEDOUT:
        eStatus = MB_EX_SLAVE_BUSY;
        break;

    default:
        eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
        break;
    }

    return eStatus;
}
