/*
 * Copyright 2004-2005 The Unichrome Project  [unichrome.sf.net]
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

#include "via_driver.h"
#include "via_vgahw.h"
#include "via_vt162x.h"
#include "via_id.h"

#ifdef HAVE_DEBUG
/*
 *
 */
static void
VT162xPrintRegs(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    CARD8 i, buf;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Printing registers for %s\n",
	       pBIOSInfo->TVI2CDev->DevName);

    for (i = 0; i < VIA_BIOS_MAX_NUM_TV_REG; i++) {
	xf86I2CReadByte(pBIOSInfo->TVI2CDev, i, &buf);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "TV%02X: 0x%02X\n", i, buf);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "End of TV registers.\n");
}
#endif /* HAVE_DEBUG */

/*
 *                 
 */
I2CDevPtr
ViaVT162xDetect(ScrnInfoPtr pScrn, I2CBusPtr pBus, CARD8 Address)
{
    CARD8 buf;
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    I2CDevPtr pDev = xf86CreateI2CDevRec();

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT162xDetect\n"));

    pDev->DevName = "VT162x";
    pDev->SlaveAddr = Address;
    pDev->pI2CBus = pBus;
    
    if (!xf86I2CDevInit(pDev)) {
	xf86DestroyI2CDevRec(pDev, TRUE);
	return NULL;
    }

    if (!xf86I2CReadByte(pDev, 0x1B, &buf)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unable to read from %s Slave %d.\n",
		   pBus->BusName, Address);
	xf86DestroyI2CDevRec(pDev, TRUE);
	return NULL;
    }
    
    switch (buf) {
    case 2:
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		   "Detected VIA Technologies VT1621 TV Encoder\n");
	pBIOSInfo->TVEncoder = VIA_VT1621;
	pDev->DevName = "VT1621";
	break;
    case 3:
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		   "Detected VIA Technologies VT1622 TV Encoder\n");
	pBIOSInfo->TVEncoder = VIA_VT1622;
	pDev->DevName = "VT1622";
	break;
    case 16:
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		   "Detected VIA Technologies VT1622A/VT1623 TV Encoder\n");
	pBIOSInfo->TVEncoder = VIA_VT1623;
	pDev->DevName = "VT1623";
	break;
    default:
	pBIOSInfo->TVEncoder = VIA_NONETV;
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Unknown TV Encoder found at %s %X.\n", pBus->BusName, Address);
	xf86DestroyI2CDevRec(pDev,TRUE);
	pDev = NULL;
    }

#ifdef HAVE_DEBUG
    if (VIAPTR(pScrn)->PrintTVRegs && pDev)
	VT162xPrintRegs(pScrn);
#endif

    return pDev;
}

/*
 *
 */
static void
ViaVT162xSave(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    CARD8 buf = 0x00;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT162xSave\n"));

    xf86I2CWriteRead(pBIOSInfo->TVI2CDev, &buf,1, pBIOSInfo->TVRegs,
		     VIA_BIOS_MAX_NUM_TV_REG);
}

/*
 *
 */
static void
ViaVT162xRestore(ScrnInfoPtr pScrn)
{
    int i;
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT162xRestore\n"));

    for (i = 0; i < VIA_BIOS_MAX_NUM_TV_REG; i++)
	xf86I2CWriteByte(pBIOSInfo->TVI2CDev, i, pBIOSInfo->TVRegs[i]);
}

/*
 * the same for VT1621 as for VT1622/VT1622A/VT1623, result is different though
 */
static CARD8
ViaVT162xDACSenseI2C(I2CDevPtr pDev)
{
    CARD8  save, sense;

    xf86I2CReadByte(pDev, 0x0E, &save);
    xf86I2CWriteByte(pDev, 0x0E, 0x00);
    xf86I2CWriteByte(pDev, 0x0E, 0x80);
    xf86I2CWriteByte(pDev, 0x0E, 0x00);
    xf86I2CReadByte(pDev, 0x0F, &sense);
    xf86I2CWriteByte(pDev, 0x0E, save);
    
    return (sense & 0x0F);
}

/*
 * VT1621 only knows composite and s-video
 */
static Bool
ViaVT1621DACSense(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    CARD8  sense;
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1621DACDetect\n"));
    
    if (!pBIOSInfo->TVI2CDev)
	return FALSE;

    sense = ViaVT162xDACSenseI2C(pBIOSInfo->TVI2CDev);
    switch (sense) {
    case 0x00:
	pBIOSInfo->TVOutput = TVOUTPUT_SC;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT1621: S-Video & Composite connected.\n");
	return TRUE;
    case 0x01:
	pBIOSInfo->TVOutput = TVOUTPUT_COMPOSITE;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT1621: Composite connected.\n");
	return TRUE;
    case 0x02:
	pBIOSInfo->TVOutput = TVOUTPUT_SVIDEO;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT1621: S-Video connected.\n");
	return TRUE;
    case 0x03:
	pBIOSInfo->TVOutput = TVOUTPUT_NONE;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT1621: Nothing connected.\n");
	return FALSE;
    default:
	pBIOSInfo->TVOutput = TVOUTPUT_NONE;
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "VT1621: Unknown cable combination: 0x0%2X.\n",
		   sense);
	return FALSE;
    }
}


/*
 * VT1622, VT1622A and VT1623 know composite, s-video, RGB and YCBCR
 */
static Bool
ViaVT1622DACSense(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    CARD8  sense;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1622DACDetect\n"));

    if (!pBIOSInfo->TVI2CDev)
	return FALSE;

    sense = ViaVT162xDACSenseI2C(pBIOSInfo->TVI2CDev);
    switch (sense) {
    case 0x00: /* DAC A,B,C,D */
	pBIOSInfo->TVOutput = TVOUTPUT_RGB;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: RGB connected.\n");
	return TRUE;
    case 0x01: /* DAC A,B,C */
	pBIOSInfo->TVOutput = TVOUTPUT_SC;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: S-Video & Composite connected.\n");
	return TRUE;
    case 0x07: /* DAC A */
	pBIOSInfo->TVOutput = TVOUTPUT_COMPOSITE;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: Composite connected.\n");
	return TRUE;
    case 0x08: /* DAC B,C,D */
	pBIOSInfo->TVOutput = TVOUTPUT_YCBCR;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: YcBcR connected.\n");
	return TRUE;
    case 0x09: /* DAC B,C */
	pBIOSInfo->TVOutput = TVOUTPUT_SVIDEO;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: S-Video connected.\n");
	return TRUE;
    case 0x0F:
	pBIOSInfo->TVOutput = TVOUTPUT_NONE;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "VT162x: Nothing connected.\n");
	return FALSE;
    default:
	pBIOSInfo->TVOutput = TVOUTPUT_NONE;
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "VT162x: Unknown cable combination: 0x0%2X.\n",
		   sense);
	return FALSE;
    }
}


/*
 *
 */
static Bool
ViaVT1621ModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1621ModeValid\n"));

    switch (mode->CrtcHDisplay) {
    case 640:
	if (mode->CrtcVDisplay == 480)
	    return TRUE;
	return FALSE;
    case 800:
	if (mode->CrtcVDisplay == 600)
	    return TRUE;
	return FALSE;
    default:
	return FALSE;
    }
}

/*
 *
 */
static Bool
ViaVT1622ModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1622ModeValid\n"));
    
    switch (mode->CrtcHDisplay) {
    case 640:
	if (mode->CrtcVDisplay == 480)
	    return TRUE;
	return FALSE;
    case 720:
	if (mode->CrtcVDisplay == 480)
	    return TRUE;
	if (mode->CrtcVDisplay == 576)
	    return TRUE;
	return FALSE;
    case 800:
	if (mode->CrtcVDisplay == 600)
	    return TRUE;
	return FALSE;
    case 848:
	if (mode->CrtcVDisplay == 480)
	    return TRUE;
	return FALSE;
    case 1024:
	if (mode->CrtcVDisplay == 768)
	    return TRUE;
	return FALSE;
    default:
	return FALSE;
    }
}

/*
 *
 */
static void
ViaVT1621ModeI2C(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAVT1621TableRec Table;
    VIATVMASKTableRec Mask;
    CARD8   *TV;
    CARD16  *Patch2;
    CARD8   i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1621ModeI2C\n"));

    if (pBIOSInfo->TVVScan == VIA_TVOVER)
	Table = VT1621OverTable[pBIOSInfo->TVIndex];
    else /* VIA_TVNORMAL */
	Table = VT1621Table[pBIOSInfo->TVIndex];
    Mask = VT1621MaskTable;

    if (pBIOSInfo->TVType == TVTYPE_PAL) {
	Patch2 = Table.PatchPAL2;
	if (pBIOSInfo->TVOutput == TVOUTPUT_COMPOSITE)
	    TV = Table.TVPALC;
	else /* S-video */
	    TV = Table.TVPALS;
    } else { /* TVTYPE_NTSC */
	Patch2 = Table.PatchNTSC2;
	if (pBIOSInfo->TVOutput == TVOUTPUT_COMPOSITE)
	    TV = Table.TVNTSCC;
	else /* S-video */
	    TV = Table.TVNTSCS;
    }

    for (i = 0, j = 0; (j < Mask.numTV) && (i < VIA_BIOS_MAX_NUM_TV_REG); i++) {
	if (Mask.TV[i] == 0xFF) {
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, i, TV[i]);
	    j++;
	} else 
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, i, pBIOSInfo->TVRegs[i]);
    }

    /* Turn on all Composite and S-Video output */
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x0E, 0x00);

    if ((pBIOSInfo->TVType == TVTYPE_NTSC) && pBIOSInfo->TVDotCrawl) {
	CARD16 *DotCrawl = Table.DotCrawlNTSC;
	CARD8 address, save;
	
	for (i = 1; i < (DotCrawl[0] + 1); i++) {
	    address = (CARD8)(DotCrawl[i] & 0xFF);
	    
	    if (address == 0x11) {
		xf86I2CReadByte(pBIOSInfo->TVI2CDev, 0x11, &save);
		save |= (CARD8)(DotCrawl[i] >> 8);
	    } else
		save = (CARD8)(DotCrawl[i] >> 8);
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, address, save);
	}
    }

    if (pVia->IsSecondary) { /* Patch as setting 2nd path */
        j = (CARD8)(Mask.misc2 >> 5);
        for (i = 0; i < j; i++)
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, Patch2[i] & 0xFF, Patch2[i] >> 8);
    }

#ifdef HAVE_DEBUG
    if (pVia->PrintTVRegs)
	VT162xPrintRegs(pScrn);
#endif /* HAVE_DEBUG */
}

/*
 *
 */
static void
ViaVT1621ModeCrtc(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia =  VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAVT1621TableRec Table;
    VIATVMASKTableRec Mask;
    CARD8  *CRTC, *Misc;
    int  i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1621ModeCrtc\n"));

    if (pBIOSInfo->TVVScan == VIA_TVOVER)
	Table = VT1621OverTable[pBIOSInfo->TVIndex];
    else /* VIA_TVNORMAL */
	Table = VT1621Table[pBIOSInfo->TVIndex];
    Mask = VT1621MaskTable;

    if (pVia->IsSecondary) {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table.CRTCPAL2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table.CRTCPAL2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table.CRTCPAL2_8BPP;
		break;
	    }
	    Misc = Table.MiscPAL2;
	} else {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table.CRTCNTSC2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table.CRTCNTSC2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table.CRTCNTSC2_8BPP;
		break;
	    }
	    Misc = Table.MiscNTSC2;
	}

        for (i = 0, j = 0; i < Mask.numCRTC2; j++) {
            if (Mask.CRTC2[j] == 0xFF) {
		hwp->writeCrtc(hwp, j + 0x50, CRTC[j]);
                i++;
            }
        }

        if (Mask.misc2 & 0x18)
	    pBIOSInfo->Clock = (Misc[3] << 8) | Misc[4];

	ViaCrtcMask(hwp, 0x6A, 0xC0, 0xC0);
	ViaCrtcMask(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcMask(hwp, 0x6C, 0x01, 0x01);

        /* Disable LCD Scaling */
    	if (!pVia->SAMM || pVia->FirstInit)
	    hwp->writeCrtc(hwp, 0x79, 0x00);

    } else {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    CRTC = Table.CRTCPAL1;
	    Misc = Table.MiscPAL1;
	} else {
	    CRTC = Table.CRTCNTSC1;
	    Misc = Table.MiscNTSC1;
	}

        for (i = 0, j = 0; i < Mask.numCRTC1; j++) {
            if (Mask.CRTC1[j] == 0xFF) {
		hwp->writeCrtc(hwp, j, CRTC[j]);
                i++;
            }
        }

	ViaCrtcMask(hwp, 0x33, Misc[0], 0x20);
	hwp->writeCrtc(hwp, 0x6A, Misc[1]);
	hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x01);
	hwp->writeCrtc(hwp, 0x6C, Misc[3] | 0x01); /* ? */
	
        if (Mask.misc1 & 0x30)
	    pBIOSInfo->Clock = (Misc[4] << 8) | Misc[5];
    }
}

/*
 * also suited for VT1622A, VT1623
 */
static void
ViaVT1622ModeI2C(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAVT162XTableRec Table;
    VIATVMASKTableRec Mask;
    CARD8   *TV;
    CARD16  *RGB, *YCbCr, *Patch2;
    CARD8   save, i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1622ModeI2C\n"));

    if (pBIOSInfo->TVEncoder == VIA_VT1622) {
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = VT1622OverTable[pBIOSInfo->TVIndex];
	else /* VIA_TVNORMAL */
	    Table = VT1622Table[pBIOSInfo->TVIndex];
	Mask = VT1622MaskTable;
    } else { /* VT1622A/VT1623 */
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = VT1623OverTable[pBIOSInfo->TVIndex];
	else /* VIA_TVNORMAL */
	    Table = VT1623Table[pBIOSInfo->TVIndex];
	Mask = VT1623MaskTable;
    }

    if (pBIOSInfo->TVType == TVTYPE_PAL) {
	TV = Table.TVPAL;
	RGB = Table.RGBPAL;
	YCbCr = Table.YCbCrPAL;
	Patch2 = Table.PatchPAL2;
    } else {
	TV = Table.TVNTSC;
	RGB = Table.RGBNTSC;
	YCbCr = Table.YCbCrNTSC;
	Patch2 = Table.PatchNTSC2;
    }

    /* TV Reset */
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x1D, 0x00);
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x1D, 0x80);

    for (i = 0, j = 0; (j < Mask.numTV) && (i < VIA_BIOS_MAX_NUM_TV_REG); i++) {
	if (Mask.TV[i] == 0xFF) {
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, i, TV[i]);
	    j++;
	} else 
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, i, pBIOSInfo->TVRegs[i]);
    }

    /* Turn on all Composite and S-Video output */
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x0E, 0x00);

    if ((pBIOSInfo->TVType == TVTYPE_NTSC) && pBIOSInfo->TVDotCrawl) {
        CARD16 *DotCrawl = Table.DotCrawlNTSC;
	CARD8 address;

	for (i = 1; i < (DotCrawl[0] + 1); i++) {
            address = (CARD8)(DotCrawl[i] & 0xFF);

            if (address == 0x11) {
                xf86I2CReadByte(pBIOSInfo->TVI2CDev, 0x11, &save);
		save |= (CARD8)(DotCrawl[i] >> 8);
            } else
		save = (CARD8)(DotCrawl[i] >> 8);
            xf86I2CWriteByte(pBIOSInfo->TVI2CDev, address, save);
        }
    }

    if (pBIOSInfo->TVOutput == TVOUTPUT_RGB)
        for (i = 1; i < (RGB[0] + 1); i++)
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, RGB[i] & 0xFF, RGB[i] >> 8);
    else if (pBIOSInfo->TVOutput == TVOUTPUT_YCBCR)
        for (i = 1; i < (YCbCr[0] + 1); i++)
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, YCbCr[i] & 0xFF, YCbCr[i] >> 8);

    if (pVia->IsSecondary) { /* Patch as setting 2nd path */
        j = (CARD8)(Mask.misc2 >> 5);

        for (i = 0; i < j; i++)
	    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, Patch2[i] & 0xFF, Patch2[i] >> 8);
    }

    /* Configure flicker filter */
    xf86I2CReadByte(pBIOSInfo->TVI2CDev, 0x03, &save);
    save &= 0xFC;
    if(pBIOSInfo->TVDeflicker == 1)
	save |= 0x01;
    else if(pBIOSInfo->TVDeflicker == 2)
        save |= 0x02;
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x03, save);

#ifdef HAVE_DEBUG
    if (pVia->PrintTVRegs)
	VT162xPrintRegs(pScrn);
#endif /* HAVE_DEBUG */
}

/*
 * Also suited for VT1622A, VT1623
 */
static void
ViaVT1622ModeCrtc(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIAVT162XTableRec Table;
    VIATVMASKTableRec Mask;
    CARD8           *CRTC, *Misc;
    int             i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1622ModeCrtc\n"));

    if (pBIOSInfo->TVEncoder == VIA_VT1622) {
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = VT1622OverTable[pBIOSInfo->TVIndex];
	else /* VIA_TVNORMAL */
	    Table = VT1622Table[pBIOSInfo->TVIndex];
	Mask = VT1622MaskTable;
    } else { /* VT1622A/VT1623 */
	if (pBIOSInfo->TVVScan == VIA_TVOVER) 
	    Table = VT1623OverTable[pBIOSInfo->TVIndex];
	else /* VIA_TVNORMAL */
	    Table = VT1623Table[pBIOSInfo->TVIndex];
	Mask = VT1623MaskTable;
    }

    if (pVia->IsSecondary) {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table.CRTCPAL2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table.CRTCPAL2_32BPP;
		break;
	     case 8:
	    default:
		CRTC = Table.CRTCPAL2_8BPP;
		break;
	    }
	    Misc = Table.MiscPAL2;
	} else {
	    switch (pScrn->bitsPerPixel) {
	    case 16:
		CRTC = Table.CRTCNTSC2_16BPP;
		break;
	    case 24:
	    case 32:
		CRTC = Table.CRTCNTSC2_32BPP;
		break;
	    case 8:
	    default:
		CRTC = Table.CRTCNTSC2_8BPP;
		break;	
	    }
	    Misc = Table.MiscNTSC2;
	}
	
        for (i = 0, j = 0; i < Mask.numCRTC2; j++) {
            if (Mask.CRTC2[j] == 0xFF) {
		hwp->writeCrtc(hwp, j + 0x50, CRTC[j]);
		i++;
            }
        }
	
        if (Mask.misc2 & 0x18) {
            /* CLE266Ax use 2x XCLK */
            if ((pVia->Chipset == VIA_CLE266) &&
                CLE266_REV_IS_AX(pVia->ChipRev)) {
		ViaCrtcMask(hwp, 0x6B, 0x20, 0x20);

		/* Fix TV clock Polarity for CLE266A2 */
                if (pVia->ChipRev == 0x02)
		    ViaCrtcMask(hwp, 0x6C, 0x1C, 0x1C);

		pBIOSInfo->Clock = 0x471C;
            } else
		pBIOSInfo->Clock = (Misc[3] << 8) | Misc[4];
        }
	
	ViaCrtcMask(hwp, 0x6A, 0xC0, 0xC0);
	ViaCrtcMask(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcMask(hwp, 0x6C, 0x01, 0x01);
	
        /* Disable LCD Scaling */
        if (!pVia->SAMM || pVia->FirstInit)
	    hwp->writeCrtc(hwp, 0x79, 0x00);
    } else {
	if (pBIOSInfo->TVType == TVTYPE_PAL) {
	    CRTC = Table.CRTCPAL1;
	    Misc = Table.MiscPAL1;
	} else {
	    CRTC = Table.CRTCNTSC1;
	    Misc = Table.MiscNTSC1;
	}

	for (i = 0, j = 0; i < Mask.numCRTC1; j++) {
            if (Mask.CRTC1[j] == 0xFF) {
                hwp->writeCrtc(hwp, j, CRTC[j]);
                i++;
            }
        }

        ViaCrtcMask(hwp, 0x33, Misc[0], 0x20);
	hwp->writeCrtc(hwp, 0x6A, Misc[1]);

        if ((pVia->Chipset == VIA_CLE266) &&
	    CLE266_REV_IS_AX(pVia->ChipRev)) {
	    hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x81);
	    /* Fix TV clock Polarity for CLE266A2 */
            if (pVia->ChipRev == 0x02)
		hwp->writeCrtc(hwp, 0x6C, Misc[3] | 0x01);
        } else
	    hwp->writeCrtc(hwp, 0x6B, Misc[2] | 0x01);

        if (Mask.misc1 & 0x30) {
	    /* CLE266Ax use 2x XCLK */
	    if ((pVia->Chipset == VIA_CLE266) &&
		CLE266_REV_IS_AX(pVia->ChipRev))
		pBIOSInfo->Clock = 0x471C;
	    else
		pBIOSInfo->Clock = (Misc[4] << 8) | Misc[5];
        }

	ViaCrtcMask(hwp, 0x6A, 0x40, 0x40);
	ViaCrtcMask(hwp, 0x6B, 0x01, 0x01);
	ViaCrtcMask(hwp, 0x6C, 0x01, 0x01);
    }

    ViaSeqMask(hwp, 0x1E, 0xC0, 0xC0); /* Enable DI0/DVP0 */
}

/*
 *
 */
static void
ViaVT1621Power(ScrnInfoPtr pScrn, Bool On)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1621Power\n"));
    
    if (On)
	xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x00, 0x03);
    else
	xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x0E, 0x03);
}

/*
 *
 */
static void
ViaVT1622Power(ScrnInfoPtr pScrn, Bool On)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT1622Power\n"));
    
    if (On)
	xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x00, 0x03);
    else
	xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x0E, 0x0F);
}

/*
 *
 */
void
ViaVT162xInit(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVT162xInit\n"));

    switch (pBIOSInfo->TVEncoder) {
    case VIA_VT1621:
	pBIOSInfo->TVSave = ViaVT162xSave;
	pBIOSInfo->TVRestore = ViaVT162xRestore;
	pBIOSInfo->TVDACSense = ViaVT1621DACSense;
	pBIOSInfo->TVModeValid = ViaVT1621ModeValid;
	pBIOSInfo->TVModeI2C = ViaVT1621ModeI2C;
	pBIOSInfo->TVModeCrtc = ViaVT1621ModeCrtc;
	pBIOSInfo->TVPower = ViaVT1621Power;
	break;
    case VIA_VT1622:
    case VIA_VT1623:
	pBIOSInfo->TVSave = ViaVT162xSave;
	pBIOSInfo->TVRestore = ViaVT162xRestore;
	pBIOSInfo->TVDACSense = ViaVT1622DACSense;
	pBIOSInfo->TVModeValid = ViaVT1622ModeValid;
	pBIOSInfo->TVModeI2C = ViaVT1622ModeI2C;
	pBIOSInfo->TVModeCrtc = ViaVT1622ModeCrtc;
	pBIOSInfo->TVPower = ViaVT1622Power;
	break;
    default:
	break;
    }

    /* Save before continuing */
    if (pBIOSInfo->TVSave)
	pBIOSInfo->TVSave(pScrn);
}
