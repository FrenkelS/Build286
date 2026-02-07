// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __TILES__
#define __TILES__

#include "compiler.h"


#define MAXTILES 256 /*9216*/

/*************************************************************************
tilesizx[MAXTILES] - simply the x-dimension of the tile number.
tilesizy[MAXTILES] - simply the y-dimension of the tile number.
picanm[MAXTILES] - flags for animating the tile.
***************************************************************************/
extern int16_t tilesizx[MAXTILES];
extern int16_t tilesizy[MAXTILES];
extern int32_t picanm[MAXTILES];

extern uint8_t picsiz[MAXTILES];


void loadpics(char *filename);
void unloadpics(void);
uint8_t __far* loadtile(int16_t tilenume);
uint8_t __far* tryloadtile(int16_t tilenume);
void setTotalclocklock(void);
int_fast16_t animateoffs(int16_t tilenum);

#endif
