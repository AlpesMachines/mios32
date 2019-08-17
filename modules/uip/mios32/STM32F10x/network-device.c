// $Id$
/*
 * Access functions to network device
 *
 * ==========================================================================
 *
 *  Copyright (C) 2009 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#include <mios32.h>
#include "uip.h"
#include "network-device.h"


/////////////////////////////////////////////////////////////////////////////
// for optional debugging messages via DEBUG_MSG (defined in mios32_config.h)
/////////////////////////////////////////////////////////////////////////////

#define DEBUG_VERBOSE_LEVEL 0


/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Network Device Functions
/////////////////////////////////////////////////////////////////////////////

void network_device_init(void)
{
}

void network_device_check(void)
{
}

int network_device_available(void)
{
}

int network_device_read(void)
{
  return 0;
}

void network_device_send(void)
{
}


unsigned char *network_device_mac_addr(void)
{
    return 0;
}
