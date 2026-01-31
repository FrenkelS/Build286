// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __P_SETUP__
#define __P_SETUP__

#include <stdint.h>

/*************************************************************************
numsectors - the total number of existing sectors.  Modified every time
	you call the loadboard function.
***************************************************************************/
extern int16_t numsectors;

extern int16_t numsprites;

extern sectortype __far sector[MAXSECTORS];
extern walltype __far wall[MAXWALLS];
extern spritetype __far sprite[MAXSPRITES];


void loadboard(char *filename, int32_t *daposx, int32_t *daposy, int32_t *daposz, int16_t *daang, int16_t *dacursectnum);

#endif
