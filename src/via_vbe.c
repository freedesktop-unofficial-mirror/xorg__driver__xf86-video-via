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

/*
 *  VBE OEM Extensions
 *
 *  Most of these are either not used or not working properly.
 *  Need information from via.
 * 
 */

#include "via_driver.h"

/*
 * CARD8 ViaVBEGetActiveDevice(ScrnInfoPtr pScrn);
 *
 *     - Determine which devices (CRT1, LCD, TV, DFP) are active
 *
 * Need more information: return does not match my biossetting -- luc
 *
 *
 * VBE OEM subfunction 0x0103 (from via code)
 *     cx = 0x00
 * returns:
 *     cx = active device
 *
 */
CARD8 
ViaVBEGetActiveDevice(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {

	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	CARD8 device = 0;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetActiveDevice\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0103;

	pInt10->cx = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetActiveDevice: VBE call failed.\n");
	    return 0xFF;
	}
	
	if (pInt10->cx & 0x01)
	    device = VIA_DEVICE_CRT1;
	if (pInt10->cx & 0x02)
	    device |= VIA_DEVICE_LCD;
	if (pInt10->cx & 0x04)
	    device |= VIA_DEVICE_TV;
	if (pInt10->cx & 0x20)
	    device |= VIA_DEVICE_DFP;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Active Device(s): %u\n", device));
	return device;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetActiveDevice: VBE not initialised.\n");
    return 0xFF;
}

/*
 * CARD16 ViaVBEGetDisplayDeviceInfo(pScrn, *numDevice);
 *
 *     - Returns the maximal vertical resolution of the Display Device
 *       provided in numDevice (CRT (0), DVI (1), LCD/Panel (2))
 *
 *
 * VBE OEM subfunction 0x0806 (from via code)
 *     cx = *numDevice
 *     di = 0x00
 * returns:
 *     cx = *numDevice
 *     di = max. vertical resolution
 *
 */
CARD16
ViaVBEGetDisplayDeviceInfo(ScrnInfoPtr pScrn, CARD8 *numDevice)
{
    if (VIAPTR(pScrn)->pVbe) {

	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetDisplayDeviceInfo\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0806;

	pInt10->cx = *numDevice;
	pInt10->di = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetDisplayDeviceInfo: VBE call failed.\n");
	    return 0xFFFF;
	}
	
	*numDevice = pInt10->cx;

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vertical Resolution: %u\n", pInt10->di & 0xFFFF));
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel ID: %u\n", *numDevice));
	
	return (pInt10->di & 0xFFFF);
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetDisplayDeviceInfo: VBE not initialised.\n");
    return 0xFFFF;
}

#ifdef UNUSED
/*
 * CARD8 ViaVBEGetDisplayDeviceAttached(pScrn);
 *
 *     - Find out which display devices are being used.
 *
 * Why is CRT2 ignored?   
 *
 *
 * VBE OEM subfunction 0x0004 (from via code)
 *     cx = 0x00
 * returns:
 *     cx = display devices
 *         Bit[4] = CRT2
 *         Bit[3] = DFP
 *         Bit[2] = TV
 *         Bit[1] = LCD
 *         Bit[0] = CRT 
 */
static CARD8
ViaVBEGetDisplayDeviceAttached(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {

	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	CARD8 device = 0;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetDisplayDeviceAttached\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x04;

	pInt10->cx = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetDisplayDeviceAttached: VBE call failed.\n");
	    return 0xFF;
	}
	
	if (pInt10->cx & 0x01)
	    device = VIA_DEVICE_CRT1;
	if (pInt10->cx & 0x02)
	    device |= VIA_DEVICE_LCD;
	if (pInt10->cx & 0x04)
	    device |= VIA_DEVICE_TV;
	if (pInt10->cx & 0x20)
	    device |= VIA_DEVICE_DFP;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Attached Device(s): %d\n", device));
	return device;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetDisplayDeviceAttached: VBE not initialised.\n");
    return 0xFF;
}
#endif /* UNUSED */

/*
 * Bool ViaVBEGetBIOSDate(ScrnInfoPtr pScrn);
 *
 *     - Get the BIOS release date and store it in the BIOSInfo.
 *
 * 
 * VBE OEM subfunction 0x0100 (from via code)
 *     cx = 0x00
 *     dx = 0x00
 *     si = 0x00
 * returns:
 *     bx = year
 *     cx = month
 *     dx = day
 *
 */
Bool 
ViaVBEGetBIOSDate(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {

	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetBIOSDate\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0100;

	pInt10->cx = 0x00;
	pInt10->dx = 0x00;
	pInt10->si = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xff) != 0x4f) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetBIOSDate: VBE call failed.\n");
	    return FALSE;
	}
	
	pBIOSInfo->BIOSDateYear = ((pInt10->bx >> 8) - 48) + ((pInt10->bx & 0xFF) - 48) * 10;
	pBIOSInfo->BIOSDateMonth = ((pInt10->cx >> 8) - 48) + ((pInt10->cx & 0xFF) - 48) * 10;
	pBIOSInfo->BIOSDateDay = ((pInt10->dx >> 8) - 48) + ((pInt10->dx & 0xFF) - 48) * 10;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BIOS Release Date: %d/%d/%d\n",
			 pBIOSInfo->BIOSDateYear + 2000, pBIOSInfo->BIOSDateMonth,
			 pBIOSInfo->BIOSDateDay));
	return TRUE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetBIOSDate: VBE not initialised.\n");
    return FALSE;
}

/*
 * Bool ViaVBEGetBIOSVersion(ScrnInfoPtr pScrn);
 *
 *     - Return the BIOS version.
 *
 * Calls VBE subfunction 00h (VBE supplemental specification information.) 
 * Not functional.
 * 
 * VBE OEM subfunction 0x0000 (??? - from via code)
 *     cx = 0x00
 * returns:
 *     bx = version.
 *
 */
Bool
ViaVBEGetBIOSVersion(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {

	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetBIOSVersion\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x00; /* ??? */
	pInt10->cx = 0x00;
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xff) != 0x4f) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetBIOSVersion: VBE call failed.\n");
	    return FALSE;
	}
	
	pBIOSInfo->BIOSMajorVersion = (pInt10->bx >> 8) & 0xFF;
	pBIOSInfo->BIOSMinorVersion = pInt10->bx & 0xFF;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "BIOS Version: %d.%d\n",
			 pBIOSInfo->BIOSMajorVersion, pBIOSInfo->BIOSMinorVersion));
	return TRUE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetBIOSVersion: VBE not initialised.\n");
    return FALSE;
}

#ifdef UNUSED
/*
 * CARD8 ViaVBEGetFlatPanelInfo(ScrnInfoPtr pScrn);
 *
 *     - Return the flat panel id
 *
 * 
 * VBE OEM subfunction 0x0006 (from via code)
 *     cx = 0x00
 * returns:
 *     cx = panel id.
 *
 */
static CARD8
ViaVBEGetFlatPanelInfo(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetFlatPanelInfo\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x06;

	pInt10->cx = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetFlatPanelInfo: VBE call failed!\n");
	    return 0xFF;
	}
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel ID: %u\n", pInt10->cx & 0x0F));
	return (pInt10->cx & 0x0F);
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetFlatPanelInfo: VBE not initialised.\n");
    return 0xFF;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * CARD16 ViaVBEGetTVConfiguration(ScrnInfoPtr pScrn, CARD16 dx);
 *
 *     - ...
 *
 * will only return 0x0000 or 0xFFFF.
 *
 * VBE OEM subfunction 0x8107 (from via code)
 *     cx = 0x01
 *     dx = ?
 * returns:
 *     dx = tv configuration?
 *
 */
static CARD16 
ViaVBEGetTVConfiguration(ScrnInfoPtr pScrn, CARD16 dx)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	CARD16 config = 0;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetTVConfiguration\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x8107;
	pInt10->cx = 0x01;
	pInt10->dx = dx;
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xff) != 0x4f) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetTVConfiguration: VBE call failed.\n");
	    return 0xFFFF;
	}
	
	if (pInt10->dx)
	    config = ViaVBEGetTVConfiguration(pScrn, pInt10->dx);

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "TV Configuration: %u\n", config));
	return config;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetTVConfiguration: VBE not initialised.\n");
    return 0xFFFF;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * CARD8 ViaVBEGetTVEncoderType(ScrnInfoPtr pScrn);
 *
 *     - Return the type of tv encoder attached.
 *
 * Calls VBE subfunction 00h (VBE supplemental specification information.) 
 * Not functional.
 *
 * VBE OEM subfunction 0x0000 (??? - from via code)
 *     cx = 0x00
 * returns:
 *     cx = TV encoder type?
 *
 */
static CARD8
ViaVBEGetTVEncoderType(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10; 
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetTVEncoderType\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0000; /* ??? */

	pInt10->cx = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetTVEncoderType: VBE call failed.\n");
	    return 0xFF;
	}
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "TV Encoder: %u\n", pInt10->cx >> 8));
	return (pInt10->cx >> 8);
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetTVEncoderType: VBE not initialised.\n");
    return 0xFF;
}
#endif /* UNUSED */

/*
 * int ViaVBEGetVideoMemSize(ScrnInfoPtr pScrn);
 *
 *     - Get the memory size from VBE OEM.
 *
 * Calls VBE subfunction 00h (VBE supplemental specification information.) 
 * Not functional.
 *
 *
 * VBE OEM subfunction 0x0000 (??? - from via code)
 *     cx = 0x00
 *     dx = 0x00
 *     di = 0x00
 *     si = 0x00
 * returns:
 *     si = memory size
 *
 */
int 
ViaVBEGetVideoMemSize(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) { 
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10; 
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetVideoMemSize\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0000; /* ??? */

	pInt10->cx = 0x00;
	pInt10->dx = 0x00;
	pInt10->di = 0x00;
	pInt10->si = 0x00;

	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetVideoMemSize: VBE call failed.\n");
	    return 0;
	}
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Memory Size: %d\n", pInt10->si));
	if (pInt10->si > 1)
	    return pInt10->si;
	return 0;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetVideoMemSize: VBE not initialised.\n");
    return 0;
}

#ifdef UNUSED
/*
 * Bool ViaVBESetActiveDevice(ScrnInfoPtr pScrn);
 *
 *     - Set the active display device from pBIOSInfo
 *
 *
 * VBE OEM subfunction 0x8003 (from via code)
 *     cx = ActiveDevice
 *     dx = Mode numbers
 *     di = Refreshrate
 *
 */
static Bool 
ViaVBESetActiveDevice(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10; 
	VIABIOSInfoPtr  pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
	VIAModeTablePtr pViaModeTable = pBIOSInfo->pModeTable;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBESetActiveDevice\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x8003;
	
	/* Set Active Device and Translate BIOS byte definition */
	pInt10->cx = 0x00;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_CRT1)
	    pInt10->cx = 0x01;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_LCD)
	    pInt10->cx |= 0x02;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_TV)
	    pInt10->cx |= 0x04;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_DFP)
	    pInt10->cx |= 0x20;
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Active Device: %d\n", pInt10->cx));
	
	/* Set Current mode */
	pInt10->dx = pViaModeTable->Modes[pBIOSInfo->ModeIndex].Mode;
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Mode Number: %d\n", pInt10->dx));
	
	/* Set Current Refresh rate */
	switch(pBIOSInfo->Refresh) {
	case 60:
	    pInt10->di = 0;
	    break;
	case 75:
	    pInt10->di = 5;
	    break;
	case 85:
	    pInt10->di = 7;
	    break;
	case 100:
	    pInt10->di = 9;
	    break;
	case 120:
	    pInt10->di = 10;
	    break;
	default:
	    pInt10->di = 0;
	    break;
	}
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Refresh Rate Index: %d\n", pInt10->di));
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetActiveDevice: VBE call failed.\n");
	    return FALSE;
	}
	return TRUE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetActiveDevice: VBE not initialised.\n");
    return FALSE;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * Bool ViaVBEGetModeInfo(ScrnInfoPtr pScrn, int ModeNo, int *Xres, int *Yres, int *bpp);
 *
 *     - Retrieve resolution and bitdepth from VESA OEM for a given modenumber.
 *
 * Introduced in CLEXF40037
 *
 * VBE OEM subfunction 0x0302 (from via code)
 *     cx = ModeNo
 *     dx = 0x00
 * returns:
 *     bx = Vertical resolution
 *     cx = Horizontal resolution
 *     dx & 0xFF = bitdepth
 *
 */
static Bool 
ViaVBEGetModeInfo(ScrnInfoPtr pScrn, int ModeNo, int *Xres, int *Yres, int *bpp)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10; 
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEGetModeInfo\n"));
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Retrieving info for Mode: %d\n", ModeNo));

	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0302;

	pInt10->cx = ModeNo;
	pInt10->dx = 0x00;
	/* pInt10->di = 0x00;
	   pInt10->si = 0x00; */
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);

	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetModeInfo: VBE call failed.\n");
	    return FALSE;
	}

	*Xres = pInt10->cx;
	*Yres = pInt10->bx;
	*bpp = pInt10->dx & 0xFF;

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Retrieved Xres: %d; Yres: %d; bpp %d\n", 
			 *Xres, *Yres, *bpp));
    	return TRUE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEGetModeInfo: VBE not initialised.\n");
    return FALSE;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * int ViaVBEQueryModeList(ScrnInfoPtr pScrn, int serno, int* ModeNo);
 *
 *     - ...
 *
 * Introduced in CLEXF40037
 *
 * VBE OEM subfunction 0x0202 (from via code)
 *     cx = 0x00
 *     dx = serno
 * returns
 *     cx = Mode number
 *     dx = serno (next?)
 */
static int 
ViaVBEQueryModeList(ScrnInfoPtr pScrn, int serno, int* ModeNo)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEQueryModeList\n"));

	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0202;
	
	pInt10->cx = 0x00;
	pInt10->dx = serno;
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEQueryModeList: VBE call failed.\n");
	    return 0x00;
	}
    
	*ModeNo = pInt10->cx;
	
	return pInt10->dx;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEQueryModeList: VBE not initialised.\n");
    return 0x00;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * int ViaVBEQuerySupportedRefreshRate(ScrnInfoPtr pScrn, int ModeNo,
 *                                     int serno, int *refIndex);
 *
 *    - ...
 *
 * Introduced in CLEXF40037
 *
 * VBE OEM subfunction 0x0201 (from via code)
 *    cx = Mode Number
 *    dx = serial number
 * returns
 *    bx = index to refresh rate table
 *    dx = serial number
 */
static int 
ViaVBEQuerySupportedRefreshRate(ScrnInfoPtr pScrn, int ModeNo, int serno, int *refIndex)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBEQuerySupportRefreshRate\n"));
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ModeNo:%x  SerialNo:%d\n", ModeNo, serno));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0201;
	
	pInt10->cx = ModeNo;
	pInt10->dx = serno;
	/* pInt10->di = 0x00; */
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEQuerySupportedRefreshRate: VBE call failed.\n");
	    return 0x00;
	}
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "refIndex:%d  SerialNo:%d\n", pInt10->bx, pInt10->dx));
	
	*refIndex = pInt10->bx;
	return pInt10->dx;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBEQuerySupportedRefreshRate: VBE not initialised.\n");
    return 0x00;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * Bool ViaVBESetDeviceRefreshRate(ScrnInfoPtr pScrn);
 *
 *    - ...
 *
 * Introduced in CLEXF40037
 *
 * VBE OEM subfunction 0x0001 (from via code)
 *     cx = active device
 *     dx = mode (set to 0 currently (by via))
 *     di = refresh rate index
 */
static Bool 
ViaVBESetDeviceRefreshRate(ScrnInfoPtr pScrn)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;
	VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBESetDeviceRefreshRate\n"));
	
	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0001;
	
	/* Set Active Device and Translate BIOS byte definition */
	pInt10->cx = 0;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_CRT1)
	    pInt10->cx = 0x01;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_LCD)
	    pInt10->cx |= 0x02;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_TV)
	    pInt10->cx |= 0x04;
	if (pBIOSInfo->ActiveDevice & VIA_DEVICE_DFP)
	    pInt10->cx |= 0x20;
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Active device: %d\n", pInt10->cx));
	
	/* Set Current mode */
	pInt10->dx = 0;
	/*
	   pInt10->dx = pViaModeTable->Modes[pBIOSInfo->ModeIndex].Mode;
	   DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Mode number: %d\n", pInt10->dx));
	*/
	
	/* Set Current Refresh rate */
	switch(pBIOSInfo->Refresh) {
	case 60:
	    pInt10->di = 0;
	    break;
	case 75:
	    pInt10->di = 5;
	    break;
	case 85:
	    pInt10->di = 7;
	    break;
	case 100:
	    pInt10->di = 9;
	    break;
	case 120:
	    pInt10->di = 10;
	    break;
	default:
	    pInt10->di = 0;
	    break;
	}
	
	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Refresh rate index: %d\n", pInt10->di));
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
    
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetDeviceRefreshRate: VBE call failed.\n");
	    return FALSE;
	}
	return TRUE;
    }
     xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetDeviceRefreshRate: VBE not initialised.\n");
     return FALSE;
}
#endif /* UNUSED */

#ifdef UNUSED
/*
 * Bool ViaVBESetFlatPanelState(ScrnInfoPtr pScrn, Bool expand);
 *
 *    - ...
 *
 * Introduced in CLEXF40037
 *
 * VBE OEM subfunction 0x0306 (from via code)
 *     cx = expand
 */
static Bool 
ViaVBESetFlatPanelState(ScrnInfoPtr pScrn, Bool expand)
{
    if (VIAPTR(pScrn)->pVbe) {
	xf86Int10InfoPtr pInt10 = VIAPTR(pScrn)->pVbe->pInt10;

	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaVBESetFlatPanelState\n"));
       	DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "expand: %d\n", expand));

	pInt10->ax = 0x4F14;
	pInt10->bx = 0x0306;
	
	if (expand)
	    pInt10->cx = 0x81;
	else
	    pInt10->cx = 0x80;
	
	pInt10->num = 0x10;
	xf86ExecX86int10(pInt10);
	
	if ((pInt10->ax & 0xFF) != 0x4F) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetFlatPanelState: VBE call failed.\n");
	    return FALSE;
	}
	return TRUE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaVBESetFlatPanelState: VBE not initialised.\n");
    return FALSE;
}
#endif /* UNUSED */
