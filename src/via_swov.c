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
 
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86fbman.h"

#include "via.h"
#ifdef XF86DRI
#include "xf86drm.h"
#endif

#include "via_overlay.h"
#include "via_driver.h"
#include "via_regrec.h"
#include "via_priv.h"
#include "via_swov.h"
#ifdef XF86DRI
#include "via_common.h"
#endif
#include "via_vgahw.h"
#include "via_id.h"

/*
 * Warning: this file contains revision checks which are CLE266 specific.
 * There seems to be no checking present for KM400 or more recent devices.
 *
 * TODO:
 *   - pVia->Chipset checking of course.
 *   - move content of pVia->HWDiff into pVia->swov
 *   - merge with CLEXF40040
 */
/*
 * HW Difference Flag
 * Moved here from via_hwdiff.c
 *
 * These are the entries of HWDiff used in our code (currently):
 *                     CLE266Ax   CLE266Cx   KM400     K8M800    PM800
 * ThreeHQVBuffer      FALSE      TRUE       TRUE      TRUE      TRUE
 * HQVFetchByteUnit    FALSE      TRUE       TRUE      TRUE      TRUE
 * SupportTwoColorKey  FALSE      TRUE       FALSE     FALSE     TRUE
 * HQVInitPatch        TRUE       FALSE      FALSE     FALSE     FALSE
 * HQVDisablePatch     FALSE      TRUE       TRUE      TRUE      FALSE
 *
 * This is now up to date with CLEXF40040. All unused entries were removed.
 * The functions depending on this struct are untouched. 
 *
 */
void 
VIAVidHWDiffInit(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIAHWDiff *HWDiff = &pVia->HWDiff;

    switch(pVia->Chipset)
    {
    case VIA_CLE266:
        if (CLE266_REV_IS_AX(pVia->ChipRev)) {
            HWDiff->dwThreeHQVBuffer = VID_HWDIFF_FALSE;
            HWDiff->dwHQVFetchByteUnit = VID_HWDIFF_FALSE;
            HWDiff->dwSupportTwoColorKey = VID_HWDIFF_FALSE;
            HWDiff->dwHQVInitPatch = VID_HWDIFF_TRUE;
            HWDiff->dwHQVDisablePatch = VID_HWDIFF_FALSE;
        } else {
            HWDiff->dwThreeHQVBuffer = VID_HWDIFF_TRUE;
            HWDiff->dwHQVFetchByteUnit = VID_HWDIFF_TRUE;
            HWDiff->dwSupportTwoColorKey = VID_HWDIFF_TRUE;
            HWDiff->dwHQVInitPatch = VID_HWDIFF_FALSE;
            HWDiff->dwHQVDisablePatch = VID_HWDIFF_TRUE;
        }
        break;
    case VIA_KM400:
        HWDiff->dwThreeHQVBuffer = VID_HWDIFF_TRUE;
        HWDiff->dwHQVFetchByteUnit = VID_HWDIFF_TRUE;
        HWDiff->dwSupportTwoColorKey = VID_HWDIFF_FALSE;
        HWDiff->dwHQVInitPatch = VID_HWDIFF_FALSE;
        HWDiff->dwHQVDisablePatch = VID_HWDIFF_TRUE;
        break;
#ifdef HAVE_K8M800
    case VIA_K8M800:
        HWDiff->dwThreeHQVBuffer = VID_HWDIFF_TRUE;
        HWDiff->dwHQVFetchByteUnit = VID_HWDIFF_TRUE;
        HWDiff->dwSupportTwoColorKey = VID_HWDIFF_FALSE;
        HWDiff->dwHQVInitPatch = VID_HWDIFF_FALSE;
        HWDiff->dwHQVDisablePatch = VID_HWDIFF_TRUE;
        break;
#endif /* HAVE_K8M800 */
#ifdef HAVE_PM800
    case VIA_PM800:
        HWDiff->dwThreeHQVBuffer = VID_HWDIFF_TRUE;
        HWDiff->dwHQVFetchByteUnit = VID_HWDIFF_TRUE;
        HWDiff->dwSupportTwoColorKey = VID_HWDIFF_TRUE;
        HWDiff->dwHQVInitPatch = VID_HWDIFF_FALSE;
        HWDiff->dwHQVDisablePatch = VID_HWDIFF_FALSE;
        break;
#endif /* HAVE_PM800 */
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "VIAVidHWDiffInit: Unhandled ChipSet.\n");
    }
}     

void viaSetColorSpace(VIAPtr pVia, int hue, int saturation, int brightness, int contrast,
                      Bool reset)
{
    CARD32 col1,col2;

    viaCalculateVideoColor(pVia,hue,saturation,brightness, contrast,reset,&col1,&col2);
    switch ( pVia->ChipId ) {
    case PCI_CHIP_VT3205:
        VIDOutD(V3_ColorSpaceReg_1, col1);
        VIDOutD(V3_ColorSpaceReg_2, col2);
        DBG_DD(ErrorF("000002C4 %08x\n",col1));
        DBG_DD(ErrorF("000002C8 %08x\n",col2));
        break;
    case PCI_CHIP_CLE3122:
        VIDOutD(V1_ColorSpaceReg_2, col2);
        VIDOutD(V1_ColorSpaceReg_1, col1);
        VIDOutD(V3_ColorSpaceReg_2, col2);
        VIDOutD(V3_ColorSpaceReg_1, col1);

        DBG_DD(ErrorF("00000288 %08x\n",col2));
        DBG_DD(ErrorF("00000284 %08x\n",col1));
        break;
    default:
        DBG_DD(ErrorF("Unknown DeviceID\n"));
        break;
    }
}

static unsigned long ViaInitVideoStatusFlag(VIAPtr pVia) 
{
    switch ( pVia->ChipId ) {
    case PCI_CHIP_VT3205:
        return VIDEO_HQV_INUSE | SW_USE_HQV | VIDEO_3_INUSE;
    case PCI_CHIP_CLE3122:
        return VIDEO_HQV_INUSE | SW_USE_HQV | VIDEO_1_INUSE;
    default:
        DBG_DD(ErrorF("Unknown DeviceID\n"));
        break;
    }
    return 0UL;
}

static unsigned long ViaSetVidCtl(VIAPtr pVia, unsigned int videoFlag)
{
    if (videoFlag & VIDEO_1_INUSE) {
        /*=* Modify for C1 FIFO *=*/
        /* WARNING: not checking Chipset! */
        if (CLE266_REV_IS_CX(pVia->ChipRev))
            return V1_ENABLE | V1_EXPIRE_NUM_F;
        else {
            /* Overlay source format for V1 */
            if (pVia->swov.gdwUseExtendedFIFO)
                return V1_ENABLE | V1_EXPIRE_NUM_A | V1_FIFO_EXTENDED;
            else
                return V1_ENABLE | V1_EXPIRE_NUM;
        }
    }
    else {
        switch (pVia->ChipId)
        {
        case PCI_CHIP_VT3205:
            return V3_ENABLE | V3_EXPIRE_NUM_3205;

        case PCI_CHIP_CLE3122:
            if (CLE266_REV_IS_CX(pVia->ChipRev)) 
                return V3_ENABLE | V3_EXPIRE_NUM_F;
            else 
                return V3_ENABLE | V3_EXPIRE_NUM;
            break;

        default:
            DBG_DD(ErrorF("Unknown DeviceID\n"));
            break;
        }
    }
    return 0;
}/*End of DeviceID*/

/*
 * Fill the buffer with 0x8000 (YUV2 black)
 */
static void
ViaYUVFillBlack(VIAPtr pVia, int offset, int num)
{
    CARD16 *ptr = (CARD16 *)(pVia->FBBase + offset);

    while(num-- > 0)
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
        *ptr++ = 0x0080;
#else
        *ptr++ = 0x8000;
#endif
}

/*
 * Add an HQV surface to an existing FOURCC surface.
 * numbuf: number of buffers, 1, 2 or 3
 * fourcc: FOURCC code of the current (already existing) surface
 */
static long AddHQVSurface(ScrnInfoPtr pScrn, unsigned int numbuf, CARD32 fourcc)
{
    unsigned int i, width, height, pitch, fbsize, addr;
    unsigned long retCode;
    BOOL isplanar;

    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 AddrReg[3] = {HQV_DST_STARTADDR0, HQV_DST_STARTADDR1, HQV_DST_STARTADDR2};

    isplanar = ((fourcc == FOURCC_YV12) || (fourcc == FOURCC_VIA));

    width  = pVia->swov.SWDevice.gdwSWSrcWidth;
    height = pVia->swov.SWDevice.gdwSWSrcHeight;
    pitch  = pVia->swov.SWDevice.dwPitch; 
    fbsize = pitch * height * (isplanar ? 2 : 1);

    VIAFreeLinear(&pVia->swov.HQVMem);
    retCode = VIAAllocLinear(&pVia->swov.HQVMem, pScrn, fbsize * numbuf);
    if(retCode != Success) return retCode;
    addr = pVia->swov.HQVMem.base;

    ViaYUVFillBlack(pVia, addr, fbsize);

    for (i = 0; i < numbuf; i++) {
        pVia->swov.overlayRecordV1.dwHQVAddr[i] = addr;
        VIDOutD(AddrReg[i], addr);
        addr += fbsize;
    }

    return Success;
}

/*
 * Create a FOURCC surface (Supported: YUY2, YV12 or VIA)
  * doalloc: set true to actually allocate memory for the framebuffers
 */
static long CreateSurface(ScrnInfoPtr pScrn, LPDDSURFACEDESC surfaceDesc, BOOL doalloc)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    unsigned long width, height, pitch, fbsize, addr;
    unsigned long retCode;
    BOOL isplanar;

    pVia->swov.DPFsrc.dwFlags = DDPF_FOURCC;
    pVia->swov.DPFsrc.dwFourCC = surfaceDesc->dwFourCC;
    pVia->swov.gdwVideoFlagSW = ViaInitVideoStatusFlag(pVia);

    isplanar = ((surfaceDesc->dwFourCC == FOURCC_YV12) ||
        (surfaceDesc->dwFourCC == FOURCC_VIA));

    width  = surfaceDesc->dwWidth;
    height = surfaceDesc->dwHeight;
    pitch  = ALIGN_TO(width, 32) * (isplanar ? 1 : 2);
    fbsize = pitch * height * (isplanar ? 1.5 : 1.0);

    VIAFreeLinear(&pVia->swov.SWfbMem);

    if (doalloc) {
        retCode = VIAAllocLinear(&pVia->swov.SWfbMem, pScrn, fbsize * 2);
        if(retCode != Success) return retCode;
        addr = pVia->swov.SWfbMem.base;

        ViaYUVFillBlack(pVia, addr, fbsize);

        pVia->swov.SWDevice.dwSWPhysicalAddr[0]   = addr;
        pVia->swov.SWDevice.dwSWPhysicalAddr[1]   = addr + fbsize;
        pVia->swov.SWDevice.lpSWOverlaySurface[0] = pVia->FBBase + addr;
        pVia->swov.SWDevice.lpSWOverlaySurface[1] =
            pVia->swov.SWDevice.lpSWOverlaySurface[0] + fbsize;

        if (isplanar) {
            pVia->swov.SWDevice.dwSWCrPhysicalAddr[0] =
                pVia->swov.SWDevice.dwSWPhysicalAddr[0] + (pitch*height);
            pVia->swov.SWDevice.dwSWCrPhysicalAddr[1] =
                pVia->swov.SWDevice.dwSWPhysicalAddr[1] + (pitch*height);
            pVia->swov.SWDevice.dwSWCbPhysicalAddr[0] =
                pVia->swov.SWDevice.dwSWCrPhysicalAddr[0] + ((pitch>>1)*(height>>1));
            pVia->swov.SWDevice.dwSWCbPhysicalAddr[1] =
                pVia->swov.SWDevice.dwSWCrPhysicalAddr[1] + ((pitch>>1)*(height>>1));
        }
    }

    pVia->swov.SWDevice.gdwSWSrcWidth = width;
    pVia->swov.SWDevice.gdwSWSrcHeight = height;
    pVia->swov.SWDevice.dwPitch = pitch;

    pVia->swov.overlayRecordV1.dwV1OriWidth = width;
    pVia->swov.overlayRecordV1.dwV1OriHeight = height;
    pVia->swov.overlayRecordV1.dwV1OriPitch = pitch;

    return Success;
}

/*************************************************************************
   Function : VIAVidCreateSurface
   Create overlay surface depend on FOURCC
*************************************************************************/
unsigned long VIAVidCreateSurface(ScrnInfoPtr pScrn, LPDDSURFACEDESC surfaceDesc)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    unsigned long retCode = Success;
    int numbuf = pVia->HWDiff.dwThreeHQVBuffer ? 3 : 2;

    if (surfaceDesc == NULL) return BadAccess;

    switch (surfaceDesc->dwFourCC)
    {
    case FOURCC_YUY2:
        retCode = CreateSurface(pScrn, surfaceDesc, TRUE);
        if (retCode != Success) break;
        if (!(pVia->swov.gdwVideoFlagSW & SW_USE_HQV)) break;

    case FOURCC_HQVSW:
        retCode = AddHQVSurface(pScrn, numbuf, FOURCC_YUY2);
        break;

    case FOURCC_YV12:
        retCode = CreateSurface(pScrn, surfaceDesc, TRUE);
        if (retCode == Success)
            retCode = AddHQVSurface(pScrn, numbuf, FOURCC_YV12);
        break;

    case FOURCC_VIA:
        retCode = CreateSurface(pScrn, surfaceDesc, FALSE);
        if (retCode == Success)
            retCode = AddHQVSurface(pScrn, numbuf, FOURCC_VIA);
        break;

    default:
        break;
    }

    return retCode;

} /*VIAVidCreateSurface*/

/*************************************************************************
   Function : VIAVidLockSurface
   Lock Surface
*************************************************************************/
unsigned long VIAVidLockSurface(ScrnInfoPtr pScrn, LPDDLOCK lpLock)
{
    VIAPtr  pVia = VIAPTR(pScrn);

    switch (lpLock->dwFourCC)
    {
    case FOURCC_YUY2:
    case FOURCC_YV12:
    case FOURCC_VIA:
        lpLock->SWDevice = pVia->swov.SWDevice;
        lpLock->dwPhysicalBase = pVia->FrameBufferBase;
    }

    return PI_OK;

} /*VIAVidLockSurface*/

/*************************************************************************
 *  Destroy Surface
*************************************************************************/
unsigned long VIAVidDestroySurface(ScrnInfoPtr pScrn,  LPDDSURFACEDESC lpDDSurfaceDesc)
{
    VIAPtr pVia = VIAPTR(pScrn);

    DBG_DD(ErrorF("//VIAVidDestroySurface: \n"));

    switch (lpDDSurfaceDesc->dwFourCC)
    {
    case FOURCC_YUY2:
        pVia->swov.DPFsrc.dwFlags = 0;
        pVia->swov.DPFsrc.dwFourCC = 0;

        VIAFreeLinear(&pVia->swov.SWfbMem);
        if (!(pVia->swov.gdwVideoFlagSW & SW_USE_HQV))
        {
            pVia->swov.gdwVideoFlagSW = 0;
            break;
        }

    case FOURCC_HQVSW:
        VIAFreeLinear(&pVia->swov.HQVMem);
        pVia->swov.gdwVideoFlagSW = 0;
        break;

    case FOURCC_YV12:
    case FOURCC_VIA:
        pVia->swov.DPFsrc.dwFlags = 0;
        pVia->swov.DPFsrc.dwFourCC = 0;

        VIAFreeLinear(&pVia->swov.SWfbMem);
        VIAFreeLinear(&pVia->swov.HQVMem);
        pVia->swov.gdwVideoFlagSW = 0;
        break;
    }
    return PI_OK;

} /*VIAVidDestroySurface*/

static void SetFIFO_V1(VIAPtr pVia, CARD8 depth, CARD8 prethreshold, CARD8 threshold) 
{
    SaveVideoRegister(pVia, V_FIFO_CONTROL, ((depth-1) & 0x7f) |
        ((prethreshold & 0x7f) << 24) | ((threshold & 0x7f) << 8));
}

static void SetFIFO_V3(VIAPtr pVia, CARD8 depth, CARD8 prethreshold, CARD8 threshold)
{
    SaveVideoRegister(pVia, ALPHA_V3_FIFO_CONTROL,
        (VIDInD(ALPHA_V3_FIFO_CONTROL) & ALPHA_FIFO_MASK) |
        ((depth - 1) & 0xff) | ((threshold & 0xff) << 8));
    SaveVideoRegister(pVia, ALPHA_V3_PREFIFO_CONTROL,
        (VIDInD(ALPHA_V3_PREFIFO_CONTROL) & ~V3_FIFO_MASK) |
        (prethreshold & 0x7f));
}

static void SetFIFO_64or32(VIAPtr pVia)
{
    /*=* Modify for C1 FIFO *=*/
    /* WARNING: not checking Chipset! */
    if (CLE266_REV_IS_CX(pVia->ChipRev))
        SetFIFO_V1(pVia, 64, 56, 56);
    else
        SetFIFO_V1(pVia, 32, 29, 16);
}

static void SetFIFO_64or16(VIAPtr pVia)
{
    /*=* Modify for C1 FIFO *=*/
    /* WARNING: not checking Chipset! */
    if (CLE266_REV_IS_CX(pVia->ChipRev))
        SetFIFO_V1(pVia, 64, 56, 56);
    else
        SetFIFO_V1(pVia, 16, 12, 8);
}

static void SetFIFO_64or48or32(VIAPtr pVia)
{
    /*=* Modify for C1 FIFO *=*/
    /* WARNING: not checking Chipset! */
    if (CLE266_REV_IS_CX(pVia->ChipRev))
        SetFIFO_V1(pVia, 64, 56, 56);
    else {
        if (pVia->swov.gdwUseExtendedFIFO)
            SetFIFO_V1(pVia, 48, 40, 40);
        else
            SetFIFO_V1(pVia, 32, 29, 16);
    }                        
}

static void SetFIFO_V3_64or32or32(VIAPtr pVia)
{
    switch (pVia->ChipId)
    {
    case PCI_CHIP_VT3205:
        SetFIFO_V3(pVia, 32, 29, 29);
        break;

    case PCI_CHIP_CLE3122:
        if (CLE266_REV_IS_CX(pVia->ChipRev))
            SetFIFO_V3(pVia, 64, 56, 56);
        else
            SetFIFO_V3(pVia, 32, 16, 16);
        break;

    default:
        break;
    }
}

static void SetFIFO_V3_64or32or16(VIAPtr pVia)
{
    switch (pVia->ChipId)
    {
    case PCI_CHIP_VT3205:
        SetFIFO_V3(pVia, 32, 29, 29);
        break;

    case PCI_CHIP_CLE3122:
        if (CLE266_REV_IS_CX(pVia->ChipRev))
            SetFIFO_V3(pVia, 64, 56, 56);
        else
            SetFIFO_V3(pVia, 16, 16, 8);
        break;

    default:
        break;
    }
}

static void
SetupFIFOs(VIAPtr pVia, unsigned long videoFlag, unsigned long miniCtl,
	   unsigned long srcWidth)
{
    if (miniCtl & V1_Y_INTERPOLY)
    {
        if (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12 ||
            pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA)
        {
            if (videoFlag & VIDEO_HQV_INUSE)
            {
                if (videoFlag & VIDEO_1_INUSE)
                    SetFIFO_64or32(pVia);
                else
                    SetFIFO_V3_64or32or16(pVia);
            }
            else
            {
                /* Minified video will be skewed without this workaround. */
                if (srcWidth <= 80) /* Fetch count <= 5 */
                {
                    if (videoFlag & VIDEO_1_INUSE)
                        SetFIFO_V1(pVia, 16, 0, 0);
                    else
                        SetFIFO_V3(pVia, 16, 16, 0);
                }
                else
                {
                    if (videoFlag & VIDEO_1_INUSE)
                        SetFIFO_64or16(pVia);
                    else
                        SetFIFO_V3_64or32or16(pVia);
                }
            }
        }
        else
        {
            if (videoFlag & VIDEO_1_INUSE)
                SetFIFO_64or48or32(pVia);
            else
            {
                /* Fix V3 bug. */
                if (srcWidth <= 8)
                    SetFIFO_V3(pVia, 1, 0, 0);
                else
                    SetFIFO_V3_64or32or32(pVia);
            }
        }
    }
    else 
    {
        if (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12 ||
            pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA)
        {
            if (videoFlag & VIDEO_HQV_INUSE)
            {
                if (videoFlag & VIDEO_1_INUSE)
                    SetFIFO_64or32(pVia);
                else
                    SetFIFO_V3_64or32or16(pVia);
            }
            else
            {
                /* Minified video will be skewed without this workaround. */
                if (srcWidth <= 80) /* Fetch count <= 5 */
                {
                    if (videoFlag & VIDEO_1_INUSE)
                        SetFIFO_V1(pVia, 16, 0, 0);
                    else
                        SetFIFO_V3(pVia, 16, 16, 0);
                }
                else
                {
                    if (videoFlag & VIDEO_1_INUSE)
                        SetFIFO_64or16(pVia);
                    else
                        SetFIFO_V3_64or32or16(pVia);
                }
            }
        }
        else
        {
            if (videoFlag & VIDEO_1_INUSE)
                SetFIFO_64or48or32(pVia);
            else
            {
                /* Fix V3 bug. */
                if (srcWidth <= 8)
                    SetFIFO_V3(pVia, 1, 0, 0);
                else
                    SetFIFO_V3_64or32or32(pVia);
            }
        }
    }
}

static CARD32 SetColorKey(VIAPtr pVia, unsigned long videoFlag,
                          CARD32 keyLow, CARD32 keyHigh, CARD32 compose)
{
    keyLow &= 0x00FFFFFF;
    /*SaveVideoRegister(pVia, V_COLOR_KEY, keyLow);*/

    if (videoFlag & VIDEO_1_INUSE) {
        SaveVideoRegister(pVia, V_COLOR_KEY, keyLow);
    }
    else {
        if (pVia->HWDiff.dwSupportTwoColorKey )    /*CLE_C0*/
            SaveVideoRegister(pVia, V3_COLOR_KEY, keyLow);
    }

    /*compose = (compose & ~0x0f) | SELECT_VIDEO_IF_COLOR_KEY;*/
    /*CLE_C0*/
    compose = (compose & ~0x0f) | SELECT_VIDEO_IF_COLOR_KEY | SELECT_VIDEO3_IF_COLOR_KEY;
    /*compose = (compose & ~0x0f)  ;*/

    return compose;
}


static CARD32 SetChromaKey(VIAPtr pVia, unsigned long videoFlag,
                           CARD32 chromaLow, CARD32 chromaHigh,
                           CARD32 miniCtl, CARD32 compose)
{
    unsigned long lowR, lowG, lowB, lowC; /* Low RGB and chroma. */
    unsigned long highR, highG, highB, highC; /* High RGB and chroma. */

    chromaLow  &= CHROMA_KEY_LOW;
    chromaHigh &= CHROMA_KEY_HIGH;

    chromaLow  |= (VIDInD(V_CHROMAKEY_LOW) & ~CHROMA_KEY_LOW);
    chromaHigh |= (VIDInD(V_CHROMAKEY_HIGH)& ~CHROMA_KEY_HIGH);

    /*Added by Scottie[2001.12.5] for Chroma Key*/
    if (pVia->swov.DPFsrc.dwFlags & DDPF_FOURCC)
    {
        switch (pVia->swov.DPFsrc.dwFourCC)
        {
        case FOURCC_YV12:
        case FOURCC_VIA:
            /*to be continued...*/
            break;
        case FOURCC_YUY2:
            /*to be continued...*/
            break;
        default:
            /*TOINT3;*/
            break;
        }
    }
    else if (pVia->swov.DPFsrc.dwFlags & DDPF_RGB)
    {
        switch (pVia->swov.DPFsrc.dwRGBBitCount) 
        {
        case 16:
            if (pVia->swov.DPFsrc.dwGBitMask==0x07E0) /*RGB16(5:6:5)*/
            {
                lowR = (((chromaLow >> 11) << 3) | ((chromaLow >> 13) & 0x07)) & 0xFF;
                lowG = (((chromaLow >> 5) << 2) | ((chromaLow >> 9) & 0x03)) & 0xFF;

                highR = (((chromaHigh >> 11) << 3) | ((chromaHigh >> 13) & 0x07)) & 0xFF;
                highG = (((chromaHigh >> 5) << 2) | ((chromaHigh >> 9) & 0x03)) & 0xFF;
            }
            else /*RGB15(5:5:5)*/
            {
                lowR = (((chromaLow >> 10) << 3) | ((chromaLow >> 12) & 0x07)) & 0xFF;
                lowG = (((chromaLow >> 5) << 3) | ((chromaLow >> 7) & 0x07)) & 0xFF;

                highR = (((chromaHigh >> 10) << 3) | ((chromaHigh >> 12) & 0x07)) & 0xFF;
                highG = (((chromaHigh >> 5) << 3) | ((chromaHigh >> 7) & 0x07)) & 0xFF;
            }
            lowB = (((chromaLow << 3) | (chromaLow >> 2)) & 0x07) & 0xFF;
            lowC = (lowG << 16) | (lowR << 8) | lowB;
            chromaLow = ((chromaLow >> 24) << 24) | lowC;

            highB = (((chromaHigh << 3) | (chromaHigh >> 2)) & 0x07) & 0xFF;
            highC = (highG << 16) | (highR << 8) | highB;
            chromaHigh = ((chromaHigh >> 24) << 24) | highC;
            break;

        case 32: /*32 bit RGB*/
            lowR = (chromaLow >> 16) & 0xFF;
            lowG = (chromaLow >> 8) & 0xFF;
            lowB = chromaLow & 0xFF;
            lowC = (lowG << 16) | (lowR << 8) | lowB;
            chromaLow = ((chromaLow >> 24) << 24) | lowC;

            highR = (chromaHigh >> 16) & 0xFF;
            highG = (chromaHigh >> 8) & 0xFF;
            highB = chromaHigh & 0xFF;
            highC = (highG << 16) | (highR << 8) | highB;
            chromaHigh = ((chromaHigh >> 24) << 24) | highC;
            break;

        default:
            /*TOINT3;*/
            break;
        }
    }/*End of DDPF_FOURCC*/

    SaveVideoRegister(pVia, V_CHROMAKEY_HIGH, chromaHigh);
    if (videoFlag & VIDEO_1_INUSE)
    {
        SaveVideoRegister(pVia, V_CHROMAKEY_LOW, chromaLow);
        /*Temporarily solve the H/W Interpolation error when using Chroma Key*/
        SaveVideoRegister(pVia, V1_MINI_CONTROL, miniCtl & 0xFFFFFFF8);
    }
    else
    {
        SaveVideoRegister(pVia, V_CHROMAKEY_LOW, chromaLow | V_CHROMAKEY_V3);
        SaveVideoRegister(pVia, V3_MINI_CONTROL, miniCtl & 0xFFFFFFF8);
    }

    /*Modified by Scottie[2001.12.5] for select video if (color key & chroma key)*/
    if (compose == SELECT_VIDEO_IF_COLOR_KEY)
        compose = SELECT_VIDEO_IF_COLOR_KEY | SELECT_VIDEO_IF_CHROMA_KEY;
    else
        compose = (compose & ~0x0f) | SELECT_VIDEO_IF_CHROMA_KEY;

    return compose;
}

static void SetVideoStart(VIAPtr pVia, unsigned long videoFlag,
                          unsigned int numbufs, CARD32 a1, CARD32 a2, CARD32 a3)
{
    CARD32 V1Addr[3] = {V1_STARTADDR_0, V1_STARTADDR_1, V1_STARTADDR_2};
    CARD32 V3Addr[3] = {V3_STARTADDR_0, V3_STARTADDR_1, V3_STARTADDR_2};
    CARD32* VideoAddr = (videoFlag & VIDEO_1_INUSE) ? V1Addr : V3Addr;

    SaveVideoRegister(pVia, VideoAddr[0], a1);
    if (numbufs > 1) SaveVideoRegister(pVia, VideoAddr[1], a2);
    if (numbufs > 2) SaveVideoRegister(pVia, VideoAddr[2], a3);
}

static void SetHQVFetch(VIAPtr pVia, CARD32 srcFetch, unsigned long srcHeight)
{
    if (!pVia->HWDiff.dwHQVFetchByteUnit) {   /* CLE_C0 */
        srcFetch >>= 3; /* fetch unit is 8-byte */
    }
    SaveVideoRegister(pVia, HQV_SRC_FETCH_LINE, ((srcFetch - 1) << 16) | (srcHeight - 1));
}

static void SetFetch(VIAPtr pVia, unsigned long videoFlag, CARD32 fetch)
{
    fetch <<= 20;
    if (videoFlag & VIDEO_1_INUSE) {
        SaveVideoRegister(pVia, V12_QWORD_PER_LINE, fetch);
    }
    else {
        fetch |= VIDInD(V3_ALPHA_QWORD_PER_LINE) & ~V3_FETCH_COUNT;
        SaveVideoRegister(pVia, V3_ALPHA_QWORD_PER_LINE, fetch);
    }
}

static void SetDisplayCount(VIAPtr pVia, unsigned long videoFlag,
                            unsigned long srcHeight, CARD32 displayCountW)
{
    if (videoFlag & VIDEO_1_INUSE)
        SaveVideoRegister(pVia, V1_SOURCE_HEIGHT, (srcHeight << 16) | displayCountW);
    else
        SaveVideoRegister(pVia, V3_SOURCE_WIDTH, displayCountW);
}

static void SetMiniAndZoom(VIAPtr pVia, unsigned long videoFlag,
                           CARD32 miniCtl, CARD32 zoomCtl)
{
    if (videoFlag & VIDEO_1_INUSE) {
        SaveVideoRegister(pVia, V1_MINI_CONTROL, miniCtl);
        SaveVideoRegister(pVia, V1_ZOOM_CONTROL, zoomCtl);
    }
    else {
        SaveVideoRegister(pVia, V3_MINI_CONTROL, miniCtl);
        SaveVideoRegister(pVia, V3_ZOOM_CONTROL, zoomCtl);
    }
}

static void SetVideoControl(VIAPtr pVia, unsigned long videoFlag, CARD32 vidCtl)
{
    if (videoFlag & VIDEO_1_INUSE)
        SaveVideoRegister(pVia, V1_CONTROL, vidCtl);
    else
        SaveVideoRegister(pVia, V3_CONTROL, vidCtl);
}

static void FireVideoCommand(VIAPtr pVia, unsigned long videoFlag, CARD32 compose)
{
    if (videoFlag & VIDEO_1_INUSE)
        SaveVideoRegister(pVia, V_COMPOSE_MODE, compose | V1_COMMAND_FIRE);
    else
        SaveVideoRegister(pVia, V_COMPOSE_MODE, compose | V3_COMMAND_FIRE);
}

static void SetVideoWindow(VIAPtr pVia, unsigned long videoFlag,
                           int left, int top, int right, int bottom)
{
    if (top < 0) top = 0;
    if (bottom < 0) bottom = 0;
    if (left < 0) left = 0;
    if (right < 0) right = 0;

    if (top > 2047) top = 2047;
    if (bottom > 2047) bottom = 2047;
    if (left > 2047) left = 2047;
    if (right > 2047) right = 2047;

    if (videoFlag & VIDEO_1_INUSE) {
        SaveVideoRegister(pVia, V1_WIN_END_Y,  (right << 16) | bottom);
        SaveVideoRegister(pVia, V1_WIN_START_Y, (left << 16) | top);
    }
    else {
        SaveVideoRegister(pVia, V3_WIN_END_Y,  (right << 16) | bottom);
        SaveVideoRegister(pVia, V3_WIN_START_Y, (left << 16) | top);
    }
}



/****************************************************************************
 *
 * Upd_Video()
 *
 ***************************************************************************/
static unsigned long Upd_Video(ScrnInfoPtr pScrn, unsigned long videoFlag,
                               unsigned long startAddr, RECTL rSrc, RECTL rDest,
                               unsigned long srcPitch,
                               unsigned long oriSrcWidth,  unsigned long oriSrcHeight,
                               LPDDPIXELFORMAT pPFsrc,
                               unsigned long deinterlaceMode,
                               unsigned long haveColorKey, unsigned long haveChromaKey,
                               unsigned long colorKeyLow,  unsigned long colorKeyHigh,
                               unsigned long chromaKeyLow, unsigned long chromaKeyHigh,
                               unsigned long flags)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAHWDiff *hwDiff = &pVia->HWDiff;

    int i, j;
    unsigned long vidCtl = 0, compose;
    unsigned long srcWidth, srcHeight, dstWidth, dstHeight;
    unsigned long zoomCtl = 0, miniCtl = 0;
    unsigned long hqvCtl = 0;
    unsigned long hqvFilterCtl = 0, hqvMiniCtl = 0;
    unsigned long haveHQVzoomH = 0, haveHQVzoomV = 0;
    unsigned long hqvSrcWidth = 0, hqvDstWidth = 0;
    unsigned long hqvSrcFetch = 0, hqvOffset = 0;
    unsigned long dwOffset = 0,fetch = 0,tmp = 0;
    unsigned long displayCountW = 0;

    compose = (VIDInD(V_COMPOSE_MODE) & 
        ~(SELECT_VIDEO_IF_COLOR_KEY | V1_COMMAND_FIRE | V3_COMMAND_FIRE)) | V_COMMAND_LOAD_VBI;

    DBG_DD(ErrorF("// Upd_Video:\n"));
    DBG_DD(ErrorF("Modified rSrc  X (%ld,%ld) Y (%ld,%ld)\n",
        rSrc.left, rSrc.right,rSrc.top, rSrc.bottom));
    DBG_DD(ErrorF("Modified rDest  X (%ld,%ld) Y (%ld,%ld)\n",
        rDest.left, rDest.right,rDest.top, rDest.bottom));

    if (videoFlag & VIDEO_SHOW)
    {        
        pVia->swov.overlayRecordV1.dwWidth  = dstWidth  = rDest.right - rDest.left;
        pVia->swov.overlayRecordV1.dwHeight = dstHeight = rDest.bottom - rDest.top;
        srcWidth  = (unsigned long) rSrc.right - rSrc.left;
        srcHeight = (unsigned long) rSrc.bottom - rSrc.top;
        DBG_DD(ErrorF("===srcWidth= %ld \n", srcWidth));
        DBG_DD(ErrorF("===srcHeight= %ld \n", srcHeight));

        vidCtl = ViaSetVidCtl(pVia, videoFlag);
        viaOverlayGetV1V3Format(pVia, (videoFlag & VIDEO_1_INUSE) ? 1 : 3,
            videoFlag, pPFsrc, &vidCtl, &hqvCtl);

        if (hwDiff->dwThreeHQVBuffer) { /* CLE_C0: HQV supports triple-buffering */
            hqvCtl &= ~HQV_SW_FLIP;
            hqvCtl |= HQV_TRIPLE_BUFF | HQV_FLIP_STATUS; 
        }

        /* Starting address of source and Source offset*/
        dwOffset = viaOverlayGetSrcStartAddress(pVia, videoFlag,
            rSrc, rDest, srcPitch, pPFsrc, &hqvOffset);
        DBG_DD(ErrorF("===dwOffset= 0x%lx \n", dwOffset));

        pVia->swov.overlayRecordV1.dwOffset = dwOffset;

        if (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12 ||
            pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA)
        {
            YCBCRREC YCbCr;
            if (videoFlag & VIDEO_HQV_INUSE) {
                hqvSrcWidth = (unsigned long) rSrc.right - rSrc.left;
                hqvDstWidth = (unsigned long) rDest.right - rDest.left;

                SetVideoStart(pVia, videoFlag,
                    hwDiff->dwThreeHQVBuffer ? 3 : 2,
                    pVia->swov.overlayRecordV1.dwHQVAddr[0]+dwOffset,
                    pVia->swov.overlayRecordV1.dwHQVAddr[1]+dwOffset,
                    pVia->swov.overlayRecordV1.dwHQVAddr[2]+dwOffset);

                YCbCr = viaOverlayGetYCbCrStartAddress(videoFlag,
                    startAddr,pVia->swov.overlayRecordV1.dwOffset,
                    pVia->swov.overlayRecordV1.dwUVoffset,
                    srcPitch,oriSrcHeight);

                SaveVideoRegister(pVia, HQV_SRC_STARTADDR_Y, YCbCr.dwY);
                SaveVideoRegister(pVia, HQV_SRC_STARTADDR_U, YCbCr.dwCR);
                SaveVideoRegister(pVia, HQV_SRC_STARTADDR_V, YCbCr.dwCB);
            }
            else {
                YCbCr = viaOverlayGetYCbCrStartAddress(videoFlag,
                    startAddr, pVia->swov.overlayRecordV1.dwOffset,
                    pVia->swov.overlayRecordV1.dwUVoffset,
                    srcPitch, oriSrcHeight);

                if (videoFlag & VIDEO_1_INUSE) {
                    SaveVideoRegister(pVia, V1_STARTADDR_0, YCbCr.dwY);
                    SaveVideoRegister(pVia, V1_STARTADDR_CB0, YCbCr.dwCR);
                    SaveVideoRegister(pVia, V1_STARTADDR_CR0, YCbCr.dwCB);
                }
                else {
                    DBG_DD(ErrorF("Upd_Video() : We do not support YV12 with V3!\n"));
                }
            }
        }
        else
        {
            if (videoFlag & VIDEO_HQV_INUSE) {
                hqvSrcWidth = (unsigned long) rSrc.right - rSrc.left;
                hqvDstWidth = (unsigned long) rDest.right - rDest.left;

                if (hqvSrcWidth > hqvDstWidth) {
                    dwOffset = dwOffset * hqvDstWidth / hqvSrcWidth;
                }

                SetVideoStart(pVia, videoFlag,
                    hwDiff->dwThreeHQVBuffer ? 3 : 2,
                    pVia->swov.overlayRecordV1.dwHQVAddr[0] + hqvOffset,
                    pVia->swov.overlayRecordV1.dwHQVAddr[1] + hqvOffset,
                    pVia->swov.overlayRecordV1.dwHQVAddr[2] + hqvOffset);

                SaveVideoRegister(pVia, HQV_SRC_STARTADDR_Y, startAddr);
            }
            else {
                startAddr += dwOffset;
                SetVideoStart(pVia, videoFlag, 1, startAddr, 0, 0);
            }
        }

        fetch = viaOverlayGetFetch(videoFlag, pPFsrc,
            srcWidth, dstWidth, oriSrcWidth, &hqvSrcFetch);
        DBG_DD(ErrorF("===fetch= 0x%lx \n", fetch));
/*
        //For DCT450 test-BOB INTERLEAVE
        if ((deinterlaceMode & DDOVER_INTERLEAVED) && (deinterlaceMode & DDOVER_BOB))
        {
            if (videoFlag & VIDEO_HQV_INUSE) {
                hqvCtl |= HQV_FIELD_2_FRAME | HQV_FRAME_2_FIELD | HQV_DEINTERLACE;
            }
            else {
                vidCtl |= V1_BOB_ENABLE | V1_FRAME_BASE;
            }
        }
        else if (deinterlaceMode & DDOVER_BOB) {
            if (videoFlag & VIDEO_HQV_INUSE)
            {
                //The HQV source data line count should be two times of the original line count
                hqvCtl |= HQV_FIELD_2_FRAME | HQV_DEINTERLACE;
            }
            else {
                vidCtl |= V1_BOB_ENABLE;
            }
        }
*/
        if (videoFlag & VIDEO_HQV_INUSE)
        {
            if (!(deinterlaceMode & DDOVER_INTERLEAVED) && (deinterlaceMode & DDOVER_BOB))
                SetHQVFetch(pVia, hqvSrcFetch, oriSrcHeight << 1);
            else
                SetHQVFetch(pVia, hqvSrcFetch, oriSrcHeight);

            if (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12 ||
                pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA)
            {
                if (videoFlag & VIDEO_1_INUSE)
                    SaveVideoRegister(pVia, V1_STRIDE, srcPitch << 1);
                else
                    SaveVideoRegister(pVia, V3_STRIDE, srcPitch << 1);

                SaveVideoRegister(pVia, HQV_SRC_STRIDE, ((srcPitch >> 1) << 16) | srcPitch);
                SaveVideoRegister(pVia, HQV_DST_STRIDE, (srcPitch << 1));
            }
            else
            {
                if (videoFlag & VIDEO_1_INUSE)
                    SaveVideoRegister(pVia, V1_STRIDE, srcPitch);
                else
                    SaveVideoRegister(pVia, V3_STRIDE, srcPitch);

                SaveVideoRegister(pVia, HQV_SRC_STRIDE, srcPitch);
                SaveVideoRegister(pVia, HQV_DST_STRIDE, srcPitch);
            }

        }
        else
        {
            if (videoFlag & VIDEO_1_INUSE)
                SaveVideoRegister(pVia, V1_STRIDE, srcPitch | (srcPitch << 15));
            else
                SaveVideoRegister(pVia, V3_STRIDE, srcPitch | (srcPitch << 15));
        }

        DBG_DD(ErrorF("rSrc  X (%ld,%ld) Y (%ld,%ld)\n",
            rSrc.left, rSrc.right,rSrc.top, rSrc.bottom));
        DBG_DD(ErrorF("rDest  X (%ld,%ld) Y (%ld,%ld)\n",
            rDest.left, rDest.right,rDest.top, rDest.bottom));

        /* Set destination window */

        i = rDest.top;
        j = rDest.bottom - 1;

        if (videoFlag & VIDEO_1_INUSE)    
        {
            /* modify for HW DVI limitation,
             * When we enable the CRT and DVI both, then change resolution.
             * If the resolution small than the panel physical size,
             * the video display in Y direction will be cut.
             * So, we need to adjust the Y top and bottom position.
             */
            if  (pVia->pBIOSInfo->SetDVI && pVia->pBIOSInfo->scaleY) {
                i = rDest.top * pVia->pBIOSInfo->panelY / pScrn->currentMode->VDisplay;
                j = rDest.bottom * pVia->pBIOSInfo->panelY / pScrn->currentMode->VDisplay;
            }
        }
        if (rDest.top < 0) i = 0;
        SetVideoWindow(pVia, videoFlag, rDest.left, i, rDest.right - 1, j);

        compose |= ALWAYS_SELECT_VIDEO;

        /* Setup X zoom factor*/

        pVia->swov.overlayRecordV1.dwFetchAlignment = 0;

        if (viaOverlayHQVCalcZoomWidth(pVia, videoFlag, srcWidth, dstWidth,
            &zoomCtl, &miniCtl, &hqvFilterCtl, &hqvMiniCtl, &haveHQVzoomH) == FALSE)
        {
            /* Need to scale (minify) too much - can't handle it. */
            SetFetch(pVia, videoFlag, fetch);
            FireVideoCommand(pVia, videoFlag, compose);
            FlushVidRegBuffer(pVia);
            return PI_ERR;
        }

        SetFetch(pVia, videoFlag, fetch);

        /* Setup Y zoom factor */

        /*For DCT450 test-BOB INTERLEAVE*/
        if ((deinterlaceMode & DDOVER_INTERLEAVED) && (deinterlaceMode & DDOVER_BOB))
        {
            if (!(videoFlag & VIDEO_HQV_INUSE)) {
                srcHeight /= 2;
                if (videoFlag & VIDEO_1_INUSE)
                    vidCtl |= V1_BOB_ENABLE | V1_FRAME_BASE;
                else
                    vidCtl |= V3_BOB_ENABLE | V3_FRAME_BASE;
            }
            else {
                hqvCtl |= HQV_FIELD_2_FRAME | HQV_FRAME_2_FIELD | HQV_DEINTERLACE;
            }
        }
        else if (deinterlaceMode & DDOVER_BOB)
        {
            if (videoFlag & VIDEO_HQV_INUSE) {
                srcHeight <<= 1;
                hqvCtl |= HQV_FIELD_2_FRAME | HQV_DEINTERLACE;
            }
            else {
                if (videoFlag & VIDEO_1_INUSE)
                    vidCtl |= V1_BOB_ENABLE;
                else
                    vidCtl |= V3_BOB_ENABLE;
            }
        }

        viaOverlayGetDisplayCount(pVia, videoFlag, pPFsrc, srcWidth, &displayCountW);
        SetDisplayCount(pVia, videoFlag, srcHeight, displayCountW);

        if (viaOverlayHQVCalcZoomHeight(pVia, srcHeight, dstHeight,
            &zoomCtl, &miniCtl, &hqvFilterCtl, &hqvMiniCtl ,&haveHQVzoomV) == FALSE)
        {
            /* Need to scale (minify) too much - can't handle it. */
            FireVideoCommand(pVia, videoFlag, compose);
            FlushVidRegBuffer(pVia);
            return PI_ERR;
        }

        SetupFIFOs(pVia, videoFlag, miniCtl, srcWidth);
        
        if (videoFlag & VIDEO_HQV_INUSE)
        {
            miniCtl=0;
            if (haveHQVzoomH || haveHQVzoomV) {
                tmp = 0;
                if (haveHQVzoomH) {
                    miniCtl = V1_X_INTERPOLY;
                    tmp = zoomCtl & 0xffff0000;
                }
                if (haveHQVzoomV) {
                    miniCtl |= V1_Y_INTERPOLY | V1_YCBCR_INTERPOLY;
                    tmp |= zoomCtl & 0x0000ffff;
                    hqvFilterCtl &= 0xfffdffff;
                }

                /* Temporary fix for 2D bandwidth problem. 2002/08/01*/
                if (pVia->swov.gdwUseExtendedFIFO) {
                    miniCtl &= ~V1_Y_INTERPOLY;
                }

                SetMiniAndZoom(pVia, videoFlag, miniCtl, tmp);
            }
            else {
                if (srcHeight == dstHeight) {
                    hqvFilterCtl &= 0xfffdffff;
                }
                SetMiniAndZoom(pVia, videoFlag, 0, 0);
            }
            SaveVideoRegister(pVia, HQV_MINIFY_CONTROL, hqvMiniCtl);
            SaveVideoRegister(pVia, HQV_FILTER_CONTROL, hqvFilterCtl);
        }
        else {
            SetMiniAndZoom(pVia, videoFlag, miniCtl, zoomCtl);
        }

        if (haveColorKey) {
            compose = SetColorKey(pVia, videoFlag, colorKeyLow, colorKeyHigh, compose);
        }

        if (haveChromaKey) {
            compose = SetChromaKey(pVia, videoFlag,
                chromaKeyLow, chromaKeyHigh, miniCtl, compose);
        }
        
        /* determine which video stream is on top */
        /*
        DBG_DD(ErrorF("        flags= 0x%08lx\n", flags));
        if (flags & DDOVER_CLIP)
        compose |= COMPOSE_V3_TOP;
        else
        compose |= COMPOSE_V1_TOP;
        */    

        /* Setup video control*/
        if (videoFlag & VIDEO_HQV_INUSE) {
            if (!pVia->swov.SWVideo_ON)
                /*if (0)*/
            {
                DBG_DD(ErrorF("    First HQV\n"));

                FlushVidRegBuffer(pVia);

                DBG_DD(ErrorF(" Wait flips"));

                if (hwDiff->dwHQVInitPatch) {
                    DBG_DD(ErrorF(" Initializing HQV twice ..."));
                    for (i = 0; i < 2; i++) {
                        viaWaitHQVFlipClear(pVia,
                            ((hqvCtl & ~HQV_SW_FLIP) | HQV_FLIP_STATUS) & ~HQV_ENABLE);
                        VIDOutD(HQV_CONTROL, hqvCtl);
                        viaWaitHQVFlip(pVia);
                    }
                    DBG_DD(ErrorF(" done.\n"));
                }
                else    /* CLE_C0 */
                {
                    CARD32 volatile *HQVCtrl =
                        (CARD32 volatile *) (pVia->VidMapBase + HQV_CONTROL);

                    /* check HQV is idle */

                    DBG_DD(ErrorF("HQV control wf - %08x\n", *HQVCtrl));
                    while(!(*HQVCtrl & HQV_IDLE)) {
                        DBG_DD(ErrorF("HQV control busy - %08x\n", *HQVCtrl));
                        usleep(1);
                    }

                    VIDOutD(HQV_CONTROL, hqvCtl & ~HQV_SW_FLIP);
                    VIDOutD(HQV_CONTROL, hqvCtl | HQV_SW_FLIP);

                    DBG_DD(ErrorF("HQV control wf5 - %08x\n", *HQVCtrl));
                    DBG_DD(ErrorF(" Wait flips5")); 

                    for (i = 0; (i < 50) && !(*HQVCtrl & HQV_FLIP_STATUS); i++) {
                        DBG_DD(ErrorF(" HQV wait %d %08x\n",i, *HQVCtrl));
                        *HQVCtrl |= HQV_SW_FLIP | HQV_FLIP_STATUS;
                        usleep(1);
                    }
#if 0
                    viaWaitHQVFlip(pVia);
#endif
                    DBG_DD(ErrorF(" Wait flips6"));
                }

                if (videoFlag & VIDEO_1_INUSE)
                {
                    VIDOutD(V1_CONTROL, vidCtl);
                    VIDOutD(V_COMPOSE_MODE, compose | V1_COMMAND_FIRE);
                    if (pVia->swov.gdwUseExtendedFIFO)
                    {
                        /*Set Display FIFO*/
                        DBG_DD(ErrorF(" Wait flips7"));
                        viaWaitVBI(pVia);
                        DBG_DD(ErrorF(" Wait flips 8"));
                        hwp->writeSeq(hwp, 0x17, 0x2F);
                        ViaSeqMask(hwp, 0x16, 0x14, 0x1F);
                        hwp->writeSeq(hwp, 0x18, 0x56);
                        DBG_DD(ErrorF(" Wait flips 9"));
                    }
                }
                else
                {
                    DBG_DD(ErrorF(" Wait flips 10"));
                    VIDOutD(V3_CONTROL, vidCtl);
                    VIDOutD(V_COMPOSE_MODE, compose | V3_COMMAND_FIRE);
                }
                DBG_DD(ErrorF(" Done flips"));
            }
            else
            {
                DBG_DD(ErrorF("    Normal called\n"));
                SetVideoControl(pVia, videoFlag, vidCtl);
                FireVideoCommand(pVia, videoFlag, compose);
                SaveVideoRegister(pVia, HQV_CONTROL, hqvCtl | HQV_FLIP_STATUS);
                viaWaitHQVDone(pVia);
                FlushVidRegBuffer(pVia);
            }
        }
        else
        {
            SetVideoControl(pVia, videoFlag, vidCtl);
            FireVideoCommand(pVia, videoFlag, compose);
            viaWaitHQVDone(pVia);
            FlushVidRegBuffer(pVia);
        }
        pVia->swov.SWVideo_ON = TRUE;
    }
    else
    {
        /* Hide overlay */

        if (hwDiff->dwHQVDisablePatch)     /*CLE_C0*/
            ViaSeqMask(hwp, 0x2E, 0x00, 0x10);

        SaveVideoRegister(pVia, V_FIFO_CONTROL, V1_FIFO_PRETHRESHOLD12 |
            V1_FIFO_THRESHOLD8 | V1_FIFO_DEPTH16);
        SaveVideoRegister(pVia, ALPHA_V3_FIFO_CONTROL, ALPHA_FIFO_THRESHOLD4
            | ALPHA_FIFO_DEPTH8 | V3_FIFO_THRESHOLD24 | V3_FIFO_DEPTH32);

        if (videoFlag & VIDEO_HQV_INUSE)
            SaveVideoRegister(pVia, HQV_CONTROL, VIDInD(HQV_CONTROL) & ~HQV_ENABLE);

        if (videoFlag & VIDEO_1_INUSE)
            SaveVideoRegister(pVia, V1_CONTROL, VIDInD(V1_CONTROL) & ~V1_ENABLE);
        else
            SaveVideoRegister(pVia, V3_CONTROL, VIDInD(V3_CONTROL) & ~V3_ENABLE);

        FireVideoCommand(pVia, videoFlag, VIDInD(V_COMPOSE_MODE));
        FlushVidRegBuffer(pVia);

        if (hwDiff->dwHQVDisablePatch)     /*CLE_C0*/
            ViaSeqMask(hwp, 0x2E, 0x10, 0x10);
    }
    DBG_DD(ErrorF(" Done Upd_Video"));

    return PI_OK;

} /* Upd_Video */

/*************************************************************************
 *  VIAVidUpdateOverlay
 *  Parameters:   src rectangle, dst rectangle, colorkey...
 *  Return Value: unsigned long of state
 *  note: Update the overlay image param.
 *************************************************************************/
unsigned long VIAVidUpdateOverlay(ScrnInfoPtr pScrn, LPDDUPDATEOVERLAY pUpdate)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    OVERLAYRECORD* ovlV1 = &pVia->swov.overlayRecordV1;

    unsigned long flags = pUpdate->dwFlags;
    unsigned long videoFlag = 0;
    unsigned long startAddr = 0;
    unsigned long deinterlaceMode=0;

    unsigned long haveColorKey = 0, haveChromaKey = 0;
    unsigned long colorKeyLow = 0, colorKeyHigh = 0;
    unsigned long chromaKeyLow = 0, chromaKeyHigh = 0;


    unsigned long scrnWidth, scrnHeight;
    int dstTop, dstBottom, dstLeft, dstRight;
    int panDX,panDY; /* Panning delta */


    panDX = pVia->swov.panning_x - pVia->swov.panning_old_x;
    panDY = pVia->swov.panning_y - pVia->swov.panning_old_y;

    /* Adjust to fix panning mode bug */

    pUpdate->rDest.left   -= panDX;
    pUpdate->rDest.top    -= panDY;
    pUpdate->rDest.right  -= panDX;
    pUpdate->rDest.bottom -= panDY;

    DBG_DD(ErrorF("Raw rSrc  X (%ld,%ld) Y (%ld,%ld)\n",
        pUpdate->rSrc.left, pUpdate->rSrc.right,
        pUpdate->rSrc.top, pUpdate->rSrc.bottom));
    DBG_DD(ErrorF("Raw rDest  X (%ld,%ld) Y (%ld,%ld)\n",
        pUpdate->rDest.left, pUpdate->rDest.right,
        pUpdate->rDest.top, pUpdate->rDest.bottom));

    if ((pVia->swov.DPFsrc.dwFourCC == FOURCC_YUY2) ||
        (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12) ||
        (pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA))
    {
        videoFlag = pVia->swov.gdwVideoFlagSW;
    }

    flags |= DDOVER_INTERLEAVED;

    /* Disable destination color keying if the alpha window is in use. */
    if (pVia->swov.gdwAlphaEnabled)
        flags &= ~DDOVER_KEYDEST;

    ResetVidRegBuffer(pVia);

    if (flags & DDOVER_HIDE)
    {
        videoFlag &= ~VIDEO_SHOW;
        if (Upd_Video(pScrn, videoFlag,0,pUpdate->rSrc,pUpdate->rDest,0,0,0,
            &pVia->swov.DPFsrc,0,0,0,0,0,0,0, flags)== PI_ERR)
        {
            return PI_ERR;
        }
        pVia->swov.SWVideo_ON = FALSE;

        if (pVia->swov.gdwUseExtendedFIFO)
        {
            /*Restore Display fifo*/
            hwp->writeSeq(hwp, 0x16, pVia->swov.Save_3C4_16);
            hwp->writeSeq(hwp, 0x17, pVia->swov.Save_3C4_17);
            hwp->writeSeq(hwp, 0x18, pVia->swov.Save_3C4_18);
            DBG_DD(ErrorF("Restore 3c4.16 : %08x \n", hwp->readSeq(hwp, 0x16)));
            DBG_DD(ErrorF("        3c4.17 : %08x \n", hwp->readSeq(hwp, 0x17)));
            DBG_DD(ErrorF("        3c4.18 : %08x \n", hwp->readSeq(hwp, 0x18)));
            pVia->swov.gdwUseExtendedFIFO = 0;
        }
        return PI_OK;
    }

    if (flags & DDOVER_SHOW)
    {
        /*for SW decode HW overlay use*/
        startAddr = VIDInD(HQV_SRC_STARTADDR_Y);

        if (flags & DDOVER_KEYDEST) {
            haveColorKey = 1;
            colorKeyLow = pUpdate->dwColorSpaceLowValue;
        }
        if (flags & DDOVER_INTERLEAVED) {
            deinterlaceMode |= DDOVER_INTERLEAVED;
        }
        if (flags & DDOVER_BOB) {
            deinterlaceMode |= DDOVER_BOB;
        }

        if (pVia->ChipId == PCI_CHIP_CLE3122) {
            if ((pScrn->currentMode->HDisplay > 1024))
            {
                DBG_DD(ErrorF("UseExtendedFIFO\n"));
                pVia->swov.gdwUseExtendedFIFO = 1;
            }

            /*
            else
            {
            //Set Display FIFO
            ViaSeqMask(hwp, 0x16, 0x0C, 0xE0);
            DBG_DD(ErrorF("set     3c4.16 : %08x \n",hwp->readSeq(hwp, 0x16)));
            hwp->writeSeq(hwp, 0x18, 0x4c);
            DBG_DD(ErrorF("        3c4.18 : %08x \n",hwp->readSeq(hwp, 0x18)));
            }
            */
        } else {
            pVia->swov.gdwUseExtendedFIFO = 0;
        }

        videoFlag |= VIDEO_SHOW;

        /* Figure out actual rSrc rectangle */

        dstLeft = pUpdate->rDest.left;
        dstTop = pUpdate->rDest.top;
        dstRight = pUpdate->rDest.right;
        dstBottom = pUpdate->rDest.bottom;

        scrnWidth  = pScrn->currentMode->HDisplay;
        scrnHeight = pScrn->currentMode->VDisplay;

        if (dstLeft < 0) {
            pUpdate->rSrc.left = (((-dstLeft) * ovlV1->dwV1OriWidth) +
                ((dstRight - dstLeft) >> 1)) / (dstRight - dstLeft);
        }
        if (dstRight > scrnWidth) {
            pUpdate->rSrc.right = (((scrnWidth - dstLeft) * ovlV1->dwV1OriWidth) +
                ((dstRight - dstLeft) >> 1)) / (dstRight - dstLeft);
        }
        if (dstTop < 0) {
            pUpdate->rSrc.top = (((-dstTop) * ovlV1->dwV1OriHeight) +
                ((dstBottom - dstTop) >> 1)) / (dstBottom - dstTop);
        }
        if (dstBottom > scrnHeight) {
            pUpdate->rSrc.bottom = (((scrnHeight - dstTop) * ovlV1->dwV1OriHeight) +
                ((dstBottom - dstTop) >> 1)) / (dstBottom - dstTop);
        }

        /* Save modified src & original dest rectangle param. */

        if ((pVia->swov.DPFsrc.dwFourCC == FOURCC_YUY2) ||
            (pVia->swov.DPFsrc.dwFourCC == FOURCC_YV12) ||
            (pVia->swov.DPFsrc.dwFourCC == FOURCC_VIA))
        {   
            pVia->swov.SWDevice.gdwSWDstLeft   = pUpdate->rDest.left + panDX;
            pVia->swov.SWDevice.gdwSWDstTop    = pUpdate->rDest.top + panDY;
            pVia->swov.SWDevice.gdwSWDstWidth  = pUpdate->rDest.right - pUpdate->rDest.left;
            pVia->swov.SWDevice.gdwSWDstHeight = pUpdate->rDest.bottom - pUpdate->rDest.top;

            pVia->swov.SWDevice.gdwSWSrcWidth  =
                ovlV1->dwV1SrcWidth = pUpdate->rSrc.right - pUpdate->rSrc.left;
            pVia->swov.SWDevice.gdwSWSrcHeight =
                ovlV1->dwV1SrcHeight = pUpdate->rSrc.bottom - pUpdate->rSrc.top;
        }

        ovlV1->dwV1SrcLeft  = pUpdate->rSrc.left;
        ovlV1->dwV1SrcRight = pUpdate->rSrc.right;
        ovlV1->dwV1SrcTop   = pUpdate->rSrc.top;
        ovlV1->dwV1SrcBot   = pUpdate->rSrc.bottom;
        
        /* Figure out actual rDest rectangle */

        pUpdate->rDest.left = (dstLeft < 0) ? 0 : dstLeft;
        pUpdate->rDest.top = (dstTop < 0) ? 0 : dstTop;
        if (pUpdate->rDest.top >= scrnHeight) pUpdate->rDest.top = scrnHeight-1;
        pUpdate->rDest.right = (dstRight > scrnWidth) ? scrnWidth : dstRight;
        pUpdate->rDest.bottom = (dstBottom > scrnHeight) ? scrnHeight : dstBottom;

        /* Update the overlay */

        if (Upd_Video(pScrn, videoFlag, startAddr, pUpdate->rSrc, pUpdate->rDest,
                pVia->swov.SWDevice.dwPitch, ovlV1->dwV1OriWidth,
                ovlV1->dwV1OriHeight, &pVia->swov.DPFsrc,
                deinterlaceMode, haveColorKey, haveChromaKey,
                colorKeyLow, colorKeyHigh, chromaKeyLow, chromaKeyHigh, flags) == PI_ERR)
        {
            return PI_ERR;
        }
        pVia->swov.SWVideo_ON = FALSE;

        return PI_OK;

    } /*end of DDOVER_SHOW*/

    pVia->swov.panning_old_x = pVia->swov.panning_x;
    pVia->swov.panning_old_y = pVia->swov.panning_y;

    return PI_OK;

} /*VIAVidUpdateOverlay*/


/*************************************************************************
 *  ADJUST FRAME
 *************************************************************************/
void VIAVidAdjustFrame(ScrnInfoPtr pScrn, int x, int y)
{
    VIAPtr pVia = VIAPTR(pScrn);
    pVia->swov.panning_x = x;
    pVia->swov.panning_y = y;
}
