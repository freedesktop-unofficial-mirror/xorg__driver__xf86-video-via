/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_priv.h,v 1.3 2003/08/27 15:16:12 tsi Exp $ */

#ifndef _VIA_PRIV_H_
#define _VIA_PRIV_H_ 1

#include "ddmpeg.h"
#include "via_common.h"

typedef struct  {
    unsigned long   gdwVideoFlagTV1;
    unsigned long   gdwVideoFlagSW;
    unsigned long   gdwVideoFlagMPEG;
    unsigned long   gdwAlphaEnabled;		/* For Alpha blending use*/

#if 0    
/* memory management */
    ViaMMReq  SWMemRequest;
    ViaMMReq  HQVMemRequest;
/*ViaMMReq  MPEGMemRequest;
  ViaMMReq  SUBPMemRequest;*/
#endif
    
/* for DRM memory management */
#ifdef XF86DRI
    drmViaMem MPEGfbRequest;
    drmViaMem SUBPfbRequest;
    drmViaMem HQVfbRequest;
    drmViaMem TV0fbRequest;
    drmViaMem TV1fbRequest;
    drmViaMem ALPHAfbRequest;
    drmViaMem SWfbRequest;
    drmViaMem drm_SWOV_fb;
    drmViaMem drm_HQV_fb;
    int  drm_SWOV_fd;
    int  drm_HQV_fd;
#endif

    DDPIXELFORMAT DPFsrc; 
    DDUPDATEOVERLAY UpdateOverlayBackup;    /* For HQVcontrol func use
					    // To save MPEG updateoverlay info.*/

/* device struct */
    SWDEVICE   SWDevice;
    SUBDEVICE   SUBDevice;
    MPGDEVICE   MPGDevice;
    OVERLAYRECORD   overlayRecordV1;
    OVERLAYRECORD   overlayRecordV3;

    BoxRec  AvailFBArea;
    FBLinearPtr   SWOVlinear;

    Bool MPEG_ON;
    Bool SWVideo_ON;

/*To solve the bandwidth issue */
    unsigned long   gdwUseExtendedFIFO;

/* For panning mode use */
    int panning_old_x;
    int panning_old_y;
    int panning_x;
    int panning_y;

/*To solve the bandwidth issue */
    unsigned char Save_3C4_16;
    unsigned char Save_3C4_17;
    unsigned char Save_3C4_18;

} swovRec, *swovPtr;

#endif /* _VIA_PRIV_H_ */
