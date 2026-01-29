// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "engine.h"
#include "filesystem.h"
#include "tables.h"
#include "test.h"
#include "tiles.h"
#include "z_zone.h"


int16_t tilesizx[MAXTILES];
int16_t tilesizy[MAXTILES];
int32_t picanm[MAXTILES];
uint8_t picsiz[MAXTILES];


static uint8_t __far* waloff[MAXTILES]; //the actual 32-bit offset pointing to the top-left corner of the tile.
static char artfilename[20];
static uint8_t tilefilenum[MAXTILES];
static int32_t tilefileoffs[MAXTILES];

static int32_t artfil    = -1;
static int32_t artfilnum = -1;
static int32_t artfilplc = 0;

static int32_t totalclocklock;


void loadpics(char *filename)
{
	int16_t i;
	int32_t numtilefiles;
	int16_t k;
	int32_t fil;
	int32_t artversion;
	int32_t numtiles; // the number of tiles found in TILES###.DAT
	int32_t localtilestart;
	int32_t localtileend;
	int32_t offscount;
	int32_t dasiz;
	int16_t j;

	strcpy(artfilename, filename);

	for (i = 0; i < MAXTILES; i++)
	{
		tilesizx[i] = 0;
		tilesizy[i] = 0;
		picanm[i]   = 0;
	}

	numtilefiles = 0;
	do
	{
		k = numtilefiles;

		artfilename[7] = '0' + ( k        % 10);
		artfilename[6] = '0' + ((k /  10) % 10);
		artfilename[5] = '0' + ((k / 100) % 10);
		fil = kopen4load(artfilename, 0);
		if (fil != -1)
		{
			kread(fil, &artversion, 4);
#if defined RANGECHECK
			if (artversion != 1)
				I_Error("artversion %li != 1", artversion);
#endif

			kread(fil, &numtiles, 4);
			kread(fil, &localtilestart, 4);
			kread(fil, &localtileend, 4);
			kread(fil, &tilesizx[localtilestart], (localtileend - localtilestart + 1) << 1);
			kread(fil, &tilesizy[localtilestart], (localtileend - localtilestart + 1) << 1);
			kread(fil, &picanm[localtilestart],   (localtileend - localtilestart + 1) << 2);

			kclose(fil);

			offscount = 4 + 4 + 4 + 4 + ((localtileend - localtilestart + 1) << 3);
			for (i = localtilestart; i <= localtileend; i++)
			{
				tilefilenum[i] = k;
				tilefileoffs[i] = offscount;
				dasiz = (int32_t)tilesizx[i] * tilesizy[i];
				offscount += dasiz;
			}

			numtilefiles++;
		}
	}
	while (k != numtilefiles);

	for (i = 0; i < MAXTILES; i++)
	{
		j = 15;
		while ((j > 1) && (pow2long[j] > tilesizx[i]))
			j--;
		picsiz[i] = (uint8_t)j;

		j = 15;
		while ((j > 1) && (pow2long[j] > tilesizy[i]))
			j--;
		picsiz[i] += (uint8_t)(j << 4);
	}
}


void unloadpics(void)
{
	if (artfil != -1)
		kclose(artfil);
}


static uint8_t __far* staticloadtile(int16_t tilenume, uint16_t dasiz)
{
	int32_t i;

	i = tilefilenum[tilenume];
	if (i != artfilnum)
	{
		if (artfil != -1)
			kclose(artfil);

		artfilnum = i;
		artfilplc = 0;

		artfilename[7] = '0' + ( i        % 10);
		artfilename[6] = '0' + ((i /  10) % 10);
		artfilename[5] = '0' + ((i / 100) % 10);
		artfil = kopen4load(artfilename, 0);
		faketimerhandler();
	}

	if (artfilplc != tilefileoffs[tilenume])
	{
		klseek(artfil, tilefileoffs[tilenume] - artfilplc, SEEK_CUR);
		faketimerhandler();
	}

	waloff[tilenume] = Z_MallocStaticWithUser(dasiz, (void __far*__far*)&waloff[tilenume]);
	kread(artfil, waloff[tilenume], dasiz);
	faketimerhandler();
	artfilplc = tilefileoffs[tilenume] + dasiz;

	return waloff[tilenume];
}


uint8_t __far* loadtile(int16_t tilenume)
{
	int32_t dasiz;

#if defined RANGECHECK
	if (!(tilenume < MAXTILES))
		I_Error("tilenume %i too big", tilenume);
#endif

	if (waloff[tilenume] != NULL)
	{
		Z_ChangeTagToStatic(waloff[tilenume]);
		return waloff[tilenume];
	}

	dasiz = (int32_t)tilesizx[tilenume] * tilesizy[tilenume];
#if defined RANGECHECK
	if (dasiz <= 0)
		I_Error("dasiz %li too small", dasiz);
	if (dasiz >= 0x10000)
		I_Error("dasiz %li too big, %i %i %i", dasiz, tilenume, tilesizx[tilenume], tilesizy[tilenume]);
#endif

	return staticloadtile(tilenume, (uint16_t)dasiz);
}


uint8_t __far* tryloadtile(int16_t tilenume)
{
	int32_t dasiz;

#if defined RANGECHECK
	if (!(tilenume < MAXTILES))
		I_Error("tilenume %i too big", tilenume);
#endif

	if (waloff[tilenume] != NULL)
	{
		Z_ChangeTagToStatic(waloff[tilenume]);
		return waloff[tilenume];
	}

	dasiz = (int32_t)tilesizx[tilenume] * tilesizy[tilenume];
#if defined RANGECHECK
	if (dasiz <= 0)
		I_Error("dasiz %li too small", dasiz);
	if (dasiz >= 0x10000)
		I_Error("dasiz %li too big, %i %i %i", dasiz, tilenume, tilesizx[tilenume], tilesizy[tilenume]);
#else
	if (dasiz >= 0x10000)
		return NULL;
#endif

	if (!Z_IsEnoughFreeMemory((uint16_t)dasiz))
		return NULL;

	return staticloadtile(tilenume, (uint16_t)dasiz);
}


void setTotalclocklock(void)
{
	totalclocklock = totalclock;
}


int32_t animateoffs(int16_t tilenum)
{
	int32_t offs;
	int32_t i;
	int32_t k;

	offs = 0;
	i = totalclocklock >> ((picanm[tilenum] >> 24) & 15);
	if ((picanm[tilenum] & 63) > 0)
	{
		switch (picanm[tilenum] & 192)
		{
			case 64:
				k = i % ((picanm[tilenum] & 63) << 1);
				if (k < (picanm[tilenum] & 63))
					offs = k;
				else
					offs = ((picanm[tilenum] & 63) << 1) - k;
				break;
			case 128:
				offs = i % ((picanm[tilenum] & 63) + 1);
				break;
			case 192:
				offs = -(i % ((picanm[tilenum] & 63) + 1));
				break;
		}
	}

	return offs;
}
