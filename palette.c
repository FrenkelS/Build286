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


static uint8_t palette[768];
static int16_t numpalookups;
static size_t curbrightness = 0;


//#define TRANSLUCENCY

#if defined TRANSLUCENCY
static uint8_t transluc[65536];
#endif


void loadpalette(void)
{
	int fil;

	fil = kopen4load("palette.dat", 0);
#if defined RANGECHECK
	if (fil == -1)
		I_Error("Can't open palette.dat");
#endif

	kread(fil, palette, sizeof(palette));
	kread(fil, &numpalookups, sizeof(numpalookups));

	palookup[0] = Z_MallocStatic(numpalookups << 8);
	kread(fil, palookup[0], numpalookups << 8);

#if defined TRANSLUCENCY
	kread(fil, transluc, 65536);
#endif

	kclose(fil);
}


static void setpal(uint8_t *dapal)
{
	int_fast16_t i;

	outp(0x3c8, 0);
	for (i = 256 >> 1; i > 0; i--)
	{
		outp(0x3c9, *dapal++);

		while (  inp(0x3da) & 1);
		while (!(inp(0x3da) & 1));

		outp(0x3c9, *dapal++);
		outp(0x3c9, *dapal++);

		outp(0x3c9, *dapal++);
		outp(0x3c9, *dapal++);
		outp(0x3c9, *dapal++);
	}
}


void setBrightness(size_t dabrightness)
{
	curbrightness = dabrightness;
}


void setPalette(void)
{
	int_fast16_t i;
	uint8_t tempbuf[256 * 3];
	uint8_t *s;
	uint8_t *d;

	s = palette;
	d = tempbuf;
	for (i = 0; i < 256 * 3; i++)
	{
		*d++ = britable[curbrightness][*s++];
	}

	setpal(tempbuf);
}


void makepalookup(size_t palnum, uint8_t *remapbuf)
{
	size_t i;
	int16_t j;
	uint8_t __far* ptr;
	uint8_t __far* ptr2;

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


int16_t getpalookup(int32_t davis, int32_t dashade)
{
	return min(max((davis >> 8) + dashade, 0), numpalookups - 1);
}


uint8_t translucfunc(uint8_t lo, uint8_t hi)
{
#if defined TRANSLUCENCY
	return transluc[lo + (hi << 8)];
#else
	static uint8_t toggle = 0;
	toggle ^= 1;
	return toggle ? lo : hi;
#endif
}
