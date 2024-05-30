/*
 *  Nedomolkin KO-35
 *  NIITM
 *  26.01.2024
 */

#ifndef _TYPES_H
#define _TYPES_H

#include "stdbool.h" //for bool
#include "stdint.h"
#include "string.h" //for using mem

#include <assert.h>
#include <inttypes.h>

#include "stdlib.h"
/*--------------------Variables--------------------*/
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
/*-------------------------------------------------*/

/*--------------------User types--------------------*/
typedef enum
{
    EV_READY,          /*!< Startup finished. */
    EV_FRAME_RECEIVED, /*!< Frame received. */
    EV_EXECUTE,        /*!< Execute function. */
    EV_FRAME_SENT      /*!< Frame sent. */
} eMBEventType;

/*-------------------------------------------------*/

#endif
