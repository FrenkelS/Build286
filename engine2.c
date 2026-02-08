// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "draw.h"
#include "engine.h"
#include "p_setup.h"
#include "palette.h"
#include "tables.h"
#include "test.h"
#include "tiles.h"
#include "z_zone.h"


typedef struct {
	int32_t x1, y1, x2, y2;
} linetype;

static linetype __far clipit[MAXCLIPNUM];


static int16_t clipnum;
static int16_t hitwalls[4];


static int32_t rxi[8], ryi[8];
static int32_t rzi[8], rxi2[8], ryi2[8], rzi2[8];
static int32_t xsi[8], ysi[8];


#define klabs labs


static int32_t ksgn(int32_t a)
{
	return (a > 0) - (a < 0);
}


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
			a += b * 4;
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


static int32_t scale(int32_t a, int32_t b, int32_t c)
{
	return ((int64_t)a * b) / c;
}


static int32_t mulscale(int32_t a, int32_t b, uint_fast8_t c)
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


static int32_t dmulscale(int32_t a, int32_t b, int32_t c, int32_t d, uint_fast8_t e)
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


static int32_t divscale(int32_t a, int32_t b, uint_fast8_t c)
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


int32_t krecip(int32_t num)
{
	if (num == 0)
		return 1L << 30;

	return (1L << 30) / num;
}


void clearshortbuf(int16_t __far* s, int16_t c, size_t n)
{
	while (n--)
		*s++ = c;
}


void clearlongbuf(int32_t __far* s, int32_t c, size_t n)
{
	while (n--)
		*s++ = c;
}


static int_fast8_t rintersect(int32_t x1, int32_t y1, int32_t z1, int32_t vx, int32_t vy, int32_t vz, int32_t x3,
			  int32_t y3, int32_t x4, int32_t y4, int32_t *intx, int32_t *inty, int32_t *intz)
{     //p1 towards p2 is a ray
	int32_t x34, y34, x31, y31, bot, topt, topu, t;

	x34 = x3 - x4;
	y34 = y3 - y4;
	bot = vx * y34 - vy * x34;

	if (bot >= 0)
	{
		if (bot == 0)
			return 0;

		x31 = x3 - x1;
		y31 = y3 - y1;
		topt = x31 * y34 - y31 * x34;
		if (topt < 0)
			return 0;

		topu = vx * y31 - vy * x31;
		if ((topu < 0) || (topu >= bot))
			return 0;
	}
	else
	{
		x31 = x3 - x1;
		y31 = y3 - y1;
		topt = x31 * y34 - y31 * x34;
		if (topt > 0)
			return 0;

		topu = vx * y31 - vy * x31;
		if ((topu > 0) || (topu <= bot))
			return 0;
	}

	t = divscale16(topt, bot);
	*intx = x1 + mulscale16(vx, t);
	*inty = y1 + mulscale16(vy, t);
	*intz = z1 + mulscale16(vz, t);
	return 1;
}


static int32_t raytrace(int32_t x3, int32_t y3, int32_t *x4, int32_t *y4)
{
	int32_t x1, y1, x2, y2, bot, topu, nintx, ninty, cnt, hitwall;
	int16_t z;
	int32_t x21, y21, x43, y43;

	hitwall = -1;
	for(z=clipnum-1;z>=0;z--)
	{
		x1 = clipit[z].x1; x2 = clipit[z].x2; x21 = x2-x1;
		y1 = clipit[z].y1; y2 = clipit[z].y2; y21 = y2-y1;

		topu = x21*(y3-y1) - (x3-x1)*y21; if (topu <= 0) continue;
		if (x21*(*y4-y1) > (*x4-x1)*y21) continue;
		x43 = *x4-x3; y43 = *y4-y3;
		if (x43*(y1-y3) > (x1-x3)*y43) continue;
		if (x43*(y2-y3) <= (x2-x3)*y43) continue;
		bot = x43*y21 - x21*y43; if (bot == 0) continue;

		cnt = 256;
		do
		{
			cnt--; if (cnt < 0) { *x4 = x3; *y4 = y3; return z; }
			nintx = x3 + scale(x43,topu,bot);
			ninty = y3 + scale(y43,topu,bot);
			topu--;
		} while (x21*(ninty-y1) <= (nintx-x1)*y21);

		if (klabs(x3-nintx)+klabs(y3-ninty) < klabs(x3-*x4)+klabs(y3-*y4))
			{ *x4 = nintx; *y4 = ninty; hitwall = z; }
	}

	return hitwall;
}


static void keepaway(int32_t *x, int32_t *y, int16_t w)
{
	int32_t dx, dy, ox, oy, x1, y1;
	uint8_t first;

	x1 = clipit[w].x1; dx = clipit[w].x2-x1;
	y1 = clipit[w].y1; dy = clipit[w].y2-y1;
	ox = ksgn(-dy); oy = ksgn(dx);
	first = (klabs(dx) <= klabs(dy));
	while (1)
	{
		if (dx*(*y-y1) > (*x-x1)*dy) return;
		if (first == 0) *x += ox; else *y += oy;
		first ^= 1;
	}
}


#define addclipline(dax1, day1, dax2, day2)      \
{                                                        \
	clipit[clipnum].x1 = dax1; clipit[clipnum].y1 = day1; \
	clipit[clipnum].x2 = dax2; clipit[clipnum].y2 = day2; \
	clipnum++;                                            \
}                                                        \


static const int32_t clipmoveboxtracenum = 3;


void clipmove(int32_t *x, int32_t *y, int32_t *z, int16_t *sectnum,
			 int32_t xvect, int32_t yvect,
			 int32_t walldist, int32_t ceildist, int32_t flordist, uint32_t cliptype)
{
	walltype __far* wal;
	walltype __far* wal2;
	spritetype __far* spr;
	sectortype __far* sec;
	sectortype __far* sec2;
	int32_t i, templong1, templong2;
	int16_t j;
	int32_t oxvect, oyvect, goalx, goaly, intx, inty, lx, ly;
	int32_t k, l, endwall, cstat;
	int16_t clipsectcnt, startwall, dasect;
	int32_t x1, y1, x2, y2, cx, cy, rad, xmin, ymin, xmax, ymax, daz, daz2;
	int32_t bsz, dax, day, xoff, yoff, xspan, yspan, cosang, sinang;
	int16_t tilenum;
	int32_t xrepeat, yrepeat, gx, gy, dx, dy, dasprclipmask, dawalclipmask;
	int32_t hitwall, cnt, clipyou;

	if (((xvect|yvect) == 0) || (*sectnum < 0))
		return;

	oxvect = xvect;
	oyvect = yvect;

	goalx = (*x) + (xvect>>14);
	goaly = (*y) + (yvect>>14);


	clipnum = 0;

	cx = (((*x)+goalx)>>1);
	cy = (((*y)+goaly)>>1);
		//Extra walldist for sprites on sector lines
	gx = goalx-(*x); gy = goaly-(*y);
	rad = msqrtasm(gx*gx + gy*gy) + MAXCLIPDIST+walldist + 8;
	xmin = cx-rad; ymin = cy-rad;
	xmax = cx+rad; ymax = cy+rad;

	dawalclipmask = (cliptype&65535);        //CLIPMASK0 = 0x00010001
	dasprclipmask = (cliptype>>16);          //CLIPMASK1 = 0x01000040

	clipsectorlist[0] = (*sectnum);
	clipsectcnt = 0; clipsectnum = 1;
	do
	{
		dasect = clipsectorlist[clipsectcnt++];
		sec = &sector[dasect];
		startwall = sec->wallptr; endwall = startwall + sec->wallnum;
		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			wal2 = &wall[wal->point2];
			if ((wal->x < xmin) && (wal2->x < xmin)) continue;
			if ((wal->x > xmax) && (wal2->x > xmax)) continue;
			if ((wal->y < ymin) && (wal2->y < ymin)) continue;
			if ((wal->y > ymax) && (wal2->y > ymax)) continue;

			x1 = wal->x; y1 = wal->y; x2 = wal2->x; y2 = wal2->y;

			dx = x2-x1; dy = y2-y1;
			if (dx*((*y)-y1) < ((*x)-x1)*dy) continue;  //If wall's not facing you

			if (dx > 0) dax = dx*(ymin-y1); else dax = dx*(ymax-y1);
			if (dy > 0) day = dy*(xmax-x1); else day = dy*(xmin-x1);
			if (dax >= day) continue;

			clipyou = 0;
			if ((wal->nextsector < 0) || (wal->cstat&dawalclipmask)) clipyou = 1;
			else if (editstatus == 0)
			{
				if (rintersect(*x,*y,0,gx,gy,0,x1,y1,x2,y2,&dax,&day,&daz) == 0)
					dax = *x, day = *y;
				daz = getflorzofslope((int16_t)dasect,dax,day);
				daz2 = getflorzofslope(wal->nextsector,dax,day);

				sec2 = &sector[wal->nextsector];
				if (daz2 < daz-(1<<8))
					if ((sec2->floorstat&1) == 0)
						if ((*z) >= daz2-(flordist-1)) clipyou = 1;
				if (clipyou == 0)
				{
					daz = getceilzofslope((int16_t)dasect,dax,day);
					daz2 = getceilzofslope(wal->nextsector,dax,day);
					if (daz2 > daz+(1<<8))
						if ((sec2->ceilingstat&1) == 0)
							if ((*z) <= daz2+(ceildist-1)) clipyou = 1;
				}
			}

			if (clipyou)
			{
					//Add 2 boxes at endpoints
				bsz = walldist; if (gx < 0) bsz = -bsz;
				addclipline(x1 - bsz, y1 - bsz, x1 - bsz, y1 + bsz);
				addclipline(x2 - bsz, y2 - bsz, x2 - bsz, y2 + bsz);
				bsz = walldist; if (gy < 0) bsz = -bsz;
				addclipline(x1 + bsz, y1 - bsz, x1 - bsz, y1 - bsz);
				addclipline(x2 + bsz, y2 - bsz, x2 - bsz, y2 - bsz);

				dax = walldist; if (dy > 0) dax = -dax;
				day = walldist; if (dx < 0) day = -day;
				addclipline(x1 + dax, y1 + day, x2 + dax, y2 + day);
			}
			else
			{
				for(i=clipsectnum-1;i>=0;i--)
					if (wal->nextsector == clipsectorlist[(size_t)i]) break;
				if (i < 0) clipsectorlist[clipsectnum++] = wal->nextsector;
			}
		}

		for(j=headspritesect[dasect];j>=0;j=nextspritesect[j])
		{
			spr = &sprite[j];
			cstat = spr->cstat;
			if ((cstat&dasprclipmask) == 0) continue;
			x1 = spr->x; y1 = spr->y;
			switch(cstat&48)
			{
				case 0:
					if ((x1 >= xmin) && (x1 <= xmax) && (y1 >= ymin) && (y1 <= ymax))
					{
						k = ((tilesizy[spr->picnum]*spr->yrepeat)<<2);
						if (cstat&128) daz = spr->z+(k>>1); else daz = spr->z;
						if (picanm[spr->picnum]&0x00ff0000) daz -= ((int32_t)((int8_t)((picanm[spr->picnum]>>16)&255))*spr->yrepeat<<2);
						if (((*z) < daz+ceildist) && ((*z) > daz-k-flordist))
						{
							bsz = (spr->clipdist<<2)+walldist; if (gx < 0) bsz = -bsz;
							addclipline(x1 - bsz, y1 - bsz, x1 - bsz, y1 + bsz);
							bsz = (spr->clipdist<<2)+walldist; if (gy < 0) bsz = -bsz;
							addclipline(x1 + bsz, y1 - bsz, x1 - bsz, y1 - bsz);
						}
					}
					break;
				case 16:
					k = ((tilesizy[spr->picnum]*spr->yrepeat)<<2);
					if (cstat&128) daz = spr->z+(k>>1); else daz = spr->z;
					if (picanm[spr->picnum]&0x00ff0000) daz -= ((int32_t)((int8_t)((picanm[spr->picnum]>>16)&255))*spr->yrepeat<<2);
					daz2 = daz-k;
					daz += ceildist; daz2 -= flordist;
					if (((*z) < daz) && ((*z) > daz2))
					{
							//These lines get the 2 points of the rotated sprite
							//Given: (x1, y1) starts out as the center point
						tilenum = spr->picnum;
						xoff = (int32_t)((int8_t)((picanm[tilenum]>>8)&255))+((int32_t)spr->xoffset);
						if ((cstat&4) > 0) xoff = -xoff;
						k = spr->ang; l = spr->xrepeat;
						dax = sintable[(size_t)k & 2047]*l; day = sintable[((size_t)k + 1536) & 2047]*l;
						l = tilesizx[tilenum]; k = (l>>1)+xoff;
						x1 -= mulscale16(dax,k); x2 = x1+mulscale16(dax,l);
						y1 -= mulscale16(day,k); y2 = y1+mulscale16(day,l);
						if (clipinsideboxline(cx,cy,x1,y1,x2,y2,rad) != 0)
						{
							dax = mulscale14(sintable[(spr->ang+256+512)&2047],walldist);
							day = mulscale14(sintable[(spr->ang+256)&2047],walldist);

							if ((x1-(*x))*(y2-(*y)) >= (x2-(*x))*(y1-(*y)))   //Front
							{
								addclipline(x1 + dax, y1 + day, x2 + day, y2 - dax);
							}
							else
							{
								if ((cstat&64) != 0) continue;
								addclipline(x2 - dax, y2 - day, x1 - day, y1 + dax);
							}

								//Side blocker
							if ((x2-x1)*((*x)-x1) + (y2-y1)*((*y)-y1) < 0)
								{ addclipline(x1 - day, y1 + dax, x1 + dax, y1 + day); }
							else if ((x1-x2)*((*x)-x2) + (y1-y2)*((*y)-y2) < 0)
								{ addclipline(x2 + day, y2 - dax, x2 - dax, y2 - day); }
						}
					}
					break;
				case 32:
					daz = spr->z+ceildist;
					daz2 = spr->z-flordist;
					if (((*z) < daz) && ((*z) > daz2))
					{
						if ((cstat&64) != 0)
							if (((*z) > spr->z) == ((cstat&8)==0)) continue;

						tilenum = spr->picnum;
						xoff = (int32_t)((int8_t)((picanm[tilenum]>>8)&255))+((int32_t)spr->xoffset);
						yoff = (int32_t)((int8_t)((picanm[tilenum]>>16)&255))+((int32_t)spr->yoffset);
						if ((cstat&4) > 0) xoff = -xoff;
						if ((cstat&8) > 0) yoff = -yoff;

						k = spr->ang;
						cosang = sintable[((size_t)k + 512) & 2047]; sinang = sintable[(size_t)k];
						xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
						yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

						dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
						rxi[0] = x1 + dmulscale16(sinang,dax,cosang,day);
						ryi[0] = y1 + dmulscale16(sinang,day,-cosang,dax);
						l = xspan*xrepeat;
						rxi[1] = rxi[0] - mulscale16(sinang,l);
						ryi[1] = ryi[0] + mulscale16(cosang,l);
						l = yspan*yrepeat;
						k = -mulscale16(cosang,l); rxi[2] = rxi[1]+k; rxi[3] = rxi[0]+k;
						k = -mulscale16(sinang,l); ryi[2] = ryi[1]+k; ryi[3] = ryi[0]+k;

						dax = mulscale14(sintable[(spr->ang-256+512)&2047],walldist);
						day = mulscale14(sintable[(spr->ang-256)&2047],walldist);

						if ((rxi[0]-(*x))*(ryi[1]-(*y)) < (rxi[1]-(*x))*(ryi[0]-(*y)))
						{
							if (clipinsideboxline(cx,cy,rxi[1],ryi[1],rxi[0],ryi[0],rad) != 0)
								addclipline(rxi[1] - day, ryi[1] + dax, rxi[0] + dax, ryi[0] + day);
						}
						else if ((rxi[2]-(*x))*(ryi[3]-(*y)) < (rxi[3]-(*x))*(ryi[2]-(*y)))
						{
							if (clipinsideboxline(cx,cy,rxi[3],ryi[3],rxi[2],ryi[2],rad) != 0)
								addclipline(rxi[3] + day, ryi[3] - dax, rxi[2] - dax, ryi[2] - day);
						}

						if ((rxi[1]-(*x))*(ryi[2]-(*y)) < (rxi[2]-(*x))*(ryi[1]-(*y)))
						{
							if (clipinsideboxline(cx,cy,rxi[2],ryi[2],rxi[1],ryi[1],rad) != 0)
								addclipline(rxi[2] - dax, ryi[2] - day, rxi[1] - day, ryi[1] + dax);
						}
						else if ((rxi[3]-(*x))*(ryi[0]-(*y)) < (rxi[0]-(*x))*(ryi[3]-(*y)))
						{
							if (clipinsideboxline(cx,cy,rxi[0],ryi[0],rxi[3],ryi[3],rad) != 0)
								addclipline(rxi[0] + dax, ryi[0] + day, rxi[3] + day, ryi[3] - dax);
						}
					}
					break;
			}
		}
	} while (clipsectcnt < clipsectnum);


	hitwall = 0;
	cnt = clipmoveboxtracenum;
	do
	{
		intx = goalx; inty = goaly;
		if ((hitwall = raytrace(*x, *y, &intx, &inty)) >= 0)
		{
			lx = clipit[(size_t)hitwall].x2-clipit[(size_t)hitwall].x1;
			ly = clipit[(size_t)hitwall].y2-clipit[(size_t)hitwall].y1;
			templong2 = lx*lx + ly*ly;
			if (templong2 > 0)
			{
				templong1 = (goalx-intx)*lx + (goaly-inty)*ly;

				if ((klabs(templong1)>>11) < templong2)
					i = divscale20(templong1,templong2);
				else
					i = 0;
				goalx = mulscale20(lx,i)+intx;
				goaly = mulscale20(ly,i)+inty;
			}

			templong1 = dmulscale6(lx,oxvect,ly,oyvect);
			for(i=cnt+1;i<=clipmoveboxtracenum;i++)
			{
				j = hitwalls[(size_t)i];
				templong2 = dmulscale6(clipit[j].x2-clipit[j].x1,oxvect,clipit[j].y2-clipit[j].y1,oyvect);
				if ((templong1^templong2) < 0)
				{
					updatesector(*x,*y,sectnum);
					return;
				}
			}

			keepaway(&goalx, &goaly, hitwall);
			xvect = ((goalx-intx)<<14);
			yvect = ((goaly-inty)<<14);

			hitwalls[(size_t)cnt] = hitwall;
		}
		cnt--;

		*x = intx;
		*y = inty;
	} while (((xvect|yvect) != 0) && (hitwall >= 0) && (cnt > 0));

	for(j=0;j<clipsectnum;j++)
		if (inside(*x,*y,clipsectorlist[j]) == 1)
		{
			*sectnum = clipsectorlist[j];
			return;
		}

	*sectnum = -1; templong1 = 0x7fffffff;
	for(j=numsectors-1;j>=0;j--)
		if (inside(*x,*y,j) == 1)
		{
			if (sector[j].ceilingstat&2)
				templong2 = (getceilzofslope(j,*x,*y)-(*z));
			else
				templong2 = (sector[j].ceilingz-(*z));

			if (templong2 > 0)
			{
				if (templong2 < templong1)
					{ *sectnum = j; templong1 = templong2; }
			}
			else
			{
				if (sector[j].floorstat&2)
					templong2 = ((*z)-getflorzofslope(j,*x,*y));
				else
					templong2 = ((*z)-sector[j].floorz);

				if (templong2 <= 0)
				{
					*sectnum = j;
					return;
				}
				if (templong2 < templong1)
					{ *sectnum = j; templong1 = templong2; }
			}
		}
}


int32_t getceilzofslope(int16_t sectnum, int32_t dax, int32_t day)
{
	int32_t dx, dy, i, j;
	walltype __far* wal;

	if (!(sector[sectnum].ceilingstat & 2))
		return sector[sectnum].ceilingz;

	wal = &wall[sector[sectnum].wallptr];
	dx = wall[wal->point2].x - wal->x;
	dy = wall[wal->point2].y - wal->y;
	i = msqrtasm(dx * dx + dy * dy) << 5;
	if (i == 0)
		return sector[sectnum].ceilingz;

	j = dmulscale3(dx, day - wal->y, -dy, dax - wal->x);
	return sector[sectnum].ceilingz + scale(sector[sectnum].ceilingheinum, j, i);
}


int32_t getflorzofslope(int16_t sectnum, int32_t dax, int32_t day)
{
	int32_t dx, dy, i, j;
	walltype __far* wal;

	if (!(sector[sectnum].floorstat & 2))
		return(sector[sectnum].floorz);

	wal = &wall[sector[sectnum].wallptr];
	dx = wall[wal->point2].x - wal->x;
	dy = wall[wal->point2].y - wal->y;
	i = msqrtasm(dx * dx + dy * dy) << 5;
	if (i == 0)
		return sector[sectnum].floorz;

	j = dmulscale3(dx, day - wal->y, -dy, dax - wal->x);
	return sector[sectnum].floorz + scale(sector[sectnum].floorheinum, j, i);
}


int_fast8_t clipinsideboxline(int32_t x, int32_t y, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t walldist)
{
	int32_t r;

	r = walldist << 1;

	x1 += walldist - x;
	x2 += walldist - x;
	if ((x1 < 0) && (x2 < 0))
		return 0;
	if ((x1 >= r) && (x2 >= r))
		return 0;

	y1 += walldist - y;
	y2 += walldist - y;
	if ((y1 < 0) && (y2 < 0))
		return 0;
	if ((y1 >= r) && (y2 >= r))
		return 0;

	x2 -= x1;
	y2 -= y1;
	if (x2 * (walldist - y1) >= y2 * (walldist - x1))  //Front
	{
		if (x2 > 0)
			x2 *= (0 - y1);
		else
			x2 *= (r - y1);

		if (y2 > 0)
			y2 *= (r - x1);
		else
			y2 *= (0 - x1);

		return x2 < y2;
	}

	if (x2 > 0)
		x2 *= (r - y1);
	else
		x2 *= (0 - y1);

	if (y2 > 0)
		y2 *= (0 - x1);
	else
		y2 *= (r - x1);

	return (x2 >= y2) << 1;
}


int_fast8_t inside(int32_t x, int32_t y, int16_t sectnum)
{
	walltype __far* wal;
	int16_t i;
	int32_t x1, y1, x2, y2;
	uint32_t cnt;

	if ((sectnum < 0) || (sectnum >= numsectors))
		return -1;

	cnt = 0;
	wal = &wall[sector[sectnum].wallptr];
	i = sector[sectnum].wallnum;
	do
	{
		y1 = wal->y - y;
		y2 = wall[wal->point2].y - y;
		if ((y1 ^ y2) < 0)
		{
			x1 = wal->x - x;
			x2 = wall[wal->point2].x - x;
			if ((x1 ^ x2) >= 0)
				cnt ^= x1;
			else
				cnt ^= (x1 * y2 - x2 * y1) ^ y2;
		}
		wal++;
		i--;
	} while (i);

	return cnt >> 31;
}


static void qinterpolatedown16(void __far* bufptr, int32_t num, int32_t val, int32_t add)
{
	int32_t i;
	int32_t __far* lptr = (int32_t __far*)bufptr;
	for (i = 0; i < num; i++)
	{
		lptr[(size_t)i] = (val >> 16);
		val += add;
	}
}


void qinterpolatedown16short(void __far* bufptr, int32_t num, int32_t val, int32_t add)
{
	int32_t i;
	int16_t __far* sptr = (int16_t __far*)bufptr;
	for (i = 0; i < num; i++)
	{
		sptr[(size_t)i] = (int16_t)(val >> 16);
		val += add;
	}
}


	//Floor sprite horizontal line function
static void a_mhline(uint8_t __far* bufplc, uint8_t __far* paloffs, uint32_t bx, int32_t cntup16, uint32_t by, uint8_t __far* p)
{
	uint8_t ch;

	for (cntup16 >>= 16; cntup16 > 0; cntup16--)
	{
		ch = bufplc[(size_t)(((bx >> (32 - _a_glogx)) << _a_glogy) + (by >> (32 - _a_glogy)))];
		if (ch != 255)
			*p = paloffs[ch];
		bx += asm1;
		by += asm2;
		p++;
	}
}


	//Floor sprite horizontal line function
static void a_thline(uint8_t __far* bufplc, uint8_t __far* paloffs, uint32_t bx, int32_t cntup16, uint32_t by, uint8_t __far* p)
{
	uint8_t ch;

	if (_a_transmode)
	{
		for(cntup16 >>= 16; cntup16 > 0; cntup16--)
		{
			ch = bufplc[(size_t)(((bx >> (32 - _a_glogx)) << _a_glogy) + (by >> (32 - _a_glogy)))];
			if (ch != 255)
				*p = translucfunc(*p, paloffs[ch]);
			bx += asm1;
			by += asm2;
			p++;
		}
	}
	else
	{
		for(cntup16 >>= 16; cntup16 > 0; cntup16--)
		{
			ch = bufplc[(size_t)(((bx >> (32 - _a_glogx)) << _a_glogy) + (by >> (32 - _a_glogy)))];
			if (ch != 255)
				*p = translucfunc(paloffs[ch], *p);
			bx += asm1;
			by += asm2;
			p++;
		}
	}
}


static void ceilspritehline(int32_t x2, int32_t y, uint8_t __far* globalbufplc)
{
	int32_t x1, v, bx, by;
	uint8_t __far* palookupoffs;

	//x = x1 + (x2-x1)t + (y1-y2)u  ³  x = 160v
	//y = y1 + (y2-y1)t + (x2-x1)u  ³  y = (scrx-160)v
	//z = z1 = z2                   ³  z = posz + (scry-horiz)v

	x1 = lastx[(size_t)y];
	if (x2 < x1)
		return;

	v = mulscale20(globalzd,horizlookup[(size_t)(y-globalhoriz+((YDIM*4)>>1))]);
	bx = mulscale14(globalx2*x1+globalx1,v) + globalxpanning;
	by = mulscale14(globaly2*x1+globaly1,v) + globalypanning;
	asm1 = mulscale14(globalx2,v);
	asm2 = mulscale14(globaly2,v);

	palookupoffs = palookup[(size_t)globalpal] + (getpalookup(mulscale28(klabs(v),globvis),globalshade)<<8);

	if ((globalorientation&2) == 0)
		a_mhline(globalbufplc,palookupoffs,bx,(x2-x1)<<16,by,ylookup[(size_t)y]+x1+_s_screen);
	else
		a_thline(globalbufplc,palookupoffs,bx,(x2-x1)<<16,by,ylookup[(size_t)y]+x1+_s_screen);
}


static void ceilspritescan(int32_t x1, int32_t x2, uint8_t __far* globalbufplc)
{
	int32_t x, twall, bwall;
	int16_t y1, y2;

	y1 = uwall[(size_t)x1]; y2 = y1;
	for(x=x1;x<=x2;x++)
	{
		twall = uwall[(size_t)x]-1; bwall = dwall[(size_t)x];
		if (twall < bwall-1)
		{
			if (twall >= y2)
			{
				while (y1 < y2-1)
					ceilspritehline(x - 1, ++y1, globalbufplc);

				y1 = twall;
			}
			else
			{
				while (y1 < twall)
					ceilspritehline(x - 1, ++y1, globalbufplc);

				while (y1 > twall)
					lastx[y1--] = x;
			}
			while (y2 > bwall)
				ceilspritehline(x - 1, --y2, globalbufplc);

			while (y2 < bwall)
				lastx[y2++] = x;
		}
		else
		{
			while (y1 < y2-1)
				ceilspritehline(x - 1, ++y1, globalbufplc);

			if (x == x2)
				break;

			y1 = uwall[(size_t)(x + 1)];
			y2 = y1;
		}
	}

	while (y1 < y2-1)
		ceilspritehline(x2, ++y1, globalbufplc);

	faketimerhandler();
}


int_fast8_t spritewallfront(spritetype __far* s, int_fast16_t w)
{
	walltype __far* wal;
	int32_t x1, y1;

	wal = &wall[w];
	x1 = wal->x;
	y1 = wal->y;
	wal = &wall[wal->point2];
	return dmulscale32(wal->x - x1, s->y - y1, -(s->x - x1), wal->y - y1) >= 0;
}


	//Wall,face sprite/wall sprite vertical line function
static void a_mvlineasm1(int32_t vinc, uint8_t __far* paloffs, int16_t cnt, uint32_t vplc, uint8_t __far* bufplc, uint8_t __far* p)
{
	uint8_t ch;

	for (; cnt >= 0; cnt--)
	{
		ch = bufplc[(size_t)(vplc >> _a_glogy)];

		if (ch != 255)
			*p = paloffs[ch];

		p    += XDIM;
		vplc += vinc;
	}
}


void maskwallscan(int_fast16_t x1, int_fast16_t x2, int16_t __far* uwal, int16_t __far* dwal, int32_t __far* swal, int32_t __far* lwal, int16_t globalpicnum)
{
	int_fast16_t x;
	int32_t startx, xnice, ynice;
	int16_t y1ve, y2ve;
	int32_t tsizx, tsizy;
	uint8_t __far* p;
	uint8_t __far* fpalookup;
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

	bufplc = loadtile(globalpicnum);

	startx = x1;

	xnice = (pow2long[picsiz[globalpicnum]&15] == tsizx);
	if (xnice) tsizx = (tsizx-1);
	ynice = (pow2long[picsiz[globalpicnum]>>4] == tsizy);
	if (ynice) tsizy = (picsiz[globalpicnum]>>4);

	fpalookup = palookup[(size_t)globalpal];

	_a_glogy = globalshiftval;

	p = startx+_s_screen;
	for(x=startx;x<=x2;x++,p++)
	{
		y1ve = max(uwal[x], startumost[x]);
		y2ve = min(dwal[x], startdmost[x]);
		if (y2ve <= y1ve) continue;

		palookupoffse = fpalookup+(getpalookup(mulscale16(swal[x],globvis),globalshade)<<8);

		bufplce = lwal[x] + globalxpanning;
		if (bufplce >= tsizx) { if (xnice == 0) bufplce %= tsizx; else bufplce &= tsizx; }
		if (ynice == 0) bufplce *= tsizy; else bufplce <<= tsizy;

		vince = swal[x] * globalyscale;
		vplce = globalzd + vince * (y1ve - globalhoriz + 1);

		a_mvlineasm1(vince, palookupoffse, y2ve - y1ve - 1, vplce, bufplce + bufplc, p + ylookup[y1ve]);
	}

	Z_ChangeTagToCache(bufplc);

	faketimerhandler();
}


	//Wall,face sprite/wall sprite vertical line function
static void a_tvlineasm1(int32_t vinc, uint8_t __far* paloffs, int32_t cnt, uint32_t vplc, uint8_t __far* bufplc, uint8_t __far* p)
{
	uint8_t ch;

	if (_a_transmode)
	{
		for(; cnt >= 0; cnt--)
		{
			ch = bufplc[(size_t)(vplc >> _a_glogy)];
			if (ch != 255)
				*p = translucfunc(*p, paloffs[ch]);
			p    += XDIM;
			vplc += vinc;
		}
	}
	else
	{
		for(; cnt >= 0; cnt--)
		{
			ch = bufplc[(size_t)(vplc >> _a_glogy)];
			if (ch != 255)
				*p = translucfunc(paloffs[ch], *p);
			p    += XDIM;
			vplc += vinc;
		}
	}
}


static void transmaskvline(int_fast16_t x, int16_t globalpicnum, uint8_t __far* bufplc)
{
	int32_t vplc, vinc, i;
	uint8_t __far* p;
	uint8_t __far* palookupoffs;
	int16_t y1v, y2v;

	if ((x < 0) || (x >= XDIM))
		return;

	y1v = max(uwall[x],startumost[x]);
	y2v = min(dwall[x],startdmost[x]);
	y2v--;
	if (y2v < y1v)
		return;

	palookupoffs = palookup[(size_t)globalpal] + (getpalookup(mulscale16(swall[x],globvis),globalshade)<<8);

	vinc = swall[x]*globalyscale;
	vplc = globalzd + vinc*(y1v-globalhoriz+1);

	i = lwall[x]+globalxpanning;
	if (i >= tilesizx[globalpicnum])
		i %= tilesizx[globalpicnum];
	bufplc += i * tilesizy[globalpicnum];

	p = ylookup[y1v]+x+_s_screen;

	a_tvlineasm1(vinc,palookupoffs,y2v-y1v,vplc,bufplc,p);
}


void transmaskwallscan(int_fast16_t x1, int_fast16_t x2, int16_t globalpicnum)
{
	int_fast16_t x;
	uint8_t __far* bufplc;

	if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0))
		return;

	bufplc = loadtile(globalpicnum);

	_a_glogy = globalshiftval;

	x = x1;
	while ((startumost[x] > startdmost[x]) && (x <= x2))
		x++;

	while (x <= x2)
	{
		transmaskvline(x, globalpicnum, bufplc);
		x++;
	}

	Z_ChangeTagToCache(bufplc);

	faketimerhandler();
}


uint8_t owallmost(int16_t __far* mostbuf, int_fast16_t w, int32_t z)
{
	uint8_t bad;
	int32_t inty, xcross, y, yinc;
	int32_t s1, s2, s3, s4, ix1, ix2, iy1, iy2, t;

	z <<= 7;
	s1 = mulscale20(globaluclip,yb1[w]); s2 = mulscale20(globaluclip,yb2[w]);
	s3 = mulscale20(globaldclip,yb1[w]); s4 = mulscale20(globaldclip,yb2[w]);
	bad = (z < s1) + ((z < s2) << 1) + ((z > s3) << 2) + ((z > s4) << 3);

	ix1 = xb1[w]; iy1 = yb1[w];
	ix2 = xb2[w]; iy2 = yb2[w];

	if ((bad & 3) == 3)
	{
		_fmemset(&mostbuf[(size_t)ix1], 0, (ix2 - ix1 + 1) * sizeof(mostbuf[0]));
		return bad;
	}

	if ((bad & 12) == 12)
	{
		clearshortbuf(&mostbuf[(size_t)ix1], YDIM, ix2 - ix1 + 1);
		return bad;
	}

	if (bad & 3)
	{
		t = divscale30(z - s1, s2 - s1);
		inty   = yb1[w] + mulscale30(yb2[w] - yb1[w], t);
		xcross = xb1[w] + scale(mulscale30(yb2[w], t), xb2[w] - xb1[w], inty);

		if ((bad & 3) == 2)
		{
			if (xb1[w] <= xcross) { iy2 = inty; ix2 = xcross; }
			_fmemset(&mostbuf[(size_t)xcross + 1], 0, (xb2[w] - xcross) * sizeof(mostbuf[0]));
		}
		else
		{
			if (xcross <= xb2[w]) { iy1 = inty; ix1 = xcross; }
			_fmemset(&mostbuf[(size_t)xb1[w]], 0, (xcross - xb1[w] + 1) * sizeof(mostbuf[0]));
		}
	}

	if (bad & 12)
	{
		t = divscale30(z - s3, s4 - s3);
		inty   = yb1[w] + mulscale30(yb2[w] - yb1[w], t);
		xcross = xb1[w] + scale(mulscale30(yb2[w], t), xb2[w] - xb1[w], inty);

		if ((bad & 12) == 8)
		{
			if (xb1[w] <= xcross) { iy2 = inty; ix2 = xcross; }
			clearshortbuf(&mostbuf[(size_t)xcross + 1], YDIM, xb2[w] - xcross);
		}
		else
		{
			if (xcross <= xb2[w]) { iy1 = inty; ix1 = xcross; }
			clearshortbuf(&mostbuf[(size_t)xb1[w]], YDIM, xcross - xb1[w] + 1);
		}
	}

	y = scale(z, xdimenscale, iy1) << 4;
	yinc = ((scale(z, xdimenscale, iy2) << 4) - y) / (ix2 - ix1 + 1);
	qinterpolatedown16short(&mostbuf[(size_t)ix1], ix2 - ix1 + 1, y + (globalhoriz << 16), yinc);

	if (mostbuf[(size_t)ix1] < 0)    mostbuf[(size_t)ix1] = 0;
	if (mostbuf[(size_t)ix1] > YDIM) mostbuf[(size_t)ix1] = YDIM;
	if (mostbuf[(size_t)ix2] < 0)    mostbuf[(size_t)ix2] = 0;
	if (mostbuf[(size_t)ix2] > YDIM) mostbuf[(size_t)ix2] = YDIM;

	return bad;
}


static void drawspriteFace(int32_t yp, spritetype __far* tspr, int16_t tilenum,
							int32_t xb, int32_t xoff, int32_t cstat,
							int32_t yoff, sectortype __far* sec, int32_t sectnum,
							int32_t spritenum)
{
	int32_t startum, startdm;
	int32_t siz, xsiz, ysiz, xspan, yspan;
	int32_t x1, y1, x2, y2, lx, rx, dalx2, darx2, i, j, k, x, linum, linuminc;
	int32_t yinc, z1, z2;
	int32_t xv;
	uint8_t daclip;
	int16_t globalpicnum;

	if (yp <= (4<<8)) return;

	siz = divscale19(xdimenscale,yp);

	xv = mulscale16(((int32_t)tspr->xrepeat)<<16,xyaspect);

	xspan = tilesizx[tilenum];
	yspan = tilesizy[tilenum];
	xsiz = mulscale30(siz,xv*xspan);
	ysiz = mulscale14(siz,tspr->yrepeat*yspan);

	if (((tilesizx[tilenum]>>11) >= xsiz) || (yspan >= (ysiz>>1)))
		return;  //Watch out for divscale overflow

	x1 = xb-(xsiz>>1);
	if (xspan&1) x1 += mulscale31(siz,xv);  //Odd xspans
	i = mulscale30(siz,xv*xoff);
	if ((cstat&4) == 0) x1 -= i; else x1 += i;

	y1 = mulscale16(tspr->z-globalposz,siz);
	y1 -= mulscale14(siz,tspr->yrepeat*yoff);
	y1 += (globalhoriz<<8)-ysiz;
	if (cstat&128)
	{
		y1 += (ysiz>>1);
		if (yspan&1) y1 += mulscale15(siz,tspr->yrepeat);  //Odd yspans
	}

	x2 = x1+xsiz-1;
	y2 = y1+ysiz-1;
	if ((y1|255) >= (y2|255)) return;

	lx = (x1>>8)+1; if (lx < 0) lx = 0;
	rx = (x2>>8); if (rx >= XDIM) rx = XDIM-1;
	if (lx > rx) return;

	yinc = divscale32(yspan,ysiz);

	if ((sec->ceilingstat&3) == 0)
		startum = globalhoriz+mulscale24(siz,sec->ceilingz-globalposz)-1;
	else
		startum = 0;
	if ((sec->floorstat&3) == 0)
		startdm = globalhoriz+mulscale24(siz,sec->floorz-globalposz)+1;
	else
		startdm = 0x7fffffff;
	if ((y1>>8) > startum) startum = (y1>>8);
	if ((y2>>8) < startdm) startdm = (y2>>8);

	if (startum < -32768) startum = -32768;
	if (startdm > 32767) startdm = 32767;
	if (startum >= startdm) return;

	if ((cstat&4) == 0)
	{
		linuminc = divscale24(xspan,xsiz);
		linum = mulscale8((lx<<8)-x1,linuminc);
	}
	else
	{
		linuminc = -divscale24(xspan,xsiz);
		linum = mulscale8((lx<<8)-x2,linuminc);
	}
	if ((cstat&8) > 0)
	{
		yinc = -yinc;
		i = y1; y1 = y2; y2 = i;
	}

	for(x=lx;x<=rx;x++)
	{
		uwall[(size_t)x] = max(startumost[(size_t)x],(int16_t)startum);
		dwall[(size_t)x] = min(startdmost[(size_t)x],(int16_t)startdm);
	}
	daclip = 0;
	for(i=smostwallcnt-1;i>=0;i--)
	{
		if (smostwalltype[(size_t)i]&daclip) continue;
		j = smostwall[(size_t)i];
		if ((xb1[(size_t)j] > rx) || (xb2[(size_t)j] < lx)) continue;
		if ((yp <= yb1[(size_t)j]) && (yp <= yb2[(size_t)j])) continue;
		if (spritewallfront(tspr,(int32_t)thewall[(size_t)j]) && ((yp <= yb1[(size_t)j]) || (yp <= yb2[(size_t)j]))) continue;

		dalx2 = max(xb1[(size_t)j],lx); darx2 = min(xb2[(size_t)j],rx);

		switch(smostwalltype[(size_t)i])
		{
			case 0:
				if (dalx2 <= darx2)
				{
					if ((dalx2 == lx) && (darx2 == rx)) return;
					_fmemset(&dwall[(size_t)dalx2], 0, (darx2 - dalx2 + 1) * sizeof(dwall[0]));
				}
				break;
			case 1:
				k = smoststart[(size_t)i] - xb1[(size_t)j];
				for(x=dalx2;x<=darx2;x++)
					if (smost[(size_t)(k+x)] > uwall[(size_t)x]) uwall[(size_t)x] = smost[(size_t)(k+x)];
				if ((dalx2 == lx) && (darx2 == rx)) daclip |= 1;
				break;
			case 2:
				k = smoststart[(size_t)i] - xb1[(size_t)j];
				for(x=dalx2;x<=darx2;x++)
					if (smost[(size_t)(k+x)] < dwall[(size_t)x]) dwall[(size_t)x] = smost[(size_t)(k+x)];
				if ((dalx2 == lx) && (darx2 == rx)) daclip |= 2;
				break;
		}
	}

	if (uwall[(size_t)rx] >= dwall[(size_t)rx])
	{
		for(x=lx;x<rx;x++)
			if (uwall[(size_t)x] < dwall[(size_t)x]) break;
		if (x == rx) return;
	}

		//sprite
	if ((searchit >= 1) && (searchx >= lx) && (searchx <= rx))
		if ((searchy >= uwall[(size_t)searchx]) && (searchy < dwall[(size_t)searchx]))
		{
			searchsector = sectnum; searchwall = spritenum;
			searchstat = 3; searchit = 1;
		}

	z2 = tspr->z - ((yoff*tspr->yrepeat)<<2);
	if (cstat&128)
	{
		z2 += ((yspan*tspr->yrepeat)<<1);
		if (yspan&1) z2 += (tspr->yrepeat<<1);        //Odd yspans
	}
	z1 = z2 - ((yspan*tspr->yrepeat)<<2);

	globalorientation = 0;
	globalpicnum = tilenum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	globalxpanning = 0L;
	globalypanning = 0L;
	globvis = globalvisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globalshiftval = (picsiz[globalpicnum]>>4);
	if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
	globalshiftval = 32-globalshiftval;
	globalyscale = divscale(512,tspr->yrepeat,globalshiftval-19);
	globalzd = (((globalposz-z1)*globalyscale)<<8);
	if ((cstat&8) > 0)
	{
		globalyscale = -globalyscale;
		globalzd = (((globalposz-z2)*globalyscale)<<8);
	}

	qinterpolatedown16(&lwall[(size_t)lx],rx-lx+1,linum,linuminc);
	clearlongbuf(&swall[(size_t)lx],mulscale19(yp,xdimscale),rx-lx+1);

	if ((cstat&2) == 0)
		maskwallscan(lx, rx, uwall, dwall, swall, lwall, globalpicnum);
	else
		transmaskwallscan(lx, rx, globalpicnum);
}


static void drawspriteWall(int32_t cstat, int32_t xoff, int32_t yoff,
							int16_t tilenum, spritetype __far* tspr,
							sectortype __far* sec, int32_t sectnum,
							int32_t spritenum)
{
	int32_t xspan, yspan;
	int32_t x1, y1, x2, y2, dalx2, darx2, i, j, k, x;
	int32_t z, z1, z2, xp1, yp1, xp2, yp2;
	int32_t xv, yv, top, topinc, bot, botinc, hplc, hinc;
	int32_t y;
	int32_t zz;
	uint8_t swapped;
	int16_t globalpicnum;

	if ((cstat&4) > 0) xoff = -xoff;
	if ((cstat&8) > 0) yoff = -yoff;

	xspan = tilesizx[tilenum]; yspan = tilesizy[tilenum];
	xv = tspr->xrepeat*(int32_t)sintable[(tspr->ang+2560+1536)&2047];
	yv = tspr->xrepeat*(int32_t)sintable[(tspr->ang+2048+1536)&2047];
	i = (xspan>>1)+xoff;
	x1 = tspr->x-globalposx-mulscale16(xv,i); x2 = x1+mulscale16(xv,xspan);
	y1 = tspr->y-globalposy-mulscale16(yv,i); y2 = y1+mulscale16(yv,xspan);

	yp1 = dmulscale6(x1,cosviewingrangeglobalang,y1,sinviewingrangeglobalang);
	yp2 = dmulscale6(x2,cosviewingrangeglobalang,y2,sinviewingrangeglobalang);
	if ((yp1 <= 0) && (yp2 <= 0)) return;
	xp1 = dmulscale6(y1,cosglobalang,-x1,singlobalang);
	xp2 = dmulscale6(y2,cosglobalang,-x2,singlobalang);

	x1 += globalposx; y1 += globalposy;
	x2 += globalposx; y2 += globalposy;

	swapped = 0;
	if (dmulscale32(xp1,yp2,-xp2,yp1) >= 0)  //If wall's NOT facing you
	{
		if ((cstat&64) != 0) return;
		i = xp1, xp1 = xp2, xp2 = i;
		i = yp1, yp1 = yp2, yp2 = i;
		i = x1, x1 = x2, x2 = i;
		i = y1, y1 = y2, y2 = i;
		swapped = 1;
	}

	if (xp1 >= -yp1)
	{
		if (xp1 > yp1) return;

		if (yp1 == 0) return;
		xb1[MAXWALLSB-1] = (XDIM>>1) + scale(xp1,(XDIM>>1),yp1);
		if (xp1 >= 0) xb1[MAXWALLSB-1]++;   //Fix for SIGNED divide
		if (xb1[MAXWALLSB-1] >= XDIM) xb1[MAXWALLSB-1] = XDIM-1;
		yb1[MAXWALLSB-1] = yp1;
	}
	else
	{
		if (xp2 < -yp2) return;
		xb1[MAXWALLSB-1] = 0;
		i = yp1-yp2+xp1-xp2;
		if (i == 0) return;
		yb1[MAXWALLSB-1] = yp1 + scale(yp2-yp1,xp1+yp1,i);
	}
	if (xp2 <= yp2)
	{
		if (xp2 < -yp2) return;

		if (yp2 == 0) return;
		xb2[MAXWALLSB-1] = (XDIM>>1) + scale(xp2,(XDIM>>1),yp2) - 1;
		if (xp2 >= 0) xb2[MAXWALLSB-1]++;   //Fix for SIGNED divide
		if (xb2[MAXWALLSB-1] >= XDIM) xb2[MAXWALLSB-1] = XDIM-1;
		yb2[MAXWALLSB-1] = yp2;
	}
	else
	{
		if (xp1 > yp1) return;

		xb2[MAXWALLSB-1] = XDIM-1;
		i = xp2-xp1+yp1-yp2;
		if (i == 0) return;
		yb2[MAXWALLSB-1] = yp1 + scale(yp2-yp1,yp1-xp1,i);
	}

	if ((yb1[MAXWALLSB-1] < 256) || (yb2[MAXWALLSB-1] < 256) || (xb1[MAXWALLSB-1] > xb2[MAXWALLSB-1]))
		return;

	topinc = -mulscale10(yp1,xspan);
	top = (((mulscale10(xp1,XDIM) - mulscale9(xb1[MAXWALLSB-1]-(XDIM>>1),yp1))*xspan)>>3);
	botinc = ((yp2-yp1)>>8);
	bot = mulscale11(xp1-xp2,XDIM) + mulscale2(xb1[MAXWALLSB-1]-(XDIM>>1),botinc);

	j = xb2[MAXWALLSB-1]+3;
	z = mulscale20(top,krecip(bot));
	lwall[(size_t)xb1[MAXWALLSB-1]] = (z>>8);
	for(x=xb1[MAXWALLSB-1]+4;x<=j;x+=4)
	{
		top += topinc; bot += botinc;
		zz = z; z = mulscale20(top,krecip(bot));
		lwall[(size_t)x] = (z>>8);
		i = ((z+zz)>>1);
		lwall[(size_t)(x-2)] = (i>>8);
		lwall[(size_t)(x-3)] = ((i+zz)>>9);
		lwall[(size_t)(x-1)] = ((i+z)>>9);
	}

	if (lwall[(size_t)xb1[MAXWALLSB-1]] < 0) lwall[(size_t)xb1[MAXWALLSB-1]] = 0;
	if (lwall[(size_t)xb2[MAXWALLSB-1]] >= xspan) lwall[(size_t)xb2[MAXWALLSB-1]] = xspan-1;

	if ((swapped^((cstat&4)>0)) > 0)
	{
		j = xspan-1;
		for(x=xb1[MAXWALLSB-1];x<=xb2[MAXWALLSB-1];x++)
			lwall[(size_t)x] = j-lwall[(size_t)x];
	}

	rx1[MAXWALLSB-1] = xp1; ry1[MAXWALLSB-1] = yp1;
	rx2[MAXWALLSB-1] = xp2; ry2[MAXWALLSB-1] = yp2;

	hplc = divscale19(xdimenscale,yb1[MAXWALLSB-1]);
	hinc = divscale19(xdimenscale,yb2[MAXWALLSB-1]);
	hinc = (hinc-hplc)/(xb2[MAXWALLSB-1]-xb1[MAXWALLSB-1]+1);

	z2 = tspr->z - ((yoff*tspr->yrepeat)<<2);
	if (cstat&128)
	{
		z2 += ((yspan*tspr->yrepeat)<<1);
		if (yspan&1) z2 += (tspr->yrepeat<<1);        //Odd yspans
	}
	z1 = z2 - ((yspan*tspr->yrepeat)<<2);

	globalorientation = 0;
	globalpicnum = tilenum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	globalxpanning = 0L;
	globalypanning = 0L;
	globvis = globalvisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));
	globalshiftval = (picsiz[globalpicnum]>>4);
	if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
	globalshiftval = 32-globalshiftval;
	globalyscale = divscale(512,tspr->yrepeat,globalshiftval-19);
	globalzd = (((globalposz-z1)*globalyscale)<<8);
	if ((cstat&8) > 0)
	{
		globalyscale = -globalyscale;
		globalzd = (((globalposz-z2)*globalyscale)<<8);
	}

	if (((sec->ceilingstat&1) == 0) && (z1 < sec->ceilingz))
		z1 = sec->ceilingz;
	if (((sec->floorstat&1) == 0) && (z2 > sec->floorz))
		z2 = sec->floorz;

	owallmost(uwall,(int32_t)(MAXWALLSB-1),z1-globalposz);
	owallmost(dwall,(int32_t)(MAXWALLSB-1),z2-globalposz);
	for(i=xb1[MAXWALLSB-1];i<=xb2[MAXWALLSB-1];i++)
		{ swall[(size_t)i] = (krecip(hplc)<<2); hplc += hinc; }

	for(i=smostwallcnt-1;i>=0;i--)
	{
		j = smostwall[(size_t)i];

		if ((xb1[(size_t)j] > xb2[MAXWALLSB-1]) || (xb2[(size_t)j] < xb1[MAXWALLSB-1])) continue;

		dalx2 = xb1[(size_t)j]; darx2 = xb2[(size_t)j];
		if (max(yb1[MAXWALLSB-1],yb2[MAXWALLSB-1]) > min(yb1[(size_t)j],yb2[(size_t)j]))
		{
			if (min(yb1[MAXWALLSB-1],yb2[MAXWALLSB-1]) > max(yb1[(size_t)j],yb2[(size_t)j]))
			{
				x = 0x80000000;
			}
			else
			{
				x = thewall[(size_t)j]; xp1 = wall[(size_t)x].x; yp1 = wall[(size_t)x].y;
				x = wall[(size_t)x].point2; xp2 = wall[(size_t)x].x; yp2 = wall[(size_t)x].y;

				z1 = (xp2-xp1)*(y1-yp1) - (yp2-yp1)*(x1-xp1);
				z2 = (xp2-xp1)*(y2-yp1) - (yp2-yp1)*(x2-xp1);
				if ((z1^z2) >= 0)
					x = (z1+z2);
				else
				{
					z1 = (x2-x1)*(yp1-y1) - (y2-y1)*(xp1-x1);
					z2 = (x2-x1)*(yp2-y1) - (y2-y1)*(xp2-x1);

					if ((z1^z2) >= 0)
						x = -(z1+z2);
					else
					{
						if ((xp2-xp1)*(tspr->y-yp1) == (tspr->x-xp1)*(yp2-yp1))
						{
							if (wall[thewall[(size_t)j]].nextsector == tspr->sectnum)
								x = 0x80000000;
							else
								x = 0x7fffffff;
						}
						else
						{     //INTERSECTION!
							x = (xp1-globalposx) + scale(xp2-xp1,z1,z1-z2);
							y = (yp1-globalposy) + scale(yp2-yp1,z1,z1-z2);

							yp1 = dmulscale14(x,cosglobalang,y,singlobalang);
							if (yp1 > 0)
							{
								xp1 = dmulscale14(y,cosglobalang,-x,singlobalang);

								x = (XDIM>>1) + scale(xp1,(XDIM>>1),yp1);
								if (xp1 >= 0) x++;   //Fix for SIGNED divide

								if (z1 < 0)
									{ if (dalx2 < x) dalx2 = x; }
								else
									{ if (darx2 > x) darx2 = x; }
								x = 0x80000001;
							}
							else
								x = 0x7fffffff;
						}
					}
				}
			}
			if (x < 0)
			{
				if (dalx2 < xb1[MAXWALLSB-1]) dalx2 = xb1[MAXWALLSB-1];
				if (darx2 > xb2[MAXWALLSB-1]) darx2 = xb2[MAXWALLSB-1];
				switch(smostwalltype[(size_t)i])
				{
					case 0:
						if (dalx2 <= darx2)
						{
							if ((dalx2 == xb1[MAXWALLSB-1]) && (darx2 == xb2[MAXWALLSB-1])) return;
							_fmemset(&dwall[(size_t)dalx2], 0, (darx2 - dalx2 + 1) * sizeof(dwall[0]));
						}
						break;
					case 1:
						k = smoststart[(size_t)i] - xb1[(size_t)j];
						for(x=dalx2;x<=darx2;x++)
							if (smost[(size_t)(k+x)] > uwall[(size_t)x]) uwall[(size_t)x] = smost[(size_t)(k+x)];
						break;
					case 2:
						k = smoststart[(size_t)i] - xb1[(size_t)j];
						for(x=dalx2;x<=darx2;x++)
							if (smost[(size_t)(k+x)] < dwall[(size_t)x]) dwall[(size_t)x] = smost[(size_t)(k+x)];
						break;
				}
			}
		}
	}

		//sprite
	if ((searchit >= 1) && (searchx >= xb1[MAXWALLSB-1]) && (searchx <= xb2[MAXWALLSB-1]))
		if ((searchy >= uwall[(size_t)searchx]) && (searchy <= dwall[(size_t)searchx]))
		{
			searchsector = sectnum; searchwall = spritenum;
			searchstat = 3; searchit = 1;
		}

	if ((cstat&2) == 0)
		maskwallscan(xb1[MAXWALLSB - 1], xb2[MAXWALLSB - 1], uwall, dwall, swall, lwall, globalpicnum);
	else
		transmaskwallscan(xb1[MAXWALLSB - 1], xb2[MAXWALLSB - 1], globalpicnum);
}


static void drawspriteFloor(int32_t yp, spritetype __far* tspr, int16_t tilenum,
							int32_t xoff, int32_t cstat,
							int32_t yoff, sectortype __far* sec, int32_t sectnum,
							int32_t spritenum)
{
	int32_t xspan, yspan;
	int32_t lx, rx, dalx2, darx2, i, j, k, x;
	int32_t yinc, z, z1, z2, xp1, yp1, xp2, yp2;
	int32_t bot;
	int32_t cosang, sinang, dax, day, lpoint, lmax, rpoint, rmax, dax1, dax2, y;
	int32_t npoints, npoints2, zz, t, zsgn, zzsgn;
	int16_t globalpicnum;
	uint8_t __far* bufplc;

	if ((cstat&64) != 0)
		if ((globalposz > tspr->z) == ((cstat&8)==0))
			return;

	if ((cstat&4) > 0) xoff = -xoff;
	if ((cstat&8) > 0) yoff = -yoff;
	xspan = tilesizx[tilenum];
	yspan = tilesizy[tilenum];

		//Rotate center point
	dax = tspr->x-globalposx;
	day = tspr->y-globalposy;
	rzi[0] = dmulscale10(cosglobalang,dax,singlobalang,day);
	rxi[0] = dmulscale10(cosglobalang,day,-singlobalang,dax);

		//Get top-left corner
	i = ((tspr->ang+2048-globalang)&2047);
	cosang = sintable[(size_t)(i+512)&2047]; sinang = sintable[(size_t)i];
	dax = ((xspan>>1)+xoff)*tspr->xrepeat;
	day = ((yspan>>1)+yoff)*tspr->yrepeat;
	rzi[0] += dmulscale12(sinang,dax,cosang,day);
	rxi[0] += dmulscale12(sinang,day,-cosang,dax);

		//Get other 3 corners
	dax = xspan*tspr->xrepeat;
	day = yspan*tspr->yrepeat;
	rzi[1] = rzi[0]-mulscale12(sinang,dax);
	rxi[1] = rxi[0]+mulscale12(cosang,dax);
	dax = -mulscale12(cosang,day);
	day = -mulscale12(sinang,day);
	rzi[2] = rzi[1]+dax; rxi[2] = rxi[1]+day;
	rzi[3] = rzi[0]+dax; rxi[3] = rxi[0]+day;

		//Put all points on same z
	ryi[0] = scale((tspr->z - globalposz), yxaspect, (int32_t)XDIM << 8);
	if (ryi[0] == 0) return;
	ryi[1] = ryi[2] = ryi[3] = ryi[0];

	if ((cstat&4) == 0)
		{ z = 0; z1 = 1; z2 = 3; }
	else
		{ z = 1; z1 = 0; z2 = 2; }

	dax = rzi[(size_t)z1]-rzi[(size_t)z]; day = rxi[(size_t)z1]-rxi[(size_t)z];
	bot = dmulscale8(dax,dax,day,day);
	if (((klabs(dax)>>13) >= bot) || ((klabs(day)>>13) >= bot)) return;
	globalx1 = divscale18(dax,bot);
	globalx2 = divscale18(day,bot);

	dax = rzi[(size_t)z2]-rzi[(size_t)z]; day = rxi[(size_t)z2]-rxi[(size_t)z];
	bot = dmulscale8(dax,dax,day,day);
	if (((klabs(dax)>>13) >= bot) || ((klabs(day)>>13) >= bot)) return;
	globaly1 = divscale18(dax,bot);
	globaly2 = divscale18(day,bot);

		//Calculate globals for hline texture mapping function
	globalxpanning = (rxi[(size_t)z]<<12);
	globalypanning = (rzi[(size_t)z]<<12);
	globalzd = (ryi[(size_t)z]<<12);

	rzi[0] = mulscale16(rzi[0],viewingrange);
	rzi[1] = mulscale16(rzi[1],viewingrange);
	rzi[2] = mulscale16(rzi[2],viewingrange);
	rzi[3] = mulscale16(rzi[3],viewingrange);

	if (ryi[0] < 0)   //If ceilsprite is above you, reverse order of points
	{
		i = rxi[1]; rxi[1] = rxi[3]; rxi[3] = i;
		i = rzi[1]; rzi[1] = rzi[3]; rzi[3] = i;
	}

		//Clip polygon in 3-space
	npoints = 4;

		//Clip edge 1
	npoints2 = 0;
	zzsgn = rxi[0]+rzi[0];
	for(z=0;z<npoints;z++)
	{
		zz = z+1; if (zz == npoints) zz = 0;
		zsgn = zzsgn; zzsgn = rxi[(size_t)zz]+rzi[(size_t)zz];
		if (zsgn >= 0)
		{
			rxi2[(size_t)npoints2] = rxi[(size_t)z]; ryi2[(size_t)npoints2] = ryi[(size_t)z]; rzi2[(size_t)npoints2] = rzi[(size_t)z];
			npoints2++;
		}
		if ((zsgn^zzsgn) < 0)
		{
			t = divscale30(zsgn,zsgn-zzsgn);
			rxi2[(size_t)npoints2] = rxi[(size_t)z] + mulscale30(t,rxi[(size_t)zz]-rxi[(size_t)z]);
			ryi2[(size_t)npoints2] = ryi[(size_t)z] + mulscale30(t,ryi[(size_t)zz]-ryi[(size_t)z]);
			rzi2[(size_t)npoints2] = rzi[(size_t)z] + mulscale30(t,rzi[(size_t)zz]-rzi[(size_t)z]);
			npoints2++;
		}
	}
	if (npoints2 <= 2) return;

		//Clip edge 2
	npoints = 0;
	zzsgn = rxi2[0]-rzi2[0];
	for(z=0;z<npoints2;z++)
	{
		zz = z+1; if (zz == npoints2) zz = 0;
		zsgn = zzsgn; zzsgn = rxi2[(size_t)zz]-rzi2[(size_t)zz];
		if (zsgn <= 0)
		{
			rxi[(size_t)npoints] = rxi2[(size_t)z]; ryi[(size_t)npoints] = ryi2[(size_t)z]; rzi[(size_t)npoints] = rzi2[(size_t)z];
			npoints++;
		}
		if ((zsgn^zzsgn) < 0)
		{
			t = divscale30(zsgn,zsgn-zzsgn);
			rxi[(size_t)npoints] = rxi2[(size_t)z] + mulscale30(t,rxi2[(size_t)zz]-rxi2[(size_t)z]);
			ryi[(size_t)npoints] = ryi2[(size_t)z] + mulscale30(t,ryi2[(size_t)zz]-ryi2[(size_t)z]);
			rzi[(size_t)npoints] = rzi2[(size_t)z] + mulscale30(t,rzi2[(size_t)zz]-rzi2[(size_t)z]);
			npoints++;
		}
	}
	if (npoints <= 2) return;

		//Clip edge 3
	npoints2 = 0;
	zzsgn = ryi[0]*(XDIM>>1) + (rzi[0]*(globalhoriz-0));
	for(z=0;z<npoints;z++)
	{
		zz = z+1; if (zz == npoints) zz = 0;
		zsgn = zzsgn; zzsgn = ryi[(size_t)zz]*(XDIM>>1) + (rzi[(size_t)zz]*(globalhoriz-0));
		if (zsgn >= 0)
		{
			rxi2[(size_t)npoints2] = rxi[(size_t)z];
			ryi2[(size_t)npoints2] = ryi[(size_t)z];
			rzi2[(size_t)npoints2] = rzi[(size_t)z];
			npoints2++;
		}
		if ((zsgn^zzsgn) < 0)
		{
			t = divscale30(zsgn,zsgn-zzsgn);
			rxi2[(size_t)npoints2] = rxi[(size_t)z] + mulscale30(t,rxi[(size_t)zz]-rxi[(size_t)z]);
			ryi2[(size_t)npoints2] = ryi[(size_t)z] + mulscale30(t,ryi[(size_t)zz]-ryi[(size_t)z]);
			rzi2[(size_t)npoints2] = rzi[(size_t)z] + mulscale30(t,rzi[(size_t)zz]-rzi[(size_t)z]);
			npoints2++;
		}
	}
	if (npoints2 <= 2) return;

		//Clip edge 4
	npoints = 0;
	zzsgn = ryi2[0]*(XDIM>>1) + (rzi2[0]*(globalhoriz-YDIM));
	for(z=0;z<npoints2;z++)
	{
		zz = z+1; if (zz == npoints2) zz = 0;
		zsgn = zzsgn; zzsgn = ryi2[(size_t)zz]*(XDIM>>1) + (rzi2[(size_t)zz]*(globalhoriz-YDIM));
		if (zsgn <= 0)
		{
			rxi[(size_t)npoints] = rxi2[(size_t)z];
			ryi[(size_t)npoints] = ryi2[(size_t)z];
			rzi[(size_t)npoints] = rzi2[(size_t)z];
			npoints++;
		}
		if ((zsgn^zzsgn) < 0)
		{
			t = divscale30(zsgn,zsgn-zzsgn);
			rxi[(size_t)npoints] = rxi2[(size_t)z] + mulscale30(t,rxi2[(size_t)zz]-rxi2[(size_t)z]);
			ryi[(size_t)npoints] = ryi2[(size_t)z] + mulscale30(t,ryi2[(size_t)zz]-ryi2[(size_t)z]);
			rzi[(size_t)npoints] = rzi2[(size_t)z] + mulscale30(t,rzi2[(size_t)zz]-rzi2[(size_t)z]);
			npoints++;
		}
	}
	if (npoints <= 2) return;

		//Project onto screen
	lpoint = -1; lmax = 0x7fffffff;
	rpoint = -1; rmax = 0x80000000;
	for(z=0;z<npoints;z++)
	{
		xsi[(size_t)z] = scale(rxi[(size_t)z], (int32_t)XDIM << 15, rzi[(size_t)z]) + ((int32_t)XDIM << 15);
		ysi[(size_t)z] = scale(ryi[(size_t)z], (int32_t)XDIM << 15, rzi[(size_t)z]) + (globalhoriz << 16);
		if (xsi[(size_t)z] < 0) xsi[(size_t)z] = 0;
		if (xsi[(size_t)z] > ((int32_t)XDIM<<16)) xsi[(size_t)z] = ((int32_t)XDIM<<16);
		if (ysi[(size_t)z] < ((int32_t)0<<16)) ysi[(size_t)z] = ((int32_t)0<<16);
		if (ysi[(size_t)z] > ((int32_t)YDIM<<16)) ysi[(size_t)z] = ((int32_t)YDIM<<16);
		if (xsi[(size_t)z] < lmax) lmax = xsi[(size_t)z], lpoint = z;
		if (xsi[(size_t)z] > rmax) rmax = xsi[(size_t)z], rpoint = z;
	}

		//Get uwall arrays
	for(z=lpoint;z!=rpoint;z=zz)
	{
		zz = z+1; if (zz == npoints) zz = 0;

		dax1 = ((xsi[(size_t)z]+65535)>>16);
		dax2 = ((xsi[(size_t)zz]+65535)>>16);
		if (dax2 > dax1)
		{
			yinc = divscale16(ysi[(size_t)zz]-ysi[(size_t)z],xsi[(size_t)zz]-xsi[(size_t)z]);
			y = ysi[(size_t)z] + mulscale16((dax1<<16)-xsi[(size_t)z],yinc);
			qinterpolatedown16short(&uwall[(size_t)dax1],dax2-dax1,y,yinc);
		}
	}

		//Get dwall arrays
	for(;z!=lpoint;z=zz)
	{
		zz = z+1; if (zz == npoints) zz = 0;

		dax1 = ((xsi[(size_t)zz]+65535)>>16);
		dax2 = ((xsi[(size_t)z]+65535)>>16);
		if (dax2 > dax1)
		{
			yinc = divscale16(ysi[(size_t)zz]-ysi[(size_t)z],xsi[(size_t)zz]-xsi[(size_t)z]);
			y = ysi[(size_t)zz] + mulscale16((dax1<<16)-xsi[(size_t)zz],yinc);
			qinterpolatedown16short(&dwall[(size_t)dax1],dax2-dax1,y,yinc);
		}
	}

	lx = ((lmax+65535)>>16);
	rx = ((rmax+65535)>>16);
	for(x=lx;x<=rx;x++)
	{
		uwall[(size_t)x] = max(uwall[(size_t)x],startumost[(size_t)x]);
		dwall[(size_t)x] = min(dwall[(size_t)x],startdmost[(size_t)x]);
	}

		//Additional uwall/dwall clipping goes here
	for(i=smostwallcnt-1;i>=0;i--)
	{
		j = smostwall[(size_t)i];
		if ((xb1[(size_t)j] > rx) || (xb2[(size_t)j] < lx)) continue;
		if ((yp <= yb1[(size_t)j]) && (yp <= yb2[(size_t)j])) continue;

			//if (spritewallfront(tspr,thewall[j]) == 0)
		x = thewall[(size_t)j]; xp1 = wall[(size_t)x].x; yp1 = wall[(size_t)x].y;
		x = wall[(size_t)x].point2; xp2 = wall[(size_t)x].x; yp2 = wall[(size_t)x].y;
		x = (xp2-xp1)*(tspr->y-yp1)-(tspr->x-xp1)*(yp2-yp1);
		if ((yp > yb1[(size_t)j]) && (yp > yb2[(size_t)j])) x = -1;
		if ((x >= 0) && ((x != 0) || (wall[thewall[(size_t)j]].nextsector != tspr->sectnum))) continue;

		dalx2 = max(xb1[(size_t)j],lx); darx2 = min(xb2[(size_t)j],rx);

		switch(smostwalltype[(size_t)i])
		{
			case 0:
				if (dalx2 <= darx2)
				{
					if ((dalx2 == lx) && (darx2 == rx)) return;
					_fmemset(&dwall[(size_t)dalx2], 0, (darx2 - dalx2 + 1) * sizeof(dwall[0]));
				}
				break;
			case 1:
				k = smoststart[(size_t)i] - xb1[(size_t)j];
				for(x=dalx2;x<=darx2;x++)
					if (smost[(size_t)(k+x)] > uwall[(size_t)x]) uwall[(size_t)x] = smost[(size_t)(k+x)];
				break;
			case 2:
				k = smoststart[(size_t)i] - xb1[(size_t)j];
				for(x=dalx2;x<=darx2;x++)
					if (smost[(size_t)(k+x)] < dwall[(size_t)x]) dwall[(size_t)x] = smost[(size_t)(k+x)];
				break;
		}
	}

		//sprite
	if ((searchit >= 1) && (searchx >= lx) && (searchx <= rx))
		if ((searchy >= uwall[(size_t)searchx]) && (searchy <= dwall[(size_t)searchx]))
		{
			searchsector = sectnum; searchwall = spritenum;
			searchstat = 3; searchit = 1;
		}

	globalorientation = cstat;
	globalpicnum = tilenum;
	if ((uint32_t)globalpicnum >= (uint32_t)MAXTILES) globalpicnum = 0;
	//if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum);

	bufplc = loadtile(globalpicnum);

	globvis = mulscale16(globalhisibility,viewingrange);
	if (sec->visibility != 0) globvis = mulscale4(globvis,(int32_t)((uint8_t)(sec->visibility+16)));

	x = picsiz[globalpicnum]; y = ((x>>4)&15); x &= 15;
	if (pow2long[(size_t)x] != xspan)
	{
		x++;
		globalx1 = mulscale(globalx1,xspan,x);
		globalx2 = mulscale(globalx2,xspan,x);
	}

	dax = globalxpanning; day = globalypanning;
	globalxpanning = -dmulscale6(globalx1,day,globalx2,dax);
	globalypanning = -dmulscale6(globaly1,day,globaly2,dax);

	globalx2 = mulscale16(globalx2,viewingrange);
	globaly2 = mulscale16(globaly2,viewingrange);
	globalzd = mulscale16(globalzd,viewingrangerecip);

	globalx1 = (globalx1-globalx2)*(XDIM>>1);
	globaly1 = (globaly1-globaly2)*(XDIM>>1);

	_a_glogx = x;
	_a_glogy = y;

		//Draw it!
	ceilspritescan(lx, rx - 1, bufplc);

	Z_ChangeTagToCache(bufplc);
}


void drawsprite(int_fast16_t snum)
{
	spritetype __far* tspr;
	sectortype __far* sec;
	int16_t sectnum;
	int32_t xb, yp, cstat;
	int32_t xoff, yoff;
	int16_t tilenum, spritenum;

	tspr = tspriteptr[snum];

	xb = spritesx[snum];
	yp = spritesy[snum];
	tilenum   = tspr->picnum;
	spritenum = tspr->owner;
	cstat     = tspr->cstat;

	if (picanm[tilenum]&192) tilenum += animateoffs(tilenum);
	if ((tilesizx[tilenum] <= 0) || (tilesizy[tilenum] <= 0) || (spritenum < 0))
		return;
	if ((tspr->xrepeat == 0) || (tspr->yrepeat == 0)) return;

	sectnum     = tspr->sectnum; sec = &sector[sectnum];
	globalpal   = tspr->pal;
	globalshade = tspr->shade;

	if (cstat&2)
	{
		if (cstat&512)
			_a_transmode = 1;
		else
			_a_transmode = 0;
	}

	xoff = (int32_t)((int8_t)((picanm[tilenum]>>8)&255))+((int32_t)tspr->xoffset);
	yoff = (int32_t)((int8_t)((picanm[tilenum]>>16)&255))+((int32_t)tspr->yoffset);

	if ((cstat&48) == 0)
		drawspriteFace(yp, tspr, tilenum, xb, xoff, cstat, yoff, sec, sectnum, spritenum);
	else if ((cstat&48) == 16)
		drawspriteWall(cstat, xoff, yoff, tilenum, tspr, sec, sectnum, spritenum);
	else if ((cstat&48) == 32)
		drawspriteFloor(yp, tspr, tilenum, xoff, cstat, yoff, sec, sectnum, spritenum);
}
