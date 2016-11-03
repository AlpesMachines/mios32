/*
 * VGM Data and Playback Driver: VGM Data Source header
 *
 * ==========================================================================
 *
 *  Copyright (C) 2016 Sauraen (sauraen@gmail.com)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _VGMSOURCE_H
#define _VGMSOURCE_H

#include <mios32.h>


typedef union {
    u32 all;
    struct {
        //Don't change the order of these bits, they're accessed by bit mask operations
        u8 fm1:1;
        u8 fm2:1;
        u8 fm3:1;
        u8 fm4:1;
        u8 fm5:1;
        u8 fm6:1;
        
        u8 fm1_lfo:1;
        u8 fm2_lfo:1;
        u8 fm3_lfo:1;
        u8 fm4_lfo:1;
        u8 fm5_lfo:1;
        u8 fm6_lfo:1;
        
        u8 dac:1;
        u8 fm3_special:1;
        u8 opn2_globals:1;
        u8 lfofixed:1;
        
        u8 lfofixedspeed:3;
        u8 dummy:5;
        
        u8 sq1:1;
        u8 sq2:1;
        u8 sq3:1;
        u8 noise:1;
        u8 noisefreqsq3:1;
        u8 dummy2:3;
    };
} VgmUsageBits;


#define VGM_SOURCE_TYPE_RAM 1
#define VGM_SOURCE_TYPE_STREAM 2
#define VGM_SOURCE_TYPE_QUEUE 3

typedef union {
    u8 ALL[28];
    struct{
        u8 type;
        u8 dummy1;
        u16 mutes;
        u32 opn2clock;
        u32 psgclock;
        u32 loopaddr;
        u32 loopsamples;
        VgmUsageBits usage;
        void* data;
    };
} VgmSource;


extern s32 VGM_Source_Delete(VgmSource* source);

extern void VGM_Source_UpdateUsage(VgmSource* source);

#endif /* _VGMSOURCE_H */