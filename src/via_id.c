/*
 * Copyright 2004 The Unichrome Project  [unichrome.sf.net]
 *
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

#include "via_driver.h"
#include "via_id.h"

/*
 * Since we are going to hold a rather big structure with
 * basic card-id information, we might as well seperate this
 * into its own file.
 *
 */

/*
 * There's no reason for this to be known outside of via_id.o
 * Only a pointer to an single entry will ever be used outside.
 *
 */
struct ViaCardIdStruct ViaCardId[] = {
    {"VIA EPIA M/MII/...",                 VIA_CLE266,  0x1106, 0x3122, VIA_DEVICE_CRT | VIA_DEVICE_TV, VIA_DEVICE_NONE},
    {"Shuttle FX43",                       VIA_KM400,   0x1297, 0xF643, VIA_DEVICE_CRT | VIA_DEVICE_TV, VIA_DEVICE_NONE},
    {"Asustek A7V8X-MX",                   VIA_KM400,   0x1043, 0x80ED, VIA_DEVICE_CRT, VIA_DEVICE_NONE},
    {"ECS G320",                           VIA_CLE266,  0x1019, 0xB320, VIA_DEVICE_CRT | VIA_DEVICE_LCD, VIA_DEVICE_LCD},
    {"Acer Aspire 135x",                   VIA_KM400,   0x1025, 0x0033, VIA_DEVICE_CRT | VIA_DEVICE_LCD | VIA_DEVICE_TV, VIA_DEVICE_LCD},
    {"Averatec 322x",                      VIA_KM400,   0x14FF, 0x030D, VIA_DEVICE_CRT | VIA_DEVICE_LCD, VIA_DEVICE_LCD},
    {"Gericom Hummer Advance",             VIA_KM400,   0x1584, 0x800A, VIA_DEVICE_CRT | VIA_DEVICE_LCD | VIA_DEVICE_TV, VIA_DEVICE_LCD},
    {"Shuttle FX83",                       VIA_K8M800,  0x1297, 0xF683, VIA_DEVICE_CRT | VIA_DEVICE_TV, VIA_DEVICE_NONE},
    {"Mitac 8399/Pogolinux Konabook 3100", VIA_K8M800,  0x1071, 0x8399, VIA_DEVICE_CRT | VIA_DEVICE_LCD | VIA_DEVICE_TV, VIA_DEVICE_LCD},
    {"Giga-byte 7VM400M-RZ",               VIA_KM400,   0x1458, 0xD000, VIA_DEVICE_CRT, VIA_DEVICE_NONE},
    {"ASRock Inc. K7VM4",                  VIA_KM400,   0x1849, 0x7205, VIA_DEVICE_CRT, VIA_DEVICE_NONE},
    {NULL,                                 VIA_UNKNOWN, 0x0000, 0x0000, VIA_DEVICE_NONE}
};

/*
 * Fancy little routine stolen from fb
 * We don't do anything but warn really.
 */
void
ViaDoubleCheckCLE266Revision(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    /* Crtc 0x4F is only defined in CLE266Cx */
    CARD8 tmp = hwp->readCrtc(hwp, 0x4F);
    
    hwp->writeCrtc(hwp, 0x4F, 0x55);
    
    if (hwp->readCrtc(hwp, 0x4F) == 0x55) {
	if (CLE266_REV_IS_AX(pVia->ChipRev))
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "CLE266 Revision seems"
		       " to be Cx, yet %d was detected previously.\n", pVia->ChipRev);
    } else {
	if (CLE266_REV_IS_CX(pVia->ChipRev))
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "CLE266 Revision seems"
		       " to be Ax, yet %d was detected previously.\n", pVia->ChipRev);
    }
    hwp->writeCrtc(hwp, 0x4F, tmp);
}

/*
 *
 */
void
ViaCheckCardId(ScrnInfoPtr pScrn)
{
    struct ViaCardIdStruct *Id;
    VIAPtr pVia = VIAPTR(pScrn);
    
    
    for (Id = ViaCardId; Id->String; Id++) {
	if ((Id->Chip == pVia->Chipset) && 
	    (Id->Vendor == pVia->PciInfo->subsysVendor) &&
	    (Id->Device == pVia->PciInfo->subsysCard)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected %s.\n", Id->String);
	    pVia->Id = Id;
	    return;
	}
    }
    
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
	       "Unknown Card-Ids (%4X|%4X), report this to unichrome-devel@lists.sf.net ASAP\n"
	       , pVia->PciInfo->subsysVendor, pVia->PciInfo->subsysCard);
    pVia->Id = NULL;
}

