// $Id$
/*
 * Header file for MIDI File Importer
 *
 * ==========================================================================
 *
 *  Copyright (C) 2008 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _SEQ_MIDIMP_H
#define _SEQ_MIDIMP_H

/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

typedef enum {
  SEQ_MIDIMP_MODE_Track,
  SEQ_MIDIMP_MODE_Group,
  SEQ_MIDIMP_MODE_All,
  SEQ_MIDIMP_MODE_Song
} seq_midimp_mode_t;


/////////////////////////////////////////////////////////////////////////////
// Global Types
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

extern s32 SEQ_MIDIMP_Init(u32 mode);


/////////////////////////////////////////////////////////////////////////////
// Export global variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _SEQ_MIDIMP_H */
