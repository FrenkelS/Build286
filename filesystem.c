// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "compiler.h"
#include "filesystem.h"
#include "test.h"
#include "z_zone.h"

#define MAXGROUPFILES 1 /*4*/     //Warning: Fix groupfil if this is changed
#define MAXOPENFILES 1 /*64*/     //Warning: Fix filehan if this is changed

static const uint8_t toupperlookup[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
};

static int numgroupfiles = 0;
static int16_t gnumfiles[MAXGROUPFILES];
static int groupfil[MAXGROUPFILES] = {-1}; //{-1,-1,-1,-1};
static off_t groupfilpos[MAXGROUPFILES];
static uint8_t __far* gfilelist[MAXGROUPFILES];
static off_t __far* gfileoffs[MAXGROUPFILES];

static uint8_t filegrp[MAXOPENFILES];
static off_t filepos[MAXOPENFILES];
static int filehan[MAXOPENFILES] =
{
	-1
	//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};


#define BUFFERSIZE 512

static size_t _fread(int fd, void __far* buf, size_t nbyte)
{
	uint8_t __far* dest = buf;
	uint8_t buffer[BUFFERSIZE];
	size_t size = nbyte;

	while (nbyte >= BUFFERSIZE)
	{
		read(fd, buffer, BUFFERSIZE);
		_fmemcpy(dest, buffer, BUFFERSIZE);
		dest  += BUFFERSIZE;
		nbyte -= BUFFERSIZE;
	}

	if (nbyte > 0)
	{
		read(fd, buffer, nbyte);
		_fmemcpy(dest, buffer, nbyte);
	}

	return size;
}


void initgroupfile(char *filename)
{
	char buf[14];
	int16_t i;
	off_t j;
	int32_t k;

#if defined RANGECHECK
	if (numgroupfiles >= MAXGROUPFILES)
		I_Error("numgroupfiles %i is too big", numgroupfiles);
#endif

	groupfil[numgroupfiles] = open(filename, O_BINARY | O_RDWR, S_IREAD);
	if (groupfil[numgroupfiles] != -1)
	{
		groupfilpos[numgroupfiles] = 0;
		read(groupfil[numgroupfiles], buf, 16);
		if ((buf[0] != 'K') || (buf[1] != 'e') || (buf[2] != 'n') ||
			(buf[3] != 'S') || (buf[4] != 'i') || (buf[5] != 'l') ||
			(buf[6] != 'v') || (buf[7] != 'e') || (buf[8] != 'r') ||
			(buf[9] != 'm') || (buf[10] != 'a') || (buf[11] != 'n'))
		{
			I_Error("%s is not a valid group file", filename);
		}

		gnumfiles[numgroupfiles] = *((int16_t *)&buf[12]);

		gfilelist[numgroupfiles] = Z_MallocStatic(gnumfiles[numgroupfiles] << 4);
		gfileoffs[numgroupfiles] = Z_MallocStatic((gnumfiles[numgroupfiles] + 1) * sizeof(gfileoffs[0]));

		_fread(groupfil[numgroupfiles], gfilelist[numgroupfiles], gnumfiles[numgroupfiles] << 4);

		j = 0;
		for (i = 0; i < gnumfiles[numgroupfiles]; i++)
		{
			k = *((int32_t __far*)&gfilelist[numgroupfiles][(i << 4) + 12]);
			gfilelist[numgroupfiles][(i << 4) + 12] = '\0';
			gfileoffs[numgroupfiles][i] = j;
			j += k;
		}
		gfileoffs[numgroupfiles][gnumfiles[numgroupfiles]] = j;
	}

	numgroupfiles++;
}


void uninitgroupfile(void)
{
	int i;

	for (i = numgroupfiles - 1; i >= 0; i--)
		if (groupfil[i] != -1)
		{
			Z_Free(gfilelist[i]);
			Z_Free(gfileoffs[i]);
			close(groupfil[i]);
			groupfil[i] = -1;
		}
}


int kopen4load(char *filename, uint8_t searchfirst)
{
	int16_t i;
	int_fast8_t j;
	int k;
	int fil, newhandle;
	uint8_t bad;
	char __far* gfileptr;

	newhandle = MAXOPENFILES - 1;
	while (filehan[newhandle] != -1)
	{
		newhandle--;
#if defined RANGECHECK
		if (newhandle < 0)
			I_Error("TOO MANY FILES OPEN IN FILE GROUPING SYSTEM!");
#endif
	}

	if (searchfirst == 0)
	{
		fil = open(filename, O_BINARY | O_RDONLY);
		if (fil != -1)
		{
			filegrp[newhandle] = 255;
			filehan[newhandle] = fil;
			filepos[newhandle] = 0;
			return newhandle;
		}
	}

	for (k = numgroupfiles - 1; k >= 0; k--)
	{
		if (searchfirst != 0)
			k = 0;

		if (groupfil[k] != -1)
		{
			for (i = gnumfiles[k] - 1; i >= 0; i--)
			{
				gfileptr = (char __far*)&gfilelist[k][i << 4];

				bad = 0;
				for (j = 0; j < 13; j++)
				{
					if (!filename[j])
						break;

					if (toupperlookup[(size_t)filename[j]] != toupperlookup[(size_t)gfileptr[j]])
					{
						bad = 1;
						break;
					}
				}
				if (bad)
					continue;

				filegrp[newhandle] = k;
				filehan[newhandle] = i;
				filepos[newhandle] = 0;
				return newhandle;
			}
		}
	}

	return -1;
}


void kread(int handle, void __far* buffer, size_t leng)
{
	off_t i;
	int filenum;
	uint8_t groupnum;

	filenum  = filehan[handle];
	groupnum = filegrp[handle];
	if (groupnum == 255)
	{
		_fread(filenum, buffer, leng);
		return;
	}

	if (groupfil[groupnum] != -1)
	{
		i = gfileoffs[groupnum][filenum] + filepos[handle];
		if (i != groupfilpos[groupnum])
		{
			lseek(groupfil[groupnum], i + ((gnumfiles[groupnum] + 1) << 4), SEEK_SET);
			groupfilpos[groupnum] = i;
		}
		leng = (size_t)min((off_t)leng, (gfileoffs[groupnum][filenum + 1] - gfileoffs[groupnum][filenum]) - filepos[handle]);
		leng = _fread(groupfil[groupnum], buffer, leng);
		filepos[handle]       += leng;
		groupfilpos[groupnum] += leng;
	}
}


void klseek(int handle, off_t offset, int whence)
{
	int i;
	uint8_t groupnum;

	groupnum = filegrp[handle];

	if (groupnum == 255)
	{
		lseek(filehan[handle], offset, whence);
		return;
	}

	if (groupfil[groupnum] != -1)
	{
		switch(whence)
		{
			case SEEK_SET:
				filepos[handle] = offset;
				break;
			case SEEK_END:
				i = filehan[handle];
				filepos[handle] = (gfileoffs[groupnum][i + 1] - gfileoffs[groupnum][i]) + offset;
				break;
			case SEEK_CUR:
				filepos[handle] += offset;
				break;
		}
	}
}


void kclose(int handle)
{
	if (handle < 0)
		return;

	if (filegrp[handle] == 255)
		close(filehan[handle]);

	filehan[handle] = -1;
}
