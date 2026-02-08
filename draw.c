// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <stdint.h>
#include <string.h>
#include "compiler.h"
#include "draw.h"
#include "engine.h"
#include "palette.h"
#include "test.h"
#include "z_zone.h"


uint8_t __far* _s_screen;

static uint8_t __far* videomemory;


static int32_t divscale(int32_t a, int32_t b, int32_t c)
{
	return ((int64_t)a << c) / b;
}

#define divscale16(a,b) divscale((a),(b),16)
#define divscale32(a,b) divscale((a),(b),32)


static void setview(void)
{
	int32_t i;

	xdimenrecip = divscale32(1L,XDIM);

	setaspect(65536L, divscale16((int32_t)YDIM * XDIM, (int32_t)XDIM * YDIM));

	for(i=0;i<=XDIM-1;i++)
	{
		startumost[i] = 0;
		startdmost[i] = YDIM;
	}
}


static void clearallviews(int32_t dacol)
{
	dacol += (dacol<<8); dacol += (dacol<<16);

	_fmemset(_s_screen, 0, (size_t)((int32_t)XDIM * YDIM));
	faketimerhandler();
}


void setgamemode(void)
{
	int32_t i, j;

	setvmode(0x13);
	_s_screen = Z_MallocStatic((size_t)((int32_t)XDIM * YDIM));
	videomemory = kMK_FP(0xa000, 0 + __djgpp_conventional_base);

		//Force drawrooms to call dosetaspect & recalculate stuff
	oxyaspect = oxdimen = oviewingrange = -1;

	j = 0;
	for(i=0;i<=YDIM;i++)
	{
		ylookup[i] = j;
		j += XDIM;
	}

	setview();
	clearallviews(0L);
	setPalette();

	if (searchx < 0)
	{
		searchx = (XDIM>>1);
		searchy = (YDIM>>1);
	}
}


void nextpage(void)
{
	_fmemcpy(videomemory, _s_screen, (size_t)((int32_t)XDIM * YDIM));

	faketimerhandler();
}
