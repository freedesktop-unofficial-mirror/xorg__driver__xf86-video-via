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

/*
 * via_mode.c
 *
 * Everything to do with setting and changing modes.
 *
 */

#include "via_driver.h"
#include "via_vgahw.h"
#include "via_id.h"

/*
 * Modetable nonsense.
 *
 */
#include "via_mode.h"

/*
 *
 * TV specific code.
 *
 */

/*
 *
 */
static Bool
ViaTVDetect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIATVDetect\n"));

    /* preset some pBIOSInfo TV related values -- move up */
    pBIOSInfo->TVEncoder = VIA_NONETV;
    pBIOSInfo->TVI2CDev = NULL;
    pBIOSInfo->TVSave = NULL;
    pBIOSInfo->TVRestore = NULL;
    pBIOSInfo->TVDACSense = NULL;
    pBIOSInfo->TVModeValid = NULL;
    pBIOSInfo->TVModeI2C = NULL;
    pBIOSInfo->TVModeCrtc = NULL;
    pBIOSInfo->TVPower = NULL;

    if (pVia->pI2CBus2 && xf86I2CProbeAddress(pVia->pI2CBus2, 0x40))
	pBIOSInfo->TVI2CDev = ViaVT162xDetect(pScrn, pVia->pI2CBus2, 0x40);
    else if (pVia->pI2CBus3 && xf86I2CProbeAddress(pVia->pI2CBus3, 0x40))
	pBIOSInfo->TVI2CDev = ViaVT162xDetect(pScrn, pVia->pI2CBus3, 0x40);

    if (pBIOSInfo->TVI2CDev)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static Bool
ViaTVInit(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVInit\n"));

    switch (pBIOSInfo->TVEncoder){
    case VIA_VT1621:
    case VIA_VT1622:
    case VIA_VT1623:
	ViaVT162xInit(pScrn);
	break;
    default:
	return FALSE;
	break;
    }

    if (!pBIOSInfo->TVSave || !pBIOSInfo->TVRestore ||
	!pBIOSInfo->TVDACSense || !pBIOSInfo->TVModeValid ||
	!pBIOSInfo->TVModeI2C || !pBIOSInfo->TVModeCrtc ||
	!pBIOSInfo->TVPower) {

	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaTVInit: TVEncoder was not "
		   "properly initialised.");

	xf86DestroyI2CDevRec(pBIOSInfo->TVI2CDev, TRUE);
	pBIOSInfo->TVI2CDev = NULL;
	pBIOSInfo->TVOutput = TVOUTPUT_NONE;
	pBIOSInfo->TVEncoder = VIA_NONETV;
	pBIOSInfo->TVI2CDev = NULL;
	pBIOSInfo->TVSave = NULL;
	pBIOSInfo->TVRestore = NULL;
	pBIOSInfo->TVDACSense = NULL;
	pBIOSInfo->TVModeValid = NULL;
	pBIOSInfo->TVModeI2C = NULL;
	pBIOSInfo->TVModeCrtc = NULL;
	pBIOSInfo->TVPower = NULL;

	return FALSE;
    }
    return TRUE;
}

void
ViaTVSave(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVSave)
	pBIOSInfo->TVSave(pScrn);
}

void
ViaTVRestore(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVRestore)
	pBIOSInfo->TVRestore(pScrn);
}

static Bool
ViaTVDACSense(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVDACSense)
	return pBIOSInfo->TVDACSense(pScrn);
    return FALSE;
}

static void
ViaTVSetMode(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVModeI2C)
	pBIOSInfo->TVModeI2C(pScrn);

    if (pBIOSInfo->TVModeCrtc)
	pBIOSInfo->TVModeCrtc(pScrn);
}

void
ViaTVPower(ScrnInfoPtr pScrn, Bool On)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

#ifdef HAVE_DEBUG
    if (On)
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVPower: On.\n");
    else
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVPower: Off.\n");	
#endif

    if (pBIOSInfo->TVPower)
	pBIOSInfo->TVPower(pScrn, On);
} 

/*
 *
 */
static Bool
ViaTVGetIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVGetIndex\n"));

    pBIOSInfo->TVIndex = VIA_TVRES_INVALID;

    if (pBIOSInfo->Refresh != 60){
	xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaTVGetIndex:"
		   " TV requires 60Hz refresh rate.\n");
	return FALSE;
    }

    /* check encoder */
    if (!pBIOSInfo->TVModeValid || !pBIOSInfo->TVModeValid(pScrn, mode))
	return FALSE;
    
    /* check tv standard */
    if (pBIOSInfo->ResolutionIndex == VIA_RES_720X480) {
	if (pBIOSInfo->TVType == TVTYPE_PAL)
	    return FALSE;
    } else if (pBIOSInfo->ResolutionIndex == VIA_RES_720X576) {
	if (pBIOSInfo->TVType == TVTYPE_NTSC)
	    return FALSE;
    }

    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++)
	if (ViaResolutionTable[i].Index == pBIOSInfo->ResolutionIndex) {
	    if (ViaResolutionTable[i].TVIndex == VIA_TVRES_INVALID)
		return FALSE;
	    
	    pBIOSInfo->TVIndex = ViaResolutionTable[i].TVIndex;
	    DEBUG(xf86DrvMsg(pBIOSInfo->scrnIndex, X_INFO, "ViaTVGetIndex:"
			     " %d\n", pBIOSInfo->TVIndex));
	    return TRUE;
	}
    return FALSE;
}

/*
 *
 */
void
ViaOutputsDetect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsDetect\n"));

    /* Panel */
    if (pBIOSInfo->ForcePanel) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Enabling panel from config.\n");
	pBIOSInfo->PanelPresent = TRUE;
    } else if (pVia->Id && (pVia->Id->Outputs & VIA_DEVICE_LCD)) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Enabling panel from"
		   " PCI-Subsystem Id information.\n");
	pBIOSInfo->PanelPresent = TRUE;
    }

    /* Crt */
    if (pVia->DDC1)
	pBIOSInfo->CrtPresent = TRUE;
    /* If any of the unichromes support this, add CRT detection here */
    else if (!pBIOSInfo->PanelPresent) {
	/* Make sure that at least CRT is enabled. */
	if (!pVia->Id || (pVia->Id->Outputs & VIA_DEVICE_CRT))
	    pBIOSInfo->CrtPresent = TRUE;
    }

    /* TV encoder */
    if (ViaTVDetect(pScrn) && ViaTVInit(pScrn)) {
	if (!pBIOSInfo->TVOutput) /* Config might've set this already */
	    ViaTVDACSense(pScrn);
    } else if (pVia->Id && (pVia->Id->Outputs & VIA_DEVICE_TV)) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "This device is supposed to have a"
		   " TV encoder but we are unable to detect it (support missing?).\n");
	pBIOSInfo->TVOutput = 0;
    }
}

#ifdef HAVE_DEBUG
/*
 * Returns:
 *   Bit[7] 2nd Path
 *   Bit[6] 1/0 MHS Enable/Disable
 *   Bit[5] 0 = Bypass Callback, 1 = Enable Callback
 *   Bit[4] 0 = Hot-Key Sequence Control (OEM Specific)
 *   Bit[3] LCD
 *   Bit[2] TV
 *   Bit[1] CRT
 *   Bit[0] DVI
 */
static CARD8 
VIAGetActiveDisplay(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8    tmp;

    tmp = (hwp->readCrtc(hwp, 0x3E) >> 4);
    tmp |= ((hwp->readCrtc(hwp, 0x3B) & 0x18) << 3);

    return tmp;
}
#endif /* HAVE_DEBUG */

/*
 *
 */
Bool
ViaOutputsSelect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    if (pVia->IsSecondary) { /* we just abort for now */
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaOutputsSelect:"
		   " Not handling secondary.\n");
	return FALSE;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: X"
		     " Configuration: 0x%02x\n", pVia->ActiveDevice));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: BIOS"
		     " Initialised register: 0x%02x\n",
		     VIAGetActiveDisplay(pScrn)));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: VBE"
		     " OEM: 0x%02x\n", ViaVBEGetActiveDevice(pScrn)));

    pBIOSInfo->PanelActive = FALSE;
    pBIOSInfo->CrtActive = FALSE;
    pBIOSInfo->TVActive = FALSE;

    if (!pVia->ActiveDevice) {
	/* always enable the panel when present */
	if (pBIOSInfo->PanelPresent)
	    pBIOSInfo->PanelActive = TRUE;
	else if (pBIOSInfo->TVOutput != TVOUTPUT_NONE) /* cable is attached! */
	    pBIOSInfo->TVActive = TRUE;

	/* CRT can be used with everything when present */
	if (pBIOSInfo->CrtPresent)
	    pBIOSInfo->CrtActive = TRUE;
    } else {
	if (pVia->ActiveDevice & VIA_DEVICE_LCD) {
	    if (pBIOSInfo->PanelPresent)
		pBIOSInfo->PanelActive = TRUE;
	    else
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to activate"
			   " panel: no panel is present.\n");
	}

	if (pVia->ActiveDevice & VIA_DEVICE_TV) {
	    if (!pBIOSInfo->TVI2CDev)
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to activate"
			   " TV encoder: no TV encoder present.\n");
	    else if (pBIOSInfo->TVOutput == TVOUTPUT_NONE)
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to activate"
			   " TV encoder: no cable attached.\n");
	    else if (pBIOSInfo->PanelActive)
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to activate"
			   " TV encoder and panel simultaneously. Not using"
			   " TV encoder.\n");
	    else
		pBIOSInfo->TVActive = TRUE;
	}

	if ((pVia->ActiveDevice & VIA_DEVICE_CRT) ||
	    (!pBIOSInfo->PanelActive && !pBIOSInfo->TVActive)) {
	    pBIOSInfo->CrtPresent = TRUE;
	    pBIOSInfo->CrtActive = TRUE;
	}
    }

#ifdef HAVE_DEBUG
    if (pBIOSInfo->CrtActive)
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: CRT.\n"));
    if (pBIOSInfo->PanelActive)
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: Panel.\n"));
    if (pBIOSInfo->TVActive)
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsSelect: TV.\n"));
#endif
    return TRUE; /* !Secondary always has at least CRT */
}

/*
 * Try to interprete EDID ourselves.
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

    if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA0))
	return FALSE;

    pMon = xf86DoEDID_DDC2(pScrn->scrnIndex, pVia->pI2CBus2);
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

    if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA2))
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
    dev->pI2CBus = pVia->pI2CBus2;
    
    if (!xf86I2CDevInit(dev)) {
	xf86DestroyI2CDevRec(dev,TRUE);
	return FALSE;
    }

    xf86I2CReadByte(dev, 0x00, R_Buffer);
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
 *
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
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetModeIndex\n"));

    for (i = 0; ViaModes[i].Width; i++)
	if ((ViaModes[i].Width == mode->CrtcHDisplay) &&
	    (ViaModes[i].Height == mode->CrtcVDisplay) &&
	    (ViaModes[i].Refresh == pBIOSInfo->Refresh)) {
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
 */
static int
ViaGetVesaMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    int i;
    
    for (i = 0; ViaVesaModes[i].Width; i++)
	if ((ViaVesaModes[i].Width == mode->CrtcHDisplay) &&
	    (ViaVesaModes[i].Height == mode->CrtcVDisplay)) {
	    switch (pScrn->bitsPerPixel) {
	    case 8:
		return ViaVesaModes[i].mode_8b;
	    case 16:
		return ViaVesaModes[i].mode_16b;
	    case 24:
	    case 32:
		return ViaVesaModes[i].mode_32b;
	    default:
		return 0xFFFF;
	    }
	}
    return 0xFFFF;
}


/*
 *
 * ViaModeIndexTable[i].PanelIndex is pBIOSInfo->PanelSize 
 * pBIOSInfo->PanelIndex is the index to lcdTable.
 *
 */
static Bool
ViaPanelGetIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
    int i;
    

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex\n"));

    pBIOSInfo->PanelIndex = VIA_BIOS_NUM_PANEL;

    if (pBIOSInfo->PanelSize == VIA_PANEL_INVALID) {
	VIAGetPanelSize(pScrn);
	if (pBIOSInfo->PanelSize == VIA_PANEL_INVALID) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaPanelGetIndex: PanelSize not set.\n");
	    return FALSE;
	}
    }

    if (pBIOSInfo->Refresh != 60){
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex:"
		   " Panel requires 60Hz refresh rate.\n");
	return FALSE;
    }

    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++)
	if (ViaResolutionTable[i].PanelIndex == pBIOSInfo->PanelSize) {
	    pBIOSInfo->panelX = ViaResolutionTable[i].X;
	    pBIOSInfo->panelY = ViaResolutionTable[i].Y;
	    break;
	}

    if (ViaResolutionTable[i].Index == VIA_RES_INVALID) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaPanelGetIndex: Unable"
		   " to find matching PanelSize in ViaResolutionTable.\n");
	return FALSE;
    }


    for (i = 0; i < VIA_BIOS_NUM_PANEL; i++)
	if (lcdTable[i].fpSize == pBIOSInfo->PanelSize) {
	    int modeNum, tmp;
	    
	    modeNum = ViaGetVesaMode(pScrn, mode);
	    if (modeNum == 0xFFFF) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaPanelGetIndex: "
			   "Unable to determine matching VESA modenumber.\n");
		return FALSE;
	    }
	    
	    tmp = 0x01 << (modeNum & 0xF);
	    if ((CARD16)(tmp) & lcdTable[i].SuptMode[(modeNum >> 4)]) {
		pBIOSInfo->PanelIndex = i;
		DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex:"
				 "index: %d (%dx%d)\n", pBIOSInfo->PanelIndex,
				 pBIOSInfo->panelX, pBIOSInfo->panelY));
		return TRUE;
	    }
	    
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex: Unable"
		       " to match given mode with this PanelSize.\n");
	    return FALSE;
	}

    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaPanelGetIndex: Unable"
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

    refresh = (mode->VRefresh + 0.5);
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetNearestRefresh: "
		     "preferred: %d\n", refresh));

    /* get closest matching refresh index */
    if (refresh < supportRef[0])
	pBIOSInfo->Refresh = supportRef[0];
    else {
	for (i = 0; (i <  VIA_NUM_REFRESH_RATE) && (refresh >= supportRef[i]); i++)
	    ;
	pBIOSInfo->Refresh = supportRef[i - 1];
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetNearestRefresh: "
		     "Refresh: %d\n", pBIOSInfo->Refresh));
}

/*
 *
 */
static Bool
ViaRefreshAllowed(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaRefreshAllowed\n"));

    for (i = 0; ViaRefreshAllowedTable[i].Width; i++) {
	if ((ViaRefreshAllowedTable[i].Width == mode->CrtcHDisplay) &&
	    (ViaRefreshAllowedTable[i].Height == mode->CrtcVDisplay) &&
	    (ViaRefreshAllowedTable[i].Refresh == pBIOSInfo->Refresh)) {
	    switch (pScrn->bitsPerPixel) {
	    case 8:
		if (ViaRefreshAllowedTable[i].MemClk_8b <= pVia->MemClk)
		    return TRUE;
		return FALSE;
	    case 16:
		if (ViaRefreshAllowedTable[i].MemClk_16b <= pVia->MemClk)
		    return TRUE;
		return FALSE;
	    case 24:
	    case 32:
		if (ViaRefreshAllowedTable[i].MemClk_32b <= pVia->MemClk)
		    return TRUE;
		return FALSE;
	    default:
		return FALSE;
	    }
	}
    }
    return FALSE;
}

/*
 *
 */
Bool
ViaModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode, Bool Final)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int level;
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeInit\n"));

    if (Final)
	level = X_ERROR;
    else
	level = X_INFO;

    ViaGetNearestRefresh(pScrn, mode);
    if (!ViaRefreshAllowed(pScrn, mode)) {
	xf86DrvMsg(pScrn->scrnIndex, level, "Refreshrate (%fHz) for \"%s\" too"
		   " high for available memory bandwidth.\n",
		   mode->VRefresh, mode->name);
	return FALSE;
    }

    if (!ViaGetModeIndex(pScrn, mode) || !ViaGetResolutionIndex(pScrn, mode)) {
	xf86DrvMsg(pScrn->scrnIndex, level, "Mode \"%s\" not supported by driver.\n",
		   mode->name);
	return FALSE;
    }

    if (pBIOSInfo->TVActive) {
	if (!ViaTVGetIndex(pScrn, mode)) {
	    xf86DrvMsg(pScrn->scrnIndex, level, "Mode \"%s\" not supported by"
		       " TV encoder.\n", mode->name);
	    return FALSE;
	}
    }
    
    if (pBIOSInfo->PanelActive) {
	if (!ViaPanelGetIndex(pScrn, mode)) {
	    xf86DrvMsg(pScrn->scrnIndex, level, "Mode \"%s\" not supported by"
			   " LCD/DFP.\n", mode->name);
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
 * Should be removed now that this is set in ViaModePrimaryVGA
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

    hwp->writeSeq(hwp, 0x46, clock >> 8);
    hwp->writeSeq(hwp, 0x47, clock & 0xFF);

    ViaSeqMask(hwp, 0x40, 0x02, 0x02);
    ViaSeqMask(hwp, 0x40, 0x00, 0x02);
}

/* 
 *
 */
static void
VIASetSecondaryClock(vgaHWPtr hwp, CARD16 clock)
{
    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VIASetSecondaryClock to 0x%X\n", clock));

    hwp->writeSeq(hwp, 0x44, clock >> 8);
    hwp->writeSeq(hwp, 0x45, clock & 0xFF);

    ViaSeqMask(hwp, 0x40, 0x04, 0x04);
    ViaSeqMask(hwp, 0x40, 0x00, 0x04);
}

/*
 * Broken, only does native mode decently. I (Luc) personally broke this.
 */
static void
VIASetLCDMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIALCDModeTableRec Table = lcdTable[pBIOSInfo->PanelIndex];
    CARD8           modeNum = 0;
    int             resIdx;
    int             port, offset, data;
    int             i, j, misc;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASetLCDMode\n"));

    if (pBIOSInfo->PanelSize == VIA_PANEL12X10)
	hwp->writeCrtc(hwp, 0x89, 0x07);

    /* LCD Expand Mode Y Scale Flag */
    pBIOSInfo->scaleY = FALSE;

    /* Set LCD InitTb Regs */
    if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	if (pVia->IsSecondary)
	    pBIOSInfo->Clock = Table.InitTb.LCDClk_12Bit;
	else {
	    pBIOSInfo->Clock = Table.InitTb.VClk_12Bit;
	    /* for some reason still to be defined this is neccessary */
	    VIASetSecondaryClock(hwp, Table.InitTb.LCDClk_12Bit);
	}
    } else {
	if (pVia->IsSecondary)
	    pBIOSInfo->Clock = Table.InitTb.LCDClk;
	else {
	    pBIOSInfo->Clock = Table.InitTb.VClk;
	    VIASetSecondaryClock(hwp, Table.InitTb.LCDClk);
	}

    }

    VIASetUseExternalClock(hwp);

    for (i = 0; i < Table.InitTb.numEntry; i++) {
        port = Table.InitTb.port[i];
        offset = Table.InitTb.offset[i];
        data = Table.InitTb.data[i];
	ViaVgahwWrite(hwp, 0x300+port, offset, 0x301+port, data);
    }

    if ((mode->CrtcHDisplay != pBIOSInfo->panelX) ||
        (mode->CrtcVDisplay != pBIOSInfo->panelY)) {
	VIALCDModeEntryPtr Main;
	VIALCDMPatchEntryPtr Patch1, Patch2;
	int numPatch1, numPatch2;

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
	    Main = &(Table.MCtr[resIdx]);
	    Patch1 = Table.MPatchDP1Ctr;
	    numPatch1 = Table.numMPatchDP1Ctr;
	    Patch2 = Table.MPatchDP2Ctr;
	    numPatch2 = Table.numMPatchDP2Ctr;
	} else { /* expand! */
            /* LCD Expand Mode Y Scale Flag */
            pBIOSInfo->scaleY = TRUE;
	    Main = &(Table.MExp[resIdx]);
	    Patch1 = Table.MPatchDP1Exp;
	    numPatch1 = Table.numMPatchDP1Exp;
	    Patch2 = Table.MPatchDP2Exp;
	    numPatch2 = Table.numMPatchDP2Exp;
        }
	
	/* Set Main LCD Registers */
	for (i = 0; i < Main->numEntry; i++){
	    ViaVgahwWrite(hwp, 0x300 + Main->port[i], Main->offset[i],
			  0x301 + Main->port[i], Main->data[i]);
	}
	
	if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    if (pVia->IsSecondary)
		pBIOSInfo->Clock = Main->LCDClk_12Bit;
	    else
		pBIOSInfo->Clock = Main->VClk_12Bit;
	} else {
	    if (pVia->IsSecondary)
		pBIOSInfo->Clock = Main->LCDClk;
	    else
		pBIOSInfo->Clock = Main->VClk;
	}

	j = ViaGetVesaMode(pScrn, mode);
	if (j == 0xFFFF) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "VIASetLCDMode: "
		       "Unable to determine matching VESA modenumber.\n");
	    return;
	}
	for (i = 0; i < modeFix.numEntry; i++) {
	    if (modeFix.reqMode[i] == j) {
		modeNum = modeFix.fixMode[i];
		break;
	    }
	}

	/* Set LCD Mode patch registers. */
	for (i = 0; i < numPatch2; i++, Patch2++) {
	    if (Patch2->Mode == modeNum) {
		if (!pBIOSInfo->Center && (mode->CrtcHDisplay == pBIOSInfo->panelX))
		    pBIOSInfo->scaleY = FALSE;
		
		for (j = 0; j < Patch2->numEntry; j++){
		    ViaVgahwWrite(hwp, 0x300 + Patch2->port[j], Patch2->offset[j], 
				  0x301 + Patch2->port[j], Patch2->data[j]);
		}
		
		if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
		    if (pVia->IsSecondary)
			pBIOSInfo->Clock = Patch2->LCDClk_12Bit;
		    else
			pBIOSInfo->Clock = Patch2->VClk_12Bit;
		} else {
		    if (pVia->IsSecondary)
			pBIOSInfo->Clock = Patch2->LCDClk;
		    else
			pBIOSInfo->Clock = Patch2->VClk;
		}
		break;
	    }
	}
	

	/* Set LCD Secondary Mode Patch registers. */
	if (pVia->IsSecondary) {
	    for (i = 0; i < numPatch1; i++, Patch1++) {
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

    /* LCD patch 3D5.02 */
    misc = hwp->readCrtc(hwp, 0x01);
    hwp->writeCrtc(hwp, 0x02, misc);

    /* Enable LCD */
    if (!pVia->IsSecondary) {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
	ViaSeqMask(hwp, 0x16, 0x40, 0x40);

        /* Enable Simultaneous */
        if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    hwp->writeCrtc(hwp, 0x6B, 0xA8);

            if ((pVia->Chipset == VIA_CLE266) &&
		CLE266_REV_IS_AX(pVia->ChipRev))
		hwp->writeCrtc(hwp, 0x93, 0xB1);
            else
		hwp->writeCrtc(hwp, 0x93, 0xAF);
        } else {
	    ViaCrtcMask(hwp, 0x6B, 0x08, 0x08);
	    hwp->writeCrtc(hwp, 0x93, 0x00);
        }
	hwp->writeCrtc(hwp, 0x6A, 0x48);
    } else {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
	ViaSeqMask(hwp, 0x16, 0x00, 0x40);

        /* Enable SAMM */
        if (pBIOSInfo->BusWidth == VIA_DI_12BIT) {
	    ViaCrtcMask(hwp, 0x6B, 0x20, 0x20);
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

/*
 *
 */
static void
ViaModePrimaryVGA(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    struct ViaModeLine ViaMode = ViaModes[pBIOSInfo->ModeIndex];
    CARD16 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModePrimaryVGA\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAModePrimary: Setting up "
		     "%4dx%4d@%2d\n", ViaMode.Width, ViaMode.Height, ViaMode.Refresh));

    ViaCrtcMask(hwp, 0x11, 0x00, 0x80); /* modify starting address */
    ViaCrtcMask(hwp, 0x03, 0x80, 0x80); /* enable vertical retrace access */
    hwp->writeSeq(hwp, 0x10, 0x01); /* unlock extended registers */
    ViaCrtcMask(hwp, 0x47, 0x00, 0x01); /* unlock CRT registers */

    /* Set Misc Register */
    temp = 0x23;
    if (!ViaMode.HSyncPos)
	temp |= 0x40;
    if (!ViaMode.VSyncPos)
	temp |= 0x80;
    temp |= 0x0C; /* Undefined/external clock */
    hwp->writeMiscOut(hwp, temp);

    /* Sequence registers */
    hwp->writeSeq(hwp, 0x00, 0x00);

    /* if (mode->Flags & V_CLKDIV2)
	hwp->writeSeq(hwp, 0x01, 0x09);
    else */
    hwp->writeSeq(hwp, 0x01, 0x01);

    hwp->writeSeq(hwp, 0x02, 0x0F);
    hwp->writeSeq(hwp, 0x03, 0x00);
    hwp->writeSeq(hwp, 0x04, 0x0E);
    
    ViaSeqMask(hwp, 0x15, 0x02, 0x02);

    /* bpp */
    switch (pScrn->bitsPerPixel) {
    case 8:
	ViaSeqMask(hwp, 0x15, 0x20, 0xFC);
        break;
    case 16:
	ViaSeqMask(hwp, 0x15, 0xB4, 0xFC);
        break;
    case 24:
    case 32:
	ViaSeqMask(hwp, 0x15, 0xAC, 0xFC);
        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                   pScrn->bitsPerPixel);
        break;
    }

    ViaSeqMask(hwp, 0x16, 0x08, 0xBF);
    ViaSeqMask(hwp, 0x17, 0x1F, 0xFF);
    ViaSeqMask(hwp, 0x18, 0x4E, 0xFF);
    ViaSeqMask(hwp, 0x1A, 0x08, 0xFD);

    /* graphics registers */
    hwp->writeGr(hwp, 0x00, 0x00);
    hwp->writeGr(hwp, 0x01, 0x00);
    hwp->writeGr(hwp, 0x02, 0x00);
    hwp->writeGr(hwp, 0x03, 0x00);
    hwp->writeGr(hwp, 0x04, 0x00);
    hwp->writeGr(hwp, 0x05, 0x40);
    hwp->writeGr(hwp, 0x06, 0x05);
    hwp->writeGr(hwp, 0x07, 0x0F);
    hwp->writeGr(hwp, 0x08, 0xFF);

    ViaGrMask(hwp, 0x20, 0, 0xFF);
    ViaGrMask(hwp, 0x21, 0, 0xFF);
    ViaGrMask(hwp, 0x22, 0, 0xFF);
    
    /* attribute registers */
    hwp->writeAttr(hwp, 0x00, 0x00);
    hwp->writeAttr(hwp, 0x01, 0x01);
    hwp->writeAttr(hwp, 0x02, 0x02);
    hwp->writeAttr(hwp, 0x03, 0x03);
    hwp->writeAttr(hwp, 0x04, 0x04);
    hwp->writeAttr(hwp, 0x05, 0x05);
    hwp->writeAttr(hwp, 0x06, 0x06);
    hwp->writeAttr(hwp, 0x07, 0x07);
    hwp->writeAttr(hwp, 0x08, 0x08);
    hwp->writeAttr(hwp, 0x09, 0x09);
    hwp->writeAttr(hwp, 0x0A, 0x0A);
    hwp->writeAttr(hwp, 0x0B, 0x0B);
    hwp->writeAttr(hwp, 0x0C, 0x0C);
    hwp->writeAttr(hwp, 0x0D, 0x0D);
    hwp->writeAttr(hwp, 0x0E, 0x0E);
    hwp->writeAttr(hwp, 0x0F, 0x0F);
    hwp->writeAttr(hwp, 0x10, 0x41);
    hwp->writeAttr(hwp, 0x11, 0xFF);
    hwp->writeAttr(hwp, 0x12, 0x0F);
    hwp->writeAttr(hwp, 0x13, 0x00);
    hwp->writeAttr(hwp, 0x14, 0x00);

    /* Crtc registers */
    /* horizontal total */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHTotal: 0x%03X -- 0x%03X\n",
		     mode->CrtcHTotal, ViaMode.HTotal));
    temp = (ViaMode.HTotal >> 3) - 5;
    hwp->writeCrtc(hwp, 0x00, temp & 0xFF);
    ViaCrtcMask(hwp, 0x36, temp >> 5, 0x08);
    
    /* horizontal address */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHDisplay: 0x%02X\n",
		     mode->CrtcHDisplay));
    temp = (mode->CrtcHDisplay >> 3) - 1;
    hwp->writeCrtc(hwp, 0x01, temp & 0xFF);

    /* horizontal blanking start */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHBlankStart: 0x%02X -- 0x%02X\n",
		     mode->CrtcHBlankStart, ViaMode.HBlankStart));
    /* TODO: Limit to 2048 in ViaValidMode */
    temp = (ViaMode.HBlankStart >> 3) - 1;
    hwp->writeCrtc(hwp, 0x02, temp & 0xFF);
    /* If HblankStart has more bits anywhere, add them here */

    /* horizontal blanking end */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHBlankEnd: 0x%02X -- 0x%02X\n",
		     mode->CrtcHBlankEnd, ViaMode.HBlankEnd));
    temp = (ViaMode.HBlankEnd >> 3) - 1;
    ViaCrtcMask(hwp, 0x03, temp, 0x1F);
    ViaCrtcMask(hwp, 0x05, temp << 2, 0x80);
    ViaCrtcMask(hwp, 0x33, temp >> 1, 0x20);

    /* CrtcHSkew ??? */

    /* horizontal sync start */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHSyncStart: 0x%03X -- 0x%03X\n",
		     mode->CrtcHSyncStart, ViaMode.HSyncStart));
    temp = ViaMode.HSyncStart >> 3;
    hwp->writeCrtc(hwp, 0x04, temp & 0xFF);
    ViaCrtcMask(hwp, 0x33, temp >> 4, 0x10);

    /* horizontal sync end */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHSyncEnd: 0x%03X -- 0x%03X\n",
		     mode->CrtcHSyncEnd, ViaMode.HSyncEnd));
    temp = ViaMode.HSyncEnd >> 3;
    ViaCrtcMask(hwp, 0x05, temp, 0x1F);

    /* vertical total */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVTotal: 0x%03X -- 0x%03X\n",
		     mode->CrtcVTotal, ViaMode.VTotal));
    temp = ViaMode.VTotal - 2;
    hwp->writeCrtc(hwp, 0x06, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 8, 0x01);
    ViaCrtcMask(hwp, 0x07, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 10, 0x01);

    /* vertical address */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVDisplay: 0x%03X\n",
		     mode->CrtcVDisplay));
    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x12, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 7, 0x02);
    ViaCrtcMask(hwp, 0x07, temp >> 3, 0x40);
    ViaCrtcMask(hwp, 0x35, temp >> 8, 0x04);

    /* Primary starting address -> 0x00, adjustframe does the rest */
    hwp->writeCrtc(hwp, 0x0C, 0x00);
    hwp->writeCrtc(hwp, 0x0D, 0x00);
    hwp->writeCrtc(hwp, 0x34, 0x00);
    ViaCrtcMask(hwp, 0x48, 0x00, 0x03); /* is this even possible on CLE266A ? */

    /* vertical sync start */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVSyncStart: 0x%03X -- 0x%03X\n",
		     mode->CrtcVSyncStart, ViaMode.VSyncStart));
    temp = ViaMode.VSyncStart;
    hwp->writeCrtc(hwp, 0x10, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 6, 0x04);
    ViaCrtcMask(hwp, 0x07, temp >> 2, 0x80);
    ViaCrtcMask(hwp, 0x35, temp >> 9, 0x02);

    /* vertical sync end */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVSyncEnd: 0x%01X -- 0x%01X\n",
		     mode->CrtcVSyncEnd, ViaMode.VSyncEnd));
    ViaCrtcMask(hwp, 0x11, ViaMode.VSyncEnd, 0x0F);

    /* line compare: We are not doing splitscreen so 0x3FFF */
    hwp->writeCrtc(hwp, 0x18, 0xFF);
    ViaCrtcMask(hwp, 0x07, 0x10, 0x10);
    ViaCrtcMask(hwp, 0x09, 0x40, 0x40);
    ViaCrtcMask(hwp, 0x33, 0x07, 0x06);
    ViaCrtcMask(hwp, 0x35, 0x10, 0x10);

    /* zero Maximum scan line */
    ViaCrtcMask(hwp, 0x09, 0x00, 0x1F);
    hwp->writeCrtc(hwp, 0x14, 0x00);

    /* vertical blanking start */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVBlankStart: 0x%03X -- 0x%03X\n",
		     mode->CrtcVBlankStart, ViaMode.VBlankStart));
    temp = ViaMode.VBlankStart - 1;
    hwp->writeCrtc(hwp, 0x15, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 5, 0x08);
    ViaCrtcMask(hwp, 0x09, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 7, 0x08);

    /* vertical blanking end */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVBlankEnd: 0x%03X -- 0x%03X\n",
		     mode->CrtcVBlankEnd, ViaMode.VBlankEnd));
    temp = ViaMode.VBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x16, temp);

    /* some leftovers */
    hwp->writeCrtc(hwp, 0x08, 0x00);
    ViaCrtcMask(hwp, 0x32, 0, 0xFF); /* ? */
    ViaCrtcMask(hwp, 0x33, 0, 0xC8);
    
    /* offset */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    if (temp & 0x03) { /* Make sure that this is 32byte aligned */
	temp += 0x03;
	temp &= ~0x03;
    }
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Offset: 0x%03X\n", temp));
    hwp->writeCrtc(hwp, 0x13, temp & 0xFF);
    ViaCrtcMask(hwp, 0x35, temp >> 3, 0xE0);

    /* some leftovers */
    ViaCrtcMask(hwp, 0x32, 0, 0xFF);
    ViaCrtcMask(hwp, 0x33, 0, 0xC8);
}

/*
 *
 */
static CARD16
ViaModeDotClockTranslate(ScrnInfoPtr pScrn, int DotClock)
{
    int i;

    for (i = 0; ViaDotClocks[i].DotClock; i++)
	if (ViaDotClocks[i].DotClock == DotClock)
	    return ViaDotClocks[i].UniChrome;

    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaModeDotClockTranslate: %d not "
	       "found in table.\n", DotClock);
    return 0x0000;
}

/*
 *
 */
void
ViaModePrimary(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModePrimary\n"));
    
    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);
    
    /* Clean Second Path Status */
    hwp->writeCrtc(hwp, 0x6A, 0x00);
    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x93, 0x00);

    ViaModePrimaryVGA(pScrn, mode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, ViaModes[pBIOSInfo->ModeIndex].DotClock);

    /* Don't do this before the Sequencer is set: locks up KM400 and K8M800 */
    if (pVia->FirstInit)
	memset(pVia->FBBase, 0x00, pVia->videoRambytes);
    
    /* Enable MMIO & PCI burst (1 wait state) */
    ViaSeqMask(hwp, 0x1A, 0x06, 0x06);
    
    if (!pBIOSInfo->CrtActive)
	ViaCrtcMask(hwp, 0x36, 0x30, 0x30);

    if (pBIOSInfo->PanelActive && (pBIOSInfo->PanelIndex != VIA_BIOS_NUM_PANEL)) {
	VIASetLCDMode(pScrn, mode);
	ViaLCDPower(pScrn, TRUE);
    } else if (pBIOSInfo->PanelPresent)
	ViaLCDPower(pScrn, FALSE);
	
    if (pBIOSInfo->TVActive && (pBIOSInfo->TVIndex != VIA_TVRES_INVALID))
	ViaTVSetMode(pScrn);
    else
	ViaTVPower(pScrn, FALSE);

    ViaSetPrimaryFIFO(pScrn, mode);

    VIASetPrimaryClock(hwp, pBIOSInfo->Clock);
    VIASetUseExternalClock(hwp);

    /* Enable CRT Controller (3D5.17 Hardware Reset) */
    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);

    hwp->disablePalette(hwp);
}

/*
 *
 */
static void
ViaModeSecondaryVGA(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeSecondaryVGA\n"));

    /* bpp */
    switch (pScrn->bitsPerPixel) {
    case 8:
        ViaCrtcMask(hwp, 0x67, 0x00, 0xC0);
        break;
    case 16:
        ViaCrtcMask(hwp, 0x67, 0x40, 0xC0);
        break;
    case 24:
    case 32:
        ViaCrtcMask(hwp, 0x67, 0x80, 0xC0);
        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                   pScrn->bitsPerPixel);
        break;
    }

    /* offset */
    tmp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    if (tmp & 0x03) { /* Make sure that this is 32byte aligned */
	tmp += 0x03;
	tmp &= ~0x03;
    }
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "offset: 0x%03X\n", tmp));
    hwp->writeCrtc(hwp, 0x66, tmp & 0xFF);
    ViaCrtcMask(hwp, 0x67, tmp >> 8, 0x03);
}

/*
 *
 */
void
ViaModeSecondary(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeSecondary\n"));

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    ViaModeSecondaryVGA(pScrn, mode);

    if (pBIOSInfo->TVActive && (pBIOSInfo->TVIndex != VIA_TVRES_INVALID))
	ViaTVSetMode(pScrn);

    /* CLE266A2 apparently doesn't like this */
    if ((pVia->Chipset != VIA_CLE266) || (pVia->ChipRev != 0x02))
	ViaCrtcMask(hwp, 0x6C, 0x00, 0x1E);

    if (pBIOSInfo->PanelActive && (pBIOSInfo->PanelIndex != VIA_BIOS_NUM_PANEL)) {
        pBIOSInfo->SetDVI = TRUE;
        VIASetLCDMode(pScrn, mode);
        ViaLCDPower(pScrn, TRUE);
    } else if (pBIOSInfo->PanelPresent)
        ViaLCDPower(pScrn, FALSE);

    ViaSetSecondaryFIFO(pScrn, mode);

    VIASetSecondaryClock(hwp, pBIOSInfo->Clock);
    VIASetUseExternalClock(hwp);

    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);

    hwp->disablePalette(hwp);
}

/*
 *
 */
static void 
ViaLCDPowerSequence(vgaHWPtr hwp, VIALCDPowerSeqRec Sequence)
{
    int i;
    
    for (i = 0; i < Sequence.numEntry; i++) {
	ViaVgahwMask(hwp, 0x300 + Sequence.port[i], Sequence.offset[i],
		     0x301 + Sequence.port[i], Sequence.data[i],
		     Sequence.mask[i]);
        usleep(Sequence.delay[i]);
     }
}

/*
 *
 */
void
ViaLCDPower(ScrnInfoPtr pScrn, Bool On)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int  i;

#ifdef HAVE_DEBUG
    if (On)
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLCDPower: On.\n");
    else
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLCDPower: Off.\n");	
#endif

    /* Enable LCD */
    if (On)
	ViaCrtcMask(hwp, 0x6A, 0x08, 0x08);
    else
	ViaCrtcMask(hwp, 0x6A, 0x00, 0x08);

    /* Find Panel Size Index for PowerSeq Table */
    if (pVia->Chipset == VIA_CLE266) {
        if (pBIOSInfo->PanelSize != VIA_PANEL_INVALID) {
            for (i = 0; i < NumPowerOn; i++) {
                if (lcdTable[pBIOSInfo->PanelIndex].powerSeq == powerOn[i].powerSeq)
                    break;
            }
        } else
            i = 0;
    } else /* KM and K8M use PowerSeq Table index 2. */
        i = 2;

    usleep(1);
    if (On)
	ViaLCDPowerSequence(hwp, powerOn[i]);
    else
	ViaLCDPowerSequence(hwp, powerOff[i]);
    usleep(1);
}
