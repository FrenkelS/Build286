// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file has been modified from Ken Silverman's original release

/* -----------------------------------------------------------------------
	  This source code is the property of Ken Silverman, East Greenwich,
	  Rhode Island, and contains confidential and trade secret information.
	  It may not be transferred from the custody or control of Ken Silverman
	  except as authorized in writing by Ken Silverman.  Neither this item
	  nor the information it contains may be used, transferred, reproduced,
	  published, or disclosed, in whole or in part, and directly or
	  indirectly, except as expressly authorized by Ken Silverman,
	  pursuant to written agreement.  Copyright (c) 1995 Ken Silverman.
	  All rights reserved.  E-mail Address:  kjs@lems.brown.edu
	----------------------------------------------------------------------- */

#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "compiler.h"
#include "engine.h"
#include "filesystem.h"
#include "p_setup.h"
#include "palette.h"
#include "tables.h"
#include "test.h"
#include "tiles.h"
#include "z_zone.h"


#if defined __DJGPP__
static _go32_dpmi_seginfo oldtimerhandler, newtimerhandler;
#else
static void __interrupt __far (*oldtimerhandler)(void);
#endif
static void __interrupt __far timerhandler(void);
static uint8_t isTimerSet = 0;


#if defined __DJGPP__
static _go32_dpmi_seginfo oldkeyhandler, newkeyhandler;
#else
static void __interrupt __far (*oldkeyhandler)(void);
#endif
static void __interrupt __far keyhandler(void);
static volatile int8_t keystatus[256];
static uint8_t isKeyboardIsrSet = 0;


static int16_t vel, svel, angvel;

static int32_t posx, posy, posz;
static int16_t horiz = 100, ang, cursectnum;

static int32_t synctics = 0;


#define NUMOPTIONS 8
#define NUMKEYS 19
static uint8_t option[NUMOPTIONS];
static uint8_t buildkeys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};


static void editinput(void);
static void inittimer(void);
static void uninittimer(void);
static void initkeys(void);
static void uninitkeys(void);
static void keytimerstuff(void);
_Noreturn static void I_Quit(void);


/*
=================
=
= M_CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc - 1) or 0 if not present
=
=================
*/

static int myargc;
static const char * const * myargv;

int M_CheckParm(char *check)
{
	int i;

	for (i = 1; i < myargc; i++)
		if (!stricmp(check, myargv[i]))
			return i;

	return 0;
}


int main(int argc, const char * const *argv)
{
	static int32_t lockclock = 0;
	char boardfilename[13];
	int16_t i;
	int fil;
	int16_t daang = 0;
	spritetype __far* tspr;
	uint8_t tempbuf[256];

	myargc = argc;
	myargv = argv;

	__djgpp_nearptr_enable();
	Z_Init();

	editstatus = 1;
	if (argc >= 2)
	{
		strcpy(boardfilename, argv[1]);
		if (strchr(boardfilename, '.') == 0)
			strcat(boardfilename, ".map");
	}
	else
		strcpy(boardfilename, "test.map");

	initgroupfile("stuff.dat");

	fil = open("setup.dat", O_BINARY | O_RDWR, S_IREAD);
	if (fil != -1)
	{
		read(fil, &option[0], NUMOPTIONS);
		read(fil, &buildkeys[0], NUMKEYS);
		close(fil);
	}

	initengine();

		//You can load your own palette lookup tables here if you just
		//copy the right code!
	for (i = 0; i < 256; i++)
		tempbuf[i] = (i + 32) & 255;  //remap colors for screwy palette sectors
	makepalookup(16, tempbuf);

	pskyoff[0] = 0;
	pskyoff[1] = 0;
	pskybits = 1;

	initkeys();
	inittimer();

	loadpics("tiles000.art");

	setgamemode();

	initspritelists();

	loadboard(boardfilename, &posx, &posy, &posz, &ang, &cursectnum);

	for (i = 0; i < numsprites; i++)
	{
		insertspritestat(sprite[i].statnum);
		insertspritesect(sprite[i].sectnum);
	}

		//Must be after loading sectors, etc!
	updatesector(posx, posy, &cursectnum);

	totalclock = 0;

	while (keystatus[1] == 0)
	{
		drawrooms(posx, posy, posz, ang, horiz, cursectnum);

		for (i = 0, tspr = &tsprite[0]; i < spritesortcnt; i++, tspr++)
		{
			tspr->shade += 6;
			if (sector[tspr->sectnum].ceilingstat & 1)
				tspr->shade += sector[tspr->sectnum].ceilingshade;
			else
				tspr->shade += sector[tspr->sectnum].floorshade;
		}

		drawmasks();

		if (keystatus[0xa]) setaspect(viewingrange + (viewingrange >> 8), yxaspect + (yxaspect >> 8));
		if (keystatus[0xb]) setaspect(viewingrange - (viewingrange >> 8), yxaspect - (yxaspect >> 8));
		if (keystatus[0xc]) setaspect(viewingrange, yxaspect - (yxaspect >> 8));
		if (keystatus[0xd]) setaspect(viewingrange, yxaspect + (yxaspect >> 8));

		editinput();

		daang += (keystatus[0x6] - keystatus[0x7]) * 16;
		if (keystatus[0x2]) rotatesprite((int32_t)XDIM << 15, (int32_t)YDIM << 15, 65536L, daang, 75, 0, 0,     8 + 64, 0L, 0L, XDIM - 1L, YDIM - 1L);
		if (keystatus[0x3]) rotatesprite((int32_t)XDIM << 15, (int32_t)YDIM << 15, 65536L, daang, 75, 0, 0,     8     , 0L, 0L, XDIM - 1L, YDIM - 1L);
		if (keystatus[0x4]) rotatesprite((int32_t)XDIM << 15, (int32_t)YDIM << 15, 65536L, daang, 75, 0, 0, 1 + 8     , 0L, 0L, XDIM - 1L, YDIM - 1L);
		if (keystatus[0x5]) rotatesprite((int32_t)XDIM << 15, (int32_t)YDIM << 15, 65536L, daang, 75, 0, 0, 1 + 8 + 32, 0L, 0L, XDIM - 1L, YDIM - 1L);

		nextpage();
		synctics = totalclock - lockclock;
		lockclock += synctics;
	}

	I_Quit();
	return 0;
}


static void editinput(void)
{
	static size_t brightness = 0;
	static int32_t hvel;
	int32_t doubvel;
	int32_t goalz, xvect, yvect, hiz, loz;
	int32_t hihit, lohit;

	if (keystatus[0x57])  //F11 - brightness
	{
		keystatus[0x57] = 0;

		brightness = (brightness + 1) & 15;

		setBrightness(brightness);
		setPalette();
	}

	if (keystatus[0x3b]) posx--;
	if (keystatus[0x3c]) posx++;
	if (keystatus[0x3d]) posy--;
	if (keystatus[0x3e]) posy++;
	if (keystatus[0x43]) ang--;
	if (keystatus[0x44]) ang++;

	if (angvel != 0)          //ang += angvel * constant
	{                         //ENGINE calculates angvel for you
		doubvel = synctics;
		if (keystatus[buildkeys[4]])  //Lt. shift makes turn velocity 50% faster
			doubvel += (synctics >> 1);
		ang += ((angvel * doubvel) >> 4);
		ang = (ang + 2048) & 2047;
	}

	if ((vel | svel) != 0)
	{
		doubvel = synctics;
		if (keystatus[buildkeys[4]])     //Lt. shift doubles forward velocity
			doubvel += synctics;
		xvect = 0;
		yvect = 0;
		if (vel != 0)
		{
			xvect += ((vel * doubvel * (int32_t)sintable[(ang + 2560) & 2047]) >> 3);
			yvect += ((vel * doubvel * (int32_t)sintable[(ang + 2048) & 2047]) >> 3);
		}
		if (svel != 0)
		{
			xvect += ((svel * doubvel * (int32_t)sintable[(ang + 2048) & 2047]) >> 3);
			yvect += ((svel * doubvel * (int32_t)sintable[(ang + 1536) & 2047]) >> 3);
		}
		clipmove(&posx, &posy, &posz, &cursectnum, xvect, yvect, 128L, 4L << 8, 4L << 8, CLIPMASK0);
	}
	getzrange(posx, posy, posz, cursectnum, &hiz, &hihit, &loz, &lohit, 128L, CLIPMASK0);

	goalz = loz - (32 << 8);   //playerheight pixels above floor
	if (goalz < hiz + (16 << 8))   //ceiling&floor too close
		goalz = ((loz + hiz) >> 1);

	if (keystatus[buildkeys[8]])                            //A (stand high)
	{
		if (keystatus[0x1d])
			horiz = max(-100, horiz - ((keystatus[buildkeys[4]] + 1) << 2));
		else
		{
			goalz -= (16 << 8);
			if (keystatus[buildkeys[4]])    //Either shift key
				goalz -= (24 << 8);
		}
	}

	if (keystatus[buildkeys[9]])                            //Z (stand low)
	{
		if (keystatus[0x1d])
			horiz = min(300, horiz + ((keystatus[buildkeys[4]] + 1) << 2));
		else
		{
			goalz += (12 << 8);
			if (keystatus[buildkeys[4]])    //Either shift key
				goalz += ( 12 << 8);
		}
	}

	if (goalz != posz)
	{
		if (posz < goalz) hvel += 32;
		if (posz > goalz) hvel = (goalz - posz) >> 3;

		posz += hvel;
		if (posz > loz - (4 << 8)) posz = loz - (4 << 8), hvel = 0;
		if (posz < hiz + (4 << 8)) posz = hiz + (4 << 8), hvel = 0;
	}
}


static void inittimer(void)
{
	outp(0x43, 0x34);
	outp(0x40, (1193181 / 120) & 255);
	outp(0x40, (1193181 / 120) >> 8);
	_disable();
	replaceInterrupt(oldtimerhandler, newtimerhandler, 0x8, timerhandler);
	_enable();

	isTimerSet = 1;
}


static void uninittimer(void)
{
	if (isTimerSet == 0)
		return;

	//18.2 times/sec
	outp(0x43,0x34);
	outp(0x40,0);
	outp(0x40,0);
	_disable();
	restoreInterrupt(0x8, oldtimerhandler, newtimerhandler);
	_enable();
}


static void __interrupt __far timerhandler(void)
{
	totalclock++;
	keytimerstuff();
	outp(0x20, 0x20);
}


static void initkeys(void)
{
	_disable();
	replaceInterrupt(oldkeyhandler, newkeyhandler, 0x9, keyhandler);
	_enable();

	isKeyboardIsrSet = 1;
}


static void uninitkeys(void)
{
	int16_t __far* ptr;

	if (isKeyboardIsrSet == 0)
		return;

	restoreInterrupt(0x9, oldkeyhandler, newkeyhandler);

		//Turn off shifts to prevent stucks with quitting
	ptr = kMK_FP(0x40, 0x17 + __djgpp_conventional_base);
	*ptr &= ~0x030f;
}


#define KEYFIFOSIZ 64

static void __interrupt __far keyhandler(void)
{
	static volatile uint8_t readch, extended;
	uint8_t keytemp;
	uint8_t oldreadch;

	oldreadch = readch;
	readch  = inp(0x60);
	keytemp = inp(0x61);
	outp(0x61, keytemp | 128);
	outp(0x61, keytemp & 127);
	outp(0x20, 0x20);

	if ((readch | 1) == 0xe1)
	{
		extended = 128;
		return;
	}

	if (oldreadch != readch)
	{
		if ((readch & 128) == 0)
		{
			keytemp = readch + extended;
			keystatus[keytemp] = 1;
		}
		else
		{
			keytemp = (readch & 127) + extended;
			keystatus[keytemp] = 0;
		}
	}
	extended = 0;
}


static void keytimerstuff(void)
{
	if (keystatus[buildkeys[5]] == 0)
	{
		if (keystatus[buildkeys[2]]) angvel = max(angvel - 16, -128);
		if (keystatus[buildkeys[3]]) angvel = min(angvel + 16,  127);
	}
	else
	{
		if (keystatus[buildkeys[2]]) svel = min(svel + 8,  127);
		if (keystatus[buildkeys[3]]) svel = max(svel - 8, -128);
	}

	if (keystatus[buildkeys[0]])   vel = min( vel + 8,  127);
	if (keystatus[buildkeys[1]])   vel = max( vel - 8, -128);
	if (keystatus[buildkeys[12]]) svel = min(svel + 8,  127);
	if (keystatus[buildkeys[13]]) svel = max(svel - 8, -128);

	if (angvel < 0) angvel = min(angvel + 12, 0);
	if (angvel > 0) angvel = max(angvel - 12, 0);
	if (svel < 0) svel = min(svel + 2, 0);
	if (svel > 0) svel = max(svel - 2, 0);
	if (vel < 0)   vel = min( vel + 2, 0);
	if (vel > 0)   vel = max( vel - 2, 0);
}


static void I_Shutdown(void)
{
	setvmode(0x3);
	uninittimer();
	uninitkeys();
	unloadpics();
	uninitgroupfile();
	Z_Shutdown();
}


static void I_Quit(void)
{
	uint32_t resultfps;

	I_Shutdown();

	resultfps = 120 * 1000L * numframes / totalclock;

	printf("Frame rate = %lu.%.3lu\n", resultfps / 1000, resultfps % 1000);
	exit(0);
}


void I_Error(const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}
