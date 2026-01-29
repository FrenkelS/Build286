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

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "build.h"
#include "pragmas.h"

void (__interrupt __far *oldtimerhandler)();
void __interrupt __far timerhandler(void);

#define KEYFIFOSIZ 64
void (__interrupt __far *oldkeyhandler)();
void __interrupt __far keyhandler(void);
volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
volatile char readch, oldreadch, extended, keytemp;

long vel, svel, angvel;

long posx, posy, posz, horiz = 100;
short ang, cursectnum;
static long hvel;

static long synctics = 0, lockclock = 0;

static short brightness = 0;

extern short editstatus;

static char boardfilename[13];

static char tempbuf[256];

#define NUMOPTIONS 8
#define NUMKEYS 19
static long vesares[13][2] = {320,200,360,200,320,240,360,240,320,400,
									360,400,640,350,640,400,640,480,800,600,
									1024,768,1280,1024,1600,1200};
static char option[NUMOPTIONS] = {0,0,0,0,0,0,1,0};
static char buildkeys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};

main(int argc, char **argv)
{
	long i, fil, daang = 0;
	spritetype *tspr;

	editstatus = 1;
	if (argc >= 2)
	{
		strcpy(&boardfilename,argv[1]);
		if (strchr(boardfilename,'.') == 0)
			strcat(boardfilename,".map");
	}
	else
		strcpy(&boardfilename,"test.map");

	initgroupfile("stuff.dat");
	if ((fil = open("setup.dat",O_BINARY|O_RDWR,S_IREAD)) != -1)
	{
		read(fil,&option[0],NUMOPTIONS);
		read(fil,&buildkeys[0],NUMKEYS);
		close(fil);
	}
	if (option[4] > 0) option[4] = 0;
	initmouse();

	initengine();

		//You can load your own palette lookup tables here if you just
		//copy the right code!
	for(i=0;i<256;i++)
		tempbuf[i] = ((i+32)&255);  //remap colors for screwy palette sectors
	makepalookup(16,tempbuf,0,0,0,1);

	pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;

	initkeys();
	inittimer();

	loadpics("tiles000.art");

	if (setgamemode(option[0],vesares[option[6]&15][0],vesares[option[6]&15][1]) < 0)
	{
		uninitgroupfile();
		uninitkeys();
		uninittimer();
		printf("%ld * %ld not supported in this graphics mode\n",xdim,ydim);
		exit(0);
	}

	if (loadboard(boardfilename,&posx,&posy,&posz,&ang,&cursectnum) == -1)
	{
		uninitgroupfile();
		uninitkeys();
		uninittimer();
		setvmode(0x3);
		printf("Board not found\n");
		exit(0);
	}

	totalclock = 0;

	while (keystatus[1] == 0)
	{
		drawrooms(posx,posy,posz,ang,horiz,cursectnum);

		for(i=0,tspr=&tsprite[0];i<spritesortcnt;i++,tspr++)
		{
			tspr->shade += 6;
			if (sector[tspr->sectnum].ceilingstat&1)
				tspr->shade += sector[tspr->sectnum].ceilingshade;
			else
				tspr->shade += sector[tspr->sectnum].floorshade;
		}

		drawmasks();

		if (keystatus[0xa]) setaspect(viewingrange+(viewingrange>>8),yxaspect+(yxaspect>>8));
		if (keystatus[0xb]) setaspect(viewingrange-(viewingrange>>8),yxaspect-(yxaspect>>8));
		if (keystatus[0xc]) setaspect(viewingrange,yxaspect-(yxaspect>>8));
		if (keystatus[0xd]) setaspect(viewingrange,yxaspect+(yxaspect>>8));

		editinput();

		daang += (keystatus[0x6]-keystatus[0x7])*16;
		if (keystatus[0x2]) rotatesprite(xdim<<15,ydim<<15,65536L,daang,75,0,0,8+64,0L,0L,xdim-1L,ydim-1L);
		if (keystatus[0x3]) rotatesprite(xdim<<15,ydim<<15,65536L,daang,75,0,0,8,0L,0L,xdim-1L,ydim-1L);
		if (keystatus[0x4]) rotatesprite(xdim<<15,ydim<<15,65536L,daang,75,0,0,1+8,0L,0L,xdim-1L,ydim-1L);
		if (keystatus[0x5]) rotatesprite(xdim<<15,ydim<<15,65536L,daang,75,0,0,1+8+32,0L,0L,xdim-1L,ydim-1L);

		nextpage();
		synctics = totalclock-lockclock;
		lockclock += synctics;
	}
	uninittimer();
	uninitkeys();
	uninitgroupfile();
	uninitengine();
	setvmode(0x3);

	return(0);
}

editinput()
{
	long i, j, k, cnt, templong, doubvel;
	long goalz, xvect, yvect, hiz, loz;
	long dax, day, hihit, lohit;

	if (keystatus[0x57] > 0)  //F11 - brightness
	{
		keystatus[0x57] = 0;
		brightness++;
		if (brightness > 16) brightness = 0;
		setbrightness(brightness,palette);
	}

	if (keystatus[0x3b] > 0) posx--;
	if (keystatus[0x3c] > 0) posx++;
	if (keystatus[0x3d] > 0) posy--;
	if (keystatus[0x3e] > 0) posy++;
	if (keystatus[0x43] > 0) ang--;
	if (keystatus[0x44] > 0) ang++;

	if (angvel != 0)          //ang += angvel * constant
	{                         //ENGINE calculates angvel for you
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)  //Lt. shift makes turn velocity 50% faster
			doubvel += (synctics>>1);
		ang += ((angvel*doubvel)>>4);
		ang = (ang+2048)&2047;
	}
	if ((vel|svel) != 0)
	{
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)     //Lt. shift doubles forward velocity
			doubvel += synctics;
		xvect = 0, yvect = 0;
		if (vel != 0)
		{
			xvect += ((vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
			yvect += ((vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
		}
		if (svel != 0)
		{
			xvect += ((svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			yvect += ((svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
		}
		clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
	}
	getzrange(posx,posy,posz,cursectnum,&hiz,&hihit,&loz,&lohit,128L,CLIPMASK0);

	goalz = loz-(32<<8);   //playerheight pixels above floor
	if (goalz < hiz+(16<<8))   //ceiling&floor too close
		goalz = ((loz+hiz)>>1);
	if (keystatus[buildkeys[8]] > 0)                            //A (stand high)
	{
		if (keystatus[0x1d] > 0)
			horiz = max(-100,horiz-((keystatus[buildkeys[4]]+1)<<2));
		else
		{
			goalz -= (16<<8);
			if (keystatus[buildkeys[4]] > 0)    //Either shift key
				goalz -= (24<<8);
		}
	}
	if (keystatus[buildkeys[9]] > 0)                            //Z (stand low)
	{
		if (keystatus[0x1d] > 0)
			horiz = min(300,horiz+((keystatus[buildkeys[4]]+1)<<2));
		else
		{
			goalz += (12<<8);
			if (keystatus[buildkeys[4]] > 0)    //Either shift key
				goalz += (12<<8);
		}
	}

	if (goalz != posz)
	{
		if (posz < goalz) hvel += 32;
		if (posz > goalz) hvel = ((goalz-posz)>>3);

		posz += hvel;
		if (posz > loz-(4<<8)) posz = loz-(4<<8), hvel = 0;
		if (posz < hiz+(4<<8)) posz = hiz+(4<<8), hvel = 0;
	}
}

inittimer()
{
	outp(0x43,0x34); outp(0x40,(1193181/120)&255); outp(0x40,(1193181/120)>>8);
	oldtimerhandler = _dos_getvect(0x8);
	_disable(); _dos_setvect(0x8, timerhandler); _enable();
}

uninittimer()
{
	outp(0x43,0x34); outp(0x40,0); outp(0x40,0);           //18.2 times/sec
	_disable(); _dos_setvect(0x8, oldtimerhandler); _enable();
}

void __interrupt __far timerhandler()
{
	totalclock++;
	keytimerstuff();
	outp(0x20,0x20);
}

initkeys()
{
	long i;

	keyfifoplc = 0; keyfifoend = 0;
	for(i=0;i<256;i++) keystatus[i] = 0;
	oldkeyhandler = _dos_getvect(0x9);
	_disable(); _dos_setvect(0x9, keyhandler); _enable();
}

uninitkeys()
{
	short *ptr;

	_dos_setvect(0x9, oldkeyhandler);
		//Turn off shifts to prevent stucks with quitting
	ptr = (short *)0x417; *ptr &= ~0x030f;
}

void __interrupt __far keyhandler()
{
	oldreadch = readch; readch = kinp(0x60);
	keytemp = kinp(0x61); koutp(0x61,keytemp|128); koutp(0x61,keytemp&127);
	koutp(0x20,0x20);
	if ((readch|1) == 0xe1) { extended = 128; return; }
	if (oldreadch != readch)
	{
		if ((readch&128) == 0)
		{
			keytemp = readch+extended;
			if (keystatus[keytemp] == 0)
			{
				keystatus[keytemp] = 1;
				keyfifo[keyfifoend] = keytemp;
				keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 1;
				keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
			}
		}
		else
		{
			keytemp = (readch&127)+extended;
			keystatus[keytemp] = 0;
			keyfifo[keyfifoend] = keytemp;
			keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 0;
			keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
		}
	}
	extended = 0;
}

keytimerstuff()
{
	if (keystatus[buildkeys[5]] == 0)
	{
		if (keystatus[buildkeys[2]] > 0) angvel = max(angvel-16,-128);
		if (keystatus[buildkeys[3]] > 0) angvel = min(angvel+16,127);
	}
	else
	{
		if (keystatus[buildkeys[2]] > 0) svel = min(svel+8,127);
		if (keystatus[buildkeys[3]] > 0) svel = max(svel-8,-128);
	}
	if (keystatus[buildkeys[0]] > 0) vel = min(vel+8,127);
	if (keystatus[buildkeys[1]] > 0) vel = max(vel-8,-128);
	if (keystatus[buildkeys[12]] > 0) svel = min(svel+8,127);
	if (keystatus[buildkeys[13]] > 0) svel = max(svel-8,-128);

	if (angvel < 0) angvel = min(angvel+12,0);
	if (angvel > 0) angvel = max(angvel-12,0);
	if (svel < 0) svel = min(svel+2,0);
	if (svel > 0) svel = max(svel-2,0);
	if (vel < 0) vel = min(vel+2,0);
	if (vel > 0) vel = max(vel-2,0);
}

faketimerhandler()
{
}
