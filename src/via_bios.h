/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_bios.h,v 1.4 2003/12/31 05:42:04 dawes Exp $ */
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

#ifndef _VIA_BIOS_H_
#define _VIA_BIOS_H_ 1

#define     VIA_RES_640X480                 0
#define     VIA_RES_800X600                 1
#define     VIA_RES_1024X768                2
#define     VIA_RES_1152X864                3
#define     VIA_RES_1280X1024               4
#define     VIA_RES_1600X1200               5
#define     VIA_RES_1440X1050               6
#define     VIA_RES_1280X768                7
#define     VIA_RES_1280X960                8
#define     VIA_RES_1920X1440               9
#define     VIA_RES_848X480                 10
#define     VIA_RES_1400X1050               11
#define     VIA_RES_720X480                 12
#define     VIA_RES_720X576                 13
#define     VIA_RES_1024X512                14
#define     VIA_RES_856X480                 15
#define     VIA_RES_1024X576		    16
#define     VIA_RES_INVALID                 255

#define     VIA_TVRES_640X480               0
#define     VIA_TVRES_800X600               1
#define     VIA_TVRES_1024X768              2
#define     VIA_TVRES_848X480               3
#define     VIA_TVRES_720X480               4
#define     VIA_TVRES_720X576               5
#define     VIA_TVRES_INVALID               255

#define     VIA_NUM_REFRESH_RATE            5
#define     VIA_PANEL6X4                    0
#define     VIA_PANEL8X6                    1
#define     VIA_PANEL10X7                   2
#define     VIA_PANEL12X7                   3
#define     VIA_PANEL12X10                  4
#define     VIA_PANEL14X10                  5
#define     VIA_PANEL16X12                  6
#define     VIA_PANEL_INVALID               255

#define     VIA_NONEDVI                     0x00
#define     VIA_VT3191                      0x01
#define     VIA_TTLTYPE                     0x07
#define     VIA_VT3192                      0x09
#define     VIA_VT3193                      0x0A
#define     VIA_SIL164                      0x0B
#define     VIA_SIL168                      0x0C
#define     VIA_Hardwired                   0x0F

#define     TVTYPE_NONE                     0x00
#define     TVTYPE_NTSC                     0x01
#define     TVTYPE_PAL                      0x02

#define     TVOUTPUT_NONE                   0x00
#define     TVOUTPUT_COMPOSITE              0x01
#define     TVOUTPUT_SVIDEO                 0x02
#define     TVOUTPUT_RGB                    0x04
#define     TVOUTPUT_YCBCR                  0x08
#define     TVOUTPUT_SC                     0x16

#define     VIA_NONETV                      0
#define     VIA_VT1621                      1 /* TV2PLUS */
#define     VIA_VT1622                      2 /* TV3 */
#define     VIA_VT1622A                     7
#define     VIA_VT1623                      9

#define     VIA_TVNORMAL                    0
#define     VIA_TVOVER                      1
#define     VIA_NO_TVHSCALE                 0
#define     VIA_TVHSCALE0                   0
#define     VIA_TVHSCALE1                   1
#define     VIA_TVHSCALE2                   2
#define     VIA_TVHSCALE3                   3
#define     VIA_TVHSCALE4                   4
#define     VIA_TV_NUM_HSCALE_LEVEL         8
#define     VIA_TV_NUM_HSCALE_REG           16

#define     VIA_DEVICE_NONE                 0x00
#define	    VIA_DEVICE_CRT1		    0x01
#define	    VIA_DEVICE_LCD		    0x02
#define	    VIA_DEVICE_TV		    0x04
#define	    VIA_DEVICE_DFP		    0x08
#define	    VIA_DEVICE_CRT2		    0x10
#define     VIA_DEVICE_CRT                  VIA_DEVICE_CRT1  

/* System Memory CLK */
#define	    VIA_MEM_SDR66		    0x00
#define	    VIA_MEM_SDR100		    0x01
#define	    VIA_MEM_SDR133		    0x02
#define	    VIA_MEM_DDR200		    0x03
#define	    VIA_MEM_DDR266		    0x04
#define	    VIA_MEM_DDR333		    0x05
#define	    VIA_MEM_DDR400		    0x06

/* Digital Output Bus Width */
#define	    VIA_DI_12BIT		    0x00
#define	    VIA_DI_24BIT		    0x01

#define     CAP_WEAVE                       0x0
#define     CAP_BOB                         0x1

typedef struct _VIABIOSINFO {
    int         scrnIndex;

    CARD16      ModeIndex;
    CARD16      ResolutionIndex;
    CARD16      RefreshIndex;

    CARD8	ConnectedDevice;
    CARD8	ActiveDevice;
    CARD8	DefaultActiveDevice;
    
    /* Panel/LCD entries */
    CARD8	TMDS;
    CARD8	LVDS;
    Bool	LCDDualEdge;	/* mean add-on card is 2CH or Dual or DDR */
    Bool        DVIAttach;
    Bool        LCDAttach;
    Bool        Center;
    CARD8	BusWidth;		/* Digital Output Bus Width */
    int         PanelSize;
    int         PanelIndex;
    Bool        ForcePanel;
    Bool        SetDVI;
    /* LCD Simultaneous Expand Mode HWCursor Y Scale */
    Bool        scaleY;
    int         panelX;
    int         panelY;
    int         resY;

    /* TV entries */
    int         TVEncoder;
    int         TVType;
    Bool        TVDotCrawl;
    int         TVOutput;
    int         TVVScan;
    int         TVHScale;
    CARD16	TVIndex;
    CARD8	TVI2CAddr;
    Bool        TVUseGpioI2c;
    I2CDevPtr   TVI2CDev;
    int         TVDeflicker;

} VIABIOSInfoRec, *VIABIOSInfoPtr;

/* Function prototypes */
/* via_bios.c */
CARD8 ViaVBEGetActiveDevice(ScrnInfoPtr pScrn);
CARD16 ViaVBEGetDisplayDeviceInfo(ScrnInfoPtr pScrn, CARD8 *numDevice);
void ViaVBEPrintBIOSVersion(ScrnInfoPtr pScrn);
void ViaVBEPrintBIOSDate(ScrnInfoPtr pScrn);
#ifdef HAVE_DEBUG
void ViaDumpVGAROM(ScrnInfoPtr pScrn);
#endif /* HAVE_DEBUG */

/* via_mode.c */
void VIATVDetect(ScrnInfoPtr pScrn);
CARD8 VIAGetActiveDisplay(ScrnInfoPtr pScrn);
Bool VIAPostDVI(ScrnInfoPtr pScrn);
CARD8 VIAGetDeviceDetect(ScrnInfoPtr pScrn);
Bool VIAFindModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode, Bool Final);
void VIASetModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode);
void VIASetModeForMHS(ScrnInfoPtr pScrn, DisplayModePtr mode);
void VIALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies, 
		    LOCO *colors, VisualPtr pVisual);
void VIAEnableLCD(ScrnInfoPtr pScrn);
void VIADisableLCD(ScrnInfoPtr pScrn);

/* in via_bandwidth.c */
void ViaSetPrimaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaSetSecondaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaDisablePrimaryFIFO(ScrnInfoPtr pScrn);

#endif /* _VIA_BIOS_H_ */
