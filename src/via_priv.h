/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_priv.h,v 1.4 2003/12/17 18:57:18 dawes Exp $ */

#ifndef _VIA_PRIV_H_
#define _VIA_PRIV_H_ 1

#ifdef XF86DRI
#include "via_common.h"
#endif
#include "via_capture.h"

/*
 * Alignment macro functions
 */
#define ALIGN_TO_32_BYTES(f)         (((f) + 31) & ~31)
#define ALIGN_TO_16_BYTES(f)         (((f) + 15) & ~15)
#define ALIGN_TO_256_BITS(f)         (((f) + 255) & ~255)
#define ALIGN_TO_8_BYTES(f)          (((f) + 7) & ~7)
#define ALIGN_TO_64_BITS(f)          (((f) + 63) & ~63)
#define ENG_ALIGN_BYTE              ALIGN_TO_32_BYTES
#define ENG_ALIGN_BIT               ALIGN_TO_256_BITS

/*
 * FOURCC definitions
 */

#define FOURCC_VIA     0x4E4B4C57  /*VIA*/
#define FOURCC_TV0     0x00325654  /*TV0*/
#define FOURCC_TV1     0x00315654  /*TV1*/

/*
 * Structures for create surface
 */
typedef struct _DDSURFACEDESC
{
    unsigned long     dwSize;      /* size of the DDSURFACEDESC structure*/
    unsigned long     dwFlags;     /* determines what fields are valid*/
    unsigned long     dwHeight;    /* height of surface to be created*/
    unsigned long     dwWidth;     /* width of input surface*/
    unsigned long      lPitch;      /* distance to start of next line(return value)*/
    unsigned long     dwBackBufferCount;     /* number of back buffers requested*/
    void *    lpSurface;             /* pointer to the surface memory*/
    unsigned long     dwColorSpaceLowValue;  /* low boundary of color space that is to*/
                                     /* be treated as Color Key, inclusive*/
    unsigned long     dwColorSpaceHighValue; /* high boundary of color space that is*/
                                     /* to be treated as Color Key, inclusive*/
    unsigned long     dwFourCC;              /* (FOURCC code)*/
} DDSURFACEDESC;
typedef DDSURFACEDESC * LPDDSURFACEDESC;

typedef struct _DDPIXELFORMAT
{
	unsigned long	dwSize;			/* size of structure */
	unsigned long	dwFlags;		/* pixel format flags */
	unsigned long	dwFourCC;		/* (FOURCC code) */

	unsigned long	dwRGBBitCount;		/* how many bits per pixel */
	unsigned long	dwYUVBitCount;		/* how many bits per pixel */
	unsigned long	dwZBufferBitDepth;	/* how many bits for z buffers */
	unsigned long	dwAlphaBitDepth;	/* how many bits for alpha channels */

	unsigned long	dwRBitMask;		/* mask for red bit */
	unsigned long	dwYBitMask;		/* mask for Y bits */

	unsigned long	dwGBitMask;		/* mask for green bits */
	unsigned long	dwUBitMask;		/* mask for U bits */

	unsigned long	dwBBitMask;		/* mask for blue bits */
	unsigned long	dwVBitMask;		/* mask for V bits */

	unsigned long	dwRGBAlphaBitMask;	/* mask for alpha channel */
	unsigned long	dwYUVAlphaBitMask;	/* mask for alpha channel */
	unsigned long	dwRGBZBitMask;		/* mask for Z channel */
	unsigned long	dwYUVZBitMask;		/* mask for Z channel */
} DDPIXELFORMAT;
typedef DDPIXELFORMAT * LPDDPIXELFORMAT;


typedef struct _SWDEVICE
{
 unsigned char * lpSWOverlaySurface[2];   /* Max 2 Pointers to SW Overlay Surface*/
 unsigned long  dwSWPhysicalAddr[2];     /*Max 2 Physical address to SW Overlay Surface */
 unsigned long  dwSWCbPhysicalAddr[2];  /* Physical address to SW Cb Overlay Surface, for YV12 format use */
 unsigned long  dwSWCrPhysicalAddr[2];  /* Physical address to SW Cr Overlay Surface, for YV12 format use */
 unsigned long  dwHQVAddr[3];             /* Physical address to HQV surface -- CLE_C0   */
 /*unsigned long  dwHQVAddr[2];*/			  /*Max 2 Physical address to SW HQV Overlay Surface*/
 unsigned long  dwWidth;                  /*SW Source Width, not changed*/
 unsigned long  dwHeight;                 /*SW Source Height, not changed*/
 unsigned long  dwPitch;                  /*SW frame buffer pitch*/
 unsigned long  gdwSWSrcWidth;           /*SW Source Width, changed if window is out of screen*/
 unsigned long  gdwSWSrcHeight;          /*SW Source Height, changed if window is out of screen*/
 unsigned long  gdwSWDstWidth;           /*SW Destination Width*/
 unsigned long  gdwSWDstHeight;          /*SW Destination Height*/
 unsigned long  gdwSWDstLeft;            /*SW Position : Left*/
 unsigned long  gdwSWDstTop;             /*SW Position : Top*/
 unsigned long  dwDeinterlaceMode;        /*BOB / WEAVE*/
}SWDEVICE;
typedef SWDEVICE * LPSWDEVICE;


/*
 * Structures for LOCK surface
 */

typedef struct _DDLOCK
{
    unsigned long     dwVersion;             
    unsigned long     dwFourCC;
    unsigned long     dwPhysicalBase;
    CAPDEVICE Capdev_TV0;
    CAPDEVICE Capdev_TV1;
    SWDEVICE SWDevice;
} DDLOCK;
typedef DDLOCK * LPDDLOCK;

typedef struct _RECTL
{
    unsigned long     left;
    unsigned long     top;
    unsigned long     right;
    unsigned long     bottom;
} RECTL;

typedef struct _DDUPDATEOVERLAY
{
    RECTL     rDest;          /* dest rect */
    RECTL     rSrc;           /* src rect */
    unsigned long     dwFlags;        /* flags */
    unsigned long     dwColorSpaceLowValue;
    unsigned long     dwColorSpaceHighValue;
    unsigned long     dwFourcc;
} DDUPDATEOVERLAY;
typedef DDUPDATEOVERLAY * LPDDUPDATEOVERLAY;

typedef struct _ADJUSTFRAME
{
    int     x;
    int     y;
} ADJUSTFRAME;
typedef ADJUSTFRAME * LPADJUSTFRAME;

/* Definition for dwFlags */
#define DDOVER_HIDE       0x00000001
#define DDOVER_SHOW       0x00000002
#define DDOVER_KEYDEST    0x00000004
#define DDOVER_ENABLE     0x00000008
#define DDOVER_CLIP       0x00000010
#define DDOVER_INTERLEAVED			0x00800000l
#define DDOVER_BOB                       	0x00200000l

#define FOURCC_HQVSW   0x34565148  /*HQV4*/
#define DDPF_FOURCC				0x00000004l



typedef struct
{
    CARD32         dwWidth;
    CARD32         dwHeight;
    CARD32         dwOffset;
    CARD32         dwUVoffset;
    CARD32         dwFlipTime;
    CARD32         dwFlipTag;
    CARD32         dwStartAddr;
    CARD32         dwV1OriWidth;
    CARD32         dwV1OriHeight;
    CARD32         dwV1OriPitch;
    CARD32         dwV1SrcWidth;
    CARD32         dwV1SrcHeight;
    CARD32         dwV1SrcLeft;
    CARD32         dwV1SrcRight;
    CARD32         dwV1SrcTop;
    CARD32         dwV1SrcBot;
    CARD32         dwSPWidth;
    CARD32         dwSPHeight;
    CARD32         dwSPLeft;
    CARD32         dwSPRight;
    CARD32         dwSPTop;
    CARD32         dwSPBot;
    CARD32         dwSPOffset;
    CARD32         dwSPstartAddr;
    CARD32         dwDisplayPictStruct;
    CARD32         dwDisplayBuffIndex;        /* Display buffer Index. 0 to ( dwBufferNumber -1) */
    CARD32         dwFetchAlignment;
    CARD32         dwSPPitch;
    unsigned long  dwHQVAddr[3];          /* CLE_C0 */
    /*unsigned long   dwHQVAddr[2];*/
    CARD32         dwMPEGDeinterlaceMode; /* default value : VIA_DEINTERLACE_WEAVE */
    CARD32         dwMPEGProgressiveMode; /* default value : VIA_PROGRESSIVE */
    CARD32         dwHQVheapInfo;         /* video memory heap of the HQV buffer */
    CARD32         dwVideoControl;        /* video control flag */
    CARD32         dwminifyH; 			   /* Horizontal minify factor */
    CARD32         dwminifyV;			   /* Vertical minify factor */
    CARD32         dwMpegDecoded;
} OVERLAYRECORD;

/****************************************************************************
 *
 * PIXELFORMAT FLAGS
 *
 ****************************************************************************/

/*
 * The FourCC code is valid.
 */
#define DDPF_FOURCC				0x00000004l

/*
 * The RGB data in the pixel format structure is valid.
 */
#define DDPF_RGB				0x00000040l



/*
 * Return value of Proprietary Interface
 */

#define PI_OK                              0x00
#define PI_ERR                             0x01
#define PI_ERR_NO_X_WINDOW                 PI_ERR +1
#define PI_ERR_CANNOT_OPEN_VIDEO_DEVICE    PI_ERR +2
#define PI_ERR_CANNOT_USE_IOCTL            PI_ERR +3
#define PI_ERR_CANNOT_CREATE_SURFACE       PI_ERR +4

#define MEM_BLOCKS		4

typedef struct {
    unsigned long   base;		/* Offset into fb */
    int    pool;			/* Pool we drew from */
#ifdef XF86DRI
    int    drm_fd;			/* Fd in DRM mode */
    drmViaMem drm;			/* DRM management object */
#endif
    int    slot;			/* Pool 3 slot */
    void  *pVia;			/* VIA driver pointer */
    FBLinearPtr linear;			/* X linear pool info ptr */
} VIAMem;

typedef VIAMem *VIAMemPtr;



typedef struct  {
    unsigned long   gdwVideoFlagTV1;
    unsigned long   gdwVideoFlagSW;
    unsigned long   gdwVideoFlagMPEG;
    unsigned long   gdwAlphaEnabled;		/* For Alpha blending use*/

    VIAMem SWOVMem;
    VIAMem HQVMem;
    VIAMem SWfbMem;

    DDPIXELFORMAT DPFsrc; 
    DDUPDATEOVERLAY UpdateOverlayBackup;    /* For HQVcontrol func use
					    // To save MPEG updateoverlay info.*/

/* device struct */
    SWDEVICE   SWDevice;
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
