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

#define     VIA_CRT_SUPPORT                 TRUE
#define     VIA_LCD_SUPPORT                 TRUE
#define     VIA_UNCOVERD_LCD_PANEL          FALSE
#define     VIA_NTSC_SUPPORT                TRUE
#define     VIA_PAL_SUPPORT                 TRUE
#define     VIA_DVI_SUPPORT                 TRUE

#define     VIA_CRT_SUPPORT_BIT             0x01
#define     VIA_LCD_SUPPORT_BIT             0x02
#define     VIA_NTSC_SUPPORT_BIT            0x04
#define     VIA_PAL_SUPPORT_BIT             0x08
#define     VIA_DVI_SUPPORT_BIT		    0x20

#define     VIA_BIOS_REG_TABLE_MAX_NUM      32
#define     VIA_BIOS_REG_LCD_MAX_NUM        48
#define     VIA_BIOS_NUM_RES                17
#define     VIA_BIOS_NUM_REFRESH            5
#define     VIA_BIOS_NUM_LCD_SUPPORT_MASK   8
#define     VIA_BIOS_NUM_LCD_POWER_SEQ      4
#define     VIA_BIOS_NUM_PANEL              7
#define     VIA_BIOS_MAX_NUM_MPATCH2        18
#define     VIA_BIOS_MAX_NUM_MPATCH1        9
#define     VIA_BIOS_MAX_NUM_CTREXP         5
#define     VIA_BIOS_MAX_NUM_TV_REG         144		/* 00 - 8F, tv2,tv3 are the same */
#define     VIA_BIOS_MAX_NUM_TV_CRTC        32
#define     VIA_BIOS_NUM_TV_SPECIAL_REG     8
#define     VIA_BIOS_MAX_NUM_TV_PATCH       8
#define     VIA_BIOS_NUM_TV_OTHER           16
#define     VIA_BIOS_NUM_TV2                2
#define     VIA_BIOS_NUM_TV3                6

/* The position of some BIOS information from start of BIOS */
#define     VIA_BIOS_SIZE_POS               0x2
#define     VIA_BIOS_OFFSET_POS             0x1B

/* The offset of table from table start */
#define     VIA_BIOS_CSTAB_POS              6
#define     VIA_BIOS_STDVGAREGTAB_POS       8
#define     VIA_BIOS_COMMEXTREGTAB_POS      10
#define     VIA_BIOS_STDMODEXTREGTAB_POS    12
#define     VIA_BIOS_EXTMODEREGTAB_POS      14
#define     VIA_BIOS_REFRESHMODETAB_POS     16
#define     VIA_BIOS_BIOSVER_POS            18
#define     VIA_BIOS_BCPPOST_POS            20
#define     VIA_BIOS_LCDMODETAB_POS         40
#define     VIA_BIOS_TVMODETAB_POS          42
#define     VIA_BIOS_TVMASKTAB_POS          44
#define     VIA_BIOS_MODEOVERTAB_POS        46
#define     VIA_BIOS_TV3HSCALETAB_POS       48
#define     VIA_BIOS_LCDPOWERON_POS         50
#define     VIA_BIOS_LCDPOWEROFF_POS        52
#define     VIA_BIOS_LCDMODEFIX_POS         54
#define     ZCR                             0x0
#define     ZSR                             0x1
#define     ZGR                             0x2

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
#define     VIA_TV2PLUS                     1
#define     VIA_TV3                         2
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

typedef struct _VIABIOSSTDVGATABLE {
    CARD8           columns;
    CARD8           rows;
    CARD8           fontHeight;
    CARD16          pageSize;
    CARD8           SR[5];
    CARD8           misc;
    CARD8           CR[25];
    CARD8           AR[20];
    CARD8           GR[9];
} VIABIOSStdVGATableRec, *VIABIOSStdVGATablePtr;


typedef struct _VIABIOSREFRESHTABLE {
    CARD8           refresh;
    CARD16          VClk;
    CARD8           CR[14];
} VIABIOSRefreshTableRec, *VIABIOSRefreshTablePtr;


typedef struct _VIABIOSREGTABLE {
    CARD8   port[VIA_BIOS_REG_TABLE_MAX_NUM];
    CARD8   offset[VIA_BIOS_REG_TABLE_MAX_NUM];
    CARD8   mask[VIA_BIOS_REG_TABLE_MAX_NUM];
    CARD8   data[VIA_BIOS_REG_TABLE_MAX_NUM];
    int     numEntry;
} VIABIOSRegTableRec, *VIABIOSRegTablePtr;


typedef struct _VIAVMODEENTRY {
    unsigned short          Width;
    unsigned short          Height;
    unsigned short          Bpp;
    unsigned short          Mode;
    unsigned short          MemNeed;
    unsigned short          MClk;
    unsigned short          VClk;
    VIABIOSStdVGATableRec   stdVgaTable;
    VIABIOSRegTableRec      extModeExtTable;
} VIAModeEntry, *VIAModeEntryPtr;


typedef struct _VIALCDMODEENTRY {
    CARD16  LCDClk;
    CARD16  VClk;
    CARD16  LCDClk_12Bit;
    CARD16  VClk_12Bit;
    CARD8   port[VIA_BIOS_REG_LCD_MAX_NUM];
    CARD8   offset[VIA_BIOS_REG_LCD_MAX_NUM];
    CARD8   data[VIA_BIOS_REG_LCD_MAX_NUM];
    int     numEntry;
} VIALCDModeEntry, *VIALCDModeEntryPtr;


typedef struct _VIALCDMPATCHENTRY {
    CARD8   Mode;
    CARD16  LCDClk;
    CARD16  VClk;
    CARD16  LCDClk_12Bit;
    CARD16  VClk_12Bit;
    CARD8   port[VIA_BIOS_REG_LCD_MAX_NUM];
    CARD8   offset[VIA_BIOS_REG_LCD_MAX_NUM];
    CARD8   data[VIA_BIOS_REG_LCD_MAX_NUM];
    int     numEntry;
} VIALCDMPatchEntry, *VIALCDMPatchEntryPtr;


typedef struct _VIALCDMODEFIX {
    CARD8   reqMode[32];
    CARD8   fixMode[32];
    int     numEntry;
} VIALCDModeFixRec, *VIALCDModeFixRecPtr;


typedef struct _VIALCDPOWERSEQUENCE {
    CARD8   powerSeq;
    CARD8   port[4];
    CARD8   offset[4];
    CARD8   mask[4];
    CARD8   data[4];
    CARD16  delay[4];
    int     numEntry;
} VIALCDPowerSeqRec, *VIALCDPowerSeqRecPtr;


typedef struct _VIALCDMODETABLE {
    CARD8                   fpIndex;
    CARD8                   fpSize;
    CARD8                   powerSeq;
    int                     numMPatchDP2Ctr;
    int                     numMPatchDP2Exp;
    int                     numMPatchDP1Ctr;
    int                     numMPatchDP1Exp;
    CARD16                  SuptMode[VIA_BIOS_NUM_LCD_SUPPORT_MASK];
    VIALCDModeEntry         FPconfigTb;
    VIALCDModeEntry         InitTb;
    VIALCDMPatchEntry       MPatchDP2Ctr[VIA_BIOS_MAX_NUM_MPATCH2];
    VIALCDMPatchEntry       MPatchDP2Exp[VIA_BIOS_MAX_NUM_MPATCH2];
    VIALCDMPatchEntry       MPatchDP1Ctr[VIA_BIOS_MAX_NUM_MPATCH1];
    VIALCDMPatchEntry       MPatchDP1Exp[VIA_BIOS_MAX_NUM_MPATCH1];
    VIALCDModeEntry         LowResCtr;
    VIALCDModeEntry         LowResExp;
    VIALCDModeEntry         MCtr[VIA_BIOS_MAX_NUM_CTREXP];
    VIALCDModeEntry         MExp[VIA_BIOS_MAX_NUM_CTREXP];
} VIALCDModeTableRec, *VIALCDModePtr;


typedef struct _VIATVMASKTABLE {
    CARD8   TV[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   CRTC1[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTC2[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   misc1;
    CARD8   misc2;
    int     numTV;
    int     numCRTC1;
    int     numCRTC2;
} VIABIOSTVMASKTableRec, *VIABIOSTVMASKTablePtr;

typedef struct _VIATVMODETABLE {
    CARD8   TVNTSCC[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   TVNTSCS[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   CRTCNTSC1[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   MiscNTSC1[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   MiscNTSC2[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   CRTCNTSC2_8BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCNTSC2_16BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCNTSC2_32BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD16  PatchNTSC2[VIA_BIOS_MAX_NUM_TV_PATCH];
    CARD16  DotCrawlNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD8   TVPALC[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   TVPALS[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   CRTCPAL1[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   MiscPAL1[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   MiscPAL2[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   CRTCPAL2_8BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCPAL2_16BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCPAL2_32BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD16  PatchPAL2[VIA_BIOS_MAX_NUM_TV_PATCH];
} VIABIOSTV2TableRec, *VIABIOSTV2TablePtr;


typedef struct _VIATV3MODETABLE {
    CARD8   TVNTSC[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   CRTCNTSC1[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   MiscNTSC1[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   MiscNTSC2[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   CRTCNTSC2_8BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCNTSC2_16BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCNTSC2_32BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD16  PatchNTSC2[VIA_BIOS_MAX_NUM_TV_PATCH];
    CARD16  RGBNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD16  YCbCrNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD16  SDTV_RGBNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD16  SDTV_YCbCrNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD16  DotCrawlNTSC[VIA_BIOS_NUM_TV_OTHER];
    CARD8   TVPAL[VIA_BIOS_MAX_NUM_TV_REG];
    CARD8   CRTCPAL1[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   MiscPAL1[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   MiscPAL2[VIA_BIOS_NUM_TV_SPECIAL_REG];
    CARD8   CRTCPAL2_8BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCPAL2_16BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD8   CRTCPAL2_32BPP[VIA_BIOS_MAX_NUM_TV_CRTC];
    CARD16  PatchPAL2[VIA_BIOS_MAX_NUM_TV_PATCH];
    CARD16  RGBPAL[VIA_BIOS_NUM_TV_OTHER];
    CARD16  YCbCrPAL[VIA_BIOS_NUM_TV_OTHER];
    CARD16  SDTV_RGBPAL[VIA_BIOS_NUM_TV_OTHER];
    CARD16  SDTV_YCbCrPAL[VIA_BIOS_NUM_TV_OTHER];
} VIABIOSTV3TableRec, *VIABIOSTV3TablePtr;

typedef struct _VIAVMODETABLE {
    unsigned short          BIOSVer;
    char                    BIOSDate[9];
    unsigned short          NumModes;
    int                     NumPowerOn;
    int                     NumPowerOff;
    VIAModeEntryPtr         Modes;
    VIABIOSRegTableRec      commExtTable;
    VIABIOSRegTableRec      stdModeExtTable;
    VIABIOSRefreshTableRec  refreshTable[VIA_BIOS_NUM_RES][VIA_BIOS_NUM_REFRESH];
    VIALCDModeTableRec      lcdTable[VIA_BIOS_NUM_PANEL];
    VIALCDPowerSeqRec       powerOn[VIA_BIOS_NUM_LCD_POWER_SEQ];
    VIALCDPowerSeqRec       powerOff[VIA_BIOS_NUM_LCD_POWER_SEQ];
    VIALCDModeFixRec        modeFix;
    VIABIOSTVMASKTableRec   tv2MaskTable;
    VIABIOSTVMASKTableRec   tv3MaskTable;
    VIABIOSTVMASKTableRec   vt1622aMaskTable;
    VIABIOSTV2TableRec      tv2Table[VIA_BIOS_NUM_TV2];
    VIABIOSTV2TableRec      tv2OverTable[VIA_BIOS_NUM_TV2];
    VIABIOSTV3TableRec      tv3Table[VIA_BIOS_NUM_TV3];
    VIABIOSTV3TableRec      tv3OverTable[VIA_BIOS_NUM_TV3];
    VIABIOSTV3TableRec      vt1622aTable[VIA_BIOS_NUM_TV3];
    VIABIOSTV3TableRec      vt1622aOverTable[VIA_BIOS_NUM_TV3];
} VIAModeTableRec, *VIAModeTablePtr;

typedef struct _VIABIOSINFO {
    int         scrnIndex;

    Bool        UseBIOS;
    VIAModeTablePtr     pModeTable;
    int         BIOSMajorVersion;
    int         BIOSMinorVersion;
    CARD8	BIOSDateYear;
    CARD8	BIOSDateMonth;
    CARD8	BIOSDateDay;

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
    Bool        SetDVI;
    /* LCD Simultaneous Expand Mode HWCursor Y Scale */
    Bool        scaleY;
    int         panelX;
    int         panelY;
    int         resY;

    /* TV entries */
    int         TVEncoder;
    int         TVType;
    Bool        TVAttach;
    Bool        TVDotCrawl;
    int         TVOutput;
    int         TVVScan;
    int         TVHScale;
    CARD16	TVIndex;
    CARD8	TVI2CAdd;
    CARD8       TVRegs[0xFF];
} VIABIOSInfoRec, *VIABIOSInfoPtr;

/* Functions protype */
/* via_bios.c */
void ViaModeTablesInit(VIABIOSInfoPtr pBIOSInfo);

/* via_mode.c */
void VIACheckTVExist(ScrnInfoPtr pScrn);
CARD8 VIAGetActiveDisplay(ScrnInfoPtr pScrn);
Bool VIAPostDVI(ScrnInfoPtr pScrn);
CARD8 VIAGetDeviceDetect(ScrnInfoPtr pScrn);
Bool VIAFindModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode);
void VIASetModeUseBIOSTable(ScrnInfoPtr pScrn, DisplayModePtr mode);
void VIASetModeForMHS(ScrnInfoPtr pScrn, DisplayModePtr mode);
void VIALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies, 
		    LOCO *colors, VisualPtr pVisual);
void VIAEnableLCD(ScrnInfoPtr pScrn);
void VIADisableLCD(ScrnInfoPtr pScrn);


/* via_vbe.c */
Bool ViaVBEGetBIOSVersion(ScrnInfoPtr pScrn);
CARD8 ViaVBEGetActiveDevice(ScrnInfoPtr pScrn);
CARD16 ViaVBEGetDisplayDeviceInfo(ScrnInfoPtr pScrn, CARD8 *numDevice);
Bool ViaVBEGetBIOSDate(ScrnInfoPtr pScrn);
int ViaVBEGetVideoMemSize(ScrnInfoPtr pScrn);

/* in via_bandwidth.c */
void ViaSetPrimaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaSetSecondaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaDisablePrimaryFIFO(ScrnInfoPtr pScrn);

#endif /* _VIA_BIOS_H_ */
