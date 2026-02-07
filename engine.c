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

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dos.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "compiler.h"
#include "draw.h"
#include "engine.h"
#include "filesystem.h"
#include "p_setup.h"
#include "palette.h"
#include "tables.h"
#include "test.h"
#include "tiles.h"
#include "z_zone.h"


static int32_t _a_gbxinc, _a_gbyinc, _a_gpinc;
static uint8_t __far* _a_ghlinepal;

static uint8_t parallaxtype, showinvisibility;
static int32_t parallaxyoffs, parallaxyscale;
static int32_t visibility, parallaxvisibility;

static int16_t __far headspritestat[MAXSTATUS+1];
static int16_t __far prevspritesect[MAXSPRITES];
static int16_t __far prevspritestat[MAXSPRITES];
static int16_t __far nextspritestat[MAXSPRITES];

static uint8_t __far gotsector[(MAXSECTORS+7)>>3];


static int32_t scale(int32_t a, int32_t b, int32_t c)
{
	return ((int64_t)a * b) / c;
}


static int32_t mulscale(int32_t a, int32_t b, int32_t c)
{
	return ((int64_t)a * b) >> c;
}

#define mulscale1(a,b) mulscale((a),(b),1)
#define mulscale2(a,b) mulscale((a),(b),2)
#define mulscale4(a,b) mulscale((a),(b),4)
#define mulscale5(a,b) mulscale((a),(b),5)
#define mulscale8(a,b) mulscale((a),(b),8)
#define mulscale9(a,b) mulscale((a),(b),9)
#define mulscale10(a,b) mulscale((a),(b),10)
#define mulscale11(a,b) mulscale((a),(b),11)
#define mulscale12(a,b) mulscale((a),(b),12)
#define mulscale13(a,b) mulscale((a),(b),13)
#define mulscale14(a,b) mulscale((a),(b),14)
#define mulscale15(a,b) mulscale((a),(b),15)
#define mulscale16(a,b) mulscale((a),(b),16)
#define mulscale18(a,b) mulscale((a),(b),18)
#define mulscale19(a,b) mulscale((a),(b),19)
#define mulscale20(a,b) mulscale((a),(b),20)
#define mulscale21(a,b) mulscale((a),(b),21)
#define mulscale23(a,b) mulscale((a),(b),23)
#define mulscale24(a,b) mulscale((a),(b),24)
#define mulscale27(a,b) mulscale((a),(b),27)
#define mulscale28(a,b) mulscale((a),(b),28)
#define mulscale30(a,b) mulscale((a),(b),30)
#define mulscale31(a,b) mulscale((a),(b),31)
#define mulscale32(a,b) mulscale((a),(b),32)


static int32_t dmulscale(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e)
{
	return (((int64_t)a * b) + ((int64_t)c * d)) >> e;
}

#define dmulscale2(a,b,c,d) dmulscale((a),(b),(c),(d),2)
#define dmulscale3(a,b,c,d) dmulscale((a),(b),(c),(d),3)
#define dmulscale6(a,b,c,d) dmulscale((a),(b),(c),(d),6)
#define dmulscale8(a,b,c,d) dmulscale((a),(b),(c),(d),8)
#define dmulscale9(a,b,c,d) dmulscale((a),(b),(c),(d),9)
#define dmulscale10(a,b,c,d) dmulscale((a),(b),(c),(d),10)
#define dmulscale12(a,b,c,d) dmulscale((a),(b),(c),(d),12)
#define dmulscale14(a,b,c,d) dmulscale((a),(b),(c),(d),14)
#define dmulscale16(a,b,c,d) dmulscale((a),(b),(c),(d),16)
#define dmulscale17(a,b,c,d) dmulscale((a),(b),(c),(d),17)
#define dmulscale24(a,b,c,d) dmulscale((a),(b),(c),(d),24)
#define dmulscale25(a,b,c,d) dmulscale((a),(b),(c),(d),25)
#define dmulscale32(a,b,c,d) dmulscale((a),(b),(c),(d),32)


static int32_t divscale(int32_t a, int32_t b, int32_t c)
{
	return ((int64_t)a << c) / b;
}

#define divscale12(a,b) divscale((a),(b),12)
#define divscale14(a,b) divscale((a),(b),14)
#define divscale16(a,b) divscale((a),(b),16)
#define divscale18(a,b) divscale((a),(b),18)
#define divscale19(a,b) divscale((a),(b),19)
#define divscale20(a,b) divscale((a),(b),20)
#define divscale24(a,b) divscale((a),(b),24)
#define divscale26(a,b) divscale((a),(b),26)
#define divscale28(a,b) divscale((a),(b),28)
#define divscale30(a,b) divscale((a),(b),30)
#define divscale32(a,b) divscale((a),(b),32)


#define klabs labs


static void swaplong(void __far* a, void __far* b)
{
	int32_t __far* pa = a;
	int32_t __far* pb = b;
	int32_t tmp = *pa;
	*pa = *pb;
	*pb = tmp;
}


#define MAXPERMS 512


static int32_t beforedrawrooms = 1;

static int32_t oxdimen = -1, oviewingrange = -1, oxyaspect = -1;

static uint8_t tempbuf[MAXWALLS];

static int32_t slopalookup[2048];

static int16_t radarang2[XDIM];
static const uint8_t pow2char[8] = {1,2,4,8,16,32,64,128};

static int16_t p2[MAXWALLSB], thesector[MAXWALLSB];

static int16_t bunchfirst[MAXWALLSB], bunchlast[MAXWALLSB];

static int16_t smostcnt;

static int16_t maskwall[MAXWALLSB], maskwallcnt;

static int32_t spritesz[MAXSPRITESONSCREEN];

static int16_t umost[XDIM], dmost[XDIM];
static int16_t uplc[XDIM], dplc[XDIM];

static int32_t swplc[XDIM], lplc[XDIM];

static int32_t xdimenrecip;

static int32_t horizlookup2[YDIM * 4];

static int16_t globalcursectnum;

static uint8_t __far* globalpalwritten;

static int32_t globalpisibility, globalcisibility;
static uint8_t globparaceilclip, globparaflorclip;

static uint8_t globalxshift, globalyshift;

static int32_t globalzx;
static int32_t globalx3, globaly3;
static int32_t globalx, globaly, globalz;

static int16_t sectorborder[256], sectorbordercnt;
static int16_t pointhighlight, linehighlight, highlightcnt;


typedef struct
{
	int32_t sx, sy, z;
	int16_t a, picnum;
	int8_t dashade;
	uint8_t dapalnum, dastat, pagesleft;
	int32_t cx1, cy1, cx2, cy2;
} permfifotype;
static permfifotype permfifo[MAXPERMS];
static int32_t permhead = 0, permtail = 0;

static int16_t numscans, numhits, numbunches;


	//Simple integer square root  Ex: msqrtasm(81L) = 9L;
static uint32_t msqrtasm(uint32_t c)
{
	uint32_t a, b;

	a = 0x40000000l;
	b = 0x20000000l;
	do
	{
		if (c >= a)
		{
			c -= a;
			a += b*4;
		}

		a -= b;
		a >>= 1;
		b >>= 2;
	} while (b);

	if (c >= a)
		a++;

	a >>= 1;
	return a;
}


static void scansector (int16_t sectnum);
static int32_t bunchfront (int32_t b1, int32_t b2);
static void drawalls (int32_t bunch);
static void prepwall(int32_t z, walltype __far* wal);
static void ceilscan (int32_t x1, int32_t x2, int32_t sectnum);
static void florscan (int32_t x1, int32_t x2, int32_t sectnum);
static void wallscan(int32_t x1, int32_t x2, int16_t __far* uwal, int16_t __far* dwal, int32_t __far* swal, int32_t __far* lwal, int16_t globalpicnum);
static void hline(int32_t xr, int32_t yp, uint8_t __far* _a_gbuf);
static void drawmaskwall(int16_t damaskwallcnt);
static void setview(void);
static void dosetaspect(void);
static void dorotatesprite (int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, uint8_t dapalnum, uint8_t dastat, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2);
static int32_t clippoly4(int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2);
static void clearallviews(int32_t dacol);
static void getzsofslope(int16_t sectnum, int32_t dax, int32_t day, int32_t *ceilz, int32_t *florz);
static uint8_t wallmost(int16_t __far* mostbuf, int32_t w, int32_t sectnum, uint8_t dastat);
static void grouscan (int32_t dax1, int32_t dax2, int32_t sectnum, uint8_t dastat);
static void parascan (int32_t sectnum, uint8_t dastat, int32_t bunch);


void drawrooms(int32_t daposx, int32_t daposy, int32_t daposz,
			 int16_t daang, int16_t dahoriz, int16_t dacursectnum)
{
	int32_t i, j, cz, fz, closest;
	int16_t __far* shortptr1;
	int16_t __far* shortptr2;

	beforedrawrooms = 0;

	globalposx = daposx; globalposy = daposy; globalposz = daposz;
	globalang = (daang&2047);

	globalhoriz = mulscale16((uint32_t)dahoriz - 100, xdimenscale) + (YDIM >> 1);
	globaluclip = (0-globalhoriz)*xdimscale;
	globaldclip = (YDIM-globalhoriz)*xdimscale;

	i = mulscale16(xdimenscale,viewingrangerecip);
	globalpisibility = mulscale16(parallaxvisibility,i);
	globalvisibility = mulscale16(visibility,i);
	globalhisibility = mulscale16(globalvisibility,xyaspect);
	globalcisibility = mulscale8(globalhisibility,XDIM);

	globalcursectnum = dacursectnum;
	setTotalclocklock();

	cosglobalang = sintable[(globalang+512)&2047];
	singlobalang = sintable[globalang&2047];
	cosviewingrangeglobalang = mulscale16(cosglobalang,viewingrange);
	sinviewingrangeglobalang = mulscale16(singlobalang,viewingrange);

	if ((xyaspect != oxyaspect) || (XDIM != oxdimen) || (viewingrange != oviewingrange))
		dosetaspect();

	_fmemset(gotsector, 0, ((numsectors + 7) >> 3));

	shortptr1 = (int16_t __far*)&startumost[0];
	shortptr2 = (int16_t __far*)&startdmost[0];
	i = XDIM-1;
	do
	{
		umost[i] = shortptr1[i];
		dmost[i] = shortptr2[i];
		i--;
	} while (i != 0);
	umost[0] = shortptr1[0];
	dmost[0] = shortptr2[0];

	numhits = XDIM; numscans = 0; numbunches = 0;
	maskwallcnt = 0; smostwallcnt = 0; smostcnt = 0; spritesortcnt = 0;

	if (globalcursectnum >= MAXSECTORS)
		globalcursectnum -= MAXSECTORS;
	else
	{
		i = globalcursectnum;
		updatesector(globalposx,globalposy,&globalcursectnum);
		if (globalcursectnum < 0) globalcursectnum = i;
	}

	globparaceilclip = 1;
	globparaflorclip = 1;
	getzsofslope(globalcursectnum,globalposx,globalposy,&cz,&fz);
	if (globalposz < cz) globparaceilclip = 0;
	if (globalposz > fz) globparaflorclip = 0;

	scansector(globalcursectnum);

	while ((numbunches > 0) && (numhits > 0))
	{
		memset(&tempbuf[0],0,((numbunches+3)>>2)*4);
		tempbuf[0] = 1;

		closest = 0;              //Almost works, but not quite :(
		for(i=1;i<numbunches;i++)
		{
			if ((j = bunchfront(i,closest)) < 0) continue;
			tempbuf[i] = 1;
			if (j == 0) tempbuf[closest] = 1, closest = i;
		}
		for(i=0;i<numbunches;i++) //Double-check
		{
			if (tempbuf[i]) continue;
			if ((j = bunchfront(i,closest)) < 0) continue;
			tempbuf[i] = 1;
			if (j == 0) tempbuf[closest] = 1, closest = i, i = 0;
		}

		drawalls(closest);

		numbunches--;
		bunchfirst[closest] = bunchfirst[numbunches];
		bunchlast[closest] = bunchlast[numbunches];
	}
}

static void scansector (int16_t sectnum)
{
	walltype __far* wal;
	walltype __far* wal2;
	spritetype __far* spr;
	int32_t xs, ys, x1, y1, x2, y2, xp1, yp1, templong;
	int32_t xp2 = 0; // shut up compiler warning
	int32_t yp2 = 0; // shut up compiler warning
	int16_t z, zz, startwall, endwall, numscansbefore, scanfirst, bunchfrst;
	int16_t nextsectnum;

	if (sectnum < 0) return;

	sectorborder[0] = sectnum, sectorbordercnt = 1;
	do
	{
		sectnum = sectorborder[--sectorbordercnt];

		for(z=headspritesect[sectnum];z>=0;z=nextspritesect[z])
		{
			spr = &sprite[z];
			if ((((spr->cstat&0x8000) == 0) || (showinvisibility)) &&
				  (spr->xrepeat > 0) && (spr->yrepeat > 0) &&
				  (spritesortcnt < MAXSPRITESONSCREEN))
			{
				xs = spr->x-globalposx; ys = spr->y-globalposy;
				if ((spr->cstat&48) || (xs*cosglobalang+ys*singlobalang > 0))
				{
					_fmemcpy(&tsprite[spritesortcnt], spr, sizeof(spritetype));
					tsprite[spritesortcnt++].owner = z;
				}
			}
		}

		gotsector[sectnum>>3] |= pow2char[sectnum&7];

		bunchfrst = numbunches;
		numscansbefore = numscans;

		startwall = sector[sectnum].wallptr;
		endwall = startwall + sector[sectnum].wallnum;
		scanfirst = numscans;
		for(z=startwall,wal=&wall[z];z<endwall;z++,wal++)
		{
			nextsectnum = wal->nextsector;

			wal2 = &wall[wal->point2];
			x1 = wal->x-globalposx; y1 = wal->y-globalposy;
			x2 = wal2->x-globalposx; y2 = wal2->y-globalposy;

			if ((nextsectnum >= 0) && ((wal->cstat&32) == 0))
				if ((gotsector[nextsectnum>>3]&pow2char[nextsectnum&7]) == 0)
				{
					templong = x1*y2-x2*y1;
					if (((uint32_t)templong+262144) < 524288)
						if (mulscale5(templong,templong) <= (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))
							sectorborder[sectorbordercnt++] = nextsectnum;
				}

			if ((z == startwall) || (wall[z-1].point2 != z))
			{
				xp1 = dmulscale6(y1,cosglobalang,-x1,singlobalang);
				yp1 = dmulscale6(x1,cosviewingrangeglobalang,y1,sinviewingrangeglobalang);
			}
			else
			{
				xp1 = xp2;
				yp1 = yp2;
			}
			xp2 = dmulscale6(y2,cosglobalang,-x2,singlobalang);
			yp2 = dmulscale6(x2,cosviewingrangeglobalang,y2,sinviewingrangeglobalang);
			if ((yp1 < 256) && (yp2 < 256)) goto skipitaddwall;

				//If wall's NOT facing you
			if (dmulscale32(xp1,yp2,-xp2,yp1) >= 0) goto skipitaddwall;

			if (xp1 >= -yp1)
			{
				if ((xp1 > yp1) || (yp1 == 0)) goto skipitaddwall;
				xb1[numscans] = (XDIM>>1) + scale(xp1,(XDIM>>1),yp1);
				if (xp1 >= 0) xb1[numscans]++;   //Fix for SIGNED divide
				if (xb1[numscans] >= XDIM) xb1[numscans] = XDIM-1;
				yb1[numscans] = yp1;
			}
			else
			{
				if (xp2 < -yp2) goto skipitaddwall;
				xb1[numscans] = 0;
				templong = yp1-yp2+xp1-xp2;
				if (templong == 0) goto skipitaddwall;
				yb1[numscans] = yp1 + scale(yp2-yp1,xp1+yp1,templong);
			}
			if (yb1[numscans] < 256) goto skipitaddwall;

			if (xp2 <= yp2)
			{
				if ((xp2 < -yp2) || (yp2 == 0)) goto skipitaddwall;
				xb2[numscans] = (XDIM>>1) + scale(xp2,(XDIM>>1),yp2) - 1;
				if (xp2 >= 0) xb2[numscans]++;   //Fix for SIGNED divide
				if (xb2[numscans] >= XDIM) xb2[numscans] = XDIM-1;
				yb2[numscans] = yp2;
			}
			else
			{
				if (xp1 > yp1) goto skipitaddwall;
				xb2[numscans] = XDIM-1;
				templong = xp2-xp1+yp1-yp2;
				if (templong == 0) goto skipitaddwall;
				yb2[numscans] = yp1 + scale(yp2-yp1,yp1-xp1,templong);
			}
			if ((yb2[numscans] < 256) || (xb1[numscans] > xb2[numscans])) goto skipitaddwall;

				//Made it all the way!
			thesector[numscans] = sectnum; thewall[numscans] = z;
			rx1[numscans] = xp1; ry1[numscans] = yp1;
			rx2[numscans] = xp2; ry2[numscans] = yp2;
			p2[numscans] = numscans+1;
			numscans++;
skipitaddwall:

			if ((wall[z].point2 < z) && (scanfirst < numscans))
				p2[numscans-1] = scanfirst, scanfirst = numscans;
		}

		for(z=numscansbefore;z<numscans;z++)
			if ((wall[thewall[z]].point2 != thewall[p2[z]]) || (xb2[z] >= xb1[p2[z]]))
				bunchfirst[numbunches++] = p2[z], p2[z] = -1;

		for(z=bunchfrst;z<numbunches;z++)
		{
			for(zz=bunchfirst[z];p2[zz]>=0;zz=p2[zz]);
			bunchlast[z] = zz;
		}
	} while (sectorbordercnt > 0);
}

static int32_t wallfront (int32_t l1, int32_t l2)
{
	walltype __far* wal;
	int32_t x11, y11, x21, y21, x12, y12, x22, y22, dx, dy, t1, t2;

	wal = &wall[thewall[l1]]; x11 = wal->x; y11 = wal->y;
	wal = &wall[wal->point2]; x21 = wal->x; y21 = wal->y;
	wal = &wall[thewall[l2]]; x12 = wal->x; y12 = wal->y;
	wal = &wall[wal->point2]; x22 = wal->x; y22 = wal->y;

	dx = x21-x11; dy = y21-y11;
	t1 = dmulscale2(x12-x11,dy,-dx,y12-y11); //p1(l2) vs. l1
	t2 = dmulscale2(x22-x11,dy,-dx,y22-y11); //p2(l2) vs. l1
	if (t1 == 0) { t1 = t2; if (t1 == 0) return(-1); }
	if (t2 == 0) t2 = t1;
	if ((t1^t2) >= 0)
	{
		t2 = dmulscale2(globalposx-x11,dy,-dx,globalposy-y11); //pos vs. l1
		return((t2^t1) >= 0);
	}

	dx = x22-x12; dy = y22-y12;
	t1 = dmulscale2(x11-x12,dy,-dx,y11-y12); //p1(l1) vs. l2
	t2 = dmulscale2(x21-x12,dy,-dx,y21-y12); //p2(l1) vs. l2
	if (t1 == 0) { t1 = t2; if (t1 == 0) return(-1); }
	if (t2 == 0) t2 = t1;
	if ((t1^t2) >= 0)
	{
		t2 = dmulscale2(globalposx-x12,dy,-dx,globalposy-y12); //pos vs. l2
		return((t2^t1) < 0);
	}
	return(-2);
}


static int32_t bunchfront (int32_t b1, int32_t b2)
{
	int32_t x1b1, x2b1, x1b2, x2b2, b1f, b2f, i;

	b1f = bunchfirst[b1]; x1b1 = xb1[b1f]; x2b2 = xb2[bunchlast[b2]]+1;
	if (x1b1 >= x2b2) return(-1);
	b2f = bunchfirst[b2]; x1b2 = xb1[b2f]; x2b1 = xb2[bunchlast[b1]]+1;
	if (x1b2 >= x2b1) return(-1);

	if (x1b1 >= x1b2)
	{
		for(i=b2f;xb2[i]<x1b1;i=p2[i]);
		return(wallfront(b1f,i));
	}
	for(i=b1f;xb2[i]<x1b2;i=p2[i]);
	return(wallfront(i,b2f));
}

static void drawalls (int32_t bunch)
{
	sectortype __far* sec;
	sectortype __far* nextsec;
	walltype __far* wal;
	int32_t i, x, x1, x2, cz[5], fz[5];
	int32_t z, wallnum, sectnum, nextsectnum;
	int32_t startsmostwallcnt, startsmostcnt, gotswall;
	uint8_t andwstat1, andwstat2;
	int16_t globalpicnum;

	z = bunchfirst[bunch];
	sectnum = thesector[z]; sec = &sector[sectnum];

	andwstat1 = 0xff; andwstat2 = 0xff;
	for(;z>=0;z=p2[z])  //uplc/dplc calculation
	{
		andwstat1 &= wallmost(uplc,z,sectnum,(uint8_t)0);
		andwstat2 &= wallmost(dplc,z,sectnum,(uint8_t)1);
	}

	if ((andwstat1&3) != 3)     //draw ceilings
	{
		if ((sec->ceilingstat&3) == 2)
			grouscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,0);
		else if ((sec->ceilingstat&1) == 0)
			ceilscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum);
		else
			parascan(sectnum,0,bunch);
	}
	if ((andwstat2&12) != 12)   //draw floors
	{
		if ((sec->floorstat&3) == 2)
			grouscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,1);
		else if ((sec->floorstat&1) == 0)
			florscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum);
		else
			parascan(sectnum,1,bunch);
	}

		//DRAW WALLS SECTION!
	for(z=bunchfirst[bunch];z>=0;z=p2[z])
	{
		x1 = xb1[z]; x2 = xb2[z];
		if (umost[x2] >= dmost[x2])
		{
			for(x=x1;x<x2;x++)
				if (umost[x] < dmost[x]) break;
			if (x >= x2)
			{
				smostwall[smostwallcnt] = z;
				smostwalltype[smostwallcnt] = 0;
				smostwallcnt++;
				continue;
			}
		}

		wallnum = thewall[z]; wal = &wall[wallnum];
		nextsectnum = wal->nextsector; nextsec = &sector[nextsectnum];

		gotswall = 0;

		startsmostwallcnt = smostwallcnt;
		startsmostcnt = smostcnt;

		if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
		{
			if (searchy <= uplc[searchx]) //ceiling
			{
				searchsector = sectnum; searchwall = wallnum;
				searchstat = 1; searchit = 1;
			}
			else if (searchy >= dplc[searchx]) //floor
			{
				searchsector = sectnum; searchwall = wallnum;
				searchstat = 2; searchit = 1;
			}
		}

		if (nextsectnum >= 0)
		{
			getzsofslope((int16_t)sectnum,wal->x,wal->y,&cz[0],&fz[0]);
			getzsofslope((int16_t)sectnum,wall[wal->point2].x,wall[wal->point2].y,&cz[1],&fz[1]);
			getzsofslope((int16_t)nextsectnum,wal->x,wal->y,&cz[2],&fz[2]);
			getzsofslope((int16_t)nextsectnum,wall[wal->point2].x,wall[wal->point2].y,&cz[3],&fz[3]);
			getzsofslope((int16_t)nextsectnum,globalposx,globalposy,&cz[4],&fz[4]);

			if ((wal->cstat&48) == 16) maskwall[maskwallcnt++] = z;

			if (((sec->ceilingstat&1) == 0) || ((nextsec->ceilingstat&1) == 0))
			{
				if ((cz[2] <= cz[0]) && (cz[3] <= cz[1]))
				{
					if (globparaceilclip)
						for(x=x1;x<=x2;x++)
							if (uplc[x] > umost[x])
								if (umost[x] <= dmost[x])
								{
									umost[x] = uplc[x];
									if (umost[x] > dmost[x]) numhits--;
								}
				}
				else
				{
					wallmost(dwall,z,nextsectnum,(uint8_t)0);
					if ((cz[2] > fz[0]) || (cz[3] > fz[1]))
						for(i=x1;i<=x2;i++) if (dwall[i] > dplc[i]) dwall[i] = dplc[i];

					if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
						if (searchy <= dwall[searchx]) //wall
						{
							searchsector = sectnum; searchwall = wallnum;
							searchstat = 0; searchit = 1;
						}

					globalorientation = (int32_t)wal->cstat;
					globalpicnum = wal->picnum;
					if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
					globalxpanning = (int32_t)wal->xpanning;
					globalypanning = (int32_t)wal->ypanning;
					globalshiftval = (picsiz[globalpicnum]>>4);
					if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
					globalshiftval = 32-globalshiftval;
					if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
					globalshade = (int32_t)wal->shade;
					globvis = globalvisibility;
					if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
					globalpal = (int32_t)wal->pal;
					globalyscale = (wal->yrepeat<<(globalshiftval-19));
					if ((globalorientation&4) == 0)
						globalzd = (((globalposz-nextsec->ceilingz)*globalyscale)<<8);
					else
						globalzd = (((globalposz-sec->ceilingz)*globalyscale)<<8);
					globalzd += (globalypanning<<24);
					if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

					if (gotswall == 0) { gotswall = 1; prepwall(z,wal); }
					wallscan(x1, x2, uplc, dwall, swall, lwall, globalpicnum);

					if ((cz[2] >= cz[0]) && (cz[3] >= cz[1]))
					{
						for(x=x1;x<=x2;x++)
							if (dwall[x] > umost[x])
								if (umost[x] <= dmost[x])
								{
									umost[x] = dwall[x];
									if (umost[x] > dmost[x]) numhits--;
								}
					}
					else
					{
						for(x=x1;x<=x2;x++)
							if (umost[x] <= dmost[x])
							{
								i = max(uplc[x],dwall[x]);
								if (i > umost[x])
								{
									umost[x] = i;
									if (umost[x] > dmost[x]) numhits--;
								}
							}
					}
				}
				if ((cz[2] < cz[0]) || (cz[3] < cz[1]) || (globalposz < cz[4]))
				{
					i = x2-x1+1;
					if (smostcnt+i < MAXYSAVES)
					{
						smoststart[smostwallcnt] = smostcnt;
						smostwall[smostwallcnt] = z;
						smostwalltype[smostwallcnt] = 1;   //1 for umost
						smostwallcnt++;
						_fmemcpy(&smost[smostcnt], &umost[x1], i * sizeof(smost[0]));
						smostcnt += i;
					}
				}
			}
			if (((sec->floorstat&1) == 0) || ((nextsec->floorstat&1) == 0))
			{
				if ((fz[2] >= fz[0]) && (fz[3] >= fz[1]))
				{
					if (globparaflorclip)
						for(x=x1;x<=x2;x++)
							if (dplc[x] < dmost[x])
								if (umost[x] <= dmost[x])
								{
									dmost[x] = dplc[x];
									if (umost[x] > dmost[x]) numhits--;
								}
				}
				else
				{
					wallmost(uwall,z,nextsectnum,(uint8_t)1);
					if ((fz[2] < cz[0]) || (fz[3] < cz[1]))
						for(i=x1;i<=x2;i++) if (uwall[i] < uplc[i]) uwall[i] = uplc[i];

					if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
						if (searchy >= uwall[searchx]) //wall
						{
							searchsector = sectnum; searchwall = wallnum;
							if ((wal->cstat&2) > 0) searchwall = wal->nextwall;
							searchstat = 0; searchit = 1;
						}

					if ((wal->cstat&2) > 0)
					{
						wallnum = wal->nextwall; wal = &wall[wallnum];
						globalorientation = (int32_t)wal->cstat;
						globalpicnum = wal->picnum;
						if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
						globalxpanning = (int32_t)wal->xpanning;
						globalypanning = (int32_t)wal->ypanning;
						if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
						globalshade = (int32_t)wal->shade;
						globalpal = (int32_t)wal->pal;
						wallnum = thewall[z]; wal = &wall[wallnum];
					}
					else
					{
						globalorientation = (int32_t)wal->cstat;
						globalpicnum = wal->picnum;
						if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
						globalxpanning = (int32_t)wal->xpanning;
						globalypanning = (int32_t)wal->ypanning;
						if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
						globalshade = (int32_t)wal->shade;
						globalpal = (int32_t)wal->pal;
					}
					globvis = globalvisibility;
					if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
					globalshiftval = (picsiz[globalpicnum]>>4);
					if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
					globalshiftval = 32-globalshiftval;
					globalyscale = (wal->yrepeat<<(globalshiftval-19));
					if ((globalorientation&4) == 0)
						globalzd = (((globalposz-nextsec->floorz)*globalyscale)<<8);
					else
						globalzd = (((globalposz-sec->ceilingz)*globalyscale)<<8);
					globalzd += (globalypanning<<24);
					if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

					if (gotswall == 0) { gotswall = 1; prepwall(z,wal); }
					wallscan(x1, x2, uwall, dplc, swall, lwall, globalpicnum);

					if ((fz[2] <= fz[0]) && (fz[3] <= fz[1]))
					{
						for(x=x1;x<=x2;x++)
							if (uwall[x] < dmost[x])
								if (umost[x] <= dmost[x])
								{
									dmost[x] = uwall[x];
									if (umost[x] > dmost[x]) numhits--;
								}
					}
					else
					{
						for(x=x1;x<=x2;x++)
							if (umost[x] <= dmost[x])
							{
								i = min(dplc[x],uwall[x]);
								if (i < dmost[x])
								{
									dmost[x] = i;
									if (umost[x] > dmost[x]) numhits--;
								}
							}
					}
				}
				if ((fz[2] > fz[0]) || (fz[3] > fz[1]) || (globalposz > fz[4]))
				{
					i = x2-x1+1;
					if (smostcnt+i < MAXYSAVES)
					{
						smoststart[smostwallcnt] = smostcnt;
						smostwall[smostwallcnt] = z;
						smostwalltype[smostwallcnt] = 2;   //2 for dmost
						smostwallcnt++;
						_fmemcpy(&smost[smostcnt], &dmost[x1], i * sizeof(smost[0]));
						smostcnt += i;
					}
				}
			}
			if (numhits < 0) return;
			if ((!(wal->cstat&32)) && ((gotsector[nextsectnum>>3]&pow2char[nextsectnum&7]) == 0))
			{
				if (umost[x2] < dmost[x2])
					scansector(nextsectnum);
				else
				{
					for(x=x1;x<x2;x++)
						if (umost[x] < dmost[x])
							{ scansector(nextsectnum); break; }

						//If can't see sector beyond, then cancel smost array and just
						//store wall!
					if (x == x2)
					{
						smostwallcnt = startsmostwallcnt;
						smostcnt = startsmostcnt;
						smostwall[smostwallcnt] = z;
						smostwalltype[smostwallcnt] = 0;
						smostwallcnt++;
					}
				}
			}
		}
		if ((nextsectnum < 0) || (wal->cstat&32))   //White/1-way wall
		{
			globalorientation = (int32_t)wal->cstat;
			if (nextsectnum < 0)
				globalpicnum = wal->picnum;
			else
				globalpicnum = wal->overpicnum;
			if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
			globalxpanning = (int32_t)wal->xpanning;
			globalypanning = (int32_t)wal->ypanning;
			if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
			globalshade = (int32_t)wal->shade;
			globvis = globalvisibility;
			if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
			globalpal = (int32_t)wal->pal;
			globalshiftval = (picsiz[globalpicnum]>>4);
			if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
			globalshiftval = 32-globalshiftval;
			globalyscale = (wal->yrepeat<<(globalshiftval-19));
			if (nextsectnum >= 0)
			{
				if ((globalorientation&4) == 0) globalzd = globalposz-nextsec->ceilingz;
													else globalzd = globalposz-sec->ceilingz;
			}
			else
			{
				if ((globalorientation&4) == 0) globalzd = globalposz-sec->ceilingz;
													else globalzd = globalposz-sec->floorz;
			}
			globalzd = ((globalzd*globalyscale)<<8) + (globalypanning<<24);
			if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

			if (gotswall == 0) { gotswall = 1; prepwall(z,wal); }
			wallscan(x1, x2, uplc, dplc, swall, lwall, globalpicnum);

			for(x=x1;x<=x2;x++)
				if (umost[x] <= dmost[x])
					{ umost[x] = 1; dmost[x] = 0; numhits--; }
			smostwall[smostwallcnt] = z;
			smostwalltype[smostwallcnt] = 0;
			smostwallcnt++;

			if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
			{
				searchit = 1; searchsector = sectnum; searchwall = wallnum;
				if (nextsectnum < 0) searchstat = 0; else searchstat = 4;
			}
		}
	}
}

static void prepwall(int32_t z, walltype __far* wal)
{
	int32_t i, splc, sinc, x, topinc, top, botinc, bot, walxrepeat;
	int32_t  l = 0; // shut up compiler warning
	int32_t ol = 0; // shut up compiler warning

	walxrepeat = (wal->xrepeat<<3);

		//lwall calculation
	i = xb1[z]-(XDIM>>1);
	topinc = -(ry1[z]>>2);
	botinc = ((ry2[z]-ry1[z])>>8);
	top = mulscale5(rx1[z],XDIM)+mulscale2(topinc,i);
	bot = mulscale11(rx1[z]-rx2[z],XDIM)+mulscale2(botinc,i);

	splc = mulscale19(ry1[z],xdimscale);
	sinc = mulscale16(ry2[z]-ry1[z],xdimscale);

	x = xb1[z];
	if (bot != 0)
	{
		l = divscale12(top,bot);
		swall[x] = mulscale21(l,sinc)+splc;
		l *= walxrepeat;
		lwall[x] = (l>>18);
	}
	while (x+4 <= xb2[z])
	{
		top += topinc; bot += botinc;
		if (bot != 0)
		{
			ol = l; l = divscale12(top,bot);
			swall[x+4] = mulscale21(l,sinc)+splc;
			l *= walxrepeat;
			lwall[x+4] = (l>>18);
		}
		i = ((ol+l)>>1);
		lwall[x+2] = (i>>18);
		lwall[x+1] = ((ol+i)>>19);
		lwall[x+3] = ((l+i)>>19);
		swall[x+2] = ((swall[x]+swall[x+4])>>1);
		swall[x+1] = ((swall[x]+swall[x+2])>>1);
		swall[x+3] = ((swall[x+4]+swall[x+2])>>1);
		x += 4;
	}
	if (x+2 <= xb2[z])
	{
		top += (topinc>>1); bot += (botinc>>1);
		if (bot != 0)
		{
			ol = l; l = divscale12(top,bot);
			swall[x+2] = mulscale21(l,sinc)+splc;
			l *= walxrepeat;
			lwall[x+2] = (l>>18);
		}
		lwall[x+1] = ((l+ol)>>19);
		swall[x+1] = ((swall[x]+swall[x+2])>>1);
		x += 2;
	}
	if (x+1 <= xb2[z])
	{
		bot += (botinc>>2);
		if (bot != 0)
		{
			l = divscale12(top+(topinc>>2),bot);
			swall[x+1] = mulscale21(l,sinc)+splc;
			lwall[x+1] = mulscale18(l,walxrepeat);
		}
	}

	if (lwall[xb1[z]] < 0) lwall[xb1[z]] = 0;
	if ((lwall[xb2[z]] >= walxrepeat) && (walxrepeat)) lwall[xb2[z]] = walxrepeat-1;
	if (wal->cstat&8)
	{
		walxrepeat--;
		for(x=xb1[z];x<=xb2[z];x++) lwall[x] = walxrepeat-lwall[x];
	}
}


static void ceilscan (int32_t x1, int32_t x2, int32_t sectnum)
{
	int32_t i, j, ox, oy, x, y1, y2, twall, bwall;
	sectortype __far* sec;
	int16_t globalpicnum;
	uint8_t __far* bufplc;

	sec = &sector[sectnum];
	if (palookup[sec->ceilingpal] != globalpalwritten)
	{
		globalpalwritten = palookup[sec->ceilingpal];
		_a_ghlinepal = globalpalwritten;
	}

	globalzd = sec->ceilingz-globalposz;
	if (globalzd > 0)
		return;

	globalpicnum = sec->ceilingpicnum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) return;
	if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);

	globalshade = (int32_t)sec->ceilingshade;
	globvis = globalcisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globalorientation = (int32_t)sec->ceilingstat;


	if ((globalorientation&64) == 0)
	{
		globalx1 = singlobalang; globalx2 = singlobalang;
		globaly1 = cosglobalang; globaly2 = cosglobalang;
		globalxpanning = (globalposx<<20);
		globalypanning = -(globalposy<<20);
	}
	else
	{
		j = sec->wallptr;
		ox = wall[wall[j].point2].x - wall[j].x;
		oy = wall[wall[j].point2].y - wall[j].y;
		i = msqrtasm(ox*ox+oy*oy); if (i == 0) i = 1024; else i = 1048576/i;
		globalx1 = mulscale10(dmulscale10(ox,singlobalang,-oy,cosglobalang),i);
		globaly1 = mulscale10(dmulscale10(ox,cosglobalang,oy,singlobalang),i);
		globalx2 = -globalx1;
		globaly2 = -globaly1;

		ox = ((wall[j].x-globalposx)<<6); oy = ((wall[j].y-globalposy)<<6);
		i = dmulscale14(oy,cosglobalang,-ox,singlobalang);
		j = dmulscale14(ox,cosglobalang,oy,singlobalang);
		ox = i; oy = j;
		globalxpanning = globalx1*ox - globaly1*oy;
		globalypanning = globaly2*ox + globalx2*oy;
	}
	globalx2 = mulscale16(globalx2,viewingrangerecip);
	globaly1 = mulscale16(globaly1,viewingrangerecip);
	globalxshift = (8-(picsiz[globalpicnum]&15));
	globalyshift = (8-(picsiz[globalpicnum]>>4));
	if (globalorientation&8) { globalxshift++; globalyshift++; }

	if ((globalorientation&0x4) > 0)
	{
		i = globalxpanning; globalxpanning = globalypanning; globalypanning = i;
		i = globalx2; globalx2 = -globaly1; globaly1 = -i;
		i = globalx1; globalx1 = globaly2; globaly2 = i;
	}
	if ((globalorientation&0x10) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalxpanning = -globalxpanning;
	if ((globalorientation&0x20) > 0) globalx2 = -globalx2, globaly2 = -globaly2, globalypanning = -globalypanning;
	globalx1 <<= globalxshift; globaly1 <<= globalxshift;
	globalx2 <<= globalyshift;  globaly2 <<= globalyshift;
	globalxpanning <<= globalxshift; globalypanning <<= globalyshift;
	globalxpanning += (((int32_t)sec->ceilingxpanning)<<24);
	globalypanning += (((int32_t)sec->ceilingypanning)<<24);
	globaly1 = (-globalx1-globaly1)*(XDIM>>1);
	globalx2 = (globalx2-globaly2)*(XDIM>>1);

	_a_glogx = picsiz[globalpicnum] & 15;
	_a_glogy = picsiz[globalpicnum] >> 4;

	bufplc = loadtile(globalpicnum);

	globalx2 += globaly2*(x1-1);
	globaly1 += globalx1*(x1-1);
	globalx1 = mulscale16(globalx1,globalzd);
	globalx2 = mulscale16(globalx2,globalzd);
	globaly1 = mulscale16(globaly1,globalzd);
	globaly2 = mulscale16(globaly2,globalzd);
	globvis = klabs(mulscale10(globvis,globalzd));

	y1 = umost[x1]; y2 = y1;
	for(x=x1;x<=x2;x++)
	{
		twall = umost[x]-1; bwall = min(uplc[x],dmost[x]);
		if (twall < bwall-1)
		{
			if (twall >= y2)
			{
				while (y1 < y2-1)
					hline(x - 1, ++y1, bufplc);

				y1 = twall;
			}
			else
			{
				while (y1 < twall)
					hline(x - 1, ++y1, bufplc);

				while (y1 > twall)
					lastx[y1--] = x;
			}
			while (y2 > bwall)
				hline(x - 1, --y2, bufplc);

			while (y2 < bwall)
				lastx[y2++] = x;
		}
		else
		{
			while (y1 < y2-1)
				hline(x - 1, ++y1, bufplc);

			if (x == x2)
			{
				globalx2 += globaly2;
				globaly1 += globalx1;
				break;
			}
			y1 = umost[x+1]; y2 = y1;
		}
		globalx2 += globaly2; globaly1 += globalx1;
	}

	while (y1 < y2-1)
		hline(x2, ++y1, bufplc);

	Z_ChangeTagToCache(bufplc);

	faketimerhandler();
}

static void florscan (int32_t x1, int32_t x2, int32_t sectnum)
{
	int32_t i, j, ox, oy, x, y1, y2, twall, bwall;
	sectortype __far* sec;
	int16_t globalpicnum;
	uint8_t __far* bufplc;

	sec = &sector[sectnum];
	if (palookup[sec->floorpal] != globalpalwritten)
	{
		globalpalwritten = palookup[sec->floorpal];
		_a_ghlinepal = globalpalwritten;
	}

	globalzd = globalposz-sec->floorz;
	if (globalzd > 0)
		return;

	globalpicnum = sec->floorpicnum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) return;
	if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);

	globalshade = (int32_t)sec->floorshade;
	globvis = globalcisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globalorientation = (int32_t)sec->floorstat;


	if ((globalorientation&64) == 0)
	{
		globalx1 = singlobalang; globalx2 = singlobalang;
		globaly1 = cosglobalang; globaly2 = cosglobalang;
		globalxpanning = (globalposx<<20);
		globalypanning = -(globalposy<<20);
	}
	else
	{
		j = sec->wallptr;
		ox = wall[wall[j].point2].x - wall[j].x;
		oy = wall[wall[j].point2].y - wall[j].y;
		i = msqrtasm(ox*ox+oy*oy); if (i == 0) i = 1024; else i = 1048576/i;
		globalx1 = mulscale10(dmulscale10(ox,singlobalang,-oy,cosglobalang),i);
		globaly1 = mulscale10(dmulscale10(ox,cosglobalang,oy,singlobalang),i);
		globalx2 = -globalx1;
		globaly2 = -globaly1;

		ox = ((wall[j].x-globalposx)<<6); oy = ((wall[j].y-globalposy)<<6);
		i = dmulscale14(oy,cosglobalang,-ox,singlobalang);
		j = dmulscale14(ox,cosglobalang,oy,singlobalang);
		ox = i; oy = j;
		globalxpanning = globalx1*ox - globaly1*oy;
		globalypanning = globaly2*ox + globalx2*oy;
	}
	globalx2 = mulscale16(globalx2,viewingrangerecip);
	globaly1 = mulscale16(globaly1,viewingrangerecip);
	globalxshift = (8-(picsiz[globalpicnum]&15));
	globalyshift = (8-(picsiz[globalpicnum]>>4));
	if (globalorientation&8) { globalxshift++; globalyshift++; }

	if ((globalorientation&0x4) > 0)
	{
		i = globalxpanning; globalxpanning = globalypanning; globalypanning = i;
		i = globalx2; globalx2 = -globaly1; globaly1 = -i;
		i = globalx1; globalx1 = globaly2; globaly2 = i;
	}
	if ((globalorientation&0x10) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalxpanning = -globalxpanning;
	if ((globalorientation&0x20) > 0) globalx2 = -globalx2, globaly2 = -globaly2, globalypanning = -globalypanning;
	globalx1 <<= globalxshift; globaly1 <<= globalxshift;
	globalx2 <<= globalyshift;  globaly2 <<= globalyshift;
	globalxpanning <<= globalxshift; globalypanning <<= globalyshift;
	globalxpanning += (((int32_t)sec->floorxpanning)<<24);
	globalypanning += (((int32_t)sec->floorypanning)<<24);
	globaly1 = (-globalx1-globaly1)*(XDIM>>1);
	globalx2 = (globalx2-globaly2)*(XDIM>>1);

	_a_glogx = picsiz[globalpicnum] & 15;
	_a_glogy = picsiz[globalpicnum] >> 4;

	bufplc = loadtile(globalpicnum);

	globalx2 += globaly2*(x1-1);
	globaly1 += globalx1*(x1-1);
	globalx1 = mulscale16(globalx1,globalzd);
	globalx2 = mulscale16(globalx2,globalzd);
	globaly1 = mulscale16(globaly1,globalzd);
	globaly2 = mulscale16(globaly2,globalzd);
	globvis = klabs(mulscale10(globvis,globalzd));

	y1 = max(dplc[x1],umost[x1]); y2 = y1;
	for(x=x1;x<=x2;x++)
	{
		twall = max(dplc[x],umost[x])-1; bwall = dmost[x];
		if (twall < bwall-1)
		{
			if (twall >= y2)
			{
				while (y1 < y2-1)
					hline(x - 1, ++y1, bufplc);

				y1 = twall;
			}
			else
			{
				while (y1 < twall)
					hline(x - 1, ++y1, bufplc);

				while (y1 > twall)
					lastx[y1--] = x;
			}
			while (y2 > bwall)
				hline(x - 1, --y2, bufplc);

			while (y2 < bwall)
				lastx[y2++] = x;
		}
		else
		{
			while (y1 < y2-1)
				hline(x - 1, ++y1, bufplc);

			if (x == x2)
			{
				globalx2 += globaly2;
				globaly1 += globalx1;
				break;
			}
			y1 = max(dplc[x+1],umost[x+1]); y2 = y1;
		}
		globalx2 += globaly2; globaly1 += globalx1;
	}

	while (y1 < y2-1)
		hline(x2, ++y1, bufplc);

	Z_ChangeTagToCache(bufplc);

	faketimerhandler();
}


	//Wall,face sprite/wall sprite vertical line function
static void a_vlineasm1flat(int32_t cnt, uint8_t globalpicnumchar, uint8_t __far* p)
{
	for(; cnt >= 0; cnt--)
	{
		*p = globalpicnumchar;
		p += XDIM;
	}
}


	//Wall,face sprite/wall sprite vertical line function
static void a_vlineasm1(int32_t vinc, uint8_t __far* paloffs, int32_t cnt, uint32_t vplc, uint8_t __far* bufplc, uint8_t __far* p)
{
	for(;cnt>=0;cnt--)
	{
		*p = paloffs[bufplc[vplc>>_a_glogy]];
		p += XDIM;
		vplc += vinc;
	}
}


static void wallscan(int32_t x1, int32_t x2, int16_t __far* uwal, int16_t __far* dwal, int32_t __far* swal, int32_t __far* lwal, int16_t globalpicnum)
{
	int32_t x, xnice, ynice;
	uint8_t __far* fpalookup;
	int32_t y1ve, y2ve, tsizx, tsizy;
	uint8_t __far* palookupoffse;
	uint8_t __far* bufplc;
	int32_t bufplce;
	int32_t vince;
	int32_t vplce;

	tsizx = tilesizx[globalpicnum];
	tsizy = tilesizy[globalpicnum];
	if ((tsizx <= 0) || (tsizy <= 0)) return;
	if ((uwal[x1] > YDIM) && (uwal[x2] > YDIM)) return;
	if ((dwal[x1] < 0) && (dwal[x2] < 0)) return;

	bufplc = tryloadtile(globalpicnum);
	if (bufplc == NULL)
	{
		for (x = x1; x <= x2; x++)
		{
			y1ve = max(uwal[x], umost[x]);
			y2ve = min(dwal[x], dmost[x]);
			if (y2ve <= y1ve)
				continue;

			a_vlineasm1flat(y2ve - y1ve - 1, (uint8_t)globalpicnum, x + _s_screen + ylookup[y1ve]);
		}
	}
	else
	{
		xnice = (pow2long[picsiz[globalpicnum] & 15] == tsizx);
		if (xnice)
			tsizx--;

		ynice = (pow2long[picsiz[globalpicnum] >> 4] == tsizy);
		if (ynice)
			tsizy = (picsiz[globalpicnum] >> 4);

		fpalookup = palookup[globalpal];

		_a_glogy = globalshiftval;

		for (x = x1; x <= x2; x++)
		{
			y1ve = max(uwal[x], umost[x]);
			y2ve = min(dwal[x], dmost[x]);
			if (y2ve <= y1ve)
				continue;

			palookupoffse = fpalookup + (getpalookup(mulscale16(swal[x], globvis), globalshade) << 8);

			bufplce = lwal[x] + globalxpanning;
			if (bufplce >= tsizx)
			{
				if (xnice == 0)
					bufplce %= tsizx;
				else
					bufplce &= tsizx;
			}

			if (ynice == 0)
				bufplce *= tsizy;
			else
				bufplce <<= tsizy;

			vince = swal[x] * globalyscale;
			vplce = globalzd + vince * (y1ve - globalhoriz + 1);

			a_vlineasm1(vince, palookupoffse, y2ve - y1ve - 1, vplce, bufplce + bufplc, x + _s_screen + ylookup[y1ve]);
		}

		Z_ChangeTagToCache(bufplc);
	}

	faketimerhandler();
}


void setvmode(int32_t a)
{
	union REGS regs;
	regs.w.ax = a;
	int86(0x10, &regs, &regs);
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


	//Ceiling/floor horizontal line function
static void a_hlineasm4(int32_t cnt, int32_t paloffs, uint32_t by, uint32_t bx, uint8_t __far* p, uint8_t __far* _a_gbuf)
{
	uint8_t __far* palptr;

	palptr = &_a_ghlinepal[paloffs];

	_a_gbxinc = asm1;
	_a_gbyinc = asm2;

	for(;cnt >=0 ; cnt--)
	{
		*p = palptr[_a_gbuf[((bx >> (32 - _a_glogx)) << _a_glogy) + (by >> (32 - _a_glogy))]];
		bx -= _a_gbxinc;
		by -= _a_gbyinc;
		p--;
	}
}


static void hline(int32_t xr, int32_t yp, uint8_t __far* _a_gbuf)
{
	int32_t xl, r;

	xl = lastx[yp]; if (xl > xr) return;
	r = horizlookup2[yp-globalhoriz+((YDIM*4)>>1)];
	asm1 = globalx1*r;
	asm2 = globaly2*r;

	a_hlineasm4(xr-xl,getpalookup(mulscale16(r,globvis),globalshade)<<8,
		globalx2*r+globalypanning,globaly1*r+globalxpanning,
		ylookup[yp]+xr+_s_screen, _a_gbuf);
}


void initengine(void)
{
	int32_t i;

	loadtables();

	xyaspect = -1;

	pskyoff[0] = 0;
	pskybits   = 0;

	parallaxtype     = 2;
	parallaxyoffs    = 0L;
	parallaxyscale   = 65536;
	showinvisibility = 0;

	searchit   = 0;
	searchstat = -1;

	for (i = 0; i < MAXPALOOKUPS; i++)
		palookup[i] = NULL;

	pointhighlight = -1;
	linehighlight  = -1;
	highlightcnt   = 0;

	totalclock         = 0;
	visibility         = 512;
	parallaxvisibility = 512;

	loadpalette();

	globalpalwritten = palookup[0];
	globalpal        = 0;
	_a_ghlinepal     = globalpalwritten;
}


void nextpage(void)
{
	int32_t i;
	permfifotype *per;

	for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
	{
		per = &permfifo[i];
		if ((per->pagesleft > 0) && (per->pagesleft <= 1))
			dorotatesprite(per->sx,per->sy,per->z,per->a,per->picnum,
								per->dashade,per->dapalnum,per->dastat,
								per->cx1,per->cy1,per->cx2,per->cy2);
	}

	_fmemcpy(videomemory, _s_screen, (size_t)((int32_t)XDIM * YDIM));

	for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
	{
		per = &permfifo[i];
		if (per->pagesleft >= 130)
			dorotatesprite(per->sx,per->sy,per->z,per->a,per->picnum,
								per->dashade,per->dapalnum,per->dastat,
								per->cx1,per->cy1,per->cx2,per->cy2);

		if (per->pagesleft&127) per->pagesleft--;
		if (((per->pagesleft&127) == 0) && (i == permtail))
			permtail = ((permtail+1)&(MAXPERMS-1));
	}

	faketimerhandler();

	beforedrawrooms = 1;
	numframes++;
}


void drawmasks(void)
{
	int32_t i, j, k, l, gap, xs, ys, xp, yp, yoff, yspan;

	for(i=spritesortcnt-1;i>=0;i--)
		tspriteptr[i] = &tsprite[i];

	for(i=spritesortcnt-1;i>=0;i--)
	{
		xs = tspriteptr[i]->x-globalposx;
		ys = tspriteptr[i]->y-globalposy;
		yp = dmulscale6(xs,cosviewingrangeglobalang,ys,sinviewingrangeglobalang);
		if (yp > (4<<8))
		{
			xp = dmulscale6(ys,cosglobalang,-xs,singlobalang);
			spritesx[i] = scale(xp + yp, (int32_t)XDIM << 7, yp);
		}
		else if ((tspriteptr[i]->cstat&48) == 0)
		{
			spritesortcnt--;  //Delete face sprite if on wrong side!
			if (i != spritesortcnt)
			{
				tspriteptr[i] = tspriteptr[spritesortcnt];
				spritesx[i] = spritesx[spritesortcnt];
				spritesy[i] = spritesy[spritesortcnt];
			}
			continue;
		}
		spritesy[i] = yp;
	}

	gap = 1;
	while (gap < spritesortcnt)
		gap = (gap<<1)+1;

	for(gap>>=1;gap>0;gap>>=1)      //Sort sprite list
		for(i=0;i<spritesortcnt-gap;i++)
			for(l=i;l>=0;l-=gap)
			{
				if (spritesy[l] <= spritesy[l+gap])
					break;

				swaplong(&tspriteptr[l],&tspriteptr[l+gap]);
				swaplong(&spritesx[l],&spritesx[l+gap]);
				swaplong(&spritesy[l],&spritesy[l+gap]);
			}

	if (spritesortcnt > 0)
		spritesy[spritesortcnt] = (spritesy[spritesortcnt-1]^1);

	ys = spritesy[0]; i = 0;
	for(j=1;j<=spritesortcnt;j++)
	{
		if (spritesy[j] == ys)
			continue;

		ys = spritesy[j];
		if (j > i+1)
		{
			for(k=i;k<j;k++)
			{
				spritesz[k] = tspriteptr[k]->z;
				if ((tspriteptr[k]->cstat&48) != 32)
				{
					yoff = (int32_t)((int8_t)((picanm[tspriteptr[k]->picnum]>>16)&255))+((int32_t)tspriteptr[k]->yoffset);
					spritesz[k] -= ((yoff*tspriteptr[k]->yrepeat)<<2);
					yspan = (tilesizy[tspriteptr[k]->picnum]*tspriteptr[k]->yrepeat<<2);
					if (!(tspriteptr[k]->cstat&128)) spritesz[k] -= (yspan>>1);
					if (klabs(spritesz[k]-globalposz) < (yspan>>1)) spritesz[k] = globalposz;
				}
			}
			for(k=i+1;k<j;k++)
				for(l=i;l<k;l++)
					if (klabs(spritesz[k]-globalposz) < klabs(spritesz[l]-globalposz))
					{
						swaplong(&tspriteptr[k],&tspriteptr[l]);
						swaplong(&spritesx[k],&spritesx[l]);
						swaplong(&spritesy[k],&spritesy[l]);
						swaplong(&spritesz[k],&spritesz[l]);
					}
			for(k=i+1;k<j;k++)
				for(l=i;l<k;l++)
					if (tspriteptr[k]->statnum < tspriteptr[l]->statnum)
					{
						swaplong(&tspriteptr[k],&tspriteptr[l]);
						swaplong(&spritesx[k],&spritesx[l]);
						swaplong(&spritesy[k],&spritesy[l]);
					}
		}
		i = j;
	}

	while ((spritesortcnt > 0) && (maskwallcnt > 0))  //While BOTH > 0
	{
		j = maskwall[maskwallcnt-1];
		if (spritewallfront(tspriteptr[spritesortcnt-1],(int32_t)thewall[j]) == 0)
			drawsprite(--spritesortcnt);
		else
		{
				//Check to see if any sprites behind the masked wall...
			k = -1;
			gap = 0;
			for(i=spritesortcnt-2;i>=0;i--)
				if ((xb1[j] <= (spritesx[i]>>8)) && ((spritesx[i]>>8) <= xb2[j]))
					if (spritewallfront(tspriteptr[i],(int32_t)thewall[j]) == 0)
					{
						drawsprite(i);
						tspriteptr[i]->owner = -1;
						k = i;
						gap++;
					}
			if (k >= 0)       //remove holes in sprite list
			{
				for(i=k;i<spritesortcnt;i++)
					if (tspriteptr[i]->owner >= 0)
					{
						if (i > k)
						{
							tspriteptr[k] = tspriteptr[i];
							spritesx[k] = spritesx[i];
							spritesy[k] = spritesy[i];
						}
						k++;
					}
				spritesortcnt -= gap;
			}

				//finally safe to draw the masked wall
			drawmaskwall(--maskwallcnt);
		}
	}
	while (spritesortcnt > 0) drawsprite(--spritesortcnt);
	while (maskwallcnt > 0) drawmaskwall(--maskwallcnt);
}


static void drawmaskwall(int16_t damaskwallcnt)
{
	int32_t i, j, k, x, z, sectnum, z1, z2, lx, rx;
	sectortype __far* sec;
	sectortype __far* nsec;
	walltype __far* wal;
	int16_t globalpicnum;

	z = maskwall[damaskwallcnt];
	wal = &wall[thewall[z]];
	sectnum = thesector[z]; sec = &sector[sectnum];
	nsec = &sector[wal->nextsector];
	z1 = max(nsec->ceilingz,sec->ceilingz);
	z2 = min(nsec->floorz,sec->floorz);

	wallmost(uwall,z,sectnum,(uint8_t)0);
	wallmost(uplc,z,(int32_t)wal->nextsector,(uint8_t)0);
	for(x=xb1[z];x<=xb2[z];x++) if (uplc[x] > uwall[x]) uwall[x] = uplc[x];
	wallmost(dwall,z,sectnum,(uint8_t)1);
	wallmost(dplc,z,(int32_t)wal->nextsector,(uint8_t)1);
	for(x=xb1[z];x<=xb2[z];x++) if (dplc[x] < dwall[x]) dwall[x] = dplc[x];
	prepwall(z,wal);

	globalorientation = (int32_t)wal->cstat;
	globalpicnum = wal->overpicnum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	globalxpanning = (int32_t)wal->xpanning;
	globalypanning = (int32_t)wal->ypanning;
	if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
	globalshade = (int32_t)wal->shade;
	globvis = globalvisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globalpal = (int32_t)wal->pal;
	globalshiftval = (picsiz[globalpicnum]>>4);
	if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
	globalshiftval = 32-globalshiftval;
	globalyscale = (wal->yrepeat<<(globalshiftval-19));
	if ((globalorientation&4) == 0)
		globalzd = (((globalposz-z1)*globalyscale)<<8);
	else
		globalzd = (((globalposz-z2)*globalyscale)<<8);
	globalzd += (globalypanning<<24);
	if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

	for(i=smostwallcnt-1;i>=0;i--)
	{
		j = smostwall[i];
		if ((xb1[j] > xb2[z]) || (xb2[j] < xb1[z])) continue;
		if (wallfront(j,z)) continue;

		lx = max(xb1[j],xb1[z]); rx = min(xb2[j],xb2[z]);

		switch(smostwalltype[i])
		{
			case 0:
				if (lx <= rx)
				{
					if ((lx == xb1[z]) && (rx == xb2[z])) return;
					_fmemset(&dwall[lx], 0, (rx - lx + 1) * sizeof(dwall[0]));
				}
				break;
			case 1:
				k = smoststart[i] - xb1[j];
				for(x=lx;x<=rx;x++)
					if (smost[k+x] > uwall[x]) uwall[x] = smost[k+x];
				break;
			case 2:
				k = smoststart[i] - xb1[j];
				for(x=lx;x<=rx;x++)
					if (smost[k+x] < dwall[x]) dwall[x] = smost[k+x];
				break;
		}
	}

		//maskwall
	if ((searchit >= 1) && (searchx >= xb1[z]) && (searchx <= xb2[z]))
		if ((searchy >= uwall[searchx]) && (searchy <= dwall[searchx]))
		{
			searchsector = sectnum; searchwall = thewall[z];
			searchstat = 4; searchit = 1;
		}

	if ((globalorientation&128) == 0)
		maskwallscan(xb1[z], xb2[z], uwall, dwall, swall, lwall, globalpicnum);
	else
	{
		if (globalorientation&128)
		{
			if (globalorientation&512)
				_a_transmode = 1;
			else
				_a_transmode = 0;
		}
		transmaskwallscan(xb1[z], xb2[z], globalpicnum);
	}
}


void initspritelists(void)
{
	int32_t i;

	for (i=0;i<MAXSECTORS;i++)     //Init doubly-linked sprite sector lists
		headspritesect[i] = -1;
	headspritesect[MAXSECTORS] = 0;
	for(i=0;i<MAXSPRITES;i++)
	{
		prevspritesect[i] = i-1;
		nextspritesect[i] = i+1;
		sprite[i].sectnum = MAXSECTORS;
	}
	prevspritesect[0] = -1;
	nextspritesect[MAXSPRITES-1] = -1;


	for(i=0;i<MAXSTATUS;i++)      //Init doubly-linked sprite status lists
		headspritestat[i] = -1;
	headspritestat[MAXSTATUS] = 0;
	for(i=0;i<MAXSPRITES;i++)
	{
		prevspritestat[i] = i-1;
		nextspritestat[i] = i+1;
		sprite[i].statnum = MAXSTATUS;
	}
	prevspritestat[0] = -1;
	nextspritestat[MAXSPRITES-1] = -1;
}


void insertspritesect(int16_t sectnum)
{
	int16_t blanktouse;

	if ((sectnum >= MAXSECTORS) || (headspritesect[MAXSECTORS] == -1))
		return;  //list full

	blanktouse = headspritesect[MAXSECTORS];

	headspritesect[MAXSECTORS] = nextspritesect[blanktouse];
	if (headspritesect[MAXSECTORS] >= 0)
		prevspritesect[headspritesect[MAXSECTORS]] = -1;

	prevspritesect[blanktouse] = -1;
	nextspritesect[blanktouse] = headspritesect[sectnum];
	if (headspritesect[sectnum] >= 0)
		prevspritesect[headspritesect[sectnum]] = blanktouse;
	headspritesect[sectnum] = blanktouse;

	sprite[blanktouse].sectnum = sectnum;
}

void insertspritestat(int16_t statnum)
{
	int16_t blanktouse;

	if ((statnum >= MAXSTATUS) || (headspritestat[MAXSTATUS] == -1))
		return;  //list full

	blanktouse = headspritestat[MAXSTATUS];

	headspritestat[MAXSTATUS] = nextspritestat[blanktouse];
	if (headspritestat[MAXSTATUS] >= 0)
		prevspritestat[headspritestat[MAXSTATUS]] = -1;

	prevspritestat[blanktouse] = -1;
	nextspritestat[blanktouse] = headspritestat[statnum];
	if (headspritestat[statnum] >= 0)
		prevspritestat[headspritestat[statnum]] = blanktouse;
	headspritestat[statnum] = blanktouse;

	sprite[blanktouse].statnum = statnum;
}


void updatesector(int32_t x, int32_t y, int16_t *sectnum)
{
	walltype __far* wal;
	int32_t i, j;

	if (inside(x,y,*sectnum) == 1) return;

	if ((*sectnum >= 0) && (*sectnum < numsectors))
	{
		wal = &wall[sector[*sectnum].wallptr];
		j = sector[*sectnum].wallnum;
		do
		{
			i = wal->nextsector;
			if (i >= 0)
				if (inside(x,y,(int16_t)i) == 1)
				{
					*sectnum = i;
					return;
				}
			wal++;
			j--;
		} while (j != 0);
	}

	for(i=numsectors-1;i>=0;i--)
		if (inside(x,y,(int16_t)i) == 1)
		{
			*sectnum = i;
			return;
		}

	*sectnum = -1;
}

void getzrange(int32_t x, int32_t y, int32_t z, int16_t sectnum,
			 int32_t *ceilz, int32_t *florz,
			 int32_t walldist, uint32_t cliptype)
{
	sectortype __far* sec;
	walltype __far* wal;
	walltype __far* wal2;
	spritetype __far* spr;
	int32_t clipsectcnt, startwall, endwall, tilenum, xoff, yoff, dax, day;
	int32_t xmin, ymin, xmax, ymax, i, j, k, l, daz, daz2, dx, dy;
	int32_t x1, y1, x2, y2, x3, y3, x4, y4, ang, cosang, sinang;
	int32_t xspan, yspan, xrepeat, yrepeat, dasprclipmask, dawalclipmask;
	int16_t cstat;
	uint8_t clipyou;

	if (sectnum < 0)
	{
		*ceilz = 0x80000000;
		*florz = 0x7fffffff;
		return;
	}

		//Extra walldist for sprites on sector lines
	i = walldist+MAXCLIPDIST+1;
	xmin = x-i; ymin = y-i;
	xmax = x+i; ymax = y+i;

	getzsofslope(sectnum,x,y,ceilz,florz);

	dawalclipmask = (cliptype&65535);
	dasprclipmask = (cliptype>>16);

	clipsectorlist[0] = sectnum;
	clipsectcnt = 0; clipsectnum = 1;

	do  //Collect sectors inside your square first
	{
		sec = &sector[clipsectorlist[clipsectcnt]];
		startwall = sec->wallptr; endwall = startwall + sec->wallnum;
		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			k = wal->nextsector;
			if (k >= 0)
			{
				wal2 = &wall[wal->point2];
				x1 = wal->x; x2 = wal2->x;
				if ((x1 < xmin) && (x2 < xmin)) continue;
				if ((x1 > xmax) && (x2 > xmax)) continue;
				y1 = wal->y; y2 = wal2->y;
				if ((y1 < ymin) && (y2 < ymin)) continue;
				if ((y1 > ymax) && (y2 > ymax)) continue;

				dx = x2-x1; dy = y2-y1;
				if (dx*(y-y1) < (x-x1)*dy) continue; //back
				if (dx > 0) dax = dx*(ymin-y1); else dax = dx*(ymax-y1);
				if (dy > 0) day = dy*(xmax-x1); else day = dy*(xmin-x1);
				if (dax >= day) continue;

				if (wal->cstat&dawalclipmask) continue;
				sec = &sector[k];
				if (editstatus == 0)
				{
					if (((sec->ceilingstat&1) == 0) && (z <= sec->ceilingz+(3<<8))) continue;
					if (((sec->floorstat&1) == 0) && (z >= sec->floorz-(3<<8))) continue;
				}

				for(i=clipsectnum-1;i>=0;i--) if (clipsectorlist[i] == k) break;
				if (i < 0) clipsectorlist[clipsectnum++] = k;

				if ((x1 < xmin+MAXCLIPDIST) && (x2 < xmin+MAXCLIPDIST)) continue;
				if ((x1 > xmax-MAXCLIPDIST) && (x2 > xmax-MAXCLIPDIST)) continue;
				if ((y1 < ymin+MAXCLIPDIST) && (y2 < ymin+MAXCLIPDIST)) continue;
				if ((y1 > ymax-MAXCLIPDIST) && (y2 > ymax-MAXCLIPDIST)) continue;
				if (dx > 0) dax += dx*MAXCLIPDIST; else dax -= dx*MAXCLIPDIST;
				if (dy > 0) day -= dy*MAXCLIPDIST; else day += dy*MAXCLIPDIST;
				if (dax >= day) continue;

					//It actually got here, through all the continue's!!!
				getzsofslope((int16_t)k,x,y,&daz,&daz2);
				if (daz > *ceilz) { *ceilz = daz; }
				if (daz2 < *florz) { *florz = daz2; }
			}
		}
		clipsectcnt++;
	} while (clipsectcnt < clipsectnum);

	for(i=0;i<clipsectnum;i++)
	{
		for(j=headspritesect[clipsectorlist[i]];j>=0;j=nextspritesect[j])
		{
			spr = &sprite[j];
			cstat = spr->cstat;
			if (cstat&dasprclipmask)
			{
				x1 = spr->x; y1 = spr->y;

				clipyou = 0;
				switch(cstat&48)
				{
					case 0:
						k = walldist+(spr->clipdist<<2)+1;
						if ((klabs(x1-x) <= k) && (klabs(y1-y) <= k))
						{
							daz = spr->z;
							k = ((tilesizy[spr->picnum]*spr->yrepeat)<<1);
							if (cstat&128) daz += k;
							if (picanm[spr->picnum]&0x00ff0000) daz -= ((int32_t)((int8_t)((picanm[spr->picnum]>>16)&255))*spr->yrepeat<<2);
							daz2 = daz - (k<<1);
							clipyou = 1;
						}
						break;
					case 16:
						tilenum = spr->picnum;
						xoff = (int32_t)((int8_t)((picanm[tilenum]>>8)&255))+((int32_t)spr->xoffset);
						if ((cstat&4) > 0) xoff = -xoff;
						k = spr->ang; l = spr->xrepeat;
						dax = sintable[k&2047]*l; day = sintable[(k+1536)&2047]*l;
						l = tilesizx[tilenum]; k = (l>>1)+xoff;
						x1 -= mulscale16(dax,k); x2 = x1+mulscale16(dax,l);
						y1 -= mulscale16(day,k); y2 = y1+mulscale16(day,l);
						if (clipinsideboxline(x,y,x1,y1,x2,y2,walldist+1) != 0)
						{
							daz = spr->z; k = ((tilesizy[spr->picnum]*spr->yrepeat)<<1);
							if (cstat&128) daz += k;
							if (picanm[spr->picnum]&0x00ff0000) daz -= ((int32_t)((int8_t)((picanm[spr->picnum]>>16)&255))*spr->yrepeat<<2);
							daz2 = daz-(k<<1);
							clipyou = 1;
						}
						break;
					case 32:
						daz = spr->z; daz2 = daz;

						if ((cstat&64) != 0)
							if ((z > daz) == ((cstat&8)==0)) continue;

						tilenum = spr->picnum;
						xoff = (int32_t)((int8_t)((picanm[tilenum]>>8)&255))+((int32_t)spr->xoffset);
						yoff = (int32_t)((int8_t)((picanm[tilenum]>>16)&255))+((int32_t)spr->yoffset);
						if ((cstat&4) > 0) xoff = -xoff;
						if ((cstat&8) > 0) yoff = -yoff;

						ang = spr->ang;
						cosang = sintable[(ang+512)&2047]; sinang = sintable[ang];
						xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
						yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

						dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
						x1 += dmulscale16(sinang,dax,cosang,day)-x;
						y1 += dmulscale16(sinang,day,-cosang,dax)-y;
						l = xspan*xrepeat;
						x2 = x1 - mulscale16(sinang,l);
						y2 = y1 + mulscale16(cosang,l);
						l = yspan*yrepeat;
						k = -mulscale16(cosang,l); x3 = x2+k; x4 = x1+k;
						k = -mulscale16(sinang,l); y3 = y2+k; y4 = y1+k;

						dax = mulscale14(sintable[(spr->ang-256+512)&2047],walldist+4);
						day = mulscale14(sintable[(spr->ang-256)&2047],walldist+4);
						x1 += dax; x2 -= day; x3 -= dax; x4 += day;
						y1 += day; y2 += dax; y3 -= day; y4 -= dax;

						if ((y1^y2) < 0)
						{
							if ((x1^x2) < 0) clipyou ^= (x1*y2<x2*y1)^(y1<y2);
							else if (x1 >= 0) clipyou ^= 1;
						}
						if ((y2^y3) < 0)
						{
							if ((x2^x3) < 0) clipyou ^= (x2*y3<x3*y2)^(y2<y3);
							else if (x2 >= 0) clipyou ^= 1;
						}
						if ((y3^y4) < 0)
						{
							if ((x3^x4) < 0) clipyou ^= (x3*y4<x4*y3)^(y3<y4);
							else if (x3 >= 0) clipyou ^= 1;
						}
						if ((y4^y1) < 0)
						{
							if ((x4^x1) < 0) clipyou ^= (x4*y1<x1*y4)^(y4<y1);
							else if (x4 >= 0) clipyou ^= 1;
						}
						break;
				}

				if (clipyou != 0)
				{
					if ((z > daz) && (daz > *ceilz)) { *ceilz = daz; }
					if ((z < daz2) && (daz2 < *florz)) { *florz = daz2; }
				}
			}
		}
	}
}

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

void setaspect(int32_t daxrange, int32_t daaspect)
{
	viewingrange = daxrange;
	viewingrangerecip = divscale32(1L,daxrange);

	yxaspect = daaspect;
	xyaspect = divscale32(1,yxaspect);
	xdimenscale = scale(XDIM,yxaspect,XDIM);
	xdimscale = scale(XDIM,xyaspect,XDIM);
}

static void dosetaspect(void)
{
	int32_t i, j, k, x, xinc;

	if (xyaspect != oxyaspect)
	{
		oxyaspect = xyaspect;
		j = xyaspect*XDIM;
		horizlookup2[((YDIM*4)>>1)-1] = divscale26(131072,j);
		for(i=YDIM*4-1;i>=0;i--)
			if (i != (((YDIM*4)>>1)-1))
			{
				horizlookup[i] = divscale28(1,i-(((YDIM*4)>>1)-1));
				horizlookup2[i] = divscale14(klabs(horizlookup[i]),j);
			}
	}
	if ((XDIM != oxdimen) || (viewingrange != oviewingrange))
	{
		oxdimen = XDIM;
		oviewingrange = viewingrange;
		xinc = mulscale32(viewingrange*XDIM,xdimenrecip);
		x = ((XDIM*2L)<<16)-mulscale1(xinc,XDIM);
		for(i=0;i<XDIM;i++)
		{
			j = (x&65535); k = (x>>16); x += xinc;
			if (j != 0) j = mulscale16((int32_t)radarang[k+1]-(int32_t)radarang[k],j);
			radarang2[i] = (int16_t)(((int32_t)radarang[k]+j)>>6);
		}
	}
}

void rotatesprite (int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, uint8_t dapalnum, uint8_t dastat, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2)
{
	if ((cx1 > cx2) || (cy1 > cy2)) return;
	if (z <= 16) return;
	if (picanm[picnum]&192) picnum += animateoffs(picnum);
	if ((tilesizx[picnum] <= 0) || (tilesizy[picnum] <= 0)) return;

	dorotatesprite(sx,sy,z,a,picnum,dashade,dapalnum,dastat,cx1,cy1,cx2,cy2);

	if ((dastat&64) && (cx1 <= 0) && (cy1 <= 0) && (cx2 >= XDIM-1) && (cy2 >= YDIM-1) &&
		 (sx == ((XDIM/2L)<<16)) && (sy == ((YDIM/2L)<<16)) && (z == 65536L) && (a == 0) && ((dastat&1) == 0))
		permhead = permtail = 0;
}


	//Rotatesprite vertical line function
static void a_spritevline(int32_t bx, int32_t by, int32_t cnt, uint8_t __far* bufplc, uint8_t __far* p, uint8_t __far* paloffs)
{
	for(;cnt>1;cnt--)
	{
		*p  = paloffs[bufplc[(bx >> 16) * _a_glogy + (by >> 16)]];
		bx += _a_gbxinc;
		by += _a_gbyinc;
		p  += XDIM;
	}
}


	//Rotatesprite vertical line function
static void a_mspritevline(int32_t bx, int32_t by, int32_t cnt, uint8_t __far* bufplc, uint8_t __far* p, uint8_t __far* paloffs)
{
	uint8_t ch;

	for(;cnt>1;cnt--)
	{
		ch = bufplc[(bx>>16)*_a_glogy+(by>>16)];
		if (ch != 255)
			*p = paloffs[ch];
		bx += _a_gbxinc;
		by += _a_gbyinc;
		p  += XDIM;
	}
}


	//Rotatesprite vertical line function
static void a_tspritevline(int32_t bx, int32_t by, int32_t cnt, uint8_t __far* bufplc, uint8_t __far* p, uint8_t __far* paloffs)
{
	uint8_t ch;

	if (_a_transmode)
	{
		for(;cnt>1;cnt--)
		{
			ch = bufplc[(bx>>16)*_a_glogy+(by>>16)];
			if (ch != 255)
				*p = translucfunc(*p, paloffs[ch]);
			bx += _a_gbxinc;
			by += _a_gbyinc;
			p  += XDIM;
		}
	}
	else
	{
		for(;cnt>1;cnt--)
		{
			ch = bufplc[(bx>>16)*_a_glogy+(by>>16)];
			if (ch != 255)
				*p = translucfunc(paloffs[ch], *p);
			bx += _a_gbxinc;
			by += _a_gbyinc;
			p  += XDIM;
		}
	}
}


static void dorotatesprite (int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, uint8_t dapalnum, uint8_t dastat, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2)
{
	int32_t cosang, sinang, v, nextv, dax1, dax2, oy, bx, by;
	int32_t i, x, y, x1, y1, x2, y2, gx1, gy1;
	uint8_t __far* p;
	uint8_t __far* bufplc;
	uint8_t __far* palookupoffs;
	int32_t xsiz, ysiz, xoff, yoff, npoints, yplc, yinc, lx, rx;
	int32_t xv, yv, xv2, yv2;

	xsiz = tilesizx[picnum]; ysiz = tilesizy[picnum];
	if (dastat&16) { xoff = 0; yoff = 0; }
	else
	{
		xoff = (int32_t)((int8_t)((picanm[picnum]>>8)&255))+(xsiz>>1);
		yoff = (int32_t)((int8_t)((picanm[picnum]>>16)&255))+(ysiz>>1);
	}

	if (dastat&4) yoff = ysiz-yoff;

	cosang = sintable[(a+512)&2047]; sinang = sintable[a&2047];

	if ((dastat&2) != 0)  //Auto window size scaling
	{
		if ((dastat&8) == 0)
		{
			x = xdimenscale;   //= scale(xdimen,yxaspect,XDIM);
			sx = ((cx1 + cx2 + 2) << 15) + scale(sx - ((int32_t)XDIM << 15), XDIM, XDIM);
			sy = ((cy1 + cy2 + 2) << 15) + mulscale16(sy - ((int32_t)YDIM << 15), x);
		}
		else
		{
			  //If not clipping to startmosts, & auto-scaling on, as a
			  //hard-coded bonus, scale to full screen instead
			x = scale(XDIM,yxaspect,XDIM);
			sx = ((int32_t)XDIM << 15) + 32768 + scale(sx - ((int32_t)XDIM << 15), XDIM, XDIM);
			sy = ((int32_t)YDIM << 15) + 32768 + mulscale16(sy - ((int32_t)YDIM << 15), x);
		}
		z = mulscale16(z,x);
	}

	xv = mulscale14(cosang,z);
	yv = mulscale14(sinang,z);
	if (((dastat&2) != 0) || ((dastat&8) == 0)) //Don't aspect unscaled perms
	{
		xv2 = mulscale16(xv,xyaspect);
		yv2 = mulscale16(yv,xyaspect);
	}
	else
	{
		xv2 = xv;
		yv2 = yv;
	}

	ry1[0] = sy - (yv*xoff + xv*yoff);
	ry1[1] = ry1[0] + yv*xsiz;
	ry1[3] = ry1[0] + xv*ysiz;
	ry1[2] = ry1[1]+ry1[3]-ry1[0];
	i = (cy1<<16); if ((ry1[0]<i) && (ry1[1]<i) && (ry1[2]<i) && (ry1[3]<i)) return;
	i = (cy2<<16); if ((ry1[0]>i) && (ry1[1]>i) && (ry1[2]>i) && (ry1[3]>i)) return;

	rx1[0] = sx - (xv2*xoff - yv2*yoff);
	rx1[1] = rx1[0] + xv2*xsiz;
	rx1[3] = rx1[0] - yv2*ysiz;
	rx1[2] = rx1[1]+rx1[3]-rx1[0];
	i = (cx1<<16); if ((rx1[0]<i) && (rx1[1]<i) && (rx1[2]<i) && (rx1[3]<i)) return;
	i = (cx2<<16); if ((rx1[0]>i) && (rx1[1]>i) && (rx1[2]>i) && (rx1[3]>i)) return;

	gx1 = rx1[0]; gy1 = ry1[0];   //back up these before clipping

	if ((npoints = clippoly4(cx1<<16,cy1<<16,(cx2+1)<<16,(cy2+1)<<16)) < 3) return;

	lx = rx1[0]; rx = rx1[0];

	nextv = 0;
	for(v=npoints-1;v>=0;v--)
	{
		x1 = rx1[v]; x2 = rx1[nextv];
		dax1 = (x1>>16); if (x1 < lx) lx = x1;
		dax2 = (x2>>16); if (x1 > rx) rx = x1;
		if (dax1 != dax2)
		{
			y1 = ry1[v]; y2 = ry1[nextv];
			yinc = divscale16(y2-y1,x2-x1);
			if (dax2 > dax1)
			{
				yplc = y1 + mulscale16((dax1<<16)+65535-x1,yinc);
				qinterpolatedown16short(&uplc[dax1],dax2-dax1,yplc,yinc);
			}
			else
			{
				yplc = y2 + mulscale16((dax2<<16)+65535-x2,yinc);
				qinterpolatedown16short(&dplc[dax2],dax1-dax2,yplc,yinc);
			}
		}
		nextv = v;
	}

	bufplc = loadtile(picnum);

	palookupoffs = palookup[dapalnum] + (getpalookup(0,dashade)<<8);

	i = divscale32(1L,z);
	xv = mulscale14(sinang,i);
	yv = mulscale14(cosang,i);
	if (((dastat&2) != 0) || ((dastat&8) == 0)) //Don't aspect unscaled perms
	{
		yv2 = mulscale16(-xv,yxaspect);
		xv2 = mulscale16(yv,yxaspect);
	}
	else
	{
		yv2 = -xv;
		xv2 = yv;
	}

	x1 = (lx>>16); x2 = (rx>>16);

	oy = 0;
	x = (x1<<16)-1-gx1; y = (oy<<16)+65535-gy1;
	bx = dmulscale16(x,xv2,y,xv);
	by = dmulscale16(x,yv2,y,yv);
	if (dastat&4)
	{
		yv = -yv;
		yv2 = -yv2;
		by = (ysiz<<16)-1-by;
	}

	_a_gbxinc = xv;
	_a_gbyinc = yv;
	_a_glogy  = ysiz;

	if ((dastat&1) != 0)
	{
		if (dastat&32)
			_a_transmode = 1;
		else
			_a_transmode = 0;
	}

	for(x=x1;x<x2;x++)
	{
		bx += xv2; by += yv2;

		y1 = uplc[x]; y2 = dplc[x];
		if ((dastat&8) == 0)
		{
			if (startumost[x] > y1) y1 = startumost[x];
			if (startdmost[x] < y2) y2 = startdmost[x];
		}
		if (y2 <= y1) continue;

		switch(y1-oy)
		{
			case -1: bx -= xv; by -= yv; oy = y1; break;
			case 0: break;
			case 1: bx += xv; by += yv; oy = y1; break;
			default: bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1; break;
		}

		p = ylookup[y1]+x+_s_screen;

		if ((dastat&1) == 0)
		{
			if (dastat&64)
				a_spritevline(bx & 65535, by & 65535, y2 - y1 + 1, (bx >> 16) * ysiz + (by >> 16) + bufplc, p, palookupoffs);
			else
				a_mspritevline(bx & 65535, by & 65535, y2 - y1 + 1, (bx >> 16) * ysiz + (by >> 16) + bufplc, p, palookupoffs);
		}
		else
		{
			a_tspritevline(bx & 65535, by & 65535, y2 - y1 + 1, (bx >> 16) * ysiz + (by >> 16) + bufplc, p, palookupoffs);
			transarea += (y2-y1);
		}
		faketimerhandler();
	}

	Z_ChangeTagToCache(bufplc);
}

	//Assume npoints=4 with polygon on &rx1,&ry1
static int32_t clippoly4(int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2)
{
	int32_t n, nn, z, zz, x, x1, x2, y, y1, y2, t;

	nn = 0; z = 0;
	do
	{
		zz = ((z+1)&3);
		x1 = rx1[z]; x2 = rx1[zz]-x1;

		if ((cx1 <= x1) && (x1 <= cx2))
			rx2[nn] = x1, ry2[nn] = ry1[z], nn++;

		if (x2 <= 0) x = cx2; else x = cx1;
		t = x-x1;
		if (((t-x2)^t) < 0)
			rx2[nn] = x, ry2[nn] = ry1[z]+scale(t,ry1[zz]-ry1[z],x2), nn++;

		if (x2 <= 0) x = cx1; else x = cx2;
		t = x-x1;
		if (((t-x2)^t) < 0)
			rx2[nn] = x, ry2[nn] = ry1[z]+scale(t,ry1[zz]-ry1[z],x2), nn++;

		z = zz;
	} while (z != 0);
	if (nn < 3) return(0);

	n = 0; z = 0;
	do
	{
		zz = z+1; if (zz == nn) zz = 0;
		y1 = ry2[z]; y2 = ry2[zz]-y1;

		if ((cy1 <= y1) && (y1 <= cy2))
			ry1[n] = y1, rx1[n] = rx2[z], n++;

		if (y2 <= 0) y = cy2; else y = cy1;
		t = y-y1;
		if (((t-y2)^t) < 0)
			ry1[n] = y, rx1[n] = rx2[z]+scale(t,rx2[zz]-rx2[z],y2), n++;

		if (y2 <= 0) y = cy1; else y = cy2;
		t = y-y1;
		if (((t-y2)^t) < 0)
			ry1[n] = y, rx1[n] = rx2[z]+scale(t,rx2[zz]-rx2[z],y2), n++;

		z = zz;
	} while (z != 0);
	return(n);
}


static void clearallviews(int32_t dacol)
{
	dacol += (dacol<<8); dacol += (dacol<<16);

	_fmemset(_s_screen, 0, (size_t)((int32_t)XDIM * YDIM));
	faketimerhandler();
}


static void getzsofslope(int16_t sectnum, int32_t dax, int32_t day, int32_t *ceilz, int32_t *florz)
{
	int32_t dx, dy, i, j;
	walltype __far* wal;
	walltype __far* wal2;
	sectortype __far* sec;

	sec = &sector[sectnum];
	*ceilz = sec->ceilingz; *florz = sec->floorz;
	if ((sec->ceilingstat|sec->floorstat)&2)
	{
		wal = &wall[sec->wallptr]; wal2 = &wall[wal->point2];
		dx = wal2->x-wal->x; dy = wal2->y-wal->y;
		i = (msqrtasm(dx*dx+dy*dy)<<5); if (i == 0) return;
		j = dmulscale3(dx,day-wal->y,-dy,dax-wal->x);
		if (sec->ceilingstat&2) *ceilz = (*ceilz)+scale(sec->ceilingheinum,j,i);
		if (sec->floorstat&2) *florz = (*florz)+scale(sec->floorheinum,j,i);
	}
}


static uint8_t wallmost(int16_t __far* mostbuf, int32_t w, int32_t sectnum, uint8_t dastat)
{
	uint8_t bad;
	int32_t i, j, t, y, z, inty, intz, xcross, yinc, fw;
	int32_t x1, y1, z1, x2, y2, z2, xv, yv, dx, dy, dasqr, oz1, oz2;
	int32_t s1, s2, s3, s4, ix1, ix2, iy1, iy2;

	if (dastat == 0)
	{
		z = sector[sectnum].ceilingz-globalposz;
		if ((sector[sectnum].ceilingstat&2) == 0) return(owallmost(mostbuf,w,z));
	}
	else
	{
		z = sector[sectnum].floorz-globalposz;
		if ((sector[sectnum].floorstat&2) == 0) return(owallmost(mostbuf,w,z));
	}

	i = thewall[w];
	if (i == sector[sectnum].wallptr) return(owallmost(mostbuf,w,z));

	x1 = wall[i].x; x2 = wall[wall[i].point2].x-x1;
	y1 = wall[i].y; y2 = wall[wall[i].point2].y-y1;

	fw = sector[sectnum].wallptr; i = wall[fw].point2;
	dx = wall[i].x-wall[fw].x; dy = wall[i].y-wall[fw].y;
	dasqr = krecip(msqrtasm(dx*dx+dy*dy));

	if (xb1[w] == 0)
		{ xv = cosglobalang+sinviewingrangeglobalang; yv = singlobalang-cosviewingrangeglobalang; }
	else
		{ xv = x1-globalposx; yv = y1-globalposy; }
	i = xv*(y1-globalposy)-yv*(x1-globalposx); j = yv*x2-xv*y2;
	if (klabs(j) > klabs(i>>3)) i = divscale28(i,j);
	if (dastat == 0)
	{
		t = mulscale15(sector[sectnum].ceilingheinum,dasqr);
		z1 = sector[sectnum].ceilingz;
	}
	else
	{
		t = mulscale15(sector[sectnum].floorheinum,dasqr);
		z1 = sector[sectnum].floorz;
	}
	z1 = dmulscale24(dx*t,mulscale20(y2,i)+((y1-wall[fw].y)<<8),
						 -dy*t,mulscale20(x2,i)+((x1-wall[fw].x)<<8))+((z1-globalposz)<<7);


	if (xb2[w] == XDIM-1)
		{ xv = cosglobalang-sinviewingrangeglobalang; yv = singlobalang+cosviewingrangeglobalang; }
	else
		{ xv = (x2+x1)-globalposx; yv = (y2+y1)-globalposy; }
	i = xv*(y1-globalposy)-yv*(x1-globalposx); j = yv*x2-xv*y2;
	if (klabs(j) > klabs(i>>3)) i = divscale28(i,j);
	if (dastat == 0)
	{
		t = mulscale15(sector[sectnum].ceilingheinum,dasqr);
		z2 = sector[sectnum].ceilingz;
	}
	else
	{
		t = mulscale15(sector[sectnum].floorheinum,dasqr);
		z2 = sector[sectnum].floorz;
	}
	z2 = dmulscale24(dx*t,mulscale20(y2,i)+((y1-wall[fw].y)<<8),
						 -dy*t,mulscale20(x2,i)+((x1-wall[fw].x)<<8))+((z2-globalposz)<<7);


	s1 = mulscale20(globaluclip,yb1[w]); s2 = mulscale20(globaluclip,yb2[w]);
	s3 = mulscale20(globaldclip,yb1[w]); s4 = mulscale20(globaldclip,yb2[w]);
	bad = (z1<s1)+((z2<s2)<<1)+((z1>s3)<<2)+((z2>s4)<<3);

	ix1 = xb1[w]; ix2 = xb2[w];
	iy1 = yb1[w]; iy2 = yb2[w];
	oz1 = z1; oz2 = z2;

	if ((bad&3) == 3)
	{
		_fmemset(&mostbuf[ix1], 0, (ix2 - ix1 + 1) * sizeof(mostbuf[0]));
		return(bad);
	}

	if ((bad&12) == 12)
	{
		clearshortbuf(&mostbuf[ix1],YDIM,ix2-ix1+1);
		return(bad);
	}

	if (bad&3)
	{
			//inty = intz / (globaluclip>>16)
		t = divscale30(oz1-s1,s2-s1+oz1-oz2);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		intz = oz1 + mulscale30(oz2-oz1,t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		//t = divscale30((x1<<4)-xcross*yb1[w],xcross*(yb2[w]-yb1[w])-((x2-x1)<<4));
		//inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		//intz = z1 + mulscale30(z2-z1,t);

		if ((bad&3) == 2)
		{
			if (xb1[w] <= xcross) { z2 = intz; iy2 = inty; ix2 = xcross; }
			_fmemset(&mostbuf[xcross + 1], 0, (xb2[w] - xcross) * sizeof(mostbuf[0]));
		}
		else
		{
			if (xcross <= xb2[w]) { z1 = intz; iy1 = inty; ix1 = xcross; }
			_fmemset(&mostbuf[xb1[w]], 0, (xcross - xb1[w] + 1) * sizeof(mostbuf[0]));
		}
	}

	if (bad&12)
	{
			//inty = intz / (globaldclip>>16)
		t = divscale30(oz1-s3,s4-s3+oz1-oz2);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		intz = oz1 + mulscale30(oz2-oz1,t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		//t = divscale30((x1<<4)-xcross*yb1[w],xcross*(yb2[w]-yb1[w])-((x2-x1)<<4));
		//inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		//intz = z1 + mulscale30(z2-z1,t);

		if ((bad&12) == 8)
		{
			if (xb1[w] <= xcross) { z2 = intz; iy2 = inty; ix2 = xcross; }
			clearshortbuf(&mostbuf[xcross+1],YDIM,xb2[w]-xcross);
		}
		else
		{
			if (xcross <= xb2[w]) { z1 = intz; iy1 = inty; ix1 = xcross; }
			clearshortbuf(&mostbuf[xb1[w]],YDIM,xcross-xb1[w]+1);
		}
	}

	y = (scale(z1,xdimenscale,iy1)<<4);
	yinc = ((scale(z2,xdimenscale,iy2)<<4)-y) / (ix2-ix1+1);
	qinterpolatedown16short(&mostbuf[ix1],ix2-ix1+1,y+(globalhoriz<<16),yinc);

	if (mostbuf[ix1] < 0) mostbuf[ix1] = 0;
	if (mostbuf[ix1] > YDIM) mostbuf[ix1] = YDIM;
	if (mostbuf[ix2] < 0) mostbuf[ix2] = 0;
	if (mostbuf[ix2] > YDIM) mostbuf[ix2] = YDIM;

	return(bad);
}


static int32_t getAbsoluteAddress(uint8_t __far* p)
{
	return (int32_t)p;
}


	//Sloped ceiling/floor vertical line function
static void a_slopevlin(uint8_t __far* p, int32_t bz, int32_t *slopaloffs, int32_t cnt, int32_t bx, int32_t by, uint8_t __far* _a_gbuf)
{
	int32_t *slopalptr, i, bzinc;
	uint32_t u, v;

	bzinc = (asm1>>3);
	slopalptr = slopaloffs;
	for(;cnt>0;cnt--)
	{
		i = krecip(bz>>6);
		bz += bzinc;
		u = bx+globalx3*i;
		v = by+globaly3*i;
		*p = *(uint8_t __far*)(slopalptr[0]+_a_gbuf[((u>>(32-_a_glogx))<<_a_glogy)+(v>>(32-_a_glogy))]);
		slopalptr--;
		p += _a_gpinc;
	}
}


#define BITSOFPRECISION 3  //Don't forget to change this in A.ASM also!
static void grouscan (int32_t dax1, int32_t dax2, int32_t sectnum, uint8_t dastat)
{
	int32_t i, j, l, x, y, dx, dy, wx, wy, y1, y2, daz, bz;
	int32_t daslope, dasqr;
	int32_t shoffs, shinc, m1, m2, *mptr1, *mptr2, *nptr1, *nptr2;
	uint8_t __far* p;
	walltype __far* wal;
	sectortype __far* sec;
	int16_t globalpicnum;
	uint8_t __far* bufplc;

	sec = &sector[sectnum];

	if (dastat == 0)
	{
		if (globalposz <= getceilzofslope(sectnum,globalposx,globalposy))
			return;  //Back-face culling
		globalorientation = sec->ceilingstat;
		globalpicnum = sec->ceilingpicnum;
		globalshade = sec->ceilingshade;
		globalpal = sec->ceilingpal;
		daslope = sec->ceilingheinum;
		daz = sec->ceilingz;
	}
	else
	{
		if (globalposz >= getflorzofslope(sectnum,globalposx,globalposy))
			return;  //Back-face culling
		globalorientation = sec->floorstat;
		globalpicnum = sec->floorpicnum;
		globalshade = sec->floorshade;
		globalpal = sec->floorpal;
		daslope = sec->floorheinum;
		daz = sec->floorz;
	}

	if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs(globalpicnum);
	if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) return;

	wal = &wall[sec->wallptr];
	wx = wall[wal->point2].x - wal->x;
	wy = wall[wal->point2].y - wal->y;
	dasqr = krecip(msqrtasm(wx*wx+wy*wy));
	i = mulscale21(daslope,dasqr);
	wx *= i; wy *= i;

	globalx = -mulscale19(singlobalang,xdimenrecip);
	globaly = mulscale19(cosglobalang,xdimenrecip);
	globalx1 = (globalposx<<8);
	globaly1 = -(globalposy<<8);
	i = (dax1-(XDIM>>1))*xdimenrecip;
	globalx2 = mulscale16(cosglobalang<<4,viewingrangerecip) - mulscale27(singlobalang,i);
	globaly2 = mulscale16(singlobalang<<4,viewingrangerecip) + mulscale27(cosglobalang,i);
	globalzd = (xdimscale<<9);
	globalzx = -dmulscale17(wx,globaly2,-wy,globalx2) + mulscale10(1-globalhoriz,globalzd);
	globalz = -dmulscale25(wx,globaly,-wy,globalx);

	if (globalorientation&64)  //Relative alignment
	{
		dx = mulscale14(wall[wal->point2].x-wal->x,dasqr);
		dy = mulscale14(wall[wal->point2].y-wal->y,dasqr);

		i = msqrtasm(daslope*daslope+16777216);

		x = globalx; y = globaly;
		globalx = dmulscale16(x,dx,y,dy);
		globaly = mulscale12(dmulscale16(-y,dx,x,dy),i);

		x = ((wal->x-globalposx)<<8); y = ((wal->y-globalposy)<<8);
		globalx1 = dmulscale16(-x,dx,-y,dy);
		globaly1 = mulscale12(dmulscale16(-y,dx,x,dy),i);

		x = globalx2; y = globaly2;
		globalx2 = dmulscale16(x,dx,y,dy);
		globaly2 = mulscale12(dmulscale16(-y,dx,x,dy),i);
	}
	if (globalorientation&0x4)
	{
		i = globalx; globalx = -globaly; globaly = -i;
		i = globalx1; globalx1 = globaly1; globaly1 = i;
		i = globalx2; globalx2 = -globaly2; globaly2 = -i;
	}
	if (globalorientation&0x10) { globalx1 = -globalx1, globalx2 = -globalx2, globalx = -globalx; }
	if (globalorientation&0x20) { globaly1 = -globaly1, globaly2 = -globaly2, globaly = -globaly; }

	daz = dmulscale9(wx,globalposy-wal->y,-wy,globalposx-wal->x) + ((daz-globalposz)<<8);
	globalx2 = mulscale20(globalx2,daz); globalx = mulscale28(globalx,daz);
	globaly2 = mulscale20(globaly2,-daz); globaly = mulscale28(globaly,-daz);

	i = 8-(picsiz[globalpicnum]&15); j = 8-(picsiz[globalpicnum]>>4);
	if (globalorientation&8) { i++; j++; }
	globalx1 <<= (i+12); globalx2 <<= i; globalx <<= i;
	globaly1 <<= (j+12); globaly2 <<= j; globaly <<= j;

	if (dastat == 0)
	{
		globalx1 += (((int32_t)sec->ceilingxpanning)<<24);
		globaly1 += (((int32_t)sec->ceilingypanning)<<24);
	}
	else
	{
		globalx1 += (((int32_t)sec->floorxpanning)<<24);
		globaly1 += (((int32_t)sec->floorypanning)<<24);
	}

	asm1 = -(globalzd>>(16-BITSOFPRECISION));

	globvis = globalvisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globvis = mulscale13(globvis,daz);
	globvis = mulscale16(globvis,xdimscale);
	p = palookup[globalpal];

	_a_glogx = picsiz[globalpicnum] & 15;
	_a_glogy = picsiz[globalpicnum] >> 4;

	bufplc = loadtile(globalpicnum);

	_a_gpinc = -ylookup[1];

	l = (globalzd>>16);

	shinc = mulscale16(globalz,xdimenscale);
	if (shinc > 0)
		shoffs = 4L << 15;
	else
		shoffs = (2044L - YDIM) << 15;
	if (dastat == 0) y1 = umost[dax1]; else y1 = max(umost[dax1],dplc[dax1]);
	m1 = mulscale16(y1,globalzd) + (globalzx>>6);
		//Avoid visibility overflow by crossing horizon
	if (globalzd > 0) m1 += (globalzd>>16); else m1 -= (globalzd>>16);
	m2 = m1+l;
	mptr1 = (int32_t *)&slopalookup[y1+(shoffs>>15)]; mptr2 = mptr1+1;

	for(x=dax1;x<=dax2;x++)
	{
		if (dastat == 0) { y1 = umost[x]; y2 = min(dmost[x],uplc[x])-1; }
						else { y1 = max(umost[x],dplc[x]); y2 = dmost[x]-1; }
		if (y1 <= y2)
		{
			nptr1 = (int32_t *)&slopalookup[y1+(shoffs>>15)];
			nptr2 = (int32_t *)&slopalookup[y2+(shoffs>>15)];
			while (nptr1 <= mptr1)
			{
				*mptr1-- = getAbsoluteAddress(p + (getpalookup(mulscale24(krecip(m1), globvis), globalshade) << 8));
				m1 -= l;
			}
			while (nptr2 >= mptr2)
			{
				*mptr2++ = getAbsoluteAddress(p + (getpalookup(mulscale24(krecip(m2), globvis), globalshade) << 8));
				m2 += l;
			}

			globalx3 = (globalx2>>10);
			globaly3 = (globaly2>>10);
			bz = mulscale16(y2,globalzd) + (globalzx>>6);
			a_slopevlin(ylookup[y2] + x + _s_screen, bz, nptr2, y2 - y1 + 1, globalx1, globaly1, bufplc);

			if ((x&15) == 0)
			{
				faketimerhandler();
			}
		}
		globalx2 += globalx;
		globaly2 += globaly;
		globalzx += globalz;
		shoffs += shinc;
	}

	Z_ChangeTagToCache(bufplc);
}


static void parascan (int32_t sectnum, uint8_t dastat, int32_t bunch)
{
	sectortype __far* sec;
	int32_t j, k, l, m, n, x, z, wallnum, nextsectnum, globalhorizbak;
	int16_t *topptr, *botptr;
	int16_t globalpicnum;

	sectnum = thesector[bunchfirst[bunch]]; sec = &sector[sectnum];

	globalhorizbak = globalhoriz;
	if (parallaxyscale != 65536)
		globalhoriz = mulscale16(globalhoriz-(YDIM>>1),parallaxyscale) + (YDIM>>1);
	globvis = globalpisibility;
	//globalorientation = 0L;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));

	if (dastat == 0)
	{
		globalpal = sec->ceilingpal;
		globalpicnum = sec->ceilingpicnum;
		globalshade = (int32_t)sec->ceilingshade;
		globalxpanning = (int32_t)sec->ceilingxpanning;
		globalypanning = (int32_t)sec->ceilingypanning;
		topptr = umost;
		botptr = uplc;
	}
	else
	{
		globalpal = sec->floorpal;
		globalpicnum = sec->floorpicnum;
		globalshade = (int32_t)sec->floorshade;
		globalxpanning = (int32_t)sec->floorxpanning;
		globalypanning = (int32_t)sec->floorypanning;
		topptr = dplc;
		botptr = dmost;
	}

	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);
	globalshiftval = (picsiz[globalpicnum]>>4);
	if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
	globalshiftval = 32-globalshiftval;
	globalzd = (((tilesizy[globalpicnum]>>1)+parallaxyoffs)<<globalshiftval)+(globalypanning<<24);
	globalyscale = (8<<(globalshiftval-19));
	//if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

	k = 11 - (picsiz[globalpicnum]&15) - pskybits;
	x = -1;

	for(z=bunchfirst[bunch];z>=0;z=p2[z])
	{
		wallnum = thewall[z]; nextsectnum = wall[wallnum].nextsector;

		if (dastat == 0) j = sector[nextsectnum].ceilingstat;
						else j = sector[nextsectnum].floorstat;

		if ((nextsectnum < 0) || (wall[wallnum].cstat&32) || ((j&1) == 0))
		{
			if (x == -1) x = xb1[z];

			if (parallaxtype == 0)
			{
				n = mulscale16(xdimenrecip,viewingrange);
				for(j=xb1[z];j<=xb2[z];j++)
					lplc[j] = (((mulscale23(j-(XDIM>>1),n)+globalang)&2047)>>k);
			}
			else
			{
				for(j=xb1[z];j<=xb2[z];j++)
					lplc[j] = ((((int32_t)radarang2[j]+globalang)&2047)>>k);
			}
			if (parallaxtype == 2)
			{
				n = mulscale16(xdimscale,viewingrange);
				for(j=xb1[z];j<=xb2[z];j++)
					swplc[j] = mulscale14(sintable[((int32_t)radarang2[j]+512)&2047],n);
			}
			else
				clearlongbuf(&swplc[xb1[z]],mulscale16(xdimscale,viewingrange),xb2[z]-xb1[z]+1);
		}
		else if (x >= 0)
		{
			l = globalpicnum; m = (picsiz[globalpicnum]&15);
			globalpicnum = l+pskyoff[lplc[x]>>m];

			if (((lplc[x]^lplc[xb1[z]-1])>>m) == 0)
				wallscan(x, xb1[z] - 1, topptr, botptr, swplc, lplc, globalpicnum);
			else
			{
				j = x;
				while (x < xb1[z])
				{
					n = l+pskyoff[lplc[x]>>m];
					if (n != globalpicnum)
					{
						wallscan(j, x - 1, topptr, botptr, swplc, lplc, globalpicnum);
						j = x;
						globalpicnum = n;
					}
					x++;
				}
				if (j < x)
					wallscan(j, x - 1, topptr, botptr, swplc, lplc, globalpicnum);
			}

			globalpicnum = l;
			x = -1;
		}
	}

	if (x >= 0)
	{
		l = globalpicnum; m = (picsiz[globalpicnum]&15);
		globalpicnum = l+pskyoff[lplc[x]>>m];

		if (((lplc[x]^lplc[xb2[bunchlast[bunch]]])>>m) == 0)
			wallscan(x, xb2[bunchlast[bunch]], topptr, botptr, swplc, lplc, globalpicnum);
		else
		{
			j = x;
			while (x <= xb2[bunchlast[bunch]])
			{
				n = l+pskyoff[lplc[x]>>m];
				if (n != globalpicnum)
				{
					wallscan(j, x - 1, topptr, botptr, swplc, lplc, globalpicnum);
					j = x;
					globalpicnum = n;
				}
				x++;
			}
			if (j <= x)
				wallscan(j, x, topptr, botptr, swplc, lplc, globalpicnum);
		}
		globalpicnum = l;
	}
	globalhoriz = globalhorizbak;
}
