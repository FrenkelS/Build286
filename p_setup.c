// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <stdint.h>
#include <string.h>
#include "compiler.h"
#include "engine.h"
#include "filesystem.h"
#include "p_setup.h"
#include "test.h"


int16_t numsectors;
int16_t numsprites;


void loadboard(char *filename, int32_t *daposx, int32_t *daposy, int32_t *daposz,
				int16_t *daang, int16_t *dacursectnum)
{
	size_t i;
	uint8_t searchfirst;
	int32_t fil;
	int32_t mapversion;
	int16_t numwalls;

	i = strlen(filename) - 1;
	if (filename[i] == (char)255)
	{
		filename[i] = 0;
		searchfirst = 1;
	}
	else
		searchfirst = 0;

	fil = kopen4load(filename, searchfirst);
	if (fil == -1)
		I_Error("Can't open %s", filename);

	kread(fil, &mapversion, 4);
	if (mapversion != 7L)
		I_Error("%s has the wrong map version %li", filename, mapversion);

	kread(fil, daposx, 4);
	kread(fil, daposy, 4);
	kread(fil, daposz, 4);
	kread(fil, daang, 2);
	kread(fil, dacursectnum, 2);

#if defined RANGECHECK
	kread(fil, &numsectors, 2);
	if (!(0 <= numsectors && numsectors <= MAXSECTORS))
		I_Error("%s has invalid sector count %i", filename, numsectors);
	kread(fil, &sector[0], sizeof(sectortype) * numsectors);

	kread(fil, &numwalls, 2);
	if (!(0 <= numwalls && numwalls <= MAXWALLS))
		I_Error("%s has invalid wall count %i", filename, numwalls);
	kread(fil, &wall[0], sizeof(walltype) * numwalls);

	kread(fil, &numsprites, 2);
	if (!(0 <= numsprites && numsprites <= MAXSPRITES))
		I_Error("%s has invalid sprite count %i", filename, numsprites);
	kread(fil, &sprite[0], sizeof(spritetype) * numsprites);
#else
	kread(fil, &numsectors, 2);
	kread(fil, &sector[0], sizeof(sectortype) * numsectors);

	kread(fil, &numwalls, 2);
	kread(fil, &wall[0], sizeof(walltype) * numwalls);

	kread(fil, &numsprites, 2);
	kread(fil, &sprite[0], sizeof(spritetype) * numsprites);
#endif

	kclose(fil);
}
