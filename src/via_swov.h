/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_swov.h,v 1.4 2003/08/27 15:16:13 tsi Exp $ */
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

#ifndef _VIA_SWOV_H_
#define _VIA_SWOV_H_ 1

/*#define   XV_DEBUG      1*/     /* write log msg to /var/log/XFree86.0.log */

#ifdef XV_DEBUG
# define DBG_DD(x) (x)
#else
# define DBG_DD(x)
#endif

#include "via_priv.h"
#include "via_xvpriv.h"

/* Definition for VideoStatus */
#define VIDEO_NULL              0x00000000

/*For Video HW Difference */
#define VID_HWDIFF_TRUE           0x00000001
#define VID_HWDIFF_FALSE          0x00000000

/*
 *	Video HW Difference Structure
 */
typedef struct __VIAHWDiff
{
    unsigned long dwThreeHQVBuffer;	         /* Use Three HQV Buffers*/
    /* unsigned long dwV3SrcHeightSetting;*/	 /* Set Video Source Width and Height*/
    /* unsigned long dwSupportExtendFIFO;*/	 /* Support Extand FIFO*/
    unsigned long dwHQVFetchByteUnit;	         /* HQV Fetch Count unit is byte*/
    unsigned long dwHQVInitPatch;	         /* Initialize HQV Engine 2 times*/
    /*unsigned long dwSupportV3Gamma;*/	         /* Support V3 Gamma */
    /*unsigned long dwUpdFlip;*/		 /* Set HQV3D0[15] to flip video*/
    unsigned long dwHQVDisablePatch;	         /* Change Video Engine Clock setting for HQV disable bug*/
    /*unsigned long dwSUBFlip;*/		 /* Set HQV3D0[15] to flip video for sub-picture blending*/
    /*unsigned long dwNeedV3Prefetch;*/	         /* V3 pre-fetch function for K8*/
    /*unsigned long dwNeedV4Prefetch;*/	         /* V4 pre-fetch function for K8*/
    /*unsigned long dwUseSystemMemory;*/	 /* Use system memory for DXVA compressed data buffers*/
    /*unsigned long dwExpandVerPatch;*/          /* Patch video HW bug in expand SIM mode or same display path*/
    /*unsigned long dwExpandVerHorPatch;*/	 /* Patch video HW bug in expand SAMM mode or same display path*/
    /*unsigned long dwV3ExpireNumTune;*/ 	 /* Change V3 expire number setting for V3 bandwidth issue*/
    /*unsigned long dwV3FIFOThresholdTune;*/     /* Change V3 FIFO, Threshold and Pre-threshold setting for V3 bandwidth issue*/
    /*unsigned long dwCheckHQVFIFOEmpty;*/       /* HW Flip path, need to check HQV FIFO status */
    /*unsigned long dwUseMPEGAGP;*/              /* Use MPEG AGP function*/
    /*unsigned long dwV3FIFOPatch;*/             /* For CLE V3 FIFO Bug (srcWidth <= 8)*/
    unsigned long dwSupportTwoColorKey;          /* Support two color key*/
    /* unsigned long dwCxColorSpace; */          /* CLE_Cx ColorSpace*/
} VIAHWDiff;

void VIAVidHWDiffInit(ScrnInfoPtr pScrn); 
unsigned long VIAVidCreateSurface(ScrnInfoPtr pScrn, LPDDSURFACEDESC lpDDSurfaceDesc);
unsigned long VIAVidLockSurface(ScrnInfoPtr pScrn, LPDDLOCK lpLock);
unsigned long VIAVidDestroySurface(ScrnInfoPtr pScrn,  LPDDSURFACEDESC lpDDSurfaceDesc);
unsigned long VIAVidUpdateOverlay(ScrnInfoPtr pScrn, LPDDUPDATEOVERLAY lpUpdate);
unsigned long VIAVidAdjustFrame(ScrnInfoPtr pScr, LPADJUSTFRAME lpAdjustFrame);

#endif /* _VIA_SWOV_H_ */
