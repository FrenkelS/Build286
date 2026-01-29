// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __PALETTE__
#define __PALETTE__

#define MAXPALOOKUPS 256


extern uint8_t __far* palookup[MAXPALOOKUPS];


void loadpalette(void);
void setbrightnessbrightness(uint8_t dabrightness);
void setbrightnesspal(void);
void makepalookup(int32_t palnum, uint8_t *remapbuf);
int32_t getpalookup(int32_t davis, int32_t dashade);
uint8_t translucfunc(uint8_t lo, uint8_t hi);

#endif
