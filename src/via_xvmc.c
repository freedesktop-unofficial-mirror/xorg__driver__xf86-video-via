/*****************************************************************************
 * VIA Unichrome XvMC extension X server driver.
 *
 * Copyright (c) 2004 Thomas Hellström. All rights reserved.
 * Copyright (c) 2000 Intel Corporation. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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

#ifdef XF86DRI

#include "via.h"
#include "via_dri.h"
#include "via_driver.h"

#include "xf86xv.h"
#include "fourcc.h"
#include "xf86xvpriv.h"

#include "xf86xvmc.h"
#include "Xv.h"
#include "XvMC.h"
#include "xaa.h"
#include "xaalocal.h"
#include "dixstruct.h"
#include "via_xvmc.h"
#include "dristruct.h"
#include "dri.h"
#include "via_xvpriv.h"
#include "via_video.h"

#define MAKE_ATOM(a) MakeAtom(a, strlen(a), TRUE)


/*
 * List Attributes for the XvMC extension to handle:
 * As long as the attribute is supported by the Xv adaptor, it needs only to
 * be added here to be supported also by XvMC. 
 * Currently, only colorkey seems to be supported by Xv for Putimage.
 */

static char *attrXvMC[VIA_NUM_XVMC_ATTRIBUTES] = 
  {"XV_COLORKEY",
   "XV_AUTOPAINT_COLORKEY",
   "XV_BRIGHTNESS",
   "XV_CONTRAST",
   "XV_SATURATION",
   "XV_HUE"};
static Atom attrAtoms[VIA_NUM_XVMC_ATTRIBUTES];

/*
 * Xv Port private structure for XvMC.
 */

typedef struct{
    unsigned ctxDisplaying;
    int xvmc_port;
    ViaXvMCAttrHolder xvAttr;
    int newAttribute;
}ViaXvMCXVPriv;
    
    
/*
 * Proposed XvMC via driver extension:
 */

#define XVMC_VLD 0x0020000

/*
 * Function declarations.
 */


static int ViaXvMCCreateContext (ScrnInfoPtr pScrn, XvMCContextPtr pContext,
                           int *num_priv, long **priv );
static void ViaXvMCDestroyContext (ScrnInfoPtr pScrn, XvMCContextPtr pContext);
static int ViaXvMCCreateSurface (ScrnInfoPtr pScrn, XvMCSurfacePtr pSurf,
                           int *num_priv, long **priv );
static void ViaXvMCDestroySurface (ScrnInfoPtr pScrn, XvMCSurfacePtr pSurf);
static int ViaXvMCCreateSubpicture (ScrnInfoPtr pScrn, XvMCSubpicturePtr pSurf,
                               int *num_priv, long **priv );
static void ViaXvMCDestroySubpicture (ScrnInfoPtr pScrn, XvMCSubpicturePtr pSubp);


/*
 * Init and clean up the screen private parts of XvMC.
 */

static void initViaXvMC(ViaXvMCPtr vXvMC) 
{
    unsigned i;

    for (i=0; i < VIA_XVMC_MAX_CONTEXTS; ++i) {
	vXvMC->contexts[i] = 0;
	vXvMC->cPrivs[i] = 0;
    }

    for (i=0; i < VIA_XVMC_MAX_SURFACES; ++i) {
	vXvMC->surfaces[i] = 0;
	vXvMC->sPrivs[i] = 0;
    }

}
    
static void cleanupViaXvMC(ViaXvMCPtr vXvMC) 
{
    unsigned i;

    for (i=0; i < VIA_XVMC_MAX_CONTEXTS; ++i) {
	vXvMC->contexts[i] = 0;
	if (vXvMC->cPrivs[i]) {
	    xfree(vXvMC->cPrivs[i]);
	    vXvMC->cPrivs[i] = 0;
	}
    }

    for (i=0; i < VIA_XVMC_MAX_SURFACES; ++i) {
	vXvMC->surfaces[i] = 0;
	if (vXvMC->sPrivs[i]) {
	    xfree(vXvMC->sPrivs[i]);
	    vXvMC->sPrivs[i] = 0;
	}
    }
}

static unsigned stride(int w) 
{
    return (w + 31) & ~31;
}

static unsigned long size_yuv420(int w, int h) 
{
    unsigned yPitch = stride(w);
    return h* (yPitch + (yPitch >> 1));
} 

static unsigned long size_xx44(int w, int h) 
{
    return h * stride(w);
} 
    

static int yv12_subpicture_index_list[2] = 
{
  FOURCC_IA44,
   FOURCC_AI44
};

static XF86MCImageIDList yv12_subpicture_list =
{
  2,
  yv12_subpicture_index_list
};
 
static XF86MCSurfaceInfoRec Via_YV12_mpg2_surface =
{
    FOURCC_YV12,  
    XVMC_CHROMA_FORMAT_420,
    0,
    1024,
    1024,
    1024,
    1024,
    XVMC_MPEG_2 | XVMC_VLD,
    XVMC_OVERLAID_SURFACE | XVMC_BACKEND_SUBPICTURE,
    &yv12_subpicture_list
};

/*
 * FIXME: It remains to be checked whether the hardware can really do mpeg1. 
 */

static XF86MCSurfaceInfoRec Via_YV12_mpg1_surface =
{
    FOURCC_YV12,  
    XVMC_CHROMA_FORMAT_420,
    0,
    1024,
    1024,
    1024,
    1024,
    XVMC_MPEG_1 | XVMC_VLD,
    XVMC_OVERLAID_SURFACE | XVMC_BACKEND_SUBPICTURE,
    &yv12_subpicture_list
};

static XF86MCSurfaceInfoPtr ppSI[2] = 
{
    (XF86MCSurfaceInfoPtr)&Via_YV12_mpg2_surface,
    (XF86MCSurfaceInfoPtr)&Via_YV12_mpg1_surface
};

/* List of subpicture types that we support */
static XF86ImageRec ia44_subpicture = XVIMAGE_IA44;
static XF86ImageRec ai44_subpicture = XVIMAGE_AI44;

static XF86ImagePtr Via_subpicture_list[2] =
{
  (XF86ImagePtr)&ia44_subpicture,
  (XF86ImagePtr)&ai44_subpicture
};

/* 
 * Fill in the device dependent adaptor record. 
 * This is named "VIA Video Overlay" because this code falls under the
 * XV extenstion, the name must match or it won't be used.
 *
 * Surface and Subpicture - see above
 * Function pointers to functions below
 */

static XF86MCAdaptorRec pAdapt = 
{
    "XV_SWOV",		/* name */
    2,				/* num_surfaces */
    ppSI,				/* surfaces */
    2,				/* num_subpictures */
    Via_subpicture_list,		/* subpictures */
    (xf86XvMCCreateContextProcPtr)ViaXvMCCreateContext,
    (xf86XvMCDestroyContextProcPtr)ViaXvMCDestroyContext,
    (xf86XvMCCreateSurfaceProcPtr)ViaXvMCCreateSurface,
    (xf86XvMCDestroySurfaceProcPtr)ViaXvMCDestroySurface,
    (xf86XvMCCreateSubpictureProcPtr)ViaXvMCCreateSubpicture,
    (xf86XvMCDestroySubpictureProcPtr)ViaXvMCDestroySubpicture
};

static XF86MCAdaptorPtr ppAdapt[1] = {(XF86MCAdaptorPtr)&pAdapt};

static void mpegDisable(VIAPtr pVia,CARD32 val) 

{
    MPGOutD(0x0c, MPGInD(0x0c) & (val ^ 0xffffffff));
}


void ViaInitXVMC(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  volatile ViaXvMCSAreaPriv *saPriv;
  char *bID;
  drmVersionPtr drmVer;

  pVia->XvMCEnabled = 0;
  if(!pVia->directRenderingEnabled) {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		 "[XvMC] Cannot use XvMC without DRI!\n");
      return;
  }

  if (NULL == (drmVer = drmGetVersion(pVia->drmFD))) {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] Could not get drm version. Disabling XvMC\n");
      return;
  }
  if (((drmVer->version_major <= 2) && (drmVer->version_minor < 0))) {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] Kernel drm is not compatible with XvMC.\n"); 
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] Kernel drm version: %d.%d.%d "
		 "and need at least version 2.0.0.\n",
		 drmVer->version_major,drmVer->version_minor,
		 drmVer->version_patchlevel); 
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] Please update. Disabling XvMC.\n");
      drmFreeVersion(drmVer);
      return;
  } 
  if ((drmVer->version_major >= 3)) {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] XvMC X driver may not be compatible "
		 "with kernel drm.\n");
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] Continuing, but strange things may happen.\n");
  } 
  drmFreeVersion(drmVer);

  if (!DRIOpenConnection(pScreen,&vXvMC->sAreaBase,&bID)) {
      xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		 "[XvMC] DRIOpenConnection failed. Disabling XvMC.\n");
      return;
  } 
  DRICloseConnection(pScreen);

  vXvMC->mmioBase = pVia->registerHandle;

  if (drmAddMap(pVia->drmFD, 
		(drm_handle_t)pVia->FrameBufferBase,
                pVia->videoRambytes, DRM_FRAME_BUFFER, 
		0 , &(vXvMC->fbBase)) < 0) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
		 "[XvMC] drmAddMap(FB) failed. Disabling XvMC.\n");
      return;
  } 

  initViaXvMC(vXvMC);

  if (! xf86XvMCScreenInit(pScreen, 1, ppAdapt)) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
		 "[XvMC] XvMCScreenInit failed. Disabling XvMC.\n");
      drmRmMap(pVia->drmFD,vXvMC->fbBase);
      return;
  }   
  
  vXvMC->activePorts = 0;
  saPriv=(ViaXvMCSAreaPriv *) DRIGetSAREAPrivate(pScreen);
  saPriv->XvMCCtxNoGrabbed = ~0;

  XVMC_DECODER_FUTEX(saPriv)->lock = 0;

  xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[XvMC] Initialized XvMC extension.\n");
  pVia->XvMCEnabled = 1;
}

void ViaCleanupXVMC(ScreenPtr pScreen) 
{
  
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VIAPtr pVia = VIAPTR(pScrn);
    ViaXvMCPtr vXvMC = &(pVia->xvmc);
   
    if (pVia->XvMCEnabled) {
	mpegDisable(pVia,0);
	drmRmMap(pVia->drmFD,vXvMC->mmioBase); 
	drmRmMap(pVia->drmFD,vXvMC->fbBase); 
	cleanupViaXvMC(vXvMC);
    }
    pVia->XvMCEnabled = 0;
}

int ViaXvMCCreateContext (ScrnInfoPtr pScrn, XvMCContextPtr pContext,
                            int *num_priv, long **priv )
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  DRIInfoPtr pDRIInfo = pVia->pDRIInfo;
  VIADRIPtr pViaDRI = pDRIInfo->devPrivate;
  ViaXvMCCreateContextRec *contextRec;
  unsigned ctxNo;
  ViaXvMCContextPriv *cPriv;
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr) pContext->port_priv;
  viaPortPrivPtr pPriv = (viaPortPrivPtr) portPriv->DevPriv.ptr;
  ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;
  volatile ViaXvMCSAreaPriv *sAPriv;
  int authenticated;

  sAPriv = (ViaXvMCSAreaPriv*) DRIGetSAREAPrivate(pScrn->pScreen);

  if (-1 == vx->xvmc_port) {
      vx->xvmc_port = (vXvMC->activePorts++);
      sAPriv->XvMCSubPicOn[vx->xvmc_port] = 0;
      sAPriv->XvMCDisplaying[vx->xvmc_port] = 0; 
  }

  if(vXvMC->nContexts >= VIA_XVMC_MAX_CONTEXTS) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
        "[XvMC] ViaXvMCCreateContext: Out of contexts.\n");
    return BadAlloc;
  }

  *priv = xcalloc(1,sizeof(ViaXvMCCreateContextRec));
  contextRec = (ViaXvMCCreateContextRec *)*priv;

  if(!*priv) {
    *num_priv = 0;
    return BadAlloc;
  }

  *num_priv = sizeof(ViaXvMCCreateContextRec) >> 2;

  for (ctxNo = 0; ctxNo < VIA_XVMC_MAX_CONTEXTS; ++ctxNo) {
      if (0 == vXvMC->contexts[ctxNo] ) break;
  }

  cPriv = (ViaXvMCContextPriv *) xcalloc(1,sizeof(ViaXvMCContextPriv));

  if(!cPriv) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		 "[XvMC] ViaXvMCCreateContext: Unable to allocate memory!\n");
    xfree(*priv);
    *num_priv = 0;
    return BadAlloc;
  }

  if(drmCreateContext(pVia->drmFD, &(contextRec->drmcontext) ) < 0) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		 "[XvMC] ViaXvMCCreateContext: Unable to create DRMContext!\n");
    xfree(*priv);
    xfree(cPriv);
    return BadAlloc;
  }
  drmSetContextFlags(pVia->drmFD, contextRec->drmcontext, 
		     DRM_CONTEXT_2DONLY);
  cPriv->drmCtx = contextRec->drmcontext;

  authenticated = (drmAuthMagic(pVia->drmFD, pContext->flags) == 0);
  contextRec->fbBase = (CARD8 *)pVia->FrameBufferBase;

  /*
   * Export framebuffer and mmio to non-root clients.
   */

  contextRec->ctxNo = ctxNo;
  contextRec->xvmc_port = vx->xvmc_port;
  contextRec->fbOffset = vXvMC->fbBase;
  contextRec->fbSize = pVia->videoRambytes;
  contextRec->mmioOffset = vXvMC->mmioBase;
  contextRec->mmioSize = VIA_MMIO_REGSIZE;
  contextRec->sAreaOffset = vXvMC->sAreaBase;
  contextRec->sAreaSize = pDRIInfo->SAREASize;
  contextRec->sAreaPrivOffset = sizeof(XF86DRISAREARec);
  contextRec->major = VIAXVMC_MAJOR;
  contextRec->minor = VIAXVMC_MINOR;
  contextRec->pl = VIAXVMC_PL;
  strncpy (contextRec->busIdString,pDRIInfo->busIdString, 20);
  contextRec->initAttrs = vx->xvAttr; 
  contextRec->useAGP = pViaDRI->ringBufActive;
  contextRec->authenticated = authenticated;
  vXvMC->nContexts++;
  vXvMC->contexts[ctxNo] = pContext->context_id;
  vXvMC->cPrivs[ctxNo] = cPriv;

  return Success;
}


int ViaXvMCCreateSurface (ScrnInfoPtr pScrn, XvMCSurfacePtr pSurf,
                           int *num_priv, long **priv )
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  unsigned srfNo,numBuffers,i;
  ViaXvMCSurfacePriv *sPriv;
  XvMCContextPtr ctx;
  unsigned bufSize,yBufSize;

  if (VIA_XVMC_MAX_SURFACES == vXvMC->nSurfaces) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		 "[XvMC] ViaXvMCCreateSurface: Too many surfaces !\n");
      return BadAlloc;
  }

  sPriv = (ViaXvMCSurfacePriv *) xcalloc(1,sizeof(ViaXvMCSurfacePriv));

  if(!sPriv) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
        "[XvMC] ViaXvMCCreateSurface: Unable to allocate memory!\n");
    *num_priv = 0;
    return BadAlloc;
  }

  numBuffers = 1;

  /*
   * Some chips require more than one buffer per surface (and a special
   * flipping procedure; See Ivor Hewitt's ddmpeg.c version 1.6). The client
   * lib will detect the number of buffers allocated and determine the
   * flipping method from that.
   */ 
#if 0  /* Not enabled yet. */
  switch(pVia->ChipSet) {
  case VIA_CLE266:
      switch(pVia->ChipRev) {
      case 0x10: /* CLE266 C0 only? or all C? */
	  numBuffers = 2;
	  break;
      default:
	  break;
      }
      break;
  default:
      break;
  }
#endif
  *num_priv = numBuffers + 2;

  *priv = (long *)xcalloc(*num_priv,sizeof(long));

  if(!*priv) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
        "[XvMC] ViaXvMCCreateSurface: Unable to allocate memory!\n");
    *num_priv = 0;
    xfree(sPriv);
    return BadAlloc;
  }

  for (srfNo = 0; srfNo < VIA_XVMC_MAX_SURFACES; ++srfNo) {
      if (0 == vXvMC->sPrivs[srfNo] ) break;
  }

  (*priv)[0] = srfNo;
  
  ctx = pSurf->context;
  bufSize = size_yuv420(ctx->width, ctx->height);
  sPriv->memory_ref.pool = 0;
  if (VIAAllocLinear(&(sPriv->memory_ref), pScrn, numBuffers * bufSize + 32)) {
      xfree(*priv);
      xfree(sPriv);
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "[XvMC] ViaXvMCCreateSurface: "
		 "Unable to allocate frambuffer memory!\n");
      return BadAlloc;
  }   
  
  (*priv)[1] = numBuffers;
  (*priv)[2] = sPriv->offsets[0] = ALIGN_TO_32_BYTES(sPriv->memory_ref.base);
  for (i = 1; i < numBuffers; ++i) {
      (*priv)[i+2] = sPriv->offsets[i] = sPriv->offsets[i-1] + bufSize;
  }
  
  yBufSize = stride(ctx->width)*ctx->height;
  for (i = 0; i < numBuffers; ++i) {
      memset((CARD8 *)(pVia->FBBase) + sPriv->offsets[i], 0, yBufSize);
      memset((CARD8 *)(pVia->FBBase) + sPriv->offsets[i] + yBufSize, 0x80, 
	     yBufSize >> 1);
  }

  vXvMC->sPrivs[srfNo] = sPriv;
  vXvMC->surfaces[srfNo] = pSurf->surface_id;
  vXvMC->nSurfaces++;
  return Success;
}

int ViaXvMCCreateSubpicture (ScrnInfoPtr pScrn, XvMCSubpicturePtr pSubp,
                              int *num_priv, long **priv )
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  unsigned srfNo;
  ViaXvMCSurfacePriv *sPriv;
  XvMCContextPtr ctx;
  unsigned bufSize;

  if (VIA_XVMC_MAX_SURFACES == vXvMC->nSurfaces) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		 "[XvMC] ViaXvMCCreateSubpicture: Too many surfaces !\n");
      return BadAlloc;
  }

  sPriv = (ViaXvMCSurfacePriv *) xcalloc(1,sizeof(ViaXvMCSurfacePriv));

  if(!sPriv) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
        "[XvMC] ViaXvMCCreateSubpicture: Unable to allocate memory!\n");
    *num_priv = 0;
    return BadAlloc;
  }

  *priv = (long *)xcalloc(3,sizeof(long));

  if(!*priv) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
        "[XvMC] ViaXvMCCreateSubpicture: Unable to allocate memory!\n");
    *num_priv = 0;
    xfree(sPriv);
    return BadAlloc;
  }
  
  *num_priv = 2;

  for (srfNo = 0; srfNo < VIA_XVMC_MAX_SURFACES; ++srfNo) {
      if (0 == vXvMC->sPrivs[srfNo] ) break;
  }

  (*priv)[0] = srfNo;
  
  ctx = pSubp->context;
  bufSize = size_xx44(ctx->width,ctx->height);
  sPriv->memory_ref.pool = 0;
  if (VIAAllocLinear(&(sPriv->memory_ref), pScrn, 
		     1 * bufSize + 32)) {
      xfree(*priv);
      xfree(sPriv);
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		 "[XvMC] ViaXvMCCreateSubpicture: Unable to allocate "
		 "framebuffer memory!\n");
      return BadAlloc;
  }   
  (*priv)[1] = sPriv->offsets[0] = ALIGN_TO_32_BYTES(sPriv->memory_ref.base);
  
  vXvMC->sPrivs[srfNo] = sPriv;
  vXvMC->surfaces[srfNo] = pSubp->subpicture_id;
  vXvMC->nSurfaces++;

  return Success;
}

static void ViaXvMCDestroyContext (ScrnInfoPtr pScrn, XvMCContextPtr pContext)
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  int i;
  volatile ViaXvMCSAreaPriv *sAPriv;
  drm_context_t context;
  viaPortPrivPtr pPriv;
  XvPortRecPrivatePtr portPriv;
  ViaXvMCXVPriv *vx;

  for(i=0; i < VIA_XVMC_MAX_CONTEXTS; i++) {
    if(vXvMC->contexts[i] == pContext->context_id) {

	/*
	 * Check if the context to be destroyed currently holds the decoder. 
	 * In that case, release the decoder.
	 */

	context = vXvMC->cPrivs[i]->drmCtx;
	sAPriv=(ViaXvMCSAreaPriv *) DRIGetSAREAPrivate(pScrn->pScreen);
	portPriv = (XvPortRecPrivatePtr) pContext->port_priv;
	pPriv = (viaPortPrivPtr) portPriv->DevPriv.ptr;
	vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;
	

	if ((i | VIA_XVMC_VALID) == vx->ctxDisplaying) {
	    vx->ctxDisplaying = 0;
	}

	if ((XVMC_DECODER_FUTEX(sAPriv)->lock & ~DRM_LOCK_CONT) ==
	    (context | DRM_LOCK_HELD)) {
	    DRM_CAS_RESULT(__ret);
	    sAPriv->XvMCCtxNoGrabbed = ~0;
	    DRM_CAS( XVMC_DECODER_FUTEX(sAPriv), context | DRM_LOCK_HELD,
		     0, __ret);
	    if (__ret) {

		drm_via_futex_t fx;
		fx.func = VIA_FUTEX_WAKE;
		fx.lock = 0;
		XVMC_DECODER_FUTEX(sAPriv)->lock = 0;
		drmCommandWrite(pVia->drmFD, DRM_VIA_DEC_FUTEX,
				&fx,sizeof(fx));
		
	    }
	}
		     
	drmDestroyContext(pVia->drmFD,vXvMC->cPrivs[i]->drmCtx);
	xfree(vXvMC->cPrivs[i]);
	vXvMC->cPrivs[i] = 0;
	vXvMC->nContexts--;
	vXvMC->contexts[i] = 0;
	return;
    }
  }
  return;
}

static void ViaXvMCDestroySurface (ScrnInfoPtr pScrn, XvMCSurfacePtr pSurf)
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  int i;
  volatile ViaXvMCSAreaPriv *sAPriv;
  XvMCContextPtr pContext = pSurf->context;
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr) pContext->port_priv;
  viaPortPrivPtr pPriv = (viaPortPrivPtr) portPriv->DevPriv.ptr;
  ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;

  for(i=0; i < VIA_XVMC_MAX_SURFACES; i++) {
    if(vXvMC->surfaces[i] == pSurf->surface_id) {

	sAPriv=(ViaXvMCSAreaPriv*) DRIGetSAREAPrivate(pScrn->pScreen);
	{
	    DRM_CAS_RESULT(__ret);
	    DRM_CAS(&(sAPriv->XvMCDisplaying[vx->xvmc_port]),
		    i|VIA_XVMC_VALID,0,__ret);
	    if (!__ret) 
		viaStopSWOVerlay(pScrn);		
	}

	VIAFreeLinear(&(vXvMC->sPrivs[i]->memory_ref));
	xfree(vXvMC->sPrivs[i]);
	vXvMC->nSurfaces--;
	vXvMC->sPrivs[i] = 0;
	vXvMC->surfaces[i] = 0;
      return;
    }
  }
  return;
}


static void ViaXvMCDestroySubpicture (ScrnInfoPtr pScrn, 
				      XvMCSubpicturePtr pSubp)
{
  VIAPtr pVia = VIAPTR(pScrn);
  ViaXvMCPtr vXvMC = &(pVia->xvmc);
  int i;
  volatile ViaXvMCSAreaPriv *sAPriv;
  XvMCContextPtr pContext = pSubp->context;
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr) pContext->port_priv;
  viaPortPrivPtr pPriv = (viaPortPrivPtr) portPriv->DevPriv.ptr;
  ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;

  for(i=0; i < VIA_XVMC_MAX_SURFACES; i++) {
    if(vXvMC->surfaces[i] == pSubp->subpicture_id) {

	sAPriv=(ViaXvMCSAreaPriv*) DRIGetSAREAPrivate(pScrn->pScreen);

	{
	    DRM_CAS_RESULT(__ret);

	    DRM_CAS(&(sAPriv->XvMCSubPicOn[vx->xvmc_port]),
		    i|VIA_XVMC_VALID,0,__ret);
	    if (!__ret) {

		/*
		 * Turn subpicture off.
		 */ 

		while (VIDInD(V_COMPOSE_MODE) & 
		       (V1_COMMAND_FIRE | V3_COMMAND_FIRE));
		VIDOutD(SUBP_CONTROL_STRIDE, VIDInD(SUBP_CONTROL_STRIDE) & 
			~SUBP_HQV_ENABLE);

	    }
	}
	VIAFreeLinear(&(vXvMC->sPrivs[i]->memory_ref));
	xfree(vXvMC->sPrivs[i]);
	vXvMC->nSurfaces--;
	vXvMC->sPrivs[i] = 0;
	vXvMC->surfaces[i] = 0;

      return;
    }
  }
  return;
}

/*
 * Tell XvMC clients that Xv is currently displaying.
 * return 1 if another client was displaying before.
 */ 

static int viaXvMCSetDisplayLock(ScrnInfoPtr pScrn, ViaXvMCXVPriv *vx)
{
    VIAPtr pVia = VIAPTR(pScrn);
    ViaXvMCSAreaPriv *sAPriv =(ViaXvMCSAreaPriv*) 
	DRIGetSAREAPrivate(pScrn->pScreen);

    if (vx->xvmc_port > 0) {
	if ((VIA_XVMC_MAX_SURFACES | VIA_XVMC_VALID) !=     	
	    sAPriv->XvMCDisplaying[vx->xvmc_port]) {
	    sAPriv->XvMCDisplaying[vx->xvmc_port] = 
		(VIA_XVMC_MAX_SURFACES | VIA_XVMC_VALID);
	}

	if (sAPriv->XvMCSubPicOn[vx->xvmc_port] & VIA_XVMC_VALID) {
	    sAPriv->XvMCSubPicOn[vx->xvmc_port] = 0;
	    
	    while (VIDInD(V_COMPOSE_MODE) & 
		   (V1_COMMAND_FIRE | V3_COMMAND_FIRE));
	    VIDOutD(SUBP_CONTROL_STRIDE, VIDInD(SUBP_CONTROL_STRIDE) & 
		    ~SUBP_HQV_ENABLE);
	    
	}
    }

    return 0;
}

int viaXvMCInitXv(ScrnInfoPtr pScrn, pointer data) 
{
    viaPortPrivPtr pPriv = (viaPortPrivPtr) data;
    ViaXvMCXVPriv *vx;
    unsigned i;
	

    if (NULL == (pPriv->xvmc_priv = xcalloc(1,sizeof(ViaXvMCXVPriv)))) {
	return BadAlloc;
    }

    for (i=0; i < VIA_NUM_XVMC_ATTRIBUTES; ++i) {
      attrAtoms[i] = MAKE_ATOM(attrXvMC[i]);
    }

    vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;

    vx->ctxDisplaying = 0;
    vx->xvAttr.numAttr = VIA_NUM_XVMC_ATTRIBUTES;
    vx->xvmc_port = -1;
    vx->newAttribute = 1;

    for (i = 0; i < VIA_NUM_XVMC_ATTRIBUTES; ++i) {
	vx->xvAttr.attributes[i].attribute = attrAtoms[i];
	vx->xvAttr.attributes[i].value = 0;
	viaGetPortAttributeG( pScrn, attrAtoms[i] , 
			      &(vx->xvAttr.attributes[i].value), data);
    }   
    return Success;
}

int viaXvMCInterceptXvAttribute(ScrnInfoPtr pScrn, Atom attribute, 
				INT32 value,pointer data) 
{
    unsigned i;

    viaPortPrivPtr pPriv = (viaPortPrivPtr) data;
    ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;
    VIAPtr  pVia = VIAPTR(pScrn); 
    if (pVia->XvMCEnabled) {
	for (i=0; i< vx->xvAttr.numAttr; ++i) {
	    if (vx->xvAttr.attributes[i].attribute == attribute) {
		vx->xvAttr.attributes[i].value = value;
		if (vx->ctxDisplaying != 
		    (VIA_XVMC_MAX_CONTEXTS | VIA_XVMC_VALID)) {
		    vx->newAttribute = 1;
		    return 0;
		}
	    }
	}
    }
    return viaSetPortAttributeG( pScrn, attribute, value, data);
}

int viaXvMCInterceptXvGetAttribute(ScrnInfoPtr pScrn, Atom attribute, 
				   INT32 *value,pointer data) 
{
    unsigned i;

    viaPortPrivPtr pPriv = (viaPortPrivPtr) data;
    ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;
    VIAPtr  pVia = VIAPTR(pScrn); 
    if (pVia->XvMCEnabled) {    
	for (i=0; i< vx->xvAttr.numAttr; ++i) {
	    if (vx->xvAttr.attributes[i].attribute == attribute) {
		*value = vx->xvAttr.attributes[i].value;
		return Success;
	    }
	}
    }
    return viaGetPortAttributeG( pScrn, attribute, value, data);
}

    
static int viaXvMCDisplayAttributes(ScrnInfoPtr pScrn, 
				    const ViaXvMCAttrHolder *ah, pointer data) 
{

    unsigned i;
    int ret;

    for (i=0; i< ah->numAttr; ++i) {
	ret = viaSetPortAttributeG( pScrn, ah->attributes[i].attribute, 
				    ah->attributes[i].value, data);
	if (ret) return ret;
    }
    return Success;
}
   

int viaXvMCInterceptPutImage( ScrnInfoPtr pScrn,short src_x, short src_y,
			      short drw_x, short drw_y, short src_w, 
			      short src_h,short drw_w, short drw_h, 
			      int id, unsigned char* buf, short width, 
			      short height, Bool sync, RegionPtr clipBoxes, 
			      pointer data)
{ 
    ViaXvMCCommandBuffer *vXvMCData; 
    viaPortPrivPtr pPriv = (viaPortPrivPtr) data;
    ViaXvMCXVPriv *vx = (ViaXvMCXVPriv *) pPriv->xvmc_priv;
    VIAPtr  pVia = VIAPTR(pScrn); 
    if (pVia->XvMCEnabled) {
	if (FOURCC_VIA == id) {
	    volatile ViaXvMCSAreaPriv *sAPriv;
	    vXvMCData = (ViaXvMCCommandBuffer *) buf;
	    sAPriv=(ViaXvMCSAreaPriv*) DRIGetSAREAPrivate(pScrn->pScreen);
	    switch(vXvMCData->command) {
	    case VIA_XVMC_COMMAND_ATTRIBUTES:
	        if ((vXvMCData->ctxNo | VIA_XVMC_VALID) != vx->ctxDisplaying) {
		    return 1;
		} else {
		    viaXvMCDisplayAttributes( pScrn, &vXvMCData->attrib, data);
		    return 0;
		}
		break;
	    case VIA_XVMC_COMMAND_FDISPLAY:
		if (sAPriv->XvMCDisplaying[vx->xvmc_port] != vXvMCData->srfNo) return 1;
		viaXvMCDisplayAttributes( pScrn, &vXvMCData->attrib, data);
		vx->ctxDisplaying = vXvMCData->ctxNo; 
		break;
	    case VIA_XVMC_COMMAND_DISPLAY:
		if ((vXvMCData->ctxNo | VIA_XVMC_VALID) != vx->ctxDisplaying) {
		    viaXvMCDisplayAttributes( pScrn, &vXvMCData->attrib, data);
		}
		if (sAPriv->XvMCDisplaying[vx->xvmc_port] != vXvMCData->srfNo) return 1;
		vx->ctxDisplaying = vXvMCData->ctxNo;
		break;
	    case VIA_XVMC_COMMAND_UNDISPLAY:
	    {
		DRM_CAS_RESULT(__ret);		
		DRM_CAS(&(sAPriv->XvMCDisplaying[vx->xvmc_port]), 
			vXvMCData->srfNo, 0,__ret);
		if (!__ret) 
		    viaStopSWOVerlay(pScrn);
	    }
	    return Success;
	    break;
	    default:
		break;
	    }		
	} else {
	    if ((VIA_XVMC_MAX_CONTEXTS | VIA_XVMC_VALID) != vx->ctxDisplaying) {
		viaXvMCDisplayAttributes( pScrn, &vx->xvAttr, data);
		vx->ctxDisplaying = VIA_XVMC_MAX_CONTEXTS | VIA_XVMC_VALID;
	    } else if (vx->newAttribute) {
		vx->newAttribute = 0;
		viaXvMCDisplayAttributes( pScrn, &vx->xvAttr, data);
	    }	    
	    viaXvMCSetDisplayLock( pScrn, vx );
	}
    }
    return viaPutImageG( pScrn, src_x, src_y, drw_x, drw_y, src_w, src_h,
			 drw_w, drw_h, id, buf, width, height, sync,
			 clipBoxes, data);
}
		    			
unsigned long viaXvMCPutImageSize(ScrnInfoPtr pScrn)
{
    return sizeof(ViaXvMCCommandBuffer);
}

#endif
