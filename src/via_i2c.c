/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_i2c.c,v 1.4 2003/12/31 05:42:05 dawes Exp $ */
/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * VIA, S3 GRAPHICS, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"

#include "xf86Pci.h"
#include "xf86PciInfo.h"

#include "vgaHW.h"

#include "via_driver.h"
#include "via_vgahw.h"

/*
 * DDC2 support requires DDC_SDA_MASK and DDC_SCL_MASK
 */
#define DDC_SDA_READ_MASK  0x04
#define DDC_SCL_READ_MASK  0x08
#define DDC_SDA_WRITE_MASK 0x10
#define DDC_SCL_WRITE_MASK 0x20

/* 
 *
 * CRT I2C
 *
 */
/*
 *
 */
static void
VIAI2C1PutBits(I2CBusPtr Bus, int clock,  int data)
{
    vgaHWPtr hwp = VGAHWPTR(xf86Screens[Bus->scrnIndex]);
    CARD8 value = 0x01; /* Enable DDC */
    
    if (clock)
        value |= DDC_SCL_WRITE_MASK;
    
    if (data)
        value |= DDC_SDA_WRITE_MASK;

    ViaSeqChange(hwp, 0x26, value,
                 0x01 | DDC_SCL_WRITE_MASK | DDC_SDA_WRITE_MASK);
}

/*
 *
 */
static void
VIAI2C1GetBits(I2CBusPtr Bus, int *clock, int *data)
{
    vgaHWPtr hwp = VGAHWPTR(xf86Screens[Bus->scrnIndex]);
    CARD8 value = hwp->readSeq(hwp, 0x26);
    
    *clock = (value & DDC_SCL_READ_MASK) != 0;
    *data  = (value & DDC_SDA_READ_MASK) != 0;
}

/*
 *
 */
static I2CBusPtr
ViaI2CBus1Init(int scrnIndex)
{
    I2CBusPtr pI2CBus = xf86CreateI2CBusRec();
    
    DEBUG(xf86DrvMsg(scrnIndex, X_INFO, "ViaI2CBus1Init\n"));

    if (!pI2CBus)
	return NULL;
    
    pI2CBus->BusName    = "I2C bus 1";
    pI2CBus->scrnIndex  = scrnIndex;
    pI2CBus->I2CPutBits = VIAI2C1PutBits;
    pI2CBus->I2CGetBits = VIAI2C1GetBits;

    if (!xf86I2CBusInit(pI2CBus)) {
        xf86DestroyI2CBusRec(pI2CBus, TRUE, FALSE);
        return NULL;
    }

    return pI2CBus;
}

/*
 *
 * First data bus I2C: tends to have TV-encoders
 *
 */
/*
 *
 */
static void
VIAI2C2PutBits(I2CBusPtr Bus, int clock,  int data)
{
    vgaHWPtr hwp = VGAHWPTR(xf86Screens[Bus->scrnIndex]);
    CARD8 value = 0x01; /* Enable DDC */
    
    if (clock)
        value |= DDC_SCL_WRITE_MASK;
    
    if (data)
        value |= DDC_SDA_WRITE_MASK;

    ViaSeqChange(hwp, 0x31, value,
                 0x01 | DDC_SCL_WRITE_MASK | DDC_SDA_WRITE_MASK);
}

/*
 *
 */
static void
VIAI2C2GetBits(I2CBusPtr Bus, int *clock, int *data)
{
    vgaHWPtr hwp = VGAHWPTR(xf86Screens[Bus->scrnIndex]);
    CARD8 value = hwp->readSeq(hwp, 0x31);
    
    *clock = (value & DDC_SCL_READ_MASK) != 0;
    *data  = (value & DDC_SDA_READ_MASK) != 0;
}

/*
 *
 */
static I2CBusPtr
ViaI2CBus2Init(int scrnIndex)
{
    I2CBusPtr pI2CBus = xf86CreateI2CBusRec();
    
    DEBUG(xf86DrvMsg(scrnIndex, X_INFO, "ViaI2cBus2Init\n"));

    if (!pI2CBus)
	return NULL;
    
    pI2CBus->BusName    = "I2C bus 2";
    pI2CBus->scrnIndex  = scrnIndex;
    pI2CBus->I2CPutBits = VIAI2C2PutBits;
    pI2CBus->I2CGetBits = VIAI2C2GetBits;

    if (!xf86I2CBusInit(pI2CBus)) {
        xf86DestroyI2CBusRec(pI2CBus, TRUE, FALSE);
        return NULL;
    }

    return pI2CBus;
}

/*
 *
 * If we ever get some life in the gpioi2c code, work it into
 * xf86I2C, here.
 *
 */
/*
 *
 */
static I2CBusPtr
ViaI2CBus3Init(int scrnIndex)
{
    DEBUG(xf86DrvMsg(scrnIndex, X_INFO, "ViaI2CBus3Init\n"));

#if 0
    {
	I2CBusPtr pI2CBus = xf86CreateI2CBusRec();
    
	if (!pI2CBus)
	    return NULL;
	
	pI2CBus->BusName    = "I2C bus 3";
	pI2CBus->scrnIndex  = scrnIndex;
	pI2CBus->I2CPutBits = VIAI2C3PutBits;
	pI2CBus->I2CGetBits = VIAI2C3GetBits;
	
	if (!xf86I2CBusInit(pI2CBus)) {
	    xf86DestroyI2CBusRec(pI2CBus, TRUE, FALSE);
	    return NULL;
	}
	
	return pI2CBus; 
    }
#else
    xf86DrvMsg(scrnIndex, X_INFO, "Warning: Third I2C Bus not"
	       " implemented.\n");

    return NULL;
#endif
}

/*
 *
 * Ye Olde GPIOI2C code. -evil-
 *
 */

#define VIA_GPIOI2C_PORT 0x2C

#define GPIOI2C_MASKD           0xC0
#define GPIOI2C_SCL_MASK        0x80
#define GPIOI2C_SCL_WRITE       0x80
#define GPIOI2C_SCL_READ        0x80
#define GPIOI2C_SDA_MASK        0x40
#define GPIOI2C_SDA_WRITE       0x40
#define GPIOI2C_SDA_READ        0x00  /* wouldnt this be 0x40? */

#define I2C_SDA_SCL_MASK        0x30
#define I2C_SDA_SCL             0x30
#define I2C_OUTPUT_CLOCK        0x20
#define I2C_OUTPUT_DATA         0x10
#define I2C_INPUT_CLOCK         0x08
#define I2C_INPUT_DATA          0x04

#define READ_MAX_RETRIES        20
#define WRITE_MAX_RETRIES       20

static void
ViaGpioI2CInit(GpioI2cPtr pDev, vgaHWPtr hwp, 
	       void (*Delay)(I2CBusPtr pBus, int usec))
{
    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "ViaGpioI2cInit\n"));

    pDev->scrnIndex = hwp->pScrn->scrnIndex;
    pDev->hwp = hwp;
    pDev->RiseFallTime = 5;
    pDev->StartTimeout = 5;
    pDev->BitTimeout = 5;
    pDev->ByteTimeout = 5;
    pDev->HoldTimeout = 5;
    pDev->Delay = Delay;
}

void 
VIAGPIOI2C_Initial(GpioI2cPtr pDev, CARD8 Slave)
{
    DEBUG(xf86DrvMsg(pDev->scrnIndex, X_INFO, "GPIOI2C_Initial\n"));

    if (Slave == 0xA0 || Slave == 0xA2) {
        pDev->RiseFallTime = 40;
        pDev->StartTimeout = 550;
        pDev->ByteTimeout = 2200;
    } else {
        pDev->RiseFallTime = 5;
        pDev->StartTimeout = 5;
        pDev->ByteTimeout = 5;
    }

    pDev->Address = Slave;
}

/*
 *
 */
static void
ViaGpioI2c_Release(GpioI2cPtr pDev)
{
   ViaSeqChange(pDev->hwp,  VIA_GPIOI2C_PORT, 0x00, GPIOI2C_MASKD); 
}

/*
 *
 */
static void
ViaGpioI2c_SCLWrite(GpioI2cPtr pDev, CARD8 Data)
{
  if (Data)
      ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, 
		   GPIOI2C_SCL_WRITE | I2C_OUTPUT_CLOCK,
		   GPIOI2C_SCL_MASK | I2C_OUTPUT_CLOCK);
  else
      ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, GPIOI2C_SCL_WRITE,
		   GPIOI2C_SCL_MASK | I2C_OUTPUT_CLOCK);  
}

/*
 *
 */
#ifdef UNUSED
static void
ViaGpioI2c_SCLRead(GpioI2cPtr pDev)
{
    ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, GPIOI2C_SCL_READ, GPIOI2C_SCL_MASK);
}
#endif

/*
 *
 */
static void
ViaGpioI2c_SDAWrite(GpioI2cPtr pDev, CARD8 Data)
{
    if (Data)
	ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, 
		     GPIOI2C_SDA_WRITE | I2C_OUTPUT_DATA,
		     GPIOI2C_SDA_MASK | I2C_OUTPUT_DATA);
    else
	ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, GPIOI2C_SDA_WRITE,
		     GPIOI2C_SDA_MASK | I2C_OUTPUT_DATA);
}

/*
 * GGPIOI2C_SCL_READ ???
 */
static void
ViaGpioI2c_SDARead(GpioI2cPtr pDev)
{
    ViaSeqChange(pDev->hwp, VIA_GPIOI2C_PORT, GPIOI2C_SCL_READ, GPIOI2C_SDA_MASK);
}

/* Set SCL */
static void 
HWGPIOI2C_SetSCL(GpioI2cPtr pDev, CARD8 flag)
{
    ViaGpioI2c_SCLWrite(pDev, flag);
    if (flag) 
	pDev->Delay(NULL, pDev->RiseFallTime);
    pDev->Delay(NULL, pDev->RiseFallTime);
}

/* Set SDA */
static void 
HWGPIOI2C_SetSDA(GpioI2cPtr pDev, CARD8 flag)
{
    ViaGpioI2c_SDAWrite(pDev, flag);
    pDev->Delay(NULL, pDev->RiseFallTime);
}

/* Get SDA */
static CARD8  
HWGPIOI2C_GetSDA(GpioI2cPtr pDev)
{
    vgaHWPtr hwp = pDev->hwp;
    
    if (hwp->readSeq(hwp, 0x2C) & I2C_INPUT_DATA)
        return 1;
    else
        return 0;
}

/* START Condition */
static void 
GPIOI2C_START(GpioI2cPtr pDev)
{
    HWGPIOI2C_SetSDA(pDev, 1);
    HWGPIOI2C_SetSCL(pDev, 1);
    pDev->Delay(NULL, pDev->StartTimeout);
    HWGPIOI2C_SetSDA(pDev, 0);
    HWGPIOI2C_SetSCL(pDev, 0);
}

/* STOP Condition */
static void 
GPIOI2C_STOP(GpioI2cPtr pDev)
{
    HWGPIOI2C_SetSDA(pDev, 0);
    HWGPIOI2C_SetSCL(pDev, 1);
    HWGPIOI2C_SetSDA(pDev, 1);
    ViaGpioI2c_Release(pDev);
    /* to make the differentiation of next START condition */
    pDev->Delay(NULL, pDev->RiseFallTime);
}

/* Check ACK */
static Bool 
GPIOI2C_ACKNOWLEDGE(GpioI2cPtr pDev)
{
    CARD8   Status;

    ViaGpioI2c_SDARead(pDev);
    pDev->Delay(NULL, pDev->RiseFallTime);
    HWGPIOI2C_SetSCL(pDev, 1);
    Status = HWGPIOI2C_GetSDA(pDev);
    /* eh? SDAWrite before writing SCL? */
    ViaGpioI2c_SDAWrite(pDev, Status);
    HWGPIOI2C_SetSCL(pDev, 0);
    pDev->Delay(NULL, pDev->RiseFallTime);

    if(Status) 
	return FALSE;
    else 
	return TRUE;
}

/* Send ACK */
static Bool 
GPIOI2C_SENDACKNOWLEDGE(GpioI2cPtr pDev)
{
    HWGPIOI2C_SetSDA(pDev, 0);
    HWGPIOI2C_SetSCL(pDev, 1);
    HWGPIOI2C_SetSCL(pDev, 0);
    pDev->Delay(NULL, pDev->RiseFallTime);

    return TRUE;
}

/* Send NACK */
static Bool 
GPIOI2C_SENDNACKNOWLEDGE(GpioI2cPtr pDev)
{
    HWGPIOI2C_SetSDA(pDev, 1);
    HWGPIOI2C_SetSCL(pDev, 1);
    HWGPIOI2C_SetSCL(pDev, 0);
    pDev->Delay(NULL, pDev->RiseFallTime);

    return TRUE;
}

static Bool 
GPIOI2C_WriteBit(GpioI2cPtr pDev, int sda, int timeout)
{
    Bool ret = TRUE;

    HWGPIOI2C_SetSDA(pDev, sda);
    pDev->Delay(NULL, pDev->RiseFallTime/5);
    HWGPIOI2C_SetSCL(pDev, 1);
    pDev->Delay(NULL, pDev->HoldTimeout);
    pDev->Delay(NULL, timeout);
    HWGPIOI2C_SetSCL(pDev, 0);
    pDev->Delay(NULL, pDev->RiseFallTime/5);

    return ret;
}

/* Write Data(Bit by Bit) to I2C */
static Bool 
GPIOI2C_WriteData(GpioI2cPtr pDev, CARD8 Data)
{
    int     i;

    if (!GPIOI2C_WriteBit(pDev, (Data >> 7) & 1, pDev->ByteTimeout))
	return FALSE;

    for (i = 6; i >= 0; i--)
	if (!GPIOI2C_WriteBit(pDev, (Data >> i) & 1, pDev->BitTimeout))
	    return FALSE;

    return GPIOI2C_ACKNOWLEDGE(pDev);
}

static Bool 
GPIOI2C_ReadBit(GpioI2cPtr pDev, int *psda, int timeout)
{
    Bool ret = TRUE;

    ViaGpioI2c_SDARead(pDev);
    pDev->Delay(NULL, pDev->RiseFallTime/5);
    HWGPIOI2C_SetSCL(pDev, 1);
    pDev->Delay(NULL, pDev->HoldTimeout);
    pDev->Delay(NULL, timeout);
    *psda = HWGPIOI2C_GetSDA(pDev);
    HWGPIOI2C_SetSCL(pDev, 0);
    pDev->Delay(NULL, pDev->RiseFallTime/5);

    return ret;
}

/* Read Data(Bit by Bit) from I2C */
static CARD8 
GPIOI2C_ReadData(GpioI2cPtr pDev)
{
    int     i, sda;
    CARD8   data;

    if(!GPIOI2C_ReadBit(pDev, &sda, pDev->ByteTimeout))
	return 0;

    data = (sda > 0) << 7;
    for (i = 6; i >= 0; i--)
	if (!GPIOI2C_ReadBit(pDev, &sda, pDev->BitTimeout))
	    return 0;
	else
	    data |= (sda > 0) << i;

    return  data;
}

/* Write Data(one Byte) to Desired Device on I2C */
Bool 
VIAGPIOI2C_Write(GpioI2cPtr pDev, CARD8 SubAddress, CARD8 Data)
{
    int     Retry;
    Bool    Done = FALSE;

    DEBUG(xf86DrvMsg(pDev->scrnIndex, X_INFO, "VIAGPIOI2C_Write\n"));

    for(Retry = 1; Retry <= WRITE_MAX_RETRIES; Retry++)
    {
        GPIOI2C_START(pDev);

        if(!GPIOI2C_WriteData(pDev, pDev->Address)) {

            GPIOI2C_STOP(pDev);
            continue;
        }
	if(!GPIOI2C_WriteData(pDev, (CARD8)(SubAddress))) {
	    
	    GPIOI2C_STOP(pDev);
	    continue;
	}

        if(!GPIOI2C_WriteData(pDev, Data)) {

            GPIOI2C_STOP(pDev);
            continue;
        }
        Done = TRUE;
        break;
    }

    GPIOI2C_STOP(pDev);

    return Done;
}

/* Read Data from Desired Device on I2C */
Bool 
VIAGPIOI2C_Read(GpioI2cPtr pDev, CARD8 SubAddress, CARD8 *Buffer, int BufferLen)
{
    int     i, Retry;

    DEBUG(xf86DrvMsg(pDev->scrnIndex, X_INFO, "VIAGPIOI2C_Read\n"));

    for(Retry = 1; Retry <= READ_MAX_RETRIES; Retry++)
    {
        GPIOI2C_START(pDev);
        if(!GPIOI2C_WriteData(pDev, pDev->Address & 0xFE)) {

            GPIOI2C_STOP(pDev);
            continue;
        }
	if(!GPIOI2C_WriteData(pDev, (CARD8)(SubAddress))) {
	    
	    GPIOI2C_STOP(pDev);
	    continue;
	}

        break;
    }

    if (Retry > READ_MAX_RETRIES) 
	return FALSE;

    for(Retry = 1; Retry <= READ_MAX_RETRIES; Retry++)
    {
        GPIOI2C_START(pDev);
        if(!GPIOI2C_WriteData(pDev, pDev->Address | 0x01)) {

            GPIOI2C_STOP(pDev);
            continue;
        }
        for(i = 0; i < BufferLen; i++) {

            *Buffer = GPIOI2C_ReadData(pDev);
            Buffer ++;
            if(BufferLen == 1)
                /*GPIOI2C_SENDACKNOWLEDGE(pDev);*/ /* send ACK for normal operation */
                GPIOI2C_SENDNACKNOWLEDGE(pDev);    /* send NACK for VT3191/VT3192 only */
            else if(i < BufferLen - 1)
                GPIOI2C_SENDACKNOWLEDGE(pDev);     /* send ACK */
            else
                GPIOI2C_SENDNACKNOWLEDGE(pDev);    /* send NACK */
        }
        GPIOI2C_STOP(pDev);
        break;
    }

    if (Retry > READ_MAX_RETRIES)
        return FALSE;
    else
        return TRUE;
}

/* Read Data(one Byte) from Desired Device on I2C */
Bool 
VIAGPIOI2C_ReadByte(GpioI2cPtr pDev, CARD8 SubAddress, CARD8 *Buffer)
{
    int     Retry;

    DEBUG(xf86DrvMsg(pDev->scrnIndex, X_INFO, "VIAGPIOI2C_ReadByte\n"));

    for(Retry = 1; Retry <= READ_MAX_RETRIES; Retry++)
    {
        GPIOI2C_START(pDev);
        if(!GPIOI2C_WriteData(pDev, pDev->Address & 0xFE)) {

            GPIOI2C_STOP(pDev);
            continue;
        }
	if(!GPIOI2C_WriteData(pDev, (CARD8)(SubAddress))) {
	    
	    GPIOI2C_STOP(pDev);
	    continue;
	}

        break;
    }

    if (Retry > READ_MAX_RETRIES) 
	return FALSE;

    for(Retry = 1; Retry <= READ_MAX_RETRIES; Retry++) {

        GPIOI2C_START(pDev);

        if(!GPIOI2C_WriteData(pDev, pDev->Address | 0x01)) {

            GPIOI2C_STOP(pDev);
            continue;
        }

        *Buffer = GPIOI2C_ReadData(pDev);

        GPIOI2C_STOP(pDev);
        break;
    }

    if (Retry > READ_MAX_RETRIES)
        return FALSE;
    else
        return TRUE;
}

/*
 *
 */
Bool
ViaGpioI2c_Probe(GpioI2cPtr pDev, CARD8 Address)
{
    DEBUG(xf86DrvMsg(pDev->scrnIndex, X_INFO, "VIAGPIOI2C_Probe\n"));
    
    GPIOI2C_START(pDev);
    return GPIOI2C_WriteData(pDev, Address);
}

/*
 *
 *
 *
 */
void
VIAI2CInit(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAI2CInit\n"));

    pVia->pI2CBus1 = ViaI2CBus1Init(pScrn->scrnIndex);
    pVia->pI2CBus2 = ViaI2CBus2Init(pScrn->scrnIndex);
    pVia->pI2CBus3 = ViaI2CBus3Init(pScrn->scrnIndex);

    /* so that i dont have to clean out the lot right now. */
    pVia->I2C_Port1 = pVia->pI2CBus1;
    pVia->I2C_Port2 = pVia->pI2CBus2;

    ViaGpioI2CInit(&(pVia->GpioI2c), VGAHWPTR(pScrn), 
		     pVia->pI2CBus1->I2CUDelay);
}
