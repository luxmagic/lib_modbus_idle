/*
 *  Nedomolkin KO-35
 *  NIITM
 *  26.01.2024
 */

#ifndef _MB_H
#define _MB_H

#include "main.h"

/*---------------------MB other functions---------------------*/
#include "mbcrc.h"
/*------------------------------------------------------------*/

/*--------------------MB types and defines--------------------*/
#include "mbframe.h"
#include "mbfunc.h"
#include "mbtypes.h"
/*------------------------------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------- Defines ------------------------------------------*/

/*! \ingroup modbus
 * \brief Use the default Modbus TCP port (502)
 */
#define MB_TCP_PORT_USE_DEFAULT 0

    extern u8 rxbuff[256];
    extern uchar txbuff[256];
    extern u8 buffcmp[256];
    extern uchar *ptrbuff;

    extern volatile u16 framelen; // for rxbuff
    extern volatile u16 datalen;  // for txbuff

    extern volatile u16 numreg;
    extern volatile u16 crcval;

    extern volatile u8 slvid;
    extern volatile u8 func;

    /* ----------------------- Type definitions ---------------------------------*/
    typedef enum
    {
        MB_PAR_NONE, /*!< No parity. */
        MB_PAR_ODD,  /*!< Odd parity. */
        MB_PAR_EVEN  /*!< Even parity. */
    } eMBParity;

    typedef enum
    {
        MB_RTU,   /*!< RTU transmission mode. */
        MB_ASCII, /*!< ASCII transmission mode. */
        MB_TCP    /*!< TCP mode. */
    } eMBMode;

    extern eMBMode eMBCurrentMode;
    extern uchar ucMBAddress;
    extern volatile u16 MB_SER_PDU_SIZE_MIN;

    typedef enum
    {
        MB_REG_READ, /*!< Read register values and pass to protocol stack. */
        MB_REG_WRITE /*!< Update register values. */
    } eMBRegisterMode;

    typedef enum
    {
        MB_ENOERR,    /*!< no error. */
        MB_ENOREG,    /*!< illegal register address. */
        MB_EINVAL,    /*!< illegal argument. */
        MB_EPORTERR,  /*!< porting layer error. */
        MB_ENORES,    /*!< insufficient resources. */
        MB_EIO,       /*!< I/O error. */
        MB_EILLSTATE, /*!< protocol stack in illegal state. */
        MB_ETIMEDOUT  /*!< timeout error occurred. */
    } eMBErrorCode;

    extern volatile eMBErrorCode errflag;
    extern volatile eMBEventType stsflag;
    extern volatile eMBException eException;

    /* ----------------------- Function prototypes ------------------------------*/
    // eMBErrorCode    eMBTCPInit( ushort usTCPPort );

    void mbpoll(void);

    void clearbuff(void);

    void perifswch(bool txstate, bool rxstate);

    void modeswch(void);

    eMBErrorCode eMBSetSlaveID(uchar ucSlaveID, bool xIsRunning, uchar const *pucAdditional, ushort usAdditionalLen);

    eMBErrorCode eMBRegisterCB(uchar ucFunctionCode, pxMBFunctionHandler pxHandler);

    /* ----------------------- Callback -----------------------------------------*/
    /*---------------Input Callback---------------*/
    eMBErrorCode eMBRegInputCB(uchar *pucRegBuffer, ushort usAddress, ushort usNRegs);

    /*---------------Holding Callback---------------*/
    eMBErrorCode eMBRegHoldingCB(uchar *pucRegBuffer, ushort usAddress, ushort usNRegs, eMBRegisterMode eMode);

    /*---------------Exception Callback---------------*/
    eMBException errorCodeException(eMBErrorCode eErrorCode);

#ifdef __cplusplus
}
#endif
#endif
