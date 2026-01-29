// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <conio.h>
#include <dos.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "filesystem.h"
#include "palette.h"
#include "tables.h"
#include "test.h"
#include "z_zone.h"


uint8_t __far* palookup[MAXPALOOKUPS];


static uint8_t paletteloaded = 0;
static uint8_t palette[768];
static int16_t numpalookups;
static int32_t curbrightness = 0;


//#define TRANSLUCENCY

#if defined TRANSLUCENCY
static uint8_t transluc[65536];
#endif


void loadpalette(void)
{
	int32_t fil;

	if (paletteloaded != 0)
		return;

	fil = kopen4load("palette.dat", 0);
#if defined RANGECHECK
	if (fil == -1)
		I_Error("Can't open palette.dat");
#endif

	kread(fil, palette, 768);
	kread(fil, &numpalookups, 2);

	palookup[0] = Z_MallocStatic(numpalookups << 8);
	kread(fil, palookup[0], numpalookups << 8);

#if defined TRANSLUCENCY
	kread(fil, transluc, 65536);
#endif

	kclose(fil);

	paletteloaded = 1;
}


static void setPalette(uint8_t *dapal)
{
	int32_t i;

	outp(0x3c8, 0);
	for (i = 256 >> 1; i > 0; i--)
	{
		outp(0x3c9, dapal[2]);

		while (  inp(0x3da) & 1);
		while (!(inp(0x3da) & 1));

		outp(0x3c9,dapal[1]);
		outp(0x3c9,dapal[0]);

		outp(0x3c9,dapal[6]);
		outp(0x3c9,dapal[5]);
		outp(0x3c9,dapal[4]);

		dapal += 8;
	}
}


void setbrightnessbrightness(uint8_t dabrightness)
{
	curbrightness = min(max((int32_t)dabrightness, 0), 15);
}


void setbrightnesspal(void)
{
	int32_t i, k;
	uint8_t tempbuf[256 * 4];

	k = 0;

	for (i = 0; i < 256; i++)
	{
		tempbuf[k++] = britable[curbrightness][palette[i * 3 + 2]];
		tempbuf[k++] = britable[curbrightness][palette[i * 3 + 1]];
		tempbuf[k++] = britable[curbrightness][palette[i * 3 + 0]];
		tempbuf[k++] = 0;
	}

	setPalette(tempbuf);
}


void makepalookup(int32_t palnum, uint8_t *remapbuf)
{
	int32_t i, j;
	uint8_t __far* ptr;
	uint8_t __far* ptr2;

	if (paletteloaded == 0)
		return;

	if (palookup[palnum] == NULL)
	{
			//Allocate palookup buffer
		palookup[palnum] = Z_MallocStatic(numpalookups << 8);
	}

	for (i = 0; i < 256; i++)
	{
		ptr  = palookup[0] + remapbuf[i];
		ptr2 = palookup[palnum] + i;
		for (j = 0; j < numpalookups; j++)
		{
			*ptr2 = *ptr;
			ptr  += 256;
			ptr2 += 256;
		}
	}
}


int32_t getpalookup(int32_t davis, int32_t dashade)
{
	return min(max(dashade + (davis >> 8), 0), numpalookups - 1);
}


uint8_t translucfunc(uint8_t lo, uint8_t hi)
{
#if defined TRANSLUCENCY
	return transluc[lo + (hi << 8)];
#else
	static uint8_t x = 0;
	x = !x;
	return x ? lo : hi;
#endif
}
