// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <stdint.h>
#include <stdio.h>
#include "filesystem.h"
#include "tables.h"
#include "test.h"


const int32_t pow2long[32] =
{
	1L,2L,4L,8L,
	16L,32L,64L,128L,
	256L,512L,1024L,2048L,
	4096L,8192L,16384L,32768L,
	65536L,131072L,262144L,524288L,
	1048576L,2097152L,4194304L,8388608L,
	16777216L,33554432L,67108864L,134217728L,
	268435456L,536870912L,1073741824L,2147483647L
};


int16_t sintable[2048];
int16_t radarang[1280];
uint8_t __far britable[16][64];


void loadtables(void)
{
	int fil;
	int_fast16_t i;

	fil = kopen4load("tables.dat", 0);
#if defined RANGECHECK
	if (fil == -1)
		I_Error("Can't open tables.dat");
#endif

	kread(fil, sintable, sizeof(sintable));
	kread(fil, radarang, sizeof(radarang) / 2);

	for (i = 0; i < 640; i++)
		radarang[1279 - i] = -radarang[i];

	klseek(fil, 1024, SEEK_CUR); // textfont
	klseek(fil, 1024, SEEK_CUR); // smalltextfont
	kread(fil, britable, sizeof(britable));

	kclose(fil);
}
