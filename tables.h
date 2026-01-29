// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __TABLES__
#define __TABLES__

#include <stdint.h>

extern const int32_t pow2long[32];

/*************************************************************************
sintable[2048] is a sin table with 2048 angles rather than the
	normal 360 angles for higher precision.  Also since sintable is in
	all integers, the range is multiplied by 16383, so instead of the
	normal -1<sin(x)<1, the range of sintable is -16383<sintable[]<16383
	If you use this sintable, you can possibly speed up your code as
	well as save space in memory.  If you plan to use sintable, 2
	identities you may want to keep in mind are:
		sintable[ang&2047]       = sin(ang * (3.141592/1024)) * 16383
		sintable[(ang+512)&2047] = cos(ang * (3.141592/1024)) * 16383
***************************************************************************/
extern int16_t sintable[2048];

extern int16_t radarang[1280];
extern uint8_t britable[16][64];


void loadtables(void);

#endif
