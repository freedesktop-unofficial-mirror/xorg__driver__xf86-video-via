/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_driver.h,v 1.13 2004/02/08 17:57:10 tsi Exp $ */
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

#ifndef _VIA_DRIVER_H_
#define _VIA_DRIVER_H_ 1

/* #define HAVE_DEBUG */

#ifdef HAVE_DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#include "vgaHW.h"
#include "xf86.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Cursor.h"
#include "mipointer.h"
#include "micmap.h"
#include "fourcc.h"
#include "fb.h"
#include "xf86cmap.h"
#include "vbe.h"
#include "xaa.h"

#include "via_regs.h"
#include "via_i2c.h"
#include "via_bios.h"
#include "via_priv.h"
/* #include "ginfo.h" */
#include "via_swov.h"

#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "sarea.h"
#include "dri.h"
#include "GL/glxint.h"
#include "via_dri.h"
#endif

#define DRIVER_NAME     "via"
#define VERSION_MAJOR   0
#define VERSION_MINOR   1
#define PATCHLEVEL      27
#define VIA_VERSION     ((VERSION_MAJOR<<24) | (VERSION_MINOR<<16) | PATCHLEVEL)

#define VIA_MAX_ACCEL_X         (2047)
#define VIA_MAX_ACCEL_Y         (2047)
#define VIA_PIXMAP_CACHE_SIZE   (4 * (VIA_MAX_ACCEL_X + 1) * (VIA_MAX_ACCEL_Y +1))
#define VIA_CURSOR_SIZE         (4 * 1024)
#define VIA_VQ_SIZE             (256 * 1024)
#define VIA_CBUFFERSIZE         512

typedef struct {
    unsigned char   SR08, SR0A, SR0F;

    /*   extended Sequencer registers */
    unsigned char   SR10, SR11, SR12, SR13,SR14,SR15,SR16;
    unsigned char   SR17, SR18, SR19, SR1A,SR1B,SR1C,SR1D,SR1E;
    unsigned char   SR1F, SR20, SR21, SR22,SR23,SR24,SR25,SR26;
    unsigned char   SR27, SR28, SR29, SR2A,SR2B,SR2C,SR2D,SR2E;
    unsigned char   SR2F, SR30, SR31, SR32,SR33,SR34,SR40,SR41;
    unsigned char   SR42, SR43, SR44, SR45,SR46,SR47;

    /*   extended CRTC registers */
    unsigned char   CR13, CR30, CR31, CR32, CR33, CR34, CR35, CR36;
    unsigned char   CR37, CR38, CR39, CR3A, CR40, CR41, CR42, CR43;
    unsigned char   CR44, CR45, CR46, CR47, CR48, CR49, CR4A;
    unsigned char   CRTCRegs[68];
    unsigned char   TVRegs[0xFF];
/*    unsigned char   LCDRegs[0x40];*/
} VIARegRec, *VIARegPtr;

/*Definition for  CapturePortID*/
#define PORT0     0      /* Capture Port 0*/
#define PORT1     1      /* Capture Port 1*/

typedef struct __viaVideoControl {
  CARD16 PORTID;
  CARD32 dwCompose;
  CARD32 dwHighQVDO;
  CARD32 VideoStatus;
  CARD32 dwAction;
#define ACTION_SET_PORTID      0
#define ACTION_SET_COMPOSE     1
#define ACTION_SET_HQV         2
#define ACTION_SET_BOB	       4
#define ACTION_SET_VIDEOSTATUS 8
  Bool  Cap0OnScreen1;   /* True: Capture0 On Screen1 ; False: Capture0 On Screen0 */
  Bool  Cap1OnScreen1;   /* True: Capture1 On Screen1 ; False: Capture1 On Screen0 */
  Bool  MPEGOnScreen1;   /* True: MPEG On Screen1 ; False: MPEG On Screen0 */
} VIAVideoControlRec, VIAVideoControlPtr;

/* VIA Tuners */
typedef struct
{
    int			decoderType;		/* Decoder I2C Type */
#define SAA7108H		0
#define SAA7113H		1
#define SAA7114H		2
    I2CDevPtr 		I2C;			/* Decoder I2C */
    I2CDevPtr 		FMI2C;			/* FM Tuner I2C */
    
    /* Not yet used */
    int			autoDetect;		/* Autodetect mode */
    int			tunerMode;		/* Fixed mode */
} ViaTunerRec, *ViaTunerPtr;

/*
 * varables that need to be shared among different screens.
 */
typedef struct {
    Bool b3DRegsInitialized;
} ViaSharedRec, *ViaSharedPtr;

#ifdef XF86DRI

#define VIA_XVMC_MAX_BUFFERS 2
#define VIA_XVMC_MAX_CONTEXTS 4
#define VIA_XVMC_MAX_SURFACES 20

    
typedef struct {
    VIAMem memory_ref;
    unsigned long offsets[VIA_XVMC_MAX_BUFFERS];
} ViaXvMCSurfacePriv;

typedef struct {
    drm_context_t drmCtx;
} ViaXvMCContextPriv;

typedef struct {
    XID contexts[VIA_XVMC_MAX_CONTEXTS];
    XID surfaces[VIA_XVMC_MAX_SURFACES];
    ViaXvMCSurfacePriv *sPrivs[VIA_XVMC_MAX_SURFACES];
    ViaXvMCContextPriv *cPrivs[VIA_XVMC_MAX_CONTEXTS];
    int nContexts,nSurfaces;
    drm_handle_t mmioBase,fbBase,sAreaBase;
    unsigned sAreaSize;
    drmAddress sAreaAddr;
    unsigned activePorts;
}ViaXvMC, *ViaXvMCPtr;

#endif

typedef struct _twodContext {
    CARD32 mode;
} ViaTwodContext;

typedef struct{
    unsigned curPos;
    CARD32 buffer[VIA_CBUFFERSIZE];
    int status;
} ViaCBuffer;

typedef struct _VIA {
    VIARegRec           SavedReg;
    xf86CursorInfoPtr   CursorInfoRec;
    int                 Bpp, Bpl;
    unsigned            PlaneMask;

    Bool                FirstInit;
    unsigned long       videoRambytes;
    int                 videoRamKbytes;
    int                 FBFreeStart;
    int                 FBFreeEnd;
    int                 CursorStart;
    int                 VQStart;
    int                 VQEnd;

    /* These are physical addresses. */
    unsigned long       FrameBufferBase;
    unsigned long       MmioBase;

    /* These are linear addresses. */
    unsigned char*      MapBase;
    unsigned char*      VidMapBase;
    unsigned char*      MpegMapBase;
    unsigned char*      BltBase;
    unsigned char*      MapBaseDense;
    unsigned char*      FBBase;
    unsigned char*      FBStart;
    CARD8               MemClk;

    /* Private memory pool management */
    int			SWOVUsed[MEM_BLOCKS]; /* Free map for SWOV pool */
    unsigned long	SWOVPool;	/* Base of SWOV pool */
    unsigned long	SWOVSize;	/* Size of SWOV blocks */

    /* Here are all the Options */
    Bool                VQEnable;
    Bool                pci_burst;
    Bool                NoPCIRetry;
    Bool                hwcursor;
    Bool                NoAccel;
    Bool                shadowFB;
    int                 rotate;

    CloseScreenProcPtr  CloseScreen;
    pciVideoPtr         PciInfo;
    PCITAG              PciTag;
    int                 Chipset;
    int                 ChipId;
    int                 ChipRev;
    vbeInfoPtr          pVbe;
    int                 EntityIndex;

    /* Support for shadowFB and rotation */
    unsigned char*      ShadowPtr;
    int                 ShadowPitch;
    void                (*PointerMoved)(int index, int x, int y);

    /* Support for XAA acceleration */
    XAAInfoRecPtr       AccelInfoRec;
    xRectangle          Rect;
    CARD32              SavedCmd;
    CARD32              SavedFgColor;
    CARD32              SavedBgColor;
    CARD32              SavedPattern0;
    CARD32              SavedPattern1;
    CARD32              SavedPatternAddr;
    int                 justSetup;
    ViaTwodContext      td;
    ViaCBuffer          cBuf;
  
    /* BIOS Info Ptr */
    VIABIOSInfoPtr      pBIOSInfo;
    struct ViaCardIdStruct* Id;

    /* Support for DGA */
    int                 numDGAModes;
    DGAModePtr          DGAModes;
    Bool                DGAactive;
    int                 DGAViewportStatus;
    int			DGAOldDisplayWidth;
    int			DGAOldBitsPerPixel;
    int			DGAOldDepth;

    /* I2C & DDC */
    I2CBusPtr           pI2CBus1;    
    I2CBusPtr           pI2CBus2;    
    I2CBusPtr           pI2CBus3;    /* Future implementation: now gpioi2c */
    xf86MonPtr          DDC1;
    xf86MonPtr          DDC2;
    GpioI2cRec          GpioI2c; /* should be weened off, but we have no info,
				    no hardware that this code is known to 
				    work with (properly) */

    /* MHS */
    Bool                IsSecondary;
    Bool                HasSecondary;
    Bool                SAMM;

    /* Capture de-interlace Mode */
    CARD32              Cap0_Deinterlace;
    CARD32              Cap1_Deinterlace;

    Bool                Cap0_FieldSwap;

#ifdef XF86DRI
    Bool		directRenderingEnabled;
    Bool                XvMCEnabled;
    DRIInfoPtr		pDRIInfo;
    int 		drmFD;
    int 		numVisualConfigs;
    __GLXvisualConfig* 	pVisualConfigs;
    VIAConfigPrivPtr 	pVisualConfigsPriv;
    unsigned long 	agpHandle;
    unsigned long 	registerHandle;
    unsigned long 	agpAddr;
    drmAddress          agpMappedAddr;
    unsigned char 	*agpBase;
    unsigned int 	agpSize;
    Bool 		IsPCI;
    Bool 		drixinerama;
    ViaXvMC             xvmc;
#endif
    Bool		DRIIrqEnable;

    unsigned char	ActiveDevice;	/* if SAMM, non-equal pBIOSInfo->ActiveDevice */
    unsigned char       *CursorImage;
    CARD32		CursorFG;
    CARD32		CursorBG;
    CARD32		CursorMC;

    /* Video */
    swovRec		swov;
    VIAVideoControlRec  Video;
    VIAHWDiff		HWDiff;
    unsigned long	dwV1, dwV3;
    unsigned long	OverlaySupported;
    unsigned long	dwFrameNum;

    pointer		VidReg;
    unsigned long	gdwVidRegCounter;
    unsigned long	old_dwUseExtendedFIFO;
    
    /* Overlay TV Tuners */
    ViaTunerPtr		Tuner[2];
    I2CDevPtr		CXA2104S;
    int			AudioMode;
    int			AudioMute;

    ViaSharedPtr	sharedData;

#ifdef HAVE_DEBUG
    Bool                DumpVGAROM;
    Bool                PrintVGARegs;
    Bool                PrintTVRegs;
#endif /* HAVE_DEBUG */
} VIARec, *VIAPtr;


typedef struct
{
    Bool IsDRIEnabled;

    Bool HasSecondary;
    Bool BypassSecondary;
    /*These two registers are used to make sure the CRTC2 is
      retored before CRTC_EXT, otherwise it could lead to blank screen.*/
    Bool IsSecondaryRestored;
    Bool RestorePrimary;

    ScrnInfoPtr pSecondaryScrn;
    ScrnInfoPtr pPrimaryScrn;
}VIAEntRec, *VIAEntPtr;


/* Shortcuts.  These depend on a local symbol "pVia". */

#define WaitIdle()      pVia->myWaitIdle(pVia)
#define VIAPTR(p)       ((VIAPtr)((p)->driverPrivate))


/* Prototypes. */
void VIAAdjustFrame(int scrnIndex, int y, int x, int flags);
Bool VIASwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
void ViaWaitIdle(ScrnInfoPtr pScrn);

/* In via_cursor.c. */
Bool VIAHWCursorInit(ScreenPtr pScreen);
void VIAShowCursor(ScrnInfoPtr);
void VIAHideCursor(ScrnInfoPtr);
void ViaCursorStore(ScrnInfoPtr pScrn);
void ViaCursorRestore(ScrnInfoPtr pScrn);

/* In via_accel.c. */
Bool VIAInitAccel(ScreenPtr);
void VIAInitialize2DEngine(ScrnInfoPtr);
void VIAAccelSync(ScrnInfoPtr);
void ViaVQDisable(ScrnInfoPtr pScrn);

/* In via_shadow.c */
void VIAPointerMoved(int index, int x, int y);
void VIARefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void VIARefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void VIARefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void VIARefreshArea24(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void VIARefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* In via_dga.c */
Bool VIADGAInit(ScreenPtr);

/*In via_video.c*/
void viaInitVideo(ScreenPtr pScreen);
void viaExitVideo(ScrnInfoPtr pScrn);
void viaResetVideo(ScrnInfoPtr pScrn);
void viaSaveVideo(ScrnInfoPtr pScrn);
void viaRestoreVideo(ScrnInfoPtr pScrn);
void viaStopSWOVerlay(ScrnInfoPtr pScrn);
int viaGetPortAttributeG(ScrnInfoPtr, Atom ,INT32 *, pointer);
int viaSetPortAttributeG(ScrnInfoPtr, Atom, INT32, pointer);
int viaPutImageG( ScrnInfoPtr, short, short, short, short, short, short, 
		  short, short,int, unsigned char*, short, short, Bool, 
		  RegionPtr, pointer);
void viaSetColorSpace(VIAPtr pVia, int hue, int saturation, int brightness, int contrast,
		      Bool reset);

/* in via_overlay.c */
unsigned long viaOverlayHQVCalcZoomHeight (VIAPtr pVia, unsigned long srcHeight,unsigned long dstHeight,
                             unsigned long * lpzoomCtl, unsigned long * lpminiCtl, unsigned long * lpHQVfilterCtl, unsigned long * lpHQVminiCtl,unsigned long * lpHQVzoomflag);
unsigned long viaOverlayGetSrcStartAddress (VIAPtr pVia, unsigned long dwVideoFlag,RECTL rSrc,RECTL rDest, unsigned long dwSrcPitch,LPDDPIXELFORMAT lpDPF,unsigned long * lpHQVoffset );
void viaOverlayGetDisplayCount(VIAPtr pVIa, unsigned long dwVideoFlag,LPDDPIXELFORMAT lpDPF,unsigned long dwSrcWidth,unsigned long * lpDisplayCountW);
unsigned long viaOverlayHQVCalcZoomWidth(VIAPtr pVia, unsigned long dwVideoFlag, unsigned long srcWidth , unsigned long dstWidth,
                           unsigned long * lpzoomCtl, unsigned long * lpminiCtl, unsigned long * lpHQVfilterCtl, unsigned long * lpHQVminiCtl,unsigned long * lpHQVzoomflag);
void viaOverlayGetV1Format(VIAPtr pVia, unsigned long dwVideoFlag,LPDDPIXELFORMAT lpDPF, unsigned long * lpdwVidCtl,unsigned long * lpdwHQVCtl );
void viaOverlayGetV3Format(VIAPtr pVia, unsigned long dwVideoFlag,LPDDPIXELFORMAT lpDPF, unsigned long * lpdwVidCtl,unsigned long * lpdwHQVCtl );

void viaCalculateVideoColor(VIAPtr pVia, int hue, int saturation, 
			    int brightness, int contrast,Bool reset,
			    CARD32 *col1,CARD32 *col2);



/* In via_memory.c */
void VIAFreeLinear(VIAMemPtr);
unsigned long VIAAllocLinear(VIAMemPtr, ScrnInfoPtr, unsigned long);
void VIAInitLinear(ScreenPtr pScreen);

/* In via_tuner.c */
void ViaTunerStandard(ViaTunerPtr, int);
void ViaTunerBrightness(ViaTunerPtr, int);
void ViaTunerContrast(ViaTunerPtr, int);
void ViaTunerHue(ViaTunerPtr, int);
void ViaTunerLuminance(ViaTunerPtr, int);
void ViaTunerSaturation(ViaTunerPtr, int);
void ViaTunerInput(ViaTunerPtr, int);
#define MODE_TV		0
#define MODE_SVIDEO	1
#define MODE_COMPOSITE	2

void ViaTunerChannel(ViaTunerPtr, int, int);
void ViaAudioSelect(ScrnInfoPtr pScrn, int tuner);
void ViaAudioInit(VIAPtr pVia);
void ViaAudioMode(VIAPtr pVia, int mode);
void ViaAudioMute(VIAPtr pVia, int mute);
void ViaTunerProbe(ScrnInfoPtr pScrn);
void ViaTunerDestroy(ScrnInfoPtr pScrn);

/* In via_xwmc.c */

#ifdef XF86DRI
/* Basic init and exit functions */
void ViaInitXVMC(ScreenPtr);    
void ViaCleanupXVMC(ScreenPtr);
/* Returns the size of the fake Xv Image used as XvMC command buffer to the X server*/
unsigned long viaXvMCPutImageSize(ScrnInfoPtr pScrn);
int viaXvMCInterceptXvAttribute(ScrnInfoPtr pScrn, Atom attribute, 
				INT32 value,pointer data);
int viaXvMCInitXv(ScrnInfoPtr pScrn, pointer data);
int viaXvMCInterceptPutImage( ScrnInfoPtr, short, short, short, short, short, 
			      short, short, short,int, unsigned char*, short, 
			      short, Bool, RegionPtr, pointer);
int viaXvMCInterceptXvGetAttribute(ScrnInfoPtr pScrn, Atom attribute, 
				   INT32 *value,pointer data);

#endif


#endif /* _VIA_DRIVER_H_ */
