// $Id$
/*
 * AIN access functions for MIDIbox NG
 *
 * ==========================================================================
 *
 *  Copyright (C) 2012 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _MBNG_AIN_H
#define _MBNG_AIN_H

#include "mbng_event.h"

/////////////////////////////////////////////////////////////////////////////
// global definitions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

extern s32 MBNG_AIN_Init(u32 mode);

extern s32 MBNG_AIN_NotifyChange(u32 pin, u32 pin_value);
extern s32 MBNG_AIN_NotifyChange_SER64(u32 module, u32 pin, u32 pin_value);

extern s32 MBNG_AIN_NotifyReceivedValue(mbng_event_item_t *item, u16 value);
extern s32 MBNG_AIN_NotifyRefresh(mbng_event_item_t *item);
extern s32 MBNG_AIN_NotifyReceivedValue_SER64(mbng_event_item_t *item, u16 value);
extern s32 MBNG_AIN_NotifyRefresh_SER64(mbng_event_item_t *item);

/////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _MBNG_AIN_H */
