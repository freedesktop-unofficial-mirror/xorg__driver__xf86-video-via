/*
 * Copyright 2004 The Unichrome Project  [unichrome.sf.net]
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

/*
 * via_mode.c
 *
 * Everything to do with setting and changing modes.
 *
 */

#include "via_driver.h"
#include "via_refresh.h"
#include "via_vgahw.h"
#include "via_id.h"

/* plain and simple lookuptable for TV/PanelIndex selection */
struct {
    int Index;
    int TVIndex;
    int PanelIndex;
    int X;
    int Y;
} ViaResolutionTable[] = {
    {VIA_RES_640X480,   VIA_TVRES_640X480,  VIA_PANEL6X4,       640,  480},
    {VIA_RES_800X600,   VIA_TVRES_800X600,  VIA_PANEL8X6,       800,  600},
    {VIA_RES_1024X768,  VIA_TVRES_1024X768, VIA_PANEL10X7,     1024,  768},
    {VIA_RES_1152X864,  VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1152,  864},
    {VIA_RES_1280X1024, VIA_TVRES_INVALID,  VIA_PANEL12X10,    1280, 1024},
    {VIA_RES_1600X1200, VIA_TVRES_INVALID,  VIA_PANEL16X12,    1600, 1200},
    {VIA_RES_1440X1050, VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1440, 1050},
    {VIA_RES_1280X768,  VIA_TVRES_INVALID,  VIA_PANEL12X7,     1280,  768},
    {VIA_RES_1280X960,  VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1280,  960},
 /* {VIA_RES_1920X1440, VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1920, 1140}, */
    {VIA_RES_848X480,   VIA_TVRES_848X480,  VIA_PANEL_INVALID,  848,  480},
    {VIA_RES_1400X1050, VIA_TVRES_INVALID,  VIA_PANEL14X10,    1400, 1050},
    {VIA_RES_720X480,   VIA_TVRES_720X480,  VIA_PANEL_INVALID,  720,  480},
    {VIA_RES_720X576,   VIA_TVRES_720X576,  VIA_PANEL_INVALID,  720,  576},
    {VIA_RES_1024X512,  VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1024,  512},
    {VIA_RES_856X480,   VIA_TVRES_INVALID,  VIA_PANEL_INVALID,  856,  480},
    {VIA_RES_1024X576,  VIA_TVRES_INVALID,  VIA_PANEL_INVALID, 1024,  576},
    {VIA_RES_INVALID,   VIA_TVRES_INVALID,  VIA_PANEL_INVALID,    0,    0}
};

/*=*
 *
 * int VIACheckTVExist(VIABIOSInfoPtr) - Check TV Encoder
 *
 * Return Type:    int
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 0 - None
 *                 1 - VIA VT1621
 *                 2 - VIA VT1622
 *                 
 *=*/
void
VIACheckTVExist(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    I2CDevPtr       dev;
    CARD8   W_Buffer[1];
    CARD8   R_Buffer[1];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIACheckTVExist\n"));
    /* Check For TV2/TV3 */
    if (xf86I2CProbeAddress(pVia->I2C_Port2, 0x40)) {
        dev = xf86CreateI2CDevRec();
        dev->DevName = "TV";
        dev->SlaveAddr = 0x40;
        dev->pI2CBus = pVia->I2C_Port2;

        if (xf86I2CDevInit(dev)) {
            W_Buffer[0] = 0x1B;
            xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
            xf86DestroyI2CDevRec(dev,TRUE);
            switch (R_Buffer[0]) {
                case 2:
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Found TVEncoder VT1621!\n"));
                    pBIOSInfo->TVEncoder = VIA_TV2PLUS;
                    pBIOSInfo->TVI2CAdd = 0x40;
                    break;
                case 3:
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Found TVEncoder VT1622!\n"));
                    pBIOSInfo->TVEncoder = VIA_TV3;
                    pBIOSInfo->TVI2CAdd = 0x40;
                    break;
                case 16:
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Found TVEncoder VT1622A!\n"));
                    pBIOSInfo->TVEncoder = VIA_VT1622A;
                    pBIOSInfo->TVI2CAdd = 0x40;
                    break;
                default:
                    pBIOSInfo->TVEncoder = VIA_NONETV;
                    break;
            }
        }
        else
            xf86DestroyI2CDevRec(dev,TRUE);
    }
    else
        pBIOSInfo->TVEncoder = VIA_NONETV;

    if (pBIOSInfo->TVEncoder == VIA_NONETV) {
	VIAGPIOI2C_Initial(&(pVia->GpioI2c), 0x40);
	if (VIAGPIOI2C_Read(&(pVia->GpioI2c), 0x1B, R_Buffer, 1)) {
	    switch (R_Buffer[0]) {
	    case 16:
		DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
				 "Found TVEncoder VT1623!\n"));
		pBIOSInfo->TVEncoder = VIA_VT1623;
		break;
	    default:
		DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
				 "Unknow TVEncoder Type:%d!\n", R_Buffer[0]));
		break;
	    }
	}
    }
}

/*=*
 *
 * CARD8 VIAGetActiveDisplay(VIABIOSInfoPtr, CARD8)
 *
 *     - Get Active Display Device
 *
 * Return Type:    CARD8
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 Bit[7] 2nd Path
 *                 Bit[6] 1/0 MHS Enable/Disable
 *                 Bit[5] 0 = Bypass Callback, 1 = Enable Callback
 *                 Bit[4] 0 = Hot-Key Sequence Control (OEM Specific)
 *                 Bit[3] LCD
 *                 Bit[2] TV
 *                 Bit[1] CRT
 *                 Bit[0] DVI
 *=*/
CARD8 
VIAGetActiveDisplay(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8    tmp;

    tmp = (hwp->readCrtc(hwp, 0x3E) >> 4);
    tmp |= ((hwp->readCrtc(hwp, 0x3B) & 0x18) << 3);

    return tmp;
}

/*=*
 *
 * CARD8 VIASensorTV2(VIABIOSInfoPtr pBIOSInfo)
 *
 *     - Sense TV2+ Encoder
 *
 * Return Type:    CARD8
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 Bit[2] Bit[0]
 *                 0      0      - Composite + S-Video
 *                 0      1      - S-Video
 *                 1      0      - Composite
 *                 1      1      - None
 *=*/
static CARD8
VIASensorTV2(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    I2CDevPtr  dev;
    CARD8      save, tmp = 0x05;
    CARD8      W_Buffer[2];
    CARD8      R_Buffer[1];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASensorTV2\n"));
    dev = xf86CreateI2CDevRec();
    dev->DevName = "VT1621";
    dev->SlaveAddr = 0x40;
    dev->pI2CBus = pVia->I2C_Port2;

    if (xf86I2CDevInit(dev)) {
        W_Buffer[0] = 0x0E;
        xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
        save = R_Buffer[0];
        W_Buffer[1] = 0x08;
        xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
        W_Buffer[1] = 0;
        xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
        W_Buffer[0] = 0x0F;
        xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
        tmp = R_Buffer[0] & 0x0F;
        W_Buffer[0] = 0x0E;
        W_Buffer[1] = save;
        xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
        xf86DestroyI2CDevRec(dev,TRUE);
    } else 
        xf86DestroyI2CDevRec(dev,TRUE);

    return tmp;
}


/*=*
 *
 * CARD8 VIASensorTV3(VIABIOSInfoPtr pBIOSInfo)
 *
 *     - Sense TV3 Encoder
 *
 * Return Type:    CARD8
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 Bit[3] Bit[2] Bit[1] Bit[0]
 *                 1      1      1      1      - None
 *                 0      1      1      1      - Composite
 *                 1      1      1      0      - Composite
 *                                             - Others: S-Video
 *=*/
static CARD8
VIASensorTV3(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    I2CDevPtr  dev;
    CARD8      save, tmp = 0;
    CARD8      W_Buffer[2];
    CARD8      R_Buffer[1];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASensorTV3\n"));
    if (pBIOSInfo->TVEncoder == VIA_VT1623) {
        VIAGPIOI2C_Initial(&(pVia->GpioI2c), 0x40);
        VIAGPIOI2C_Read(&(pVia->GpioI2c), 0x0E, R_Buffer, 1);
        save = R_Buffer[0];
        W_Buffer[0] = 0;
        VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0E, W_Buffer[0]);
        W_Buffer[0] = 0x80;
        VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0E, W_Buffer[0]);
        W_Buffer[0] = 0;
        VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0E, W_Buffer[0]);
        VIAGPIOI2C_Read(&(pVia->GpioI2c), 0x0F, R_Buffer, 1);
        tmp = R_Buffer[0] & 0x0F;
        W_Buffer[0] = save;
        VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0E, W_Buffer[0]);
    } else {
        dev = xf86CreateI2CDevRec();
        dev->DevName = "VT1622";
        dev->SlaveAddr = 0x40;
        dev->pI2CBus = pVia->I2C_Port2;

        if (xf86I2CDevInit(dev)) {
            W_Buffer[0] = 0x0E;
            xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
            save = R_Buffer[0];
            W_Buffer[1] = 0;
            xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
            W_Buffer[1] = 0x80;
            xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
            W_Buffer[1] = 0;
            xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
            W_Buffer[0] = 0x0F;
            xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
            tmp = R_Buffer[0] & 0x0F;
            W_Buffer[0] = 0x0E;
            W_Buffer[1] = save;
            xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
            xf86DestroyI2CDevRec(dev,TRUE);
        }
        else {
            xf86DestroyI2CDevRec(dev,TRUE);
        }
    }
    return tmp;
}


/*=*
 *
 * Bool VIASensorDVI(pBIOSInfo)
 *
 *     - Sense DVI Connector
 *
 * Return Type:    Bool
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 DVI Attached  - TRUE
 *                 DVI Not Attached - FALSE
 *=*/
static Bool
VIASensorDVI(ScrnInfoPtr pScrn)
{
    vgaHWPtr   hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    CARD8      SlaveAddr, cr6c, cr93;
    Bool       ret = FALSE;
    I2CDevPtr  dev;
    CARD8      W_Buffer[1];
    CARD8      R_Buffer[1];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASensorDVI\n"));

    /* Enable DI0, DI1 */
    cr6c = hwp->readCrtc(hwp, 0x6C);
    ViaCrtcChange(hwp, 0x6C, 0x21, 0x21);

    cr93 = hwp->readCrtc(hwp, 0x93);
    /* check for CLE266 first!!! */
    if (CLE266_REV_IS_CX(pVia->ChipRev))
	hwp->writeCrtc(hwp, 0x93, 0xA3);
    else 
	hwp->writeCrtc(hwp, 0x93, 0xBF);

    /* Enable LCD */
    VIAEnableLCD(pScrn);

    switch (pBIOSInfo->TMDS) {
        case VIA_SIL164:
            SlaveAddr = 0x70;
            break;
        case VIA_VT3192:
            SlaveAddr = 0x10;
            break;
        default:
            return ret;
            break;
    }

    if (xf86I2CProbeAddress(pVia->I2C_Port2, SlaveAddr)) {
        dev = xf86CreateI2CDevRec();
        dev->DevName = "TMDS";
        dev->SlaveAddr = SlaveAddr;
        dev->pI2CBus = pVia->I2C_Port2;

        if (xf86I2CDevInit(dev)) {
            W_Buffer[0] = 0x09;
            xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
            xf86DestroyI2CDevRec(dev,TRUE);
            if (R_Buffer[0] & 0x04)
                ret = TRUE;
        }
        else
            xf86DestroyI2CDevRec(dev,TRUE);
    }

    if (pVia->Chipset != VIA_CLE266) {
        VIAGPIOI2C_Initial(&(pVia->GpioI2c), SlaveAddr);
        VIAGPIOI2C_Read(&(pVia->GpioI2c), 0x09, R_Buffer, 1);
	if (R_Buffer[0] & 0x04)
	    ret = TRUE;
    }

    /* Disable LCD */
    VIADisableLCD(pScrn);

    /* Restore DI0, DI1 status */
    hwp->writeCrtc(hwp, 0x6C, cr6c);
    hwp->writeCrtc(hwp, 0x93, cr93);

    return ret;
}

Bool
VIAPostDVI(ScrnInfoPtr pScrn)
{
    vgaHWPtr   hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    CARD8      cr6c, cr93;
    Bool       ret = FALSE;
    I2CDevPtr  dev;
    CARD8      W_Buffer[2];
    CARD8      R_Buffer[4];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAPostDVI\n"));

    /* Enable DI0, DI1 */
    cr6c = hwp->readCrtc(hwp, 0x6C);
    ViaCrtcChange(hwp, 0x6C, 0x21, 0x21);

    cr93 = hwp->readCrtc(hwp, 0x93);
    /* check for CLE266 first!!! */
    if (CLE266_REV_IS_CX(pVia->ChipRev))
	hwp->writeCrtc(hwp, 0x93, 0xA3);
    else 
	hwp->writeCrtc(hwp, 0x93, 0xBF);

    /* Enable LCD */
    VIAEnableLCD(pScrn);

    if (xf86I2CProbeAddress(pVia->I2C_Port2, 0x70)) {
        dev = xf86CreateI2CDevRec();
        dev->DevName = "TMDS";
        dev->SlaveAddr = 0x70;
        dev->pI2CBus = pVia->I2C_Port2;
        if (xf86I2CDevInit(dev)) {
            W_Buffer[0] = 0x00;
            xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,4);
            if (R_Buffer[0] == 0x06 && R_Buffer[1] == 0x11) {       /* This is VT3191 */
                DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                 "Found VIA LVDS Transmiter!\n"));
                if (R_Buffer[2] == 0x91 && R_Buffer[3] == 0x31) {
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                     "VIA: VT1631!\n"));
                    pBIOSInfo->LVDS = VIA_VT3191;
                    W_Buffer[0] = 0x08;
                    if (pBIOSInfo->BusWidth == VIA_DI_24BIT && pBIOSInfo->LCDDualEdge)
                        W_Buffer[1] = 0x0D;
                    else
                        W_Buffer[1] = 0x09;
                    xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
                    xf86DestroyI2CDevRec(dev,TRUE);
                    ret = TRUE;
                }
                else {
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                     "VIA: Unknow Chip!!\n"));
                    xf86DestroyI2CDevRec(dev,TRUE);
                }
            }
            else if (R_Buffer[0] == 0x01 && R_Buffer[1] == 0x00) {  /* This is Sil164 */
                W_Buffer[0] = 0x02;
                xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,2);
                if (R_Buffer[0] && R_Buffer[1]) {
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                     "Found TMDS Transmiter Silicon164.\n"));
                    pBIOSInfo->TMDS = VIA_SIL164;
                    W_Buffer[0] = 0x08;
                    if (pBIOSInfo->BusWidth == VIA_DI_24BIT) {
                        if (pBIOSInfo->LCDDualEdge)
                            W_Buffer[1] = 0x3F;
                        else
                            W_Buffer[1] = 0x37;
                    }
                    else    /* 12Bit Only has Single Mode */
                        W_Buffer[1] = 0x3B;
                    xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
                    W_Buffer[0] = 0x0C;
                    W_Buffer[1] = 0x09;
                    xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
                    xf86DestroyI2CDevRec(dev,TRUE);
                    ret = TRUE;
                }
                else {
                    xf86DestroyI2CDevRec(dev,TRUE);
                }
            }
            else {
                xf86DestroyI2CDevRec(dev,TRUE);
            }
        }
        else {
            xf86DestroyI2CDevRec(dev,TRUE);
        }
    }

    /* Check VT3192 TMDS Exist or not?*/
    if (!pBIOSInfo->TMDS) {
        if (xf86I2CProbeAddress(pVia->I2C_Port2, 0x10)) {
            dev = xf86CreateI2CDevRec();
            dev->DevName = "TMDS";
            dev->SlaveAddr = 0x10;
            dev->pI2CBus = pVia->I2C_Port2;
            if (xf86I2CDevInit(dev)) {
                W_Buffer[0] = 0x00;
                xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,4);

                if (R_Buffer[0] == 0x06 && R_Buffer[1] == 0x11) {   /* This is VT3192 */
                    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                     "Found VIA TMDS Transmiter!\n"));
                    pBIOSInfo->TMDS = VIA_VT3192;
                    if (R_Buffer[2] == 0x92 && R_Buffer[3] == 0x31) {
                        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                         "VIA: VT1632!\n"));
                        pBIOSInfo->TMDS = VIA_VT3192;
                        W_Buffer[0] = 0x08;
                        if (pBIOSInfo->BusWidth == VIA_DI_24BIT) {
                            if (pBIOSInfo->LCDDualEdge)
                                W_Buffer[1] = 0x3F;
                            else
                                W_Buffer[1] = 0x37;
                        }
                        else
                            W_Buffer[1] = 0x3B;
                        xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
                        xf86DestroyI2CDevRec(dev,TRUE);
                        ret = TRUE;
                    }
                    else {
                        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                                         "VIA: Unknow Chip!!\n"));
                        xf86DestroyI2CDevRec(dev,TRUE);
                    }
                }
                else {
                    xf86DestroyI2CDevRec(dev,TRUE);
                }
            }
            else {
                xf86DestroyI2CDevRec(dev,TRUE);
            }
        }
    }

    /* GPIO Sense */
    if (pVia->Chipset != VIA_CLE266) {
        VIAGPIOI2C_Initial(&(pVia->GpioI2c), 0x70);
        if (VIAGPIOI2C_Read(&(pVia->GpioI2c), 0, R_Buffer, 2)) {
	    if (R_Buffer[0] == 0x06 && R_Buffer[1] == 0x11) {   /* VIA LVDS */
		VIAGPIOI2C_Read(&(pVia->GpioI2c), 2, R_Buffer, 2);
		if (R_Buffer[0] == 0x91 && R_Buffer[1] == 0x31) {
		    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
				     "Found LVDS Transmiter VT1631.\n"));
		    pBIOSInfo->LVDS = VIA_VT3191;
		    if (pBIOSInfo->BusWidth == VIA_DI_24BIT && pBIOSInfo->LCDDualEdge)
			VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x0D);
		    else
			VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x09);
		    ret = TRUE;
		}
	    }
	    else if (R_Buffer[0] == 0x01 && R_Buffer[1] == 0x0) {/* Silicon TMDS */
		VIAGPIOI2C_Read(&(pVia->GpioI2c), 2, R_Buffer, 2);
		if (R_Buffer[0] && R_Buffer[1]) {
		    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
				     "Found TMDS Transmiter Silicon164.\n"));
		    pBIOSInfo->TMDS = VIA_SIL164;
		    if (pBIOSInfo->BusWidth == VIA_DI_24BIT) {
			if (pBIOSInfo->LCDDualEdge)
			    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x3F);
			else
			    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x37);
		    }
		    else {
			VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x3B);
		    }
		    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0C, 0x09);
		    ret = TRUE;
		}
	    }
	}
        VIAGPIOI2C_Initial(&(pVia->GpioI2c), 0x10);
        if (VIAGPIOI2C_Read(&(pVia->GpioI2c), 0, R_Buffer, 2)) {
	    if (R_Buffer[0] == 0x06 && R_Buffer[1] == 0x11) {   /* VIA TMDS */
		VIAGPIOI2C_Read(&(pVia->GpioI2c), 2, R_Buffer, 2);
		if (R_Buffer[0] == 0x92 && R_Buffer[1] == 0x31) {
		    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
				     "Found TMDS Transmiter VT1632.\n"));
		    pBIOSInfo->TMDS = VIA_VT3192;
		    if (pBIOSInfo->BusWidth == VIA_DI_24BIT) {
			if (pBIOSInfo->LCDDualEdge)
			    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x3F);
			else
			    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x37);
		    }
		    else {
			VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x08, 0x3B);
		    }
		    ret = TRUE;
		}
	    }
	}
    }

    /* Disable LCD */
    VIADisableLCD(pScrn);

    /* Restore DI0, DI1 status */
    hwp->writeCrtc(hwp, 0x6C, cr6c);
    hwp->writeCrtc(hwp, 0x93, cr93);

    if (pBIOSInfo->LVDS && pBIOSInfo->PanelSize == VIA_PANEL_INVALID) {
        pBIOSInfo->PanelSize = hwp->readCrtc(hwp, 0x3F) >> 4;
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
             "Get PanelID From Scratch Pad is %d\n", pBIOSInfo->PanelSize));
    }

    return ret;
}

/* remove this prototype at some stage */
static void VIAGetPanelSize(ScrnInfoPtr pScrn);

/*=*
 *
 * CARD8 VIAGetDeviceDetect(VIABIOSInfoPtr)
 *
 *     - Get Display Device Attched
 *
 * Return Type:    CARD8
 *
 * The Definition of Input Value:
 *
 *                 VIABIOSInfoPtr
 *
 * The Definition of Return Value:
 *
 *                 Bit[7] Reserved ------------ 2nd TV Connector
 *                 Bit[6] Reserved ------------ 1st TV Connector
 *                 Bit[5] Reserved
 *                 Bit[4] CRT2
 *                 Bit[3] DFP
 *                 Bit[2] TV
 *                 Bit[1] LCD
 *                 Bit[0] CRT
 *=*/
CARD8
VIAGetDeviceDetect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    CARD8 tmp, sense;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetDeviceDetect\n"));

    tmp = VIA_DEVICE_CRT1; /* Default assume color CRT attached */
/*
    if (VIA_LCD_SUPPORT) {
        if (VIA_UNCOVERD_LCD_PANEL)
            tmp |= 0x08;
    }
*/
    if (pBIOSInfo->LVDS) {
        pBIOSInfo->LCDAttach = TRUE;
        tmp |= VIA_DEVICE_LCD;
    }

    switch (pBIOSInfo->TVEncoder) {
        case VIA_NONETV:
            pBIOSInfo->TVAttach = FALSE;
            break;
        case VIA_TV2PLUS:
            sense = VIASensorTV2(pScrn);
            if (sense == 0x05) {
                pBIOSInfo->TVAttach = FALSE; /* No TV Output Attached */
            }
            else {
                tmp |= VIA_DEVICE_TV;
                pBIOSInfo->TVAttach = TRUE;
                if (!pBIOSInfo->TVOutput) {
                    if (sense == 0) {
                        /*tmp |= 0xC0;  Connect S_Video + Composite */
                        pBIOSInfo->TVOutput = TVOUTPUT_SC;
                    }
                    if (sense == 0x01) {
                        /*tmp |= 0x80;  Connect S_Video */
                        pBIOSInfo->TVOutput = TVOUTPUT_SVIDEO;
                    }
                    else {
                        /*tmp |= 0x40;  Connect Composite */
                        pBIOSInfo->TVOutput = TVOUTPUT_COMPOSITE;
                    }
                }
                DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "TV2 sense = %d ,TVOutput = %d\n", sense, pBIOSInfo->TVOutput));
            }
            break;
        case VIA_TV3:
        case VIA_VT1622A:
        case VIA_VT1623:
            sense = VIASensorTV3(pScrn);
            if (sense == 0x0F) {
                pBIOSInfo->TVAttach = FALSE; /* No TV Output Attached */
            }
            else {
                tmp |= VIA_DEVICE_TV;
                pBIOSInfo->TVAttach = TRUE;
                if (!pBIOSInfo->TVOutput) {
                    if (sense == 0x07 || sense == 0x0E) {
                        /*tmp |= 0x40;  Connect Composite */
                        pBIOSInfo->TVOutput = TVOUTPUT_COMPOSITE;
                    }
                    else {
                        /*tmp |= 0x80;  Connect S_Video */
                        pBIOSInfo->TVOutput = TVOUTPUT_SVIDEO;
                    }
                }
                DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "TV3 sense = %d ,TVOutput = %d\n", sense, pBIOSInfo->TVOutput));
            }
            break;
        default:
            break;
    }

    if (pBIOSInfo->TMDS) {
        if (VIASensorDVI(pScrn)) {
            tmp |= VIA_DEVICE_DFP;
            pBIOSInfo->DVIAttach = TRUE;
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "DVI has Attachment.\n"));
            if (pBIOSInfo->PanelSize == VIA_PANEL_INVALID)
                VIAGetPanelSize(pScrn);
        }
        else {
            pBIOSInfo->DVIAttach = FALSE;
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "DVI hasn't Attachment.\n"));
        }
    }

    if ((pVia->Id) && (pVia->Id->Force != VIA_DEVICE_NONE)) {
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Forcing %d.\n", pVia->Id->Force));
	tmp |= pVia->Id->Force;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Returning %d.\n", tmp));
    return tmp;
}

/*
 *
 * Try to interprete EDID ourselves.
 *
 */
static Bool
ViaGetPanelSizeFromEDID(ScrnInfoPtr pScrn, xf86MonPtr pMon, int *size)
{
    int i, max = 0;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromEDID\n"));

    /* !!! Why are we not checking VESA modes? */

    /* checking standard timings */
    for (i = 0; i < 8; i++)
	if ((pMon->timings2[i].hsize > 256) && (pMon->timings2[i].hsize > max))
	    max = pMon->timings2[i].hsize;
    
    if (max != 0) {
	*size = max;
	return TRUE;
    }
	
    /* checking detailed monitor section */

    /* !!! skip Ranges and standard timings */

    /* check detailed timings */
    for (i = 0; i < DET_TIMINGS; i++)
	if (pMon->det_mon[i].type == DT) {
	    struct detailed_timings timing = pMon->det_mon[i].section.d_timings;
	    /* ignore v_active for now */
	    if ((timing.clock > 15000000) && (timing.h_active > max))
		max = timing.h_active;
	}

    if (max != 0) {
	*size = max;
	return TRUE;
    }

    return FALSE;
}

/*
 *
 */
static Bool
VIAGetPanelSizeFromDDCv1(ScrnInfoPtr pScrn, int *size)
{
    VIAPtr pVia = VIAPTR(pScrn);
    xf86MonPtr      pMon;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv1\n"));

    if (!xf86I2CProbeAddress(pVia->I2C_Port2, 0xA0))
	return FALSE;

    pMon = xf86DoEDID_DDC2(pScrn->scrnIndex, pVia->I2C_Port2);
    if (!pMon)
	return FALSE;

    pVia->DDC2 =  pMon;

    if (!pVia->DDC1) {
	xf86PrintEDID(pMon);
	xf86SetDDCproperties(pScrn, pMon);
    }

    if (!ViaGetPanelSizeFromEDID(pScrn, pMon, size)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unable to read PanelSize from EDID information\n");
	return FALSE;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv1: %d\n", *size));
    return TRUE;
}

/*
 *
 */
static Bool
VIAGetPanelSizeFromDDCv2(ScrnInfoPtr pScrn, int *size)
{
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8  W_Buffer[1];
    CARD8  R_Buffer[2];
    I2CDevPtr dev;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv2\n"));

    if (!xf86I2CProbeAddress(pVia->I2C_Port2, 0xA2))
	return FALSE;

    dev = xf86CreateI2CDevRec();
    if (!dev)
	return FALSE;

    dev->DevName = "EDID2";
    dev->SlaveAddr = 0xA2;
    dev->ByteTimeout = 2200; /* VESA DDC spec 3 p. 43 (+10 %) */
    dev->StartTimeout = 550;
    dev->BitTimeout = 40;
    dev->ByteTimeout = 40;
    dev->AcknTimeout = 40;
    dev->pI2CBus = pVia->I2C_Port2;
    
    if (!xf86I2CDevInit(dev)) {
	xf86DestroyI2CDevRec(dev,TRUE);
	return FALSE;
    }

    W_Buffer[0] = 0;
    xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,1);
    if (R_Buffer[0] != 0x20) {
	xf86DestroyI2CDevRec(dev,TRUE);
	return FALSE;
    }

    /* Found EDID2 Table */

    W_Buffer[0] = 0x76;
    xf86I2CWriteRead(dev, W_Buffer,1, R_Buffer,2);
    xf86DestroyI2CDevRec(dev,TRUE);

    *size = R_Buffer[0] | (R_Buffer[1] << 8);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv2: %d\n", *size));
    return TRUE;
}

/*
 *
 */
static void
VIAGetPanelSize(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int size = 0;
    Bool ret;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSize\n"));

    ret = VIAGetPanelSizeFromDDCv1(pScrn, &size);
    if (!ret)
	ret = VIAGetPanelSizeFromDDCv2(pScrn, &size);
    
    if (ret) {
	switch (size) {
	case 640:
	    pBIOSInfo->PanelSize = VIA_PANEL6X4;
	    break;
	case 800:
	    pBIOSInfo->PanelSize = VIA_PANEL8X6;
	    break;
	case 1024:
	    pBIOSInfo->PanelSize = VIA_PANEL10X7;
	    break;
	case 1280:
	    pBIOSInfo->PanelSize = VIA_PANEL12X10;
	    break;
	case 1400:
	    pBIOSInfo->PanelSize = VIA_PANEL14X10;
	    break;
	case 1600:
	    pBIOSInfo->PanelSize = VIA_PANEL16X12;
	    break;
	default:
	    pBIOSInfo->PanelSize = VIA_PANEL_INVALID;
		break;
	}
    } else {
	pBIOSInfo->PanelSize = hwp->readCrtc(hwp, 0x3F) >> 4;
	if (pBIOSInfo->PanelSize == 0) {
	    /* VIA_PANEL6X4 == 0, but that value equals unset */
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to "
		       "retrieve PanelSize: using default (1024x768)\n");
	    pBIOSInfo->PanelSize = VIA_PANEL10X7;
	}
    }
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "PanelSize = %d\n", 
		     pBIOSInfo->PanelSize));
}

/*
 * Sadly, we need to enforce these 17 modes, hard.
 * At least until we can decide if we are able to use
 * a given refresh algorithmically.
 */
static Bool
ViaGetResolutionIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetResolutionIndex\n"));
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetResolutionIndex: Looking"
		     " for %dx%d\n", mode->CrtcHDisplay, mode->CrtcVDisplay));
    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++) {
	if ((ViaResolutionTable[i].X == mode->CrtcHDisplay) && 
	    (ViaResolutionTable[i].Y == mode->CrtcVDisplay)){
	    pBIOSInfo->ResolutionIndex = ViaResolutionTable[i].Index;
	    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetResolutionIndex:"
			     " %d\n", pBIOSInfo->ResolutionIndex));
	    return TRUE;
 	}
    }
    
    pBIOSInfo->ResolutionIndex = VIA_RES_INVALID;
    return FALSE;
}

/*
 *
 */
static Bool
ViaGetModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetModeIndex\n"));

    for (i = 0; i < pViaModeTable->NumModes; i++)
	if ((pViaModeTable->Modes[i].Bpp == pScrn->bitsPerPixel) &&
	    (pViaModeTable->Modes[i].Width == mode->CrtcHDisplay) &&
	    (pViaModeTable->Modes[i].Height == mode->CrtcVDisplay)) {
	    pBIOSInfo->ModeIndex = i;
	    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetModeIndex:"
			     " %d\n", pBIOSInfo->ModeIndex));
	    return TRUE;
	}
    pBIOSInfo->ModeIndex = -1;
    return FALSE;
}

 /*
  *
  * ViaModeIndexTable[i].PanelIndex is pBIOSInfo->PanelSize 
  * pBIOSInfo->PanelIndex is the index to lcdTable.
  *
  */
static Bool
ViaPanelGetIndex(VIABIOSInfoPtr pBIOSInfo)
{
    VIALCDModePtr Table = pBIOSInfo->pModeTable->lcdTable;
    int i;

    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaPanelGetIndex\n"));

    pBIOSInfo->PanelIndex = VIA_BIOS_NUM_PANEL;

    if (pBIOSInfo->PanelSize == VIA_PANEL_INVALID){
	xf86DrvMsg(pBIOSInfo->scrnIndex, X_ERROR, "ViaPanelGetIndex: PanelSize not set.\n");
	return FALSE;
    }

    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++)
	if (ViaResolutionTable[i].PanelIndex == pBIOSInfo->PanelSize) {
	    pBIOSInfo->panelX = ViaResolutionTable[i].X;
	    pBIOSInfo->panelY = ViaResolutionTable[i].Y;
	    break;
	}

    if (ViaResolutionTable[i].Index == VIA_RES_INVALID) {
	xf86DrvMsg(pBIOSInfo->scrnIndex, X_ERROR, "ViaPanelGetIndex: Unable"
		   " to find matching PanelSize in ViaResolutionTable.\n");
	return FALSE;
    }


    for (i = 0; i < VIA_BIOS_NUM_PANEL; i++)
	if (Table[i].fpSize == pBIOSInfo->PanelSize) {
	    int modeNum, tmp;
	    
	    modeNum = (int)pBIOSInfo->pModeTable->Modes[pBIOSInfo->ModeIndex].Mode;
	    
	    tmp = 0x01 << (modeNum & 0xF);
	    if ((CARD16)(tmp) &
		pBIOSInfo->pModeTable->lcdTable[i].SuptMode[(modeNum >> 4)]) {
		pBIOSInfo->PanelIndex = i;
		DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaPanelGetIndex:"
				 "index: %d (%dx%d)\n", pBIOSInfo->PanelIndex,
				 pBIOSInfo->panelX, pBIOSInfo->panelY));
		return TRUE;
	    }
	    
	    xf86DrvMsg(pBIOSInfo->scrnIndex, X_ERROR, "ViaPanelGetIndex: Unable"
		       " to match given mode with this PanelSize.\n");
	    return FALSE;
	}

    xf86DrvMsg(pBIOSInfo->scrnIndex, X_ERROR, "ViaPanelGetIndex: Unable"
		   " to match PanelSize with an lcdTable entry.\n");
    return FALSE;
}

/*
 *
 */
static void
ViaGetNearestRefresh(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    int refresh, i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetNearestRefresh\n"));

    /* use the monitor information & round */
    refresh = (mode->VRefresh + 0.5);
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetNearestRefresh: "
		     "preferred: %d\n", refresh));

    /* get closest matching refresh index */
    if (refresh < supportRef[0])
	pBIOSInfo->RefreshIndex = 0;
    else {
	for (i = 0; (i <  VIA_NUM_REFRESH_RATE) && (refresh >= supportRef[i]); i++)
	    ;
	pBIOSInfo->RefreshIndex = i - 1;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetNearestRefresh: Refresh:"
		     " %d (index: %d)\n", supportRef[pBIOSInfo->RefreshIndex],
		     pBIOSInfo->RefreshIndex));
}

/*
 *
 */
static Bool
ViaRefreshAllowed(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int bppIndex;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaRefreshAllowed\n"));

    /* does the memory bandwidth support this refresh rate? */
    switch (pScrn->bitsPerPixel) {
    case 8:
	bppIndex = 0;
	break;
    case 16:
	bppIndex = 1;
	break;
    case 24:
    case 32:
	bppIndex = 2;
	break;
    default:
	bppIndex = 0;
    }
    
    switch (pVia->MemClk) {
    case VIA_MEM_SDR133:
	if (SDR133[bppIndex][pBIOSInfo->ResolutionIndex][pBIOSInfo->RefreshIndex])
	    return TRUE;
	else
	    return FALSE;
    case VIA_MEM_DDR200:
	if (DDR200[bppIndex][pBIOSInfo->ResolutionIndex][pBIOSInfo->RefreshIndex])
	    return TRUE;
	else
	    return FALSE;
    case VIA_MEM_DDR266:
    case VIA_MEM_DDR333:
    case VIA_MEM_DDR400:
	if (DDR266[bppIndex][pBIOSInfo->ResolutionIndex][pBIOSInfo->RefreshIndex])
	    return TRUE;
	else
	    return FALSE;

    case VIA_MEM_SDR66:
    case VIA_MEM_SDR100:
    default:
	if (SDR100[bppIndex][pBIOSInfo->ResolutionIndex][pBIOSInfo->RefreshIndex])
	    return TRUE;
	else
	    return FALSE;
    }
}

/*
 *
 */
static Bool
ViaTVGetIndex(VIABIOSInfoPtr pBIOSInfo)
{
    int i;

    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaTVGetIndex\n"));
    
    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++)
	if (ViaResolutionTable[i].Index == pBIOSInfo->ResolutionIndex) {
	    /* check tv standard */
	    if ((pBIOSInfo->ResolutionIndex == VIA_TVRES_720X480) &&
		(pBIOSInfo->TVType == TVTYPE_PAL))
		break;
	    if ((pBIOSInfo->ResolutionIndex == VIA_TVRES_720X576) &&
		(pBIOSInfo->TVType == TVTYPE_NTSC))
		break;
	    
	    /* check encoder */
	    if ((pBIOSInfo->TVEncoder == VIA_TV2PLUS) &&
		((pBIOSInfo->ResolutionIndex != VIA_RES_640X480) &&
		 (pBIOSInfo->ResolutionIndex != VIA_RES_800X600)))
		break;

	    pBIOSInfo->TVIndex = ViaResolutionTable[i].TVIndex;
	    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaTVGetIndex:"
			     " %d\n", pBIOSInfo->TVIndex));
	    return TRUE;
	}
    pBIOSInfo->TVIndex = VIA_TVRES_INVALID;
    return FALSE;
}

/*
 *
 */
Bool
VIAFindModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAFindModeUseBIOSTable\n"));

    if (!ViaGetModeIndex(pScrn, mode) || !ViaGetResolutionIndex(pScrn, mode)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Mode not supported.\n");
	return FALSE;
    }

    ViaGetNearestRefresh(pScrn, mode);
    if (!ViaRefreshAllowed(pScrn)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Refreshrate too high for"
		   " available memory bandwidth.\n");
	return FALSE;
    }

    if (!pBIOSInfo->ActiveDevice)
        pBIOSInfo->ActiveDevice = VIAGetDeviceDetect(pScrn);
    
    /* TV + LCD/DVI has no simultaneous, block it */
    if ((pBIOSInfo->ActiveDevice & VIA_DEVICE_TV)
	&& (pBIOSInfo->ActiveDevice & (VIA_DEVICE_LCD | VIA_DEVICE_DFP)))
        pBIOSInfo->ActiveDevice = VIA_DEVICE_TV;

    if ((pBIOSInfo->ActiveDevice & VIA_DEVICE_TV)) {
	if (!ViaTVGetIndex(pBIOSInfo)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Mode not supported by TV encoder.\n");
	    return FALSE;
	}
    }
	
    if (pBIOSInfo->ActiveDevice & (VIA_DEVICE_DFP | VIA_DEVICE_LCD)) {
	if (pBIOSInfo->PanelSize == VIA_PANEL_INVALID)
	    VIAGetPanelSize(pScrn);
	
	if (!ViaPanelGetIndex(pBIOSInfo)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Mode not supported by LCD/DFP.\n");
	    return FALSE;
	}
    }
    
    return TRUE;
}

/*
 *
 * Some very common abstractions.
 *
 */

/*
 * Standard vga call really.
 */
static void
VIASetUseExternalClock(vgaHWPtr hwp)
{
    CARD8 data;

    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VIASetUseExternalClock\n"));

    data = hwp->readMiscOut(hwp);
    hwp->writeMiscOut(hwp, data | 0x0C);
}

/* 
 *
 */
static void
VIASetPrimaryClock(vgaHWPtr hwp, CARD16 clock)
{
    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VIASetPrimaryClock to 0x%X\n", clock));

    ViaCrtcChange(hwp, 0x17, 0x00, 0x80); /* desync */

    hwp->writeSeq(hwp, 0x46, clock >> 8);
    hwp->writeSeq(hwp, 0x47, clock & 0xFF);

    ViaCrtcChange(hwp, 0x17, 0x80, 0x80); /* sync */

    ViaSeqChange(hwp, 0x40, 0x02, 0x02);
    ViaSeqChange(hwp, 0x40, 0x00, 0x02);
}

/* 
 *
 */
static void
VIASetSecondaryClock(vgaHWPtr hwp, CARD16 clock)
{
    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VIASetSecondaryClock to 0x%X\n", clock));

    ViaCrtcChange(hwp, 0x17, 0x00, 0x80); /* desync */

    hwp->writeSeq(hwp, 0x44, clock >> 8);
    hwp->writeSeq(hwp, 0x45, clock & 0xFF);

    ViaCrtcChange(hwp, 0x17, 0x80, 0x80); /* sync */

    ViaSeqChange(hwp, 0x40, 0x04, 0x04);
    ViaSeqChange(hwp, 0x40, 0x00, 0x04);
}

static void
VIASetLCDMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
    VIALCDModePtr LCDTable;
    CARD8           modeNum = 0;
    int             resIdx;
    int             port, offset, data;
    int             i, j, misc;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASetLCDMode\n"));

    LCDTable = &(pViaModeTable->lcdTable[pBIOSInfo->PanelIndex]);

    if (pBIOSInfo->PanelSize == VIA_PANEL12X10)
	hwp->writeCrtc(hwp, 0x89, 0x07);

    /* LCD Expand Mode Y Scale Flag */
    pBIOSInfo->scaleY = FALSE;

    /* Set LCD InitTb Regs */
    if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	if (!pVia->IsSecondary)
	    VIASetPrimaryClock(hwp, LCDTable->InitTb.VClk_12Bit);
	VIASetSecondaryClock(hwp, LCDTable->InitTb.LCDClk_12Bit);
    } else {
	if (!pVia->IsSecondary)
	    VIASetPrimaryClock(hwp, LCDTable->InitTb.VClk);
	VIASetSecondaryClock(hwp, LCDTable->InitTb.LCDClk);
    }

    VIASetUseExternalClock(hwp);

    for (i = 0; i < LCDTable->InitTb.numEntry; i++) {
        port = LCDTable->InitTb.port[i];
        offset = LCDTable->InitTb.offset[i];
        data = LCDTable->InitTb.data[i];
	ViaVgahwWrite(hwp, 0x300+port, offset, 0x301+port, data);
    }

    if ((mode->CrtcHDisplay != pBIOSInfo->panelX) ||
        (mode->CrtcVDisplay != pBIOSInfo->panelY)) {
	VIALCDModeEntryPtr Main;
	VIALCDMPatchEntryPtr Patch1, Patch2;

        resIdx = VIA_RES_INVALID;

        /* Find MxxxCtr & MxxxExp Index and
         * HWCursor Y Scale (PanelSize Y / Res. Y) */
        pBIOSInfo->resY = mode->CrtcVDisplay;
        switch (pBIOSInfo->ResolutionIndex) {
            case VIA_RES_640X480:
                resIdx = 0;
                break;
            case VIA_RES_800X600:
                resIdx = 1;
                break;
            case VIA_RES_1024X768:
                resIdx = 2;
                break;
            case VIA_RES_1152X864:
                resIdx = 3;
                break;
            case VIA_RES_1280X768:
            case VIA_RES_1280X960:
            case VIA_RES_1280X1024:
                if (pBIOSInfo->PanelSize == VIA_PANEL12X10)
                    resIdx = VIA_RES_INVALID;
                else
                    resIdx = 4;
                break;
            default:
                resIdx = VIA_RES_INVALID;
                break;
        }

        if ((mode->CrtcHDisplay == 640) &&
            (mode->CrtcVDisplay == 400))
            resIdx = 0;

	if (resIdx == VIA_RES_INVALID) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "VIASetLCDMode: Failed "
		       "to find a suitable Panel Size index.\n");
	    return;
	}

	if (pBIOSInfo->Center) {
	    Main = &(LCDTable->MCtr[resIdx]);
	    Patch1 = LCDTable->MPatchDP1Ctr;
	    Patch2 = LCDTable->MPatchDP2Ctr;
	} else { /* expand! */
            /* LCD Expand Mode Y Scale Flag */
            pBIOSInfo->scaleY = TRUE;
	    Main = &(LCDTable->MExp[resIdx]);
	    Patch1 = LCDTable->MPatchDP1Exp;
	    Patch2 = LCDTable->MPatchDP2Exp;
        }
	
	/* Set Main LCD Registers */
	for (i = 0; i < Main->numEntry; i++){
	    ViaVgahwWrite(hwp, 0x300 + Main->port[i], Main->offset[i],
			  0x301 + Main->port[i], Main->data[i]);
	}
	
	if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    if (!pVia->IsSecondary)
		VIASetPrimaryClock(hwp, Main->VClk_12Bit);
	    VIASetSecondaryClock(hwp, Main->LCDClk_12Bit);
	} else {
	    if (!pVia->IsSecondary)
		VIASetPrimaryClock(hwp, Main->VClk);
	    VIASetSecondaryClock(hwp, Main->LCDClk);
	}

	for (i = 0; i < pViaModeTable->modeFix.numEntry; i++) {
	    if (pViaModeTable->modeFix.reqMode[i] == 
		(CARD8)(pViaModeTable->Modes[pBIOSInfo->ModeIndex].Mode)) {
		modeNum = pViaModeTable->modeFix.fixMode[i];
		break;
	    }
	}

	/* Set LCD Mode patch registers. */
	for (i = 0; i < LCDTable->numMPatchDP2Exp; i++, Patch2++) {
	    if (Patch2->Mode == modeNum) {
		if (!pBIOSInfo->Center && (mode->CrtcHDisplay == pBIOSInfo->panelX))
		    pBIOSInfo->scaleY = FALSE;
		
		for (j = 0; j < Patch2->numEntry; j++){
		    ViaVgahwWrite(hwp, 0x300 + Patch2->port[j], Patch2->offset[j], 
				  0x301 + Patch2->port[j], Patch2->data[j]);
		}
		
		if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
		    if (!pVia->IsSecondary)
			VIASetPrimaryClock(hwp, Patch2->VClk_12Bit);
		    VIASetSecondaryClock(hwp, Patch2->LCDClk_12Bit);
		} else {
		    if (!pVia->IsSecondary)
			VIASetPrimaryClock (hwp, Patch2->VClk);
		    VIASetSecondaryClock(hwp, Patch2->LCDClk);
		}
		break;
	    }
	}
	

	/* Set LCD Secondary Mode Patch registers. */
	if (pVia->IsSecondary) {
	    for (i = 0; i < LCDTable->numMPatchDP1Ctr; i++, Patch1++) {
		if (Patch1->Mode == modeNum) {
		    for (j = 0; j < Patch1->numEntry; j++) {
			ViaVgahwWrite(hwp, 0x300 + Patch1->port[j], Patch1->offset[j],
				      0x301 + Patch1->port[j], Patch1->data[j]);
		    }
		    break;
		}
	    }
	}
    }

    VIASetUseExternalClock(hwp);

    /* LCD patch 3D5.02 */
    misc = hwp->readCrtc(hwp, 0x01);
    hwp->writeCrtc(hwp, 0x02, misc);

    /* Enable LCD */
    if (!pVia->IsSecondary) {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
	ViaSeqChange(hwp, 0x16, 0x40, 0x40);

        /* Enable Simultaneous */
        if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    hwp->writeCrtc(hwp, 0x6B, 0xA8);

            if ((pVia->Chipset == VIA_CLE266) && 
		CLE266_REV_IS_AX(pVia->ChipRev))
		hwp->writeCrtc(hwp, 0x93, 0xB1);
            else
		hwp->writeCrtc(hwp, 0x93, 0xAF);
        } else {
	    ViaCrtcChange(hwp, 0x6B, 0x08, 0x08);
	    hwp->writeCrtc(hwp, 0x93, 0x00);
        }
	hwp->writeCrtc(hwp, 0x6A, 0x48);
    } else {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
	ViaSeqChange(hwp, 0x16, 0x00, 0x40);

        /* Enable SAMM */
        if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    ViaCrtcChange(hwp, 0x6B, 0x20, 0x20);
            if ((pVia->Chipset == VIA_CLE266) &&
		CLE266_REV_IS_AX(pVia->ChipRev))
		hwp->writeCrtc(hwp, 0x93, 0xB1);
            else
		hwp->writeCrtc(hwp, 0x93, 0xAF);
        } else {
	    hwp->writeCrtc(hwp, 0x6B, 0x00);
	    hwp->writeCrtc(hwp, 0x93, 0x00);
        }
	hwp->writeCrtc(hwp, 0x6A, 0xC8);
    }
}

static void
VIAPreSetTV2Mode(VIAPtr pVia)
{
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIABIOSTV2TablePtr Table;
    VIABIOSTVMASKTablePtr Mask;
    CARD8           *TV;
    CARD16          *Patch2;
    int             i, j;
    CARD8   W_Buffer[VIA_BIOS_MAX_NUM_TV_REG+1];
    CARD8   W_Other[2];
    I2CDevPtr       dev;
    CARD8   R_Buffer[1];

    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "VIAPreSetTV2Mode\n"));

    if (pBIOSInfo->TVVScan == VIA_TVOVER)
	Table = &(pBIOSInfo->pModeTable->tv2OverTable[pBIOSInfo->TVIndex]);
    else /* VIA_TVNORMAL */
	Table = &(pBIOSInfo->pModeTable->tv2Table[pBIOSInfo->TVIndex]);
    Mask = &(pBIOSInfo->pModeTable->tv2MaskTable);

    if (pBIOSInfo->TVType == TVTYPE_PAL) {
	Patch2 = Table->PatchPAL2;
	if (pBIOSInfo->TVOutput == TVOUTPUT_COMPOSITE)
	    TV = Table->TVPALC;
	else /* S-video */
	    TV = Table->TVPALS;
    } else { /* TVTYPE_NTSC */
	Patch2 = Table->PatchNTSC2;
	if (pBIOSInfo->TVOutput == TVOUTPUT_COMPOSITE)
	    TV = Table->TVNTSCC;
	else /* S-video */
	    TV = Table->TVNTSCS;
    }

    dev = xf86CreateI2CDevRec();
    dev->DevName = "VT1621";
    dev->SlaveAddr = 0x40;
    dev->pI2CBus = pVia->I2C_Port2;

    xf86I2CDevInit(dev);

    W_Buffer[0] = 0;
    for (i = 0, j = 0; (j < Mask->numTV) && (i < VIA_BIOS_MAX_NUM_TV_REG); i++) {
	if (Mask->TV[i] == 0xFF) {
	    W_Buffer[i + 1] = TV[i];
	    j++;
	} else 
	    W_Buffer[i + 1] = pBIOSInfo->TVRegs[i];
    }
    
    xf86I2CWriteRead(dev, W_Buffer, i + 1, NULL,0);

    /* Turn on all Composite and S-Video output */
    W_Other[0] = 0x0E;
    W_Other[1] = 0;
    xf86I2CWriteRead(dev, W_Other,2, NULL,0);

    if ((pBIOSInfo->TVType == TVTYPE_NTSC) && pBIOSInfo->TVDotCrawl) {
	CARD16 *DotCrawl = Table->DotCrawlNTSC;
	
	for (i = 1; i < (DotCrawl[0] + 1); i++) {
	    W_Other[0] = (CARD8)(DotCrawl[i] & 0xFF);
	    
	    if (W_Other[0] == 0x11) {
		xf86I2CWriteRead(dev, W_Other,1, R_Buffer,1);
		W_Other[1] = R_Buffer[0] | (CARD8)(DotCrawl[i] >> 8);
	    } else
		W_Other[1] = (CARD8)(DotCrawl[i] >> 8);
	    
	    xf86I2CWriteRead(dev, W_Other,2, NULL,0);
	}
    }

    if (pVia->IsSecondary) { /* Patch as setting 2nd path */
        int numPatch = (int)(Mask->misc2 >> 5);

        for (i = 0; i < numPatch; i++) {
            W_Other[0] = (CARD8)(Patch2[i] & 0xFF);
            W_Other[1] = (CARD8)(Patch2[i] >> 8);
            xf86I2CWriteRead(dev, W_Other,2, NULL,0);
        }
    }

    xf86DestroyI2CDevRec(dev,TRUE);
}

static void
VIAPreSetVT1623Mode(VIAPtr pVia)
{
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIABIOSTV3TablePtr Table;
    VIABIOSTVMASKTablePtr Mask;
    CARD8   *TV;
    CARD16  *RGB, *YCbCr, *Patch2;
    int     i, j;
    CARD8   W_Buffer[2];
    CARD8   R_Buffer[1];
    GpioI2cPtr pDev = &(pVia->GpioI2c);

    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "VIAPreSetVT1623Mode\n"));

    if (pBIOSInfo->TVVScan == VIA_TVOVER)
	Table = &(pBIOSInfo->pModeTable->vt1622aOverTable[pBIOSInfo->TVIndex]);
    else /* VIA_TVNORMAL */
	Table = &(pBIOSInfo->pModeTable->vt1622aTable[pBIOSInfo->TVIndex]);
    Mask = &(pBIOSInfo->pModeTable->vt1622aMaskTable);

    if (pBIOSInfo->TVType == TVTYPE_PAL) {
	TV = Table->TVPAL;
	RGB = Table->RGBPAL;
	YCbCr = Table->YCbCrPAL;
	Patch2 = Table->PatchPAL2;
    } else {
	TV = Table->TVNTSC;
	RGB = Table->RGBNTSC;
	YCbCr = Table->YCbCrNTSC;
	Patch2 = Table->PatchNTSC2;
    }

    VIAGPIOI2C_Initial(pDev, 0x40);

    /* TV Reset */
    VIAGPIOI2C_Write(pDev, 0x1D, 0x0);
    VIAGPIOI2C_Write(pDev, 0x1D, 0x80);

    VIAGPIOI2C_Write(pDev, 0, 0);
    for (i = 1, j = 0; (j < Mask->numTV) && (i < VIA_BIOS_MAX_NUM_TV_REG); i++) {
	if (Mask->TV[i] == 0xFF) {
	    VIAGPIOI2C_Write(pDev, i + 1, TV[j]);
	    j++;
	} else 
	    VIAGPIOI2C_Write(pDev, i + 1, pBIOSInfo->TVRegs[j]);
    }

    /* Turn on all Composite and S-Video output */
    VIAGPIOI2C_Write(pDev, 0x0E, 0x0);

    if ((pBIOSInfo->TVType == TVTYPE_NTSC) && pBIOSInfo->TVDotCrawl) {
	CARD16 *DotCrawl = Table->DotCrawlNTSC;

        for (i = 1; i < (DotCrawl[0] + 1); i++) {
            W_Buffer[0] = (CARD8)(DotCrawl[i] & 0xFF);
            if (W_Buffer[0] == 0x11) {
                VIAGPIOI2C_Read(pDev, 0x11, R_Buffer, 1);
                W_Buffer[1] = R_Buffer[0] | (CARD8)(DotCrawl[i] >> 8);
            }
            else {
                W_Buffer[1] = (CARD8)(DotCrawl[i] >> 8);
            }
            VIAGPIOI2C_Write(pDev, W_Buffer[0], W_Buffer[1]);
        }
    }

    if (pBIOSInfo->TVOutput == TVOUTPUT_RGB) {
        for (i = 1; i < (RGB[0] + 1); i++) {
            W_Buffer[0] = (CARD8)(RGB[i] & 0xFF);
            W_Buffer[1] = (CARD8)(RGB[i] >> 8);
            VIAGPIOI2C_Write(pDev, W_Buffer[0], W_Buffer[1]);
        }

    } else if (pBIOSInfo->TVOutput == TVOUTPUT_YCBCR) {
        for (i = 1; i < (YCbCr[0] + 1); i++) {
            W_Buffer[0] = (CARD8)(YCbCr[i] & 0xFF);
            W_Buffer[1] = (CARD8)(YCbCr[i] >> 8);
            VIAGPIOI2C_Write(pDev, W_Buffer[0], W_Buffer[1]);
        }

    }

    if (pVia->IsSecondary) { /* Patch as setting 2nd path */
        int numPatch = (int)(Mask->misc2 >> 5);

        for (i = 0; i < numPatch; i++) {
            W_Buffer[0] = (CARD8)(Patch2[i] & 0xFF);
            W_Buffer[1] = (CARD8)(Patch2[i] >> 8);
            VIAGPIOI2C_Write(pDev, W_Buffer[0], W_Buffer[1]);
        }
    }
}

static void
VIAPostSetTV2Mode(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia =  VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIABIOSTV2TablePtr Table;
    VIABIOSTVMASKTablePtr Mask;
    CARD8           *CRTC, *Misc;
    int             i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAPostSetTV2Mode\n"));

    if (pBIOSInfo->TVVScan == VIA_TVOVER)
	Table = &(pBIOSInfo->pModeTable->tv2OverTable[pBIOSInfo->TVIndex]);
    else /* VIA_TVNORMAL */
	Table = &(pBIOSInfo->pModeTable->tv2Table[pBIOSInfo->TVIndex]);
    Mask = &(pBIOSInfo->pModeTable->tv2MaskTable);

    if (pVia->IsSecondary) {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table->CRTCPAL2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table->CRTCPAL2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table->CRTCPAL2_8BPP;
		break;
	    }
	    Misc = Table->MiscPAL2;
	} else {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table->CRTCNTSC2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table->CRTCNTSC2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table->CRTCNTSC2_8BPP;
		break;
	    }
	    Misc = Table->MiscNTSC2;
	}

        for (i = 0, j = 0; i < Mask->numCRTC2; j++) {
            if (Mask->CRTC2[j] == 0xFF) {
		hwp->writeCrtc(hwp, j + 0x50, CRTC[j]);
                i++;
            }
        }

        if (Mask->misc2 & 0x18) {
	    VIASetSecondaryClock(hwp, (Misc[3] << 8) | Misc[4]);
	    VIASetUseExternalClock(hwp);
        }

	ViaCrtcChange(hwp, 0x6A, 0xC0, 0xC0);
	ViaCrtcChange(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcChange(hwp, 0x6C, 0x01, 0x01);

        /* Disable LCD Scaling */
    	if (!pVia->SAMM || pVia->FirstInit)
	    hwp->writeCrtc(hwp, 0x79, 0x00);

    } else {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    CRTC = Table->CRTCPAL1;
	    Misc = Table->MiscPAL1;
	} else {
	    CRTC = Table->CRTCNTSC1;
	    Misc = Table->MiscNTSC1;
	}

        for (i = 0, j = 0; i < Mask->numCRTC1; j++) {
            if (Mask->CRTC1[j] == 0xFF) {
		hwp->writeCrtc(hwp, j, CRTC[j]);
                i++;
            }
        }

	ViaCrtcChange(hwp, 0x33, Misc[0], 0x20);
	hwp->writeCrtc(hwp, 0x6A, Misc[1]);
	hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x01);
	hwp->writeCrtc(hwp, 0x6C, Misc[3] | 0x01); /* ? */
	
        if (Mask->misc1 & 0x30) {
            VIASetPrimaryClock(hwp, (Misc[4] << 8) | Misc[5]);
	    VIASetUseExternalClock(hwp);
	}
    }
}

static void
VIAPreSetTV3Mode(VIAPtr pVia)
{
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIABIOSTV3TablePtr Table;
    VIABIOSTVMASKTablePtr Mask;
    CARD8           *TV;
    CARD16          *RGB, *YCbCr, *Patch2;
    int             i, j;
    CARD8   W_Buffer[VIA_BIOS_MAX_NUM_TV_REG + 1];
    CARD8   W_Other[2];
    I2CDevPtr       dev;
    CARD8   R_Buffer[1];

    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "VIAPreSetTV3Mode\n"));

    if (pBIOSInfo->TVEncoder == VIA_TV3) {
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = &(pBIOSInfo->pModeTable->tv3OverTable[pBIOSInfo->TVIndex]);
	else /* VIA_TVNORMAL */
	    Table = &(pBIOSInfo->pModeTable->tv3Table[pBIOSInfo->TVIndex]);
	Mask = &(pBIOSInfo->pModeTable->tv3MaskTable);
    } else { /* VT1622A */
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = &(pBIOSInfo->pModeTable->vt1622aOverTable[pBIOSInfo->TVIndex]);
	else /* VIA_TVNORMAL */
	    Table = &(pBIOSInfo->pModeTable->vt1622aTable[pBIOSInfo->TVIndex]);
	Mask = &(pBIOSInfo->pModeTable->vt1622aMaskTable);
    }

    if (pBIOSInfo->TVType == TVTYPE_PAL) {
	TV = Table->TVPAL;
	RGB = Table->RGBPAL;
	YCbCr = Table->YCbCrPAL;
	Patch2 = Table->PatchPAL2;
    } else {
	TV = Table->TVNTSC;
	RGB = Table->RGBNTSC;
	YCbCr = Table->YCbCrNTSC;
	Patch2 = Table->PatchNTSC2;
    }

    dev = xf86CreateI2CDevRec();
    dev->DevName = "VT1622";
    dev->SlaveAddr = 0x40;
    dev->pI2CBus = pVia->I2C_Port2;

    xf86I2CDevInit(dev);

    W_Buffer[0] = 0;
    for (i = 0, j = 0; (j < Mask->numTV) && (i < VIA_BIOS_MAX_NUM_TV_REG); i++) {
	if (Mask->TV[i] == 0xFF) {
	    W_Buffer[i + 1] = TV[i];
	    j++;
	} else 
	    W_Buffer[i + 1] = pBIOSInfo->TVRegs[i];
    }

    /* TV Reset */
    W_Other[0] = 0x1D;
    W_Other[1] = 0;
    xf86I2CWriteRead(dev, W_Other,2, NULL,0);
    W_Other[0] = 0x1D;
    W_Other[1] = 0x80;
    xf86I2CWriteRead(dev, W_Other,2, NULL,0);

    xf86I2CWriteRead(dev, W_Buffer, i + 1, NULL,0);

    /* Turn on all Composite and S-Video output */
    W_Other[0] = 0x0E;
    W_Other[1] = 0;
    xf86I2CWriteRead(dev, W_Other,2, NULL,0);

    if ((pBIOSInfo->TVType == TVTYPE_NTSC) && pBIOSInfo->TVDotCrawl) {
        CARD16 *DotCrawl = Table->DotCrawlNTSC;

	for (i = 1; i < (DotCrawl[0] + 1); i++) {
            W_Other[0] = (CARD8)(DotCrawl[i] & 0xFF);
            if (W_Other[0] == 0x11) {
                xf86I2CWriteRead(dev, W_Other,1, R_Buffer,1);
                W_Other[1] = R_Buffer[0] | (CARD8)(DotCrawl[i] >> 8);
            } 
            else {
                W_Other[1] = (CARD8)(DotCrawl[i] >> 8);
            }
            xf86I2CWriteRead(dev, W_Other,2, NULL,0);
        }
    }

    if (pBIOSInfo->TVOutput == TVOUTPUT_RGB) {
        for (i = 1; i < (RGB[0] + 1); i++) {
            W_Other[0] = (CARD8)(RGB[i] & 0xFF);
            W_Other[1] = (CARD8)(RGB[i] >> 8);
            xf86I2CWriteRead(dev, W_Other,2, NULL,0);
        }
    } else if (pBIOSInfo->TVOutput == TVOUTPUT_YCBCR) {
        for (i = 1; i < (YCbCr[0] + 1); i++) {
            W_Other[0] = (CARD8)(YCbCr[i] & 0xFF);
            W_Other[1] = (CARD8)(YCbCr[i] >> 8);
            xf86I2CWriteRead(dev, W_Other,2, NULL,0);
        }
    }

    if (pVia->IsSecondary) { /* Patch as setting 2nd path */
        int numPatch = (int)(Mask->misc2 >> 5);

        for (i = 0; i < numPatch; i++) {
            W_Other[0] = (CARD8)(Patch2[i] & 0xFF);
            W_Other[1] = (CARD8)(Patch2[i] >> 8);
            xf86I2CWriteRead(dev, W_Other,2, NULL,0);
        }
    }

    xf86DestroyI2CDevRec(dev,TRUE);
}

static void
VIAPostSetTV3Mode(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIABIOSTV3TablePtr Table;
    VIABIOSTVMASKTablePtr Mask;
    CARD8           *CRTC, *Misc;
    int             i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAPostSetTV3Mode\n"));

    if (pBIOSInfo->TVEncoder == VIA_TV3) {
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = &(pBIOSInfo->pModeTable->tv3OverTable[pBIOSInfo->TVIndex]);
	else /* VIA_TVNORMAL */
	    Table = &(pBIOSInfo->pModeTable->tv3Table[pBIOSInfo->TVIndex]);
	Mask = &(pBIOSInfo->pModeTable->tv3MaskTable);
    } else { /* VT1622A */
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = &(pBIOSInfo->pModeTable->vt1622aOverTable[pBIOSInfo->TVIndex]);
	else /* VIA_TVNORMAL */
	    Table = &(pBIOSInfo->pModeTable->vt1622aTable[pBIOSInfo->TVIndex]);
	Mask = &(pBIOSInfo->pModeTable->vt1622aMaskTable);
    }

    if (pVia->IsSecondary) {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table->CRTCPAL2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table->CRTCPAL2_32BPP;
		break;
	     case 8:
	    default:
		CRTC = Table->CRTCPAL2_8BPP;
		break;
	    }
	    Misc = Table->MiscPAL2;
	} else {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table->CRTCNTSC2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table->CRTCNTSC2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table->CRTCNTSC2_8BPP;
		break;	
	    }
	    Misc = Table->MiscNTSC2;
	}
	
        for (i = 0, j = 0; i < Mask->numCRTC2; j++) {
            if (Mask->CRTC2[j] == 0xFF) {
		hwp->writeCrtc(hwp, j + 0x50, CRTC[j]);
		i++;
            }
        }
	
        if (Mask->misc2 & 0x18) {
            /* CLE266Ax use 2x XCLK */
            if ((pVia->Chipset == VIA_CLE266) &&
                CLE266_REV_IS_AX(pVia->ChipRev)) {
		ViaCrtcChange(hwp, 0x6B, 0x20, 0x20);

		/* Fix TV clock Polarity for CLE266A2 */
                if (pVia->ChipRev == 0x02)
		    ViaCrtcChange(hwp, 0x6C, 0x1C, 0x1C);
		
		VIASetSecondaryClock(hwp, 0x471C);
            } else
		VIASetSecondaryClock(hwp, (Misc[3] << 8) | Misc[4]);
	    
	    VIASetUseExternalClock(hwp);
        }
	
	ViaCrtcChange(hwp, 0x6A, 0xC0, 0xC0);
	ViaCrtcChange(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcChange(hwp, 0x6C, 0x01, 0x01);
	
        /* Disable LCD Scaling */
        if (!pVia->SAMM || pVia->FirstInit)
	    hwp->writeCrtc(hwp, 0x79, 0x00);
    } else {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    CRTC = Table->CRTCPAL1;
	    Misc = Table->MiscPAL1;
	} else {
	    CRTC = Table->CRTCNTSC1;
	    Misc = Table->MiscNTSC1;
	}

	for (i = 0, j = 0; i < Mask->numCRTC1; j++) {
            if (Mask->CRTC1[j] == 0xFF) {
                hwp->writeCrtc(hwp, j, CRTC[j]);
                i++;
            }
        }

        ViaCrtcChange(hwp, 0x33, Misc[0], 0x20);
	hwp->writeCrtc(hwp, 0x6A, Misc[1]);

        if ((pVia->Chipset == VIA_CLE266) &&
	    CLE266_REV_IS_AX(pVia->ChipRev)) {
	    hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x81);
	    /* Fix TV clock Polarity for CLE266A2 */
            if (pVia->ChipRev == 0x02)
		hwp->writeCrtc(hwp, 0x6C, Misc[3] | 0x01);
        } else
	    hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x01);

        if (Mask->misc1 & 0x30) {
	    /* CLE266Ax use 2x XCLK */
	    if ((pVia->Chipset == VIA_CLE266) &&
		CLE266_REV_IS_AX(pVia->ChipRev))
		VIASetPrimaryClock(hwp, 0x471C);
	    else
		VIASetPrimaryClock(hwp, (Misc[4] << 8) | Misc[5]);

	    VIASetUseExternalClock(hwp);
        }

	ViaCrtcChange(hwp, 0x6A, 0x40, 0x40);
	ViaCrtcChange(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcChange(hwp, 0x6C, 0x01, 0x01);
    }

    ViaSeqChange(hwp, 0x1E, 0xC0, 0xC0); /* Enable DI0/DVP0 */
}

/*
 *
 */
static void
ViaTVModePreset(VIAPtr pVia)
{
    switch (pVia->pBIOSInfo->TVEncoder) {
    case VIA_TV2PLUS:
	VIAPreSetTV2Mode(pVia);
	break;
    case VIA_TV3:
    case VIA_VT1622A:
	VIAPreSetTV3Mode(pVia);
	break;
    case VIA_VT1623:
	VIAPreSetVT1623Mode(pVia);
	break;
    }
}

/*
 *
 */
static void
ViaTVClose(VIAPtr pVia)
{
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    I2CDevPtr       dev;
    CARD8   W_Buffer[2];
    
    dev = xf86CreateI2CDevRec();
    dev->DevName = "TV";
    dev->SlaveAddr = pBIOSInfo->TVI2CAdd;
    dev->pI2CBus = pVia->I2C_Port2;
    if (xf86I2CDevInit(dev)) {

	switch (pBIOSInfo->TVEncoder) {
	case VIA_TV2PLUS:
	    W_Buffer[0] = 0x0E;
	    W_Buffer[1] = 0x03;
	    xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
	    break;
	case VIA_TV3:
	case VIA_VT1622A:
	    W_Buffer[0] = 0x0E;
	    W_Buffer[1] = 0x0F;
	    xf86I2CWriteRead(dev, W_Buffer,2, NULL,0);
	    break;
	case VIA_VT1623:
	    VIAGPIOI2C_Initial(&(pVia->GpioI2c), 0x40);
	    VIAGPIOI2C_Write(&(pVia->GpioI2c), 0x0E, 0x0F);
	    break;
	default:
	    break;
	}
    }

    xf86DestroyI2CDevRec(dev,TRUE);
}

/*
 *
 */
static void
ViaTVModePostSet(ScrnInfoPtr pScrn)
{
    
    switch (VIAPTR(pScrn)->pBIOSInfo->TVEncoder) {
    case VIA_TV2PLUS:
	VIAPostSetTV2Mode(pScrn);
	break;
    case VIA_TV3:
    case VIA_VT1622A:
    case VIA_VT1623:
	VIAPostSetTV3Mode(pScrn);
	break;
    }
}

/*
 * Fetch count was moved to ViaSetPrimaryFetchCount (via_bandwidth.c)
 * "Fix bandwidth problem when using virtual desktop"
 * 
 */
static void
ViaSetPrimaryOffset(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    int Offset = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaSetPrimaryOffset\n"));
    
    /* Make sure that this is 32byte aligned */
    if (Offset & 0x03) {
 	Offset += 0x03;
 	Offset &= ~0x03;
    }
    hwp->writeCrtc(hwp, 0x13, Offset & 0xFF);
    ViaCrtcChange(hwp, 0x35, Offset >> 3, 0xE0);
}
 
/*
 * Patch for horizontal blanking end bit6
 */
static void
ViaSetBlankingEndOverflow(vgaHWPtr hwp)
{
    CARD8 start, end;
    
    
    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "ViaSetBlankingEndOverflow\n"));
    
    start = hwp->readCrtc(hwp, 0x02); /* Save Blanking Start */

    end = hwp->readCrtc(hwp, 0x03) & 0x1F; /* Save Blanking End bit[4:0] */
    end |= ((hwp->readCrtc(hwp, 0x05) & 0x80) >> 2); /* Blanking End bit[5:0] */
    
    if ((start & 0x3f) > end) { /* Is Blanking End overflow ? */
 	if (start & 0x40) /* Blanking Start bit6 = ? */
 	    ViaCrtcChange(hwp, 0x33, 0x00, 0x20); /* bit6 = 1, Blanking End bit6 = 0 */
 	else
 	    ViaCrtcChange(hwp, 0x33, 0x20, 0x20); /* bit6 = 0, Blanking End bit6 = 1 */
    } else {
 	if (start & 0x40) /* Blanking Start bit6 = ? */
 	    ViaCrtcChange(hwp, 0x33, 0x20, 0x20); /* bit6 = 1, Blanking End bit6 = 1 */
 	else
 	    ViaCrtcChange(hwp, 0x33, 0x00, 0x20); /* bit6 = 0, Blanking End bit6 = 0 */
    }
}

/*
 *
 */
static Bool
ViaPrimaryRefreshPatch(vgaHWPtr hwp, int Refresh, VIABIOSRefreshTablePtr Table)
{
    int index;
    
    for (index = 0; index < VIA_NUM_REFRESH_RATE; index++) {
	if (Table[index].refresh == Refresh) {
	    VIABIOSRefreshTableRec entry = Table[index];
	    VIASetPrimaryClock(hwp, entry.VClk);
	    
	    hwp->writeCrtc(hwp, 0x00, entry.CR[0]);
	    
	    hwp->writeCrtc(hwp, 0x02, entry.CR[1]);
	    hwp->writeCrtc(hwp, 0x03, entry.CR[2]);
	    hwp->writeCrtc(hwp, 0x04, entry.CR[3]);
	    hwp->writeCrtc(hwp, 0x05, entry.CR[4]);
	    hwp->writeCrtc(hwp, 0x06, entry.CR[5]);
	    hwp->writeCrtc(hwp, 0x07, entry.CR[6]);
	    
	    hwp->writeCrtc(hwp, 0x10, entry.CR[7]);
	    hwp->writeCrtc(hwp, 0x11, entry.CR[8]);
	    
	    hwp->writeCrtc(hwp, 0x15, entry.CR[9]);
	    hwp->writeCrtc(hwp, 0x16, entry.CR[10]);

	    VIASetUseExternalClock(hwp);
	    return TRUE;
	}
    }
    return FALSE;
}
    
/*
 *
 */
void
VIASetModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
    int             port, offset, mask, data;
    int             i;
    Bool            SetTV = FALSE;
    VIAModeEntryPtr Mode;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASetModeUseBIOSTable\n"));
    /* Turn off Screen */
    ViaCrtcChange(hwp, 0x17, 0x00, 0x80);

    /* Clean Second Path Status */
    hwp->writeCrtc(hwp, 0x6A, 0x00);
    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x93, 0x00);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Active Device is %d\n",
                     pBIOSInfo->ActiveDevice));

    if ((pBIOSInfo->ActiveDevice & VIA_DEVICE_TV)) {
	if (pBIOSInfo->TVIndex != VIA_TVRES_INVALID) {
	    SetTV = TRUE;
	    ViaTVModePreset(pVia);
	} else {
	    if (!pVia->SAMM) /* from via code */
		ViaTVClose(pVia);
	}
    }
    Mode = &(pViaModeTable->Modes[pBIOSInfo->ModeIndex]);

    /* Get Standard VGA Regs */
    /* Set Sequences regs */
    for (i = 1; i < 5; i++)
	hwp->writeSeq(hwp, i , Mode->stdVgaTable.SR[i]);

    /* Set Misc reg */
    hwp->writeMiscOut(hwp, Mode->stdVgaTable.misc);

    /* Set CRTC regs */
    for (i = 0; i < 25; i++)
	hwp->writeCrtc(hwp, i, Mode->stdVgaTable.CR[i]);

    /* Set attribute regs */
    for (i = 0; i < 20; i++)
	hwp->writeAttr(hwp, i, Mode->stdVgaTable.AR[i]);

    for (i = 0; i < 9; i++)
	hwp->writeGr(hwp, i, Mode->stdVgaTable.GR[i]);

    /* Unlock Extended regs */
    hwp->writeSeq(hwp, 0x10, 0x01);

    /* Set Commmon Ext. Regs */
    for (i = 0; i < pViaModeTable->commExtTable.numEntry; i++) {
        port = pViaModeTable->commExtTable.port[i];
        offset = pViaModeTable->commExtTable.offset[i];
        mask = pViaModeTable->commExtTable.mask[i];
        data = pViaModeTable->commExtTable.data[i] & mask;
	ViaVgahwWrite(hwp, 0x300+port, offset, 0x301+port, data);
    }

    if (Mode->Mode <= 0x13) {
        /* Set Standard Mode-Spec. Extend Regs */
        for (i = 0; i < pViaModeTable->stdModeExtTable.numEntry; i++) {
            port = pViaModeTable->stdModeExtTable.port[i];
            offset = pViaModeTable->stdModeExtTable.offset[i];
            mask = pViaModeTable->stdModeExtTable.mask[i];
            data = pViaModeTable->stdModeExtTable.data[i] & mask;
	    ViaVgahwWrite(hwp, 0x300+port, offset, 0x301+port, data);
        }
    } else {
        /* Set Extended Mode-Spec. Extend Regs */
        for (i = 0; i < Mode->extModeExtTable.numEntry; i++) {
            port = Mode->extModeExtTable.port[i];
            offset = Mode->extModeExtTable.offset[i];
            mask = Mode->extModeExtTable.mask[i];
            data = Mode->extModeExtTable.data[i] & mask;
	    ViaVgahwWrite(hwp, 0x300+port, offset, 0x301+port, data);
        }
    }

    /* ugly; we should set the standard modeclock if we don't need
       to patch for the refresh rate */
    /* ResMode & RefreshIndex should be correct at this stage. */
    if (((pBIOSInfo->ResolutionIndex == VIA_RES_INVALID) || 
	(pBIOSInfo->RefreshIndex == 0xFF) || SetTV
	|| !ViaPrimaryRefreshPatch(hwp, supportRef[pBIOSInfo->RefreshIndex],
				   pViaModeTable->refreshTable[pBIOSInfo->ResolutionIndex]))
	&& (Mode->VClk != 0)) {
	VIASetPrimaryClock(hwp, Mode->VClk);
	VIASetUseExternalClock(hwp);
    }

    ViaSetPrimaryOffset(pScrn);

    /* Enable MMIO & PCI burst (1 wait state) */
    ViaSeqChange(hwp, 0x1A, 0x06, 0x06);

    /* Enable modify CRTC starting address */
    ViaCrtcChange(hwp, 0x11, 0x00, 0x80);

    ViaSetBlankingEndOverflow(hwp);

    /* LCD Simultaneous Set Mode */
    if ((pBIOSInfo->ActiveDevice & (VIA_DEVICE_DFP | VIA_DEVICE_LCD))
	&& (pBIOSInfo->PanelIndex != VIA_BIOS_NUM_PANEL)) {
        VIASetLCDMode(pScrn, mode);
        VIAEnableLCD(pScrn);
    }
    else if ((pBIOSInfo->ConnectedDevice & (VIA_DEVICE_DFP | VIA_DEVICE_LCD)) &&
             (!pVia->HasSecondary)) {
        VIADisableLCD(pScrn);
    }

    if (SetTV)
        ViaTVModePostSet(pScrn);

    ViaSetPrimaryFIFO(pScrn, mode);

    /* Enable CRT Controller (3D5.17 Hardware Reset) */
    ViaCrtcChange(hwp, 0x17, 0x80, 0x80);

    /* Turn off CRT, if user doesn't want crt on */
    if (!(pBIOSInfo->ActiveDevice & VIA_DEVICE_CRT1))
	ViaCrtcChange(hwp, 0x36, 0x30, 0x30);

    /* Open Screen */ /* -- wrong -- this is your bogstandard vga palette disable */
    hwp->disablePalette(hwp);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-- VIASetModeUseBIOSTable Done!\n"));
}

/*
 *
 */
static void
ViaSetSecondaryOffset(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 Offset = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaSetSecondaryOffset\n"));

    /* Make sure that this is 32byte aligned */
    if (Offset & 0x03) {
	Offset += 0x03;
	Offset &= ~0x03;
    }
    hwp->writeCrtc(hwp, 0x66, Offset & 0xFF);
    ViaCrtcChange(hwp, 0x67, Offset >> 8, 0x03);
}

void
VIASetModeForMHS(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASetModeForMHS\n"));
    /* Turn off Screen */
    ViaCrtcChange(hwp, 0x17, 0x00, 0x80);

    if (pBIOSInfo->ActiveDevice & VIA_DEVICE_TV) {
	if (pBIOSInfo->TVIndex != VIA_TVRES_INVALID) {
	    ViaTVModePreset(pVia);
	    ViaTVModePostSet(pScrn);
	} else {
	    if (!pVia->SAMM) /* from via code */
		ViaTVClose(pVia);
	}
    }

    /* CLE266A2 apparently doesn't like this */
    if ((pVia->Chipset != VIA_CLE266) || (pVia->ChipRev != 0x02))
	ViaCrtcChange(hwp, 0x6C, 0x00, 0x1E);

    if ((pBIOSInfo->ActiveDevice & (VIA_DEVICE_DFP | VIA_DEVICE_LCD))
	&& (pBIOSInfo->PanelIndex != VIA_BIOS_NUM_PANEL)) {
        pBIOSInfo->SetDVI = TRUE;
        VIASetLCDMode(pScrn, mode);
        VIAEnableLCD(pScrn);
    }
    else if (pBIOSInfo->ConnectedDevice & (VIA_DEVICE_DFP | VIA_DEVICE_LCD)) {
        VIADisableLCD(pScrn);
    }

    ViaSetSecondaryFIFO(pScrn, mode);
    ViaSetSecondaryOffset(pScrn);

    /* via code: 8bit = 0x00, 16bit = 0x40, 24 & 32bit = 0x80 */
    /* with this arithmetic 32bit becomes 0xC0 */
    ViaCrtcChange(hwp, 0x67, (pScrn->bitsPerPixel - 0x01) << 3, 0xC0);

    /* Open Screen */ /* -- wrong -- this is your bogstandard vga palette disable */
    hwp->disablePalette(hwp);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-- VIASetModeForMHS Done!\n"));
}

/*
 * All of via lacks consistency.
 */
static void 
ViaLCDPowerSequence(vgaHWPtr hwp, VIALCDPowerSeqRecPtr Sequence)
{
    int i;
    
    for (i = 0; i < Sequence->numEntry; i++) {
	ViaVgahwChange(hwp, 0x300 + Sequence->port[i], Sequence->offset[i],
		       0x301 + Sequence->port[i], Sequence->data[i],
		       Sequence->mask[i]);
        usleep(Sequence->delay[i]);
     }
}

void
VIAEnableLCD(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
    int  i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VIAEnableLCD\n"));

    /* Enable LCD */
    ViaCrtcChange(hwp, 0x6A, 0x08, 0x08);

    /* Find Panel Size Index for PowerSeq Table */
    if (pVia->Chipset == VIA_CLE266) {
        if (pBIOSInfo->PanelSize != VIA_PANEL_INVALID) {
            for (i = 0; i < pViaModeTable->NumPowerOn; i++) {
                if (pViaModeTable->lcdTable[pBIOSInfo->PanelIndex].powerSeq ==
                    pViaModeTable->powerOn[i].powerSeq)
                    break;
            }
        } else
            i = 0;
    } else /* KM and K8M use PowerSeq Table index 2. */
        i = 2;

    usleep(1);
    ViaLCDPowerSequence(hwp, &(pViaModeTable->powerOn[i]));
    usleep(1);
}

void
VIADisableLCD(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
    int        i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VIADisableLCD\n"));

    /* Disable LCD */
    ViaCrtcChange(hwp, 0x6A, 0x00, 0x08);

    for (i = 0; i < pViaModeTable->NumPowerOn; i++) {
        if (pViaModeTable->lcdTable[pBIOSInfo->PanelIndex].powerSeq ==
            pViaModeTable->powerOn[i].powerSeq)
            break;
    }

    ViaLCDPowerSequence(hwp, &(pViaModeTable->powerOff[i]));
}

/*
 *
 * All palette.
 *
 */
#ifdef UNUSED
static void
VIASavePalette(ScrnInfoPtr pScrn, LOCO *colors) {
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int i, sr1a, sr1b, cr67, cr6a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASavePalette\n"));
    /* sr16 = hwp->readSeq(hwp, 0x16); */
    sr1a = hwp->readSeq(hwp, 0x1A);
    sr1b = hwp->readSeq(hwp, 0x1B);
    cr67 = hwp->readCrtc(hwp, 0x67);
    cr6a = hwp->readCrtc(hwp, 0x6A);

    /*
    if (pScrn->bitsPerPixel == 8)
        ViaSeqChange(hwp, 0x16, 0x00, 0x80);
    else
        ViaSeqChange(hwp, 0x16, 0x80, 0x80); 
    */
    if (pVia->IsSecondary) {
	ViaSeqChange(hwp, 0x1A, 0x01, 0x01);
	ViaSeqChange(hwp, 0x1B, 0x80, 0x80);
	ViaCrtcChange(hwp, 0x67, 0x00, 0xC0);
	ViaCrtcChange(hwp, 0x6A, 0xC0, 0xC0);
    } else {
	ViaSeqChange(hwp, 0x1A, 0x00, 0x01);
	ViaSeqChange(hwp, 0x1B, 0x20, 0x20);
    }

    hwp->writeDacReadAddr(hwp, 0x00);
    for (i = 0; i < 256; i++) {
        colors[i].red = hwp->readDacData(hwp);
        colors[i].green = hwp->readDacData(hwp);
        colors[i].blue = hwp->readDacData(hwp);
        DEBUG(xf86Msg(X_INFO, "%d, %d, %d\n", colors[i].red, 
		      colors[i].green, colors[i].blue));
    }
    ViaWaitIdle(pScrn);

    /*
    if (pScrn->bitsPerPixel == 8)
        hwp->writeSeq(hwp, 0x16, sr16);
    hwp->writeSeq(hwp, 0x16, sr16);
    */
    hwp->writeSeq(hwp, 0x1A, sr1a);
    hwp->writeSeq(hwp, 0x1B, sr1b);

    if (pVia->IsSecondary) {
	hwp->writeCrtc(hwp, 0x67, cr67);
	hwp->writeCrtc(hwp, 0x6A, cr6a);
    }
}

static void
VIARestorePalette(ScrnInfoPtr pScrn, LOCO *colors) {
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int i, sr1a, sr1b, cr67, cr6a, sr16;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIARestorePalette\n"));
    sr16 = hwp->readSeq(hwp, 0x16);
    sr1a = hwp->readSeq(hwp, 0x1A);
    sr1b = hwp->readSeq(hwp, 0x1B);
    cr67 = hwp->readCrtc(hwp, 0x67);
    cr6a = hwp->readCrtc(hwp, 0x6A);

    /* if (pScrn->bitsPerPixel == 8)
	ViaSeqChange(hwp, 0x16, 0x00, 0x80);
    else
	ViaSeqChange(hwp, 0x16, 0x80, 0x80);
    */

    if (pVia->IsSecondary) {
	ViaSeqChange(hwp, 0x1A, 0x01, 0x01);
	ViaSeqChange(hwp, 0x1B, 0x80, 0x80);
	ViaCrtcChange(hwp, 0x67, 0x00, 0xC0);
	ViaCrtcChange(hwp, 0x6A, 0xC0, 0xC0);
    } else {
	ViaSeqChange(hwp, 0x1A, 0x00, 0x01);
	ViaSeqChange(hwp, 0x1B, 0x20, 0x20);
    }

    hwp->writeDacWriteAddr(hwp, 0x00);
    for (i = 0; i < 256; i++) {
        hwp->writeDacData(hwp, colors[i].red);
        hwp->writeDacData(hwp, colors[i].green);
        hwp->writeDacData(hwp, colors[i].blue);
        /*DEBUG(xf86Msg(X_INFO, "%d, %d, %d\n", colors[i].red, 
	  colors[i].green, colors[i].blue));*/
    }
    ViaWaitIdle(pScrn);

     /*
    if (pScrn->bitsPerPixel == 8)
        hwp->writeSeq(hwp, 0x16, sr16);
    hwp->writeSeq(hwp, 0x16, sr16);
    */
    hwp->writeSeq(hwp, 0x1A, sr1a);
    hwp->writeSeq(hwp, 0x1B, sr1b);

    if (pVia->IsSecondary) {
	hwp->writeCrtc(hwp, 0x67, cr67);
	hwp->writeCrtc(hwp, 0x6A, cr6a);
    }
}
#endif /* UNUSED */

void 
VIALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	       LOCO *colors, VisualPtr pVisual)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int i, index;
    int SR1A, SR1B, CR67, CR6A;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIALoadPalette\n"));

    if (pScrn->bitsPerPixel != 8)
	return;

    SR1A = hwp->readSeq(hwp, 0x1A);
    SR1B = hwp->readSeq(hwp, 0x1B);
    CR67 = hwp->readCrtc(hwp, 0x67);
    CR6A = hwp->readCrtc(hwp, 0x6A);

    if (pVia->IsSecondary) {
	ViaSeqChange(hwp, 0x1A, 0x01, 0x01);
	ViaSeqChange(hwp, 0x1B, 0x80, 0x80);
	ViaCrtcChange(hwp, 0x67, 0x00, 0xC0);
	ViaCrtcChange(hwp, 0x6A, 0xC0, 0xC0);
    }
    
    for (i = 0; i < numColors; i++) {
	index = indices[i];
	hwp->writeDacWriteAddr(hwp, index);
	hwp->writeDacData(hwp, colors[index].red);
	hwp->writeDacData(hwp, colors[index].green);
	hwp->writeDacData(hwp, colors[index].blue);
    }

    if (pVia->IsSecondary) {
	hwp->writeSeq(hwp, 0x1A, SR1A);
	hwp->writeSeq(hwp, 0x1B, SR1B);
	hwp->writeCrtc(hwp, 0x67, CR67);
	hwp->writeCrtc(hwp, 0x6A, CR6A);

	/* Screen 0 palette was changed by mode setting of Screen 1,
	 * so load again */
	for (i = 0; i < numColors; i++) {
	    index = indices[i];
	    hwp->writeDacWriteAddr(hwp, index);
	    hwp->writeDacData(hwp, colors[index].red);
	    hwp->writeDacData(hwp, colors[index].green);
	    hwp->writeDacData(hwp, colors[index].blue);
        }
    }
}
