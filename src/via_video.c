/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_video.c,v 1.17 2004/02/04 04:15:09 dawes Exp $ */
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
 * I N C L U D E S
 */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86fbman.h"
#include "regionstr.h"
#include "via_driver.h"
#include "via_video.h"

#include "via_capture.h"
#include "via.h"

#include "xf86xv.h"
#include "Xv.h"
#include "xaa.h"
#include "xaalocal.h"
#include "dixstruct.h"
#include "via_xvpriv.h"
#include "via_swov.h"
#include "via_memcpy.h"
#include "via_id.h"
#include "fourcc.h"

/*
 * D E F I N E
 */
#define OFF_DELAY       200  /* milliseconds */
#define FREE_DELAY      60000
#define PARAMSIZE       1024
#define SLICESIZE       65536       
#define OFF_TIMER       0x01
#define FREE_TIMER      0x02
#define TIMER_MASK      (OFF_TIMER | FREE_TIMER)
#define VIA_MAX_XVIMAGE_X 1920
#define VIA_MAX_XVIMAGE_Y 1080

#define LOW_BAND 0x0CB0
#define MID_BAND 0x1f10

#define  XV_IMAGE          0
#define  NTSC_COMPOSITE    1
#define  NTSC_TUNER        2
#define  NTSC_SVIDEO       3
#define  PAL_SVIDEO        4
#define  PAL_60_COMPOSITE  5
#define  PAL_60_TUNER      6
#define  PAL_60_SVIDEO     7
#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

#define  IN_FLIP     ( viaVidEng->ramtab & 0x00000003)
#define  IN_DISPLAY  ( viaVidEng->interruptflag & 0x00000200)
#define  IN_VBLANK   ( !IN_DISPLAY )

#ifndef XvExtension
void viaInitVideo(ScreenPtr pScreen) {}
#else

static vidCopyFunc viaFastVidCpy = NULL;


/*
 *  F U N C T I O N   D E C L A R A T I O N
 */
XF86VideoAdaptorPtr viaSetupImageVideoG(ScreenPtr);
static void viaStopVideoG(ScrnInfoPtr, pointer, Bool);
static void viaQueryBestSizeG(ScrnInfoPtr, Bool,
        short, short, short, short, unsigned int *, unsigned int *, pointer);
static int viaPutVideo(ScrnInfoPtr ,
    short , short , short , short ,short , short , short , short ,
    RegionPtr , pointer );

static int viaQueryImageAttributesG(ScrnInfoPtr, 
        int, unsigned short *, unsigned short *,  int *, int *);

static Atom xvBrightness, xvContrast, xvColorKey, xvHue, xvSaturation, xvPort,
	    xvCompose, xvEncoding, xvMute, xvVolume, xvFreq, xvAudioCtrl,
  xvHQV, xvBOB, xvExitTV, xvAutoPaint;

/*
 *  S T R U C T S
 */
/* client libraries expect an encoding */
static XF86VideoEncodingRec DummyEncoding[8] =
{
  { XV_IMAGE        , "XV_IMAGE",VIA_MAX_XVIMAGE_X,VIA_MAX_XVIMAGE_Y,{1, 1}},
  { NTSC_COMPOSITE  , "ntsc-composite",720, 480, { 1001, 60000 }},
  { NTSC_TUNER      , "ntsc-tuner",720, 480, { 1001, 60000 }},
  { NTSC_SVIDEO     , "ntsc-svideo",720, 480, { 1001, 60000 }},
  { PAL_SVIDEO      , "pal-svideo",720, 576, { 1, 50 }},
  { PAL_60_COMPOSITE, "pal_60-composite", 704, 576, { 1, 50 }},
  { PAL_60_TUNER    , "pal_60-tuner", 720, 576, { 1, 50 }},
  { PAL_60_SVIDEO   , "pal_60-svideo",720, 576, { 1, 50 }}
};

#define NUM_FORMATS_G 9

static XF86VideoFormatRec FormatsG[NUM_FORMATS_G] = 
{
  { 8, TrueColor }, /* Dithered */
  { 8, PseudoColor }, /* Using .. */
  { 8, StaticColor },
  { 8, GrayScale },
  { 8, StaticGray }, /* .. TexelLUT */
  {16, TrueColor},
  {24, TrueColor},
  {16, DirectColor},
  {24, DirectColor}
};

#define NUM_ATTRIBUTES_G 18

static XF86AttributeRec AttributesG[NUM_ATTRIBUTES_G] =
{
   {XvSettable | XvGettable, 0, (1 << 24) - 1, "XV_COLORKEY"},
   {XvSettable | XvGettable, 0, 10000, "XV_BRIGHTNESS"},
   {XvSettable | XvGettable, 0, 20000, "XV_CONTRAST"},
   {XvSettable | XvGettable, 0, 20000,"XV_SATURATION"},
   {XvSettable | XvGettable,-180,180,"XV_HUE"},
   {XvSettable | XvGettable,0,255,"XV_MUTE"},
   {XvSettable | XvGettable,0,255,"XV_VOLUME"},
   {XvSettable,0,2,"XV_PORT"},
   {XvSettable,0,2,"XV_COMPOSE"},
   {XvSettable,0,2,"XV_SVIDEO"},
   {XvSettable | XvGettable,0, 255,"XV_ENCODING"},
   {XvSettable | XvGettable,0, 255, "XV_CHANNEL"},
   {XvSettable,0,-1,"XV_FREQ"},
   {XvSettable,0,2,"XV_AUDIOCTRL"},
   {XvSettable,0,2,"XV_HIGHQVDO"},
   {XvSettable,0,2,"XV_BOB"},
   {XvSettable,0,2,"XV_EXITTV"},
   {XvSettable | XvGettable,0,1,"XV_AUTOPAINT_COLORKEY"}
};

#define NUM_IMAGES_G 3

static XF86ImageRec ImagesG[NUM_IMAGES_G] =
{
    XVIMAGE_YUY2,
    XVIMAGE_YV12,
    {
	/*
	 * Below, a dummy picture type that is used in XvPutImage only to do
	 * an overlay update. Introduced for the XvMC client lib.
	 * Defined to have a zero data size.
	 */
	
	FOURCC_VIA,
        XvYUV,
        LSBFirst,
        {'V','I','A',0x00,
          0x00,0x00,0x00,0x10,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71},
        12,
        XvPlanar,
        1,
        0, 0, 0, 0 ,
        8, 8, 8,
        1, 2, 2,
        1, 2, 2,
        {'Y','V','U',
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        XvTopToBottom
    }    
};

static char * XVPORTNAME[5] =
{
   "XV_SWOV",
   "XV_TV0" ,
   "XV_TV1" ,
   "XV_UTCTRL",
   "XV_DUMMY"
};

/*
 *  F U N C T I O N
 */
static __inline void waitVBLANK(vmmtr viaVidEng)
{
   while (IN_DISPLAY);
}

static __inline void waitIfFlip(vmmtr viaVidEng) 
{
  while( IN_FLIP );
}


static __inline void waitDISPLAYBEGIN(vmmtr viaVidEng)
{
    while (IN_VBLANK);
}

/*
 *   Decide if the mode support video overlay. This depends on the bandwidth
 *   of the mode and the type of RAM available.
 */
static Bool DecideOverlaySupport(ScrnInfoPtr pScrn)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    DisplayModePtr mode = pScrn->currentMode;

    /* Small trick here. We keep the height in 16's of lines and width in 32's 
       to avoid numeric overflow */

    if ( pVia->ChipId != PCI_CHIP_VT3205 ) {
	CARD32 bandwidth = (mode->HDisplay >> 4) * (mode->VDisplay >> 5) *
	    pScrn->bitsPerPixel * mode->VRefresh;
	
	switch (pVia->MemClk)
	    {
	    case SDR100: /* No overlay without DDR */
	    case SDR133:
		return FALSE;
	    case DDR100:
		/* Basic limit for DDR100 is about this */
		if(bandwidth > 1800000)
		    return FALSE;
		/* But we have constraints at higher than 800x600 */
		if (mode->HDisplay > 800)
		    {
			if(pScrn->bitsPerPixel != 8)
			    return FALSE;
			if(mode->VDisplay > 768)
			    return FALSE;
			if(mode->VRefresh > 60)
			    return FALSE;
		    }
		return TRUE;
	    case 0:		/*	FIXME: Why does my CLE266 report 0? */
	    case DDR133:
		if(bandwidth > 7901250)
		    return FALSE;
		return TRUE;
	    }
	return FALSE;

    } else {
	VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
	unsigned width,height,refresh,dClock;
	float mClock,memEfficiency,needBandWidth,totalBandWidth;
	int 
	    bTV = 0;

	switch(pVia->MemClk) {
	case SDR100:
	    mClock = 50; /*HW base on 128 bit*/
	    break;
	case SDR133:
	    mClock = 66.5 ;
	    break;
	case DDR100:                 
	    mClock = 100;       
	    break;                                                                          
	case DDR133:                            
	    mClock = 133;
	    break;
	case DDR166:                            
	    mClock = 166;
	    break;                 
	default:
	    /*Unknow DRAM Type*/
	    DBG_DD(ErrorF("Unknow DRAM Type!\n"));                                  
	    mClock = 166;
	    break;
	}
        
	switch(pVia->MemClk)
	    {
	    case SDR100:
	    case SDR133:
	    case DDR100:
		memEfficiency = (float)SINGLE_3205_100;
		break;            
	    case DDR133:                            
	    case DDR166:
		memEfficiency = (float)SINGLE_3205_133;
		break;                             
	    default:
		/*Unknow DRAM Type .*/
		DBG_DD(ErrorF("Unknow DRAM Type!\n"));                                  
		memEfficiency = (float)SINGLE_3205_133;
		break;
	    }
      
	width = mode->HDisplay;
	height = mode->VDisplay;
	refresh = mode->VRefresh;

	if ((pBIOSInfo->ActiveDevice == VIA_DEVICE_LCD) ||
	    (pBIOSInfo->ActiveDevice == VIA_DEVICE_DFP) ) {
	    width = pBIOSInfo->panelX;
	    height = pBIOSInfo->panelY;
	    if ((width == 1400) && (height == 1050)) {
		width = 1280;
		height = 1024;
		refresh = 60;
	    }
	} else if (pBIOSInfo->ActiveDevice == VIA_DEVICE_TV ) {
	    bTV = 1;
	}
        
	if (bTV) { 

	    /* 
	     * Approximative, VERY conservative formula in some cases.
	     * This formula and the one below are derived analyzing the
	     * tables present in VIA's own drivers. They may reject the over-
	     * lay in some cases where VIA's driver don't.
	     */

	    dClock = (width * height * 60) / 580000;   

	} else {

	    /*
	     * Approximative, slightly conservative formula. See above.
	     */
       
	    dClock = (width * height * refresh) / 680000;
	}

	if (dClock) {
	    needBandWidth = (float)(((pScrn->bitsPerPixel >> 3) + VIDEO_BPP)*dClock);
	    totalBandWidth = (float)(mClock*16.*memEfficiency);
      
	    DBG_DD(ErrorF(" via_video.c : cBitsPerPel= %d : \n",pScrn->bitsPerPixel));
	    DBG_DD(ErrorF(" via_video.c : Video_Bpp= %d : \n",VIDEO_BPP));
	    DBG_DD(ErrorF(" via_video.c : refresh = %d : \n",refresh));
	    DBG_DD(ErrorF(" via_video.c : dClock= %d : \n",dClock));
	    DBG_DD(ErrorF(" via_video.c : mClk= %f : \n",mClock));
	    DBG_DD(ErrorF(" via_video.c : memEfficiency= %f : \n",memEfficiency));
	    DBG_DD(ErrorF(" via_video.c : needBandwidth= %f : \n",needBandWidth));
	    DBG_DD(ErrorF(" via_video.c : totalBandwidth= %f : \n",totalBandWidth));
	    if (needBandWidth < totalBandWidth)
		return TRUE;
	}          
	return FALSE;
    }
    return FALSE;
}

void viaResetVideo(ScrnInfoPtr pScrn) 
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vmmtr   viaVidEng = (vmmtr) pVia->VidMapBase;    

    DBG_DD(ErrorF(" via_video.c : viaResetVideo: \n"));

    viaVidEng->video1_ctl = 0;
    viaVidEng->video3_ctl = 0;
    viaVidEng->compose    = 0x80000000;
    viaVidEng->compose    = 0x40000000;


}

void viaSaveVideo(ScrnInfoPtr pScrn)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vmmtr   viaVidEng = (vmmtr) pVia->VidMapBase;    

    pVia->dwV1 = ((vmmtr)viaVidEng)->video1_ctl;
    pVia->dwV3 = ((vmmtr)viaVidEng)->video3_ctl;
    viaVidEng->video1_ctl = 0;
    viaVidEng->video3_ctl = 0;
    viaVidEng->compose    = 0x80000000;
    viaVidEng->compose    = 0x40000000;
}

void viaRestoreVideo(ScrnInfoPtr pScrn)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vmmtr   viaVidEng = (vmmtr) pVia->VidMapBase;    

    viaVidEng->video1_ctl = pVia->dwV1 ;
    viaVidEng->video3_ctl = pVia->dwV3 ;
    viaVidEng->compose    = 0x80000000;
    viaVidEng->compose    = 0x40000000;

}

void viaExitVideo(ScrnInfoPtr pScrn)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    vmmtr   viaVidEng = (vmmtr) pVia->VidMapBase;    

    DBG_DD(ErrorF(" via_video.c : viaExitVideo : \n"));

    viaVidEng->video1_ctl = 0;
    viaVidEng->video3_ctl = 0;
    viaVidEng->compose    = 0x80000000;
    viaVidEng->compose    = 0x40000000;

}   

static XF86VideoAdaptorPtr viaAdaptPtr[XV_PORT_NUM];

/*
 * FIXME: Must free those above and their contents.  (In viaExitVideo.)
 */

void viaInitVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VIAPtr  pVia = VIAPTR(pScrn); 
    XF86VideoAdaptorPtr *adaptors, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    int num_adaptors;

    DBG_DD(ErrorF(" via_video.c : viaInitVideo : \n"));

    if (!viaFastVidCpy)
	viaFastVidCpy = viaVidCopyInit("video", pScreen);


    if ( (pVia->Chipset == VIA_CLE266) ||
	 (pVia->Chipset == VIA_KM400) ) {
      newAdaptor = viaSetupImageVideoG(pScreen);
      num_adaptors = xf86XVListGenericAdaptors(pScrn, &adaptors);
    } else {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[Xv] Unsupported Chipset. X video functionality disabled.\n");
      num_adaptors=0;
    }


    DBG_DD(ErrorF(" via_video.c : num_adaptors : %d\n",num_adaptors));
    if(newAdaptor) {
        if(!num_adaptors) {
            num_adaptors = 1;
            adaptors = &newAdaptor; /* Now ,useless */
        } else {
            DBG_DD(ErrorF(" via_video.c : viaInitVideo : Warning !!! MDS not supported yet !\n"));
            newAdaptors =  /* need to free this someplace */
                xalloc((num_adaptors + 1) * sizeof(XF86VideoAdaptorPtr*));
            if(newAdaptors) {
                memcpy(newAdaptors, adaptors, num_adaptors * 
                                        sizeof(XF86VideoAdaptorPtr));
                newAdaptors[num_adaptors] = newAdaptor;
                adaptors = newAdaptors;
                num_adaptors++;
            }
        }
    }

    if(num_adaptors) {
	xf86XVScreenInit(pScreen, viaAdaptPtr, XV_PORT_NUM);
	viaSetColorSpace(pVia,0,0,0,0,TRUE);
    }

    if(newAdaptors)
        xfree(newAdaptors);


}


XF86VideoAdaptorPtr 
viaSetupImageVideoG(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    /* XF86VideoAdaptorPtr viaAdaptPtr[XV_PORT_NUM]; */
    viaPortPrivRec *gviaPortPriv[XV_PORT_NUM];
    DevUnion  *         pdevUnion[XV_PORT_NUM];
    int  i;
    
    DBG_DD(ErrorF(" via_video.c : viaSetupImageVideoG: \n"));

    /* Look for available devices */
    ViaTunerProbe(pScrn);

    xvBrightness      = MAKE_ATOM("XV_BRIGHTNESS");
    xvContrast        = MAKE_ATOM("XV_CONTRAST");
    xvColorKey        = MAKE_ATOM("XV_COLORKEY");
    xvHue             = MAKE_ATOM("XV_HUE");
    xvSaturation      = MAKE_ATOM("XV_SATURATION");
    xvMute            = MAKE_ATOM("XV_MUTE");
    xvVolume          = MAKE_ATOM("XV_VOLUME");
    xvPort            = MAKE_ATOM("XV_PORT");
    xvCompose         = MAKE_ATOM("XV_COMPOSE");
    xvEncoding        = MAKE_ATOM("XV_ENCODING");
    xvFreq            = MAKE_ATOM("XV_FREQ");
    xvAudioCtrl       = MAKE_ATOM("XV_AUDIOCTRL");
    xvHQV             = MAKE_ATOM("XV_HIGHQVDO");
    xvBOB             = MAKE_ATOM("XV_BOB");    
    xvExitTV          = MAKE_ATOM("XV_EXITTV");
    xvAutoPaint       = MAKE_ATOM("XV_AUTOPAINT_COLORKEY");

    /* AllocatePortPriv();*/
    for ( i = 0; i< XV_PORT_NUM; i ++ ) {
        if(!(viaAdaptPtr[i] = xf86XVAllocateVideoAdaptorRec(pScrn)))
            return NULL;

        gviaPortPriv[i] =  (viaPortPrivPtr)xnfcalloc(1, sizeof(viaPortPrivRec) );
        pdevUnion[i] = (DevUnion  *)xnfcalloc(1, sizeof(DevUnion));

        if(i == 0)		/* Overlay engine */
        {
            viaAdaptPtr[i]->type = XvInputMask | XvWindowMask | XvImageMask | XvVideoMask | XvStillMask;
            viaAdaptPtr[i]->flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
        }
        else
        {
            viaAdaptPtr[i]->type = XvInputMask | XvWindowMask | XvVideoMask;
            viaAdaptPtr[i]->flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
        }
        viaAdaptPtr[i]->name = XVPORTNAME[i];
        viaAdaptPtr[i]->nEncodings = 8;
        viaAdaptPtr[i]->pEncodings = DummyEncoding;
        viaAdaptPtr[i]->nFormats = sizeof(FormatsG) / sizeof(FormatsG[0]);
        viaAdaptPtr[i]->pFormats = FormatsG;

        /* The adapter can handle 1 port simultaneously */
        viaAdaptPtr[i]->nPorts = 1; 
        viaAdaptPtr[i]->pPortPrivates = pdevUnion[i];
        viaAdaptPtr[i]->pPortPrivates->ptr = (pointer) gviaPortPriv[i];
        if (i == 3) /* Utility port doesn't need attribute */
        {
            viaAdaptPtr[i]->nAttributes = 0;
            viaAdaptPtr[i]->pAttributes = NULL;
        }
        else
        {
            viaAdaptPtr[i]->nAttributes = NUM_ATTRIBUTES_G;
            viaAdaptPtr[i]->pAttributes = AttributesG;
        }
        viaAdaptPtr[i]->nImages = NUM_IMAGES_G;
        viaAdaptPtr[i]->pImages = ImagesG;
        viaAdaptPtr[i]->PutVideo = viaPutVideo;
        viaAdaptPtr[i]->StopVideo = viaStopVideoG;
        viaAdaptPtr[i]->QueryBestSize = viaQueryBestSizeG;
#ifdef XF86DRI
	
        viaAdaptPtr[i]->GetPortAttribute = viaXvMCInterceptXvGetAttribute;
	viaAdaptPtr[i]->SetPortAttribute = viaXvMCInterceptXvAttribute;
	viaAdaptPtr[i]->PutImage = viaXvMCInterceptPutImage;
#else
        viaAdaptPtr[i]->GetPortAttribute = viaGetPortAttributeG;
        viaAdaptPtr[i]->SetPortAttribute = viaSetPortAttributeG;
        viaAdaptPtr[i]->PutImage = viaPutImageG;
#endif
        viaAdaptPtr[i]->QueryImageAttributes = viaQueryImageAttributesG;

        gviaPortPriv[i]->colorKey = 0x0821;
	gviaPortPriv[i]->autoPaint = TRUE;
        gviaPortPriv[i]->brightness = 5000.;
        gviaPortPriv[i]->saturation = 10000;
        gviaPortPriv[i]->contrast = 10000;
        gviaPortPriv[i]->hue = 0;
        gviaPortPriv[i]->xv_portnum = i;
        /* gotta uninit this someplace */
        REGION_NULL(pScreen, &gviaPortPriv[i]->clip);
#ifdef XF86DRI
	viaXvMCInitXv( pScrn, gviaPortPriv[i]);
#endif
    } /* End of for */
    viaResetVideo(pScrn);
    return viaAdaptPtr[0];
}


static Bool
RegionsEqual(RegionPtr A, RegionPtr B)
{
    int *dataA, *dataB;
    int num;

    num = REGION_NUM_RECTS(A);
    if(num != REGION_NUM_RECTS(B))
        return FALSE;

    if((A->extents.x1 != B->extents.x1) ||
       (A->extents.x2 != B->extents.x2) ||
       (A->extents.y1 != B->extents.y1) ||
       (A->extents.y2 != B->extents.y2))
        return FALSE;

    dataA = (int*)REGION_RECTS(A);
    dataB = (int*)REGION_RECTS(B);

    while(num--) {
        if((dataA[0] != dataB[0]) || (dataA[1] != dataB[1]))
           return FALSE;
        dataA += 2; 
        dataB += 2;
    }
 
    return TRUE;
}


static int CreateSWOVSurface(ScrnInfoPtr pScrn, viaPortPrivPtr pPriv, int fourcc, short width, short height)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    LPDDSURFACEDESC lpSurfaceDesc = &pPriv->SurfaceDesc;
    unsigned long retCode;

    if (pVia->Video.VideoStatus & SWOV_SURFACE_CREATED)
        return Success;

    lpSurfaceDesc->dwWidth  = (unsigned long)width;
    lpSurfaceDesc->dwHeight = (unsigned long)height;
    lpSurfaceDesc->dwBackBufferCount =1;

    lpSurfaceDesc->dwFourCC = (unsigned long)fourcc;

    if (Success != (retCode = VIAVidCreateSurface(pScrn, lpSurfaceDesc)))
	return retCode;

    pPriv->ddLock.dwFourCC = (unsigned long)fourcc;

    VIAVidLockSurface(pScrn, &pPriv->ddLock);

    pPriv->ddLock.SWDevice.lpSWOverlaySurface[0] = pVia->FBBase + pPriv->ddLock.SWDevice.dwSWPhysicalAddr[0];
    pPriv->ddLock.SWDevice.lpSWOverlaySurface[1] = pVia->FBBase + pPriv->ddLock.SWDevice.dwSWPhysicalAddr[1];

    DBG_DD(ErrorF(" lpSWOverlaySurface[0]: %p\n", pPriv->ddLock.SWDevice.lpSWOverlaySurface[0]));
    DBG_DD(ErrorF(" lpSWOverlaySurface[1]: %p\n", pPriv->ddLock.SWDevice.lpSWOverlaySurface[1]));
    
    pVia->Video.VideoStatus |= SWOV_SURFACE_CREATED|SW_VIDEO_ON;
    pVia->Video.dwAction = ACTION_SET_VIDEOSTATUS;
    return Success;
}


static void DestroySWOVSurface(ScrnInfoPtr pScrn,  viaPortPrivPtr pPriv)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    LPDDSURFACEDESC lpSurfaceDesc = &pPriv->SurfaceDesc;
    DBG_DD(ErrorF(" via_video.c : Destroy SW Overlay Surface, fourcc =0x%08x : \n",
              lpSurfaceDesc->dwFourCC));

    if (pVia->Video.VideoStatus & SWOV_SURFACE_CREATED)
    {
        DBG_DD(ErrorF(" via_video.c : Destroy SW Overlay Surface, VideoStatus =0x%08x : \n",
              pVia->Video.VideoStatus));
    }
    else
    {
        DBG_DD(ErrorF(" via_video.c : No SW Overlay Surface Destroyed, VideoStatus =0x%08x : \n",
              pVia->Video.VideoStatus));
        return;
    }

    VIAVidDestroySurface(pScrn, lpSurfaceDesc);        

    pVia->Video.VideoStatus &= ~SWOV_SURFACE_CREATED;
    pVia->Video.dwAction = ACTION_SET_VIDEOSTATUS;
}


void  viaStopSWOVerlay(ScrnInfoPtr pScrn)
{
    DDUPDATEOVERLAY      UpdateOverlay_Video;
    LPDDUPDATEOVERLAY    lpUpdateOverlay = &UpdateOverlay_Video;
    VIAPtr  pVia = VIAPTR(pScrn);

    pVia->Video.VideoStatus &= ~SW_VIDEO_ON;
    pVia->Video.dwAction = ACTION_SET_VIDEOSTATUS;

    lpUpdateOverlay->dwFlags  = DDOVER_HIDE;
    VIAVidUpdateOverlay(pScrn, lpUpdateOverlay);    
}


static void 
viaStopVideoG(ScrnInfoPtr pScrn, pointer data, Bool exit)
{
    VIAPtr  pVia = VIAPTR(pScrn);
    viaPortPrivPtr pPriv = (viaPortPrivPtr)data;

    DBG_DD(ErrorF(" via_video.c : viaStopVideoG: exit=%d\n", exit));

    REGION_EMPTY(pScrn->pScreen, &pPriv->clip);
    if(exit) {
       viaStopSWOVerlay(pScrn);
       DestroySWOVSurface(pScrn, pPriv);
       pVia->dwFrameNum = 0;    
       pPriv->old_drw_x= 0;
       pPriv->old_drw_y= 0;
       pPriv->old_drw_w= 0;
       pPriv->old_drw_h= 0;
   } else {
       viaStopSWOVerlay(pScrn);
   }
}

/* App "xawtv" attribute from -1000 to 1000 */
/* But SAA7113H needs 0 to 255              */
#define Attr_Mapping(x)   x = ( (x + 1000) >> 3 )


/****************************************************************************
 *  SetTunerChannel                                                       *
 *  Function: Sets the tuner to a requested channel                         *
 *  Inputs: CARD16 channel - the tuner channel to be set.                   *
 *  Outputs: NONE                                                          *
 ****************************************************************************/
static void SetTunerChannel (ViaTunerPtr pTuner, viaPortPrivPtr pChanPriv, INT32 frequency)
{
    int      control;
    short    divider = 0;

    switch(pChanPriv->dwEncoding)
    {
      case PAL_60_COMPOSITE :
      case PAL_60_TUNER     :
      case PAL_60_SVIDEO    :
           divider = 633 + (short)frequency;
           break;
      case NTSC_COMPOSITE   :
      case NTSC_TUNER       :
      case NTSC_SVIDEO      :
           divider = 733 + (short)frequency;
           break;
      default:
           divider = frequency;
    }

    control = 0x8E00;

    if ( divider <= LOW_BAND )
        control |= 0xA0;
    else if ( divider <= MID_BAND )
        control |= 0x90;
    else
        control |= 0x30;

    DBG_DD(ErrorF(" via_video.c : SetTunerChannel : Divider = 0x%08x, Control= 0x%08x, \n", divider,control));

    if(pTuner)
    	ViaTunerChannel(pTuner, divider, control);

} /* SetTunerChannel ()... */

/* Tuner uses range 0 - 256 (?); Xv uses low - high */

static int
xv_to_tuner(int val,int low, int high) {
  val = val * 256 / (high - low);
  if (val <    -0) val =     0;
  if (val > 256) val = 256;
  return val;
}


int 
viaSetPortAttributeG(
    ScrnInfoPtr pScrn,
    Atom attribute,
    INT32 value,
    pointer data
    ){
    VIAPtr  pVia = VIAPTR(pScrn);
    vmmtr   viaVidEng = (vmmtr) pVia->VidMapBase;    
    viaPortPrivPtr pPriv = (viaPortPrivPtr)data;
    int attr, avalue;
    ViaTunerPtr pTuner = NULL;
    
    if(pPriv->xv_portnum == COMMAND_FOR_TV0)
    	pTuner = pVia->Tuner[0];
    else if(pPriv->xv_portnum == COMMAND_FOR_TV1)
        pTuner = pVia->Tuner[1];
    ;
    DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : \n"));

    pVia->OverlaySupported = DecideOverlaySupport(pScrn);

    /* Color Key */
    if(attribute == xvColorKey) {
	DBG_DD(ErrorF("  V4L Disable  xvColorKey = %08x\n",value));

	pPriv->colorKey = value;
	/* All assume color depth is 16 */
	value &= 0x00FFFFFF;
	viaVidEng->color_key = value;
	viaVidEng->snd_color_key = value;
	REGION_EMPTY(pScrn->pScreen, &pPriv->clip);
	DBG_DD(ErrorF("  V4L Disable done  xvColorKey = %08x\n",value));

    } else if (attribute == xvAutoPaint) {
	pPriv->autoPaint = value;
	DBG_DD(ErrorF("       xvColorKey = %08x\n",value));
	/* Color Control */
    } else if (attribute == xvBrightness ||
               attribute == xvContrast   ||
               attribute == xvSaturation ||
               attribute == xvHue) {
        if (attribute == xvBrightness)
	    {
		DBG_DD(ErrorF("     xvBrightness = %08d\n",value));
		pPriv->brightness = value;
		if(pTuner) 
		    ViaTunerBrightness(pTuner, xv_to_tuner(value,0,5000));
	    }
        if (attribute == xvContrast)
	    {
		DBG_DD(ErrorF("     xvContrast = %08d\n",value));
		pPriv->contrast   = value;
		if(pTuner)
		    ViaTunerContrast(pTuner, xv_to_tuner(value,0,20000));
	    }
        if (attribute == xvSaturation)
	    {
		DBG_DD(ErrorF("     xvSaturation = %08d\n",value));
		pPriv->saturation     = value;
		if(pTuner)
		    ViaTunerSaturation(pTuner, xv_to_tuner(value,0,20000));
	    }
        if (attribute == xvHue)
	    {
		DBG_DD(ErrorF("     xvHue = %08d\n",value));
		pPriv->hue        = value;
		if(pTuner)
		    ViaTunerHue(pTuner, xv_to_tuner(value,-180,180));
	    }
	if (!pTuner);
	viaSetColorSpace(pVia, pPriv->hue, pPriv->saturation, pPriv->brightness,
			 pPriv->contrast,FALSE); 

	/* Audio control */
    } else if (attribute == xvMute){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvMute = %08d\n",value));
	if(value)
	    value = 1;
	ViaAudioMute(pVia, value);
    } else if (attribute == xvVolume){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvVolume = %08d\n",value));
	pPriv->Volume = value;
	attr = ATTR_VOLUME;
	avalue = value;
	/* FIXME */
	/* Tuner control. Channel switch */
    } else if (attribute == xvFreq){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvFreq = %08x\n",value));
	SetTunerChannel(pTuner, pPriv, value );
	/* Video decoder control. NTSC/PAL, SVIDEO/COMPOSITIVE/TV */
    } else if (attribute == xvEncoding){
	DBG_DD(ErrorF("     xvEncoding = %d. \n",value));
	pPriv->dwEncoding = value;
	if(pTuner)
	    ViaTunerStandard(pTuner, value);
	/* VIA Proprietary Attribute for Video control */
    } else if (attribute == xvPort ){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvPort=%d\n", value));

    } else if (attribute == xvCompose){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvCompose=%08x\n",value));

    } else if (attribute == xvHQV){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvHQV=%08x\n",value));

    } else if (attribute == xvBOB){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvBOB=%08x\n",value));

    } else if (attribute == xvExitTV){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvExitTV=%08x\n",value));

	/* VIA Proprietary Attribute for AUDIO control */
    } else if (attribute == xvAudioCtrl ){
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : xvAudioSwitch=%d\n", value));
	if(pTuner)
            {
            	ViaAudioMode(pVia, value);
            	if(pPriv->xv_portnum == COMMAND_FOR_TV0)
		    ViaAudioSelect(pScrn,0);
            	else
		    ViaAudioSelect(pScrn, 1);
	    }
    }else{
	DBG_DD(ErrorF(" via_video.c : viaSetPortAttributeG : is not supported the attribute"));
	return BadMatch;
    }
    
    /* attr,avalue hardware processing goes here */
    (void)attr;
    (void)avalue;
    
    return Success;
}

int 
viaGetPortAttributeG(
    ScrnInfoPtr pScrn,
    Atom attribute,
    INT32 *value,
    pointer data
){
    viaPortPrivPtr pPriv = (viaPortPrivPtr)data;

    DBG_DD(ErrorF(" via_video.c : viaGetPortAttributeG : port %d %d\n",
		  pPriv->xv_portnum, attribute));

    *value = 0;
    if (attribute == xvColorKey ) {
           *value =(INT32) pPriv->colorKey;
           DBG_DD(ErrorF(" via_video.c :    ColorKey 0x%x\n",pPriv->colorKey));
    } else if (attribute == xvAutoPaint ) {
      *value = (INT32) pPriv->autoPaint;
      DBG_DD(ErrorF("    AutoPaint = %08d\n", *value));
    /* Color Control */
    } else if (attribute == xvBrightness ||
               attribute == xvContrast   ||
               attribute == xvSaturation ||
               attribute == xvHue) {
        if (attribute == xvBrightness)
        {
	  *value = pPriv->brightness;
            DBG_DD(ErrorF("    xvBrightness = %08d\n", *value));
        }
        if (attribute == xvContrast)
        {
	  *value = pPriv->contrast;
            DBG_DD(ErrorF("    xvContrast = %08d\n", *value));
        }
        if (attribute == xvSaturation)
        {
	  *value = pPriv->saturation;
            DBG_DD(ErrorF("    xvSaturation = %08d\n", *value));
        }
        if (attribute == xvHue)
        {
	  *value = pPriv->hue;
            DBG_DD(ErrorF("    xvHue = %08d\n", *value));
        }

     }else {
           /*return BadMatch*/ ;
     }
    return Success;
}

static void 
viaQueryBestSizeG(
    ScrnInfoPtr pScrn,
    Bool motion,
    short vid_w, short vid_h,
    short drw_w, short drw_h,
    unsigned int *p_w, unsigned int *p_h,
    pointer data
){
    DBG_DD(ErrorF(" via_video.c : viaQueryBestSizeG :\n"));
    *p_w = drw_w;
    *p_h = drw_h;

    if(*p_w > 2048 )
           *p_w = 2048;
}

/*
 *  To do SW Flip
 */
static void Flip(VIAPtr pVia, viaPortPrivPtr pPriv, int fourcc, unsigned long DisplayBufferIndex)
{
    switch(fourcc)
    {
        case FOURCC_UYVY:
        case FOURCC_YUY2:
            while ((VIDInD(HQV_CONTROL) & HQV_SW_FLIP) );
            VIDOutD(HQV_SRC_STARTADDR_Y, pPriv->ddLock.SWDevice.dwSWPhysicalAddr[DisplayBufferIndex]);
            VIDOutD(HQV_CONTROL,( VIDInD(HQV_CONTROL)&~HQV_FLIP_ODD) |HQV_SW_FLIP|HQV_FLIP_STATUS);
            break;

        case FOURCC_YV12:
        default:
            while ((VIDInD(HQV_CONTROL) & HQV_SW_FLIP) );
            VIDOutD(HQV_SRC_STARTADDR_Y, pPriv->ddLock.SWDevice.dwSWPhysicalAddr[DisplayBufferIndex]);
            VIDOutD(HQV_SRC_STARTADDR_U, pPriv->ddLock.SWDevice.dwSWCbPhysicalAddr[DisplayBufferIndex]);
            VIDOutD(HQV_SRC_STARTADDR_V, pPriv->ddLock.SWDevice.dwSWCrPhysicalAddr[DisplayBufferIndex]);
            VIDOutD(HQV_CONTROL,( VIDInD(HQV_CONTROL)&~HQV_FLIP_ODD) |HQV_SW_FLIP|HQV_FLIP_STATUS);
            break;
    }
}

#ifdef UNUSED
static void CopyDataYUV422(
    ScrnInfoPtr pScrn,
    VIAPtr  pVia,
    unsigned char * src,
    unsigned char * dst,
    int srcPitch,
    int dstPitch,
    int h,
    int w )
{
    int count;

    /*  copy YUY2 data to video memory,
     *  do 32 bits alignment.
     */
     
    w <<= 1;		/* Each pixel is YU or YV - 16 bits */
    srcPitch <<= 1;
    
    count = h;
    while(count--) {
        memcpy(dst, src, w);
        src += srcPitch;
        dst += dstPitch;
    }
}
#endif /* UNUSED */

#ifdef UNUSED
static void
CopyDataYUV420(
   ScrnInfoPtr pScrn,
   VIAPtr  pVia,
   unsigned char *src1,
   unsigned char *src2,
   unsigned char *src3,
   unsigned char *dst1,
   unsigned char *dst2,
   unsigned char *dst3,
   int srcPitch,
   int dstPitch,
   int h,
   int w
){
    int count;

    /* copy Y component to video memory */
    count = h;
    while(count--) {
        memcpy(dst1, src1, w);
        src1 += srcPitch;
        dst1 += dstPitch;
    }

    /* UV component is 1/4 of Y */
    w >>= 1;
    h >>= 1;
    srcPitch >>= 1;
    dstPitch >>= 1;

    /* copy V(Cr) component to video memory */
    count = h;
    while(count--) {
        memcpy(dst2, src2, w);
        src2 += srcPitch;
        dst2 += dstPitch;
    }

    /* copy U(Cb) component to video memory */
    count = h;
    while(count--) {
        memcpy(dst3, src3, w);
        src3 += srcPitch;
        dst3 += dstPitch;
    }
}
#endif /* UNUSED */

int 
viaPutImageG( 
    ScrnInfoPtr pScrn,
    short src_x, short src_y,
    short drw_x, short drw_y,
    short src_w, short src_h,
    short drw_w, short drw_h,
    int id, unsigned char* buf,
    short width, short height,
    Bool sync,
    RegionPtr clipBoxes, pointer data
    ){
    VIAPtr  pVia = VIAPTR(pScrn);
    viaPortPrivPtr pPriv = (viaPortPrivPtr)data;
    unsigned long retCode;
/*    int i;
      BoxPtr pbox; */

# ifdef XV_DEBUG
    ErrorF(" via_video.c : viaPutImageG : called\n");
    ErrorF(" via_video.c : FourCC=0x%x width=%d height=%d sync=%d\n",id,width,height,sync);
    ErrorF(" via_video.c : src_x=%d src_y=%d src_w=%d src_h=%d colorkey=0x%x\n",src_x, src_y, src_w, src_h, pPriv->colorKey);
    ErrorF(" via_video.c : drw_x=%d drw_y=%d drw_w=%d drw_h=%d\n",drw_x,drw_y,drw_w,drw_h);
# endif

    switch ( pPriv->xv_portnum )
	{
        case COMMAND_FOR_TV0 :
        case COMMAND_FOR_TV1 :
            DBG_DD(ErrorF(" via_video.c :              : Shall not happen! \n"));
            break;

        case COMMAND_FOR_SWOV    :
        case COMMAND_FOR_DUMMY   :
	{
	    DDUPDATEOVERLAY      UpdateOverlay_Video;
	    LPDDUPDATEOVERLAY    lpUpdateOverlay = &UpdateOverlay_Video;

	    int dstPitch;
	    unsigned long dwUseExtendedFIFO=0;

	    DBG_DD(ErrorF(" via_video.c :              : S/W Overlay! \n"));
	    /*  Allocate video memory(CreateSurface),
	     *  add codes to judge if need to re-create surface
	     */
	    if ( (pPriv->old_src_w != src_w) || (pPriv->old_src_h != src_h) )
		DestroySWOVSurface(pScrn, pPriv);

	    if (Success != ( retCode = CreateSWOVSurface(pScrn, pPriv, id, width, height) ))
                {
		    DBG_DD(ErrorF("             : Fail to Create SW Video Surface\n"));
		    return retCode;
                }

		    
	    /*  Copy image data from system memory to video memory
	     *  TODO: use DRM's DMA feature to accelerate data copy
	     */
	    if (FOURCC_VIA != id) {
		dstPitch = pPriv->ddLock.SWDevice.dwPitch;

		switch(id) {
		case FOURCC_YV12:
		    (*viaFastVidCpy)(pPriv->ddLock.SWDevice.lpSWOverlaySurface[pVia->dwFrameNum&1],buf,dstPitch,width,height,0);
		    break;
		case FOURCC_UYVY:
		case FOURCC_YUY2:
		default:
		    (*viaFastVidCpy)(pPriv->ddLock.SWDevice.lpSWOverlaySurface[pVia->dwFrameNum&1],buf,dstPitch,width,height,1);			                        break;
		}
	    } 

	    /* If there is bandwidth issue, block the H/W overlay */
	    if (!pVia->OverlaySupported)
		return BadAlloc;

	    /* 
	     *  fill video overlay parameter
	     */
	    lpUpdateOverlay->rSrc.left = src_x;
	    lpUpdateOverlay->rSrc.top = src_y;
	    lpUpdateOverlay->rSrc.right = src_x + width;
	    lpUpdateOverlay->rSrc.bottom = src_y + height;

	    lpUpdateOverlay->rDest.left = drw_x;
	    lpUpdateOverlay->rDest.top = drw_y;
	    lpUpdateOverlay->rDest.right = lpUpdateOverlay->rDest.left + drw_w;
	    lpUpdateOverlay->rDest.bottom = drw_y + drw_h;

	    lpUpdateOverlay->dwFlags = DDOVER_SHOW | DDOVER_KEYDEST;
	    if (pScrn->bitsPerPixel == 8)
                {
                    lpUpdateOverlay->dwColorSpaceLowValue = pPriv->colorKey & 0xff;
                }
	    else
                {
                    lpUpdateOverlay->dwColorSpaceLowValue = pPriv->colorKey;
                }
	    lpUpdateOverlay->dwFourcc = id;

	    /* If use extend FIFO mode */
	    if (pScrn->currentMode->HDisplay > 1024)
                {
                    dwUseExtendedFIFO = 1;
                }

	    if (FOURCC_VIA != id) {

		/*
		 * XvMC flipping is done in the client lib.
		 */

		DBG_DD(ErrorF("             : Flip\n"));
		Flip(pVia, pPriv, id, pVia->dwFrameNum&1);
	    }
		
	    pVia->dwFrameNum ++;
    
	    /* If the dest rec. & extendFIFO doesn't change, don't do UpdateOverlay 
	       unless the surface clipping has changed */
	    if ( (pPriv->old_drw_x == drw_x) && (pPriv->old_drw_y == drw_y)
		 && (pPriv->old_drw_w == drw_w) && (pPriv->old_drw_h == drw_h)
		 && (pPriv->old_src_w == src_w) && (pPriv->old_src_h == src_h)
		 && (pVia->old_dwUseExtendedFIFO == dwUseExtendedFIFO)
		 && (pVia->Video.VideoStatus & SW_VIDEO_ON) &&
		 RegionsEqual(&pPriv->clip, clipBoxes))
                {
                    return Success;
                }

	    pPriv->old_drw_x = drw_x;
	    pPriv->old_drw_y = drw_y;
	    pPriv->old_drw_w = drw_w;
	    pPriv->old_drw_h = drw_h;
	    pVia->old_dwUseExtendedFIFO = dwUseExtendedFIFO;
	    pVia->Video.VideoStatus |= SW_VIDEO_ON;

	    /* add to judge if need to re-create surface */
	    pPriv->old_src_w = src_w;
	    pPriv->old_src_h = src_h;

	    /*  BitBlt: Draw the colorkey rectangle */
	    if(!RegionsEqual(&pPriv->clip, clipBoxes)) {
		REGION_COPY(pScrn->pScreen, &pPriv->clip, clipBoxes);    
		if (pPriv->autoPaint) 
		    xf86XVFillKeyHelper(pScrn->pScreen, pPriv->colorKey, clipBoxes); 
	    } 
		
	    /*
	     *  Update video overlay
	     */

	    if ( -1 == VIAVidUpdateOverlay(pScrn, lpUpdateOverlay))
                {
                    DBG_DD(ErrorF(" via_video.c :              : call v4l updateoverlay fail. \n"));
                }
	    else
                {
		    DBG_DD(ErrorF(" via_video.c : PutImageG done OK\n"));
                    return Success;
                }
            }
            break;
        default:
            DBG_DD(ErrorF(" via_video.c : XVPort not supported\n"));
            break;
	}
    DBG_DD(ErrorF(" via_video.c : PutImageG done OK\n"));
    return Success;
}


static int 
viaQueryImageAttributesG(
    ScrnInfoPtr pScrn,
    int id,
    unsigned short *w, unsigned short *h,
    int *pitches, int *offsets
){
    int size, tmp;

    DBG_DD(ErrorF(" via_video.c : viaQueryImageAttributesG : FourCC=0x%x, ", id));

    if ( (!w) || (!h) )
       return 0;

    if(*w > VIA_MAX_XVIMAGE_X) *w = VIA_MAX_XVIMAGE_X;
    if(*h > VIA_MAX_XVIMAGE_Y) *h = VIA_MAX_XVIMAGE_Y;

    *w = (*w + 1) & ~1;
    if(offsets) 
           offsets[0] = 0;

    switch(id) {
    case FOURCC_YV12:  /*Planar format : YV12 -4:2:0*/
        *h = (*h + 1) & ~1;
        size = (*w + 3) & ~3;
        if(pitches) pitches[0] = size;
        size *= *h;
        if(offsets) offsets[1] = size;
        tmp = ((*w >> 1) + 3) & ~3;
        if(pitches) pitches[1] = pitches[2] = tmp;
        tmp *= (*h >> 1);
        size += tmp;
        if(offsets) offsets[2] = size;
        size += tmp;
        break;
    case FOURCC_VIA:
        *h = (*h + 1) & ~1;
#ifdef XF86DRI
	size = viaXvMCPutImageSize(pScrn);
#else
	size = 0;
#endif
	if (pitches) pitches[0] = size;
	break;
    case FOURCC_AI44:
    case FOURCC_IA44:
      size = *w * *h;
      if (pitches) pitches[0] = *w;
      if (offsets) offsets[0] = 0;
      break;
    case FOURCC_UYVY:  /*Packed format : UYVY -4:2:2*/
    case FOURCC_YUY2:  /*Packed format : YUY2 -4:2:2*/
    default:
        size = *w << 1;
        if(pitches) 
             pitches[0] = size;
        size *= *h;
        break;
    }

    if ( pitches )
       DBG_DD(ErrorF(" pitches[0]=%d, pitches[1]=%d, pitches[2]=%d, ", pitches[0], pitches[1], pitches[2]));
    if ( offsets )
       DBG_DD(ErrorF(" offsets[0]=%d, offsets[1]=%d, offsets[2]=%d, ", offsets[0], offsets[1], offsets[2]));
    DBG_DD(ErrorF(" width=%d, height=%d \n", *w, *h));

    return size;
}


static int
viaPutVideo(ScrnInfoPtr pScrn,
    short src_x, short src_y, short drw_x, short drw_y,
    short src_w, short src_h, short drw_w, short drw_h,
    RegionPtr clipBoxes, pointer data)
{

    viaPortPrivPtr pPriv=(viaPortPrivPtr)data;

#ifdef XV_DEBUG
    ErrorF(" via_video.c : viaPutVideo : Src %dx%d, %d, %d, %p\n",src_w,src_h,src_x,src_y,clipBoxes);
    ErrorF(" via_video.c : Dst %dx%d, %d, %d \n",drw_w,drw_h,drw_x,drw_y);
    ErrorF(" via_video.c : colorkey : 0x%x \n",pPriv->colorKey);
#endif


    /*  BitBlt: Color fill */
    if(!RegionsEqual(&pPriv->clip, clipBoxes)) {
        REGION_COPY(pScrn->pScreen, &pPriv->clip, clipBoxes);
        XAAFillSolidRects(pScrn,pPriv->colorKey,GXcopy, ~0,
                          REGION_NUM_RECTS(clipBoxes),
                          REGION_RECTS(clipBoxes));
    }

    switch ( pPriv->xv_portnum )
    {
        case COMMAND_FOR_TV0 :
            pPriv->yuv_win.x = drw_x;
            pPriv->yuv_win.y = drw_y;
            pPriv->yuv_win.width = drw_w;
            pPriv->yuv_win.height = drw_h;
            pPriv->yuv_win.chromakey = pPriv->colorKey;
            break;
            
        case COMMAND_FOR_TV1 :
            pPriv->yuv_win.x = drw_x;
            pPriv->yuv_win.y = drw_y;
            pPriv->yuv_win.width = drw_w;
            pPriv->yuv_win.height = drw_h;
            pPriv->yuv_win.chromakey = pPriv->colorKey;
            break;

        case COMMAND_FOR_SWOV    :
        case COMMAND_FOR_DUMMY   :
            DBG_DD(ErrorF(" via_video.c : This port doesn't support PutVideo.\n"));
            return XvBadAlloc;
        default:
            DBG_DD(ErrorF(" via_video.c : Error port access.\n"));
            return XvBadAlloc;
    }

    return Success;
}

#endif  /* !XvExtension */
