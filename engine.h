// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __ENGINE__
#define __ENGINE__

#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"


#define MAXSECTORS 109 /*1024*/
#define MAXWALLS 731 /*8192*/
// FIXME increase MAXSPRITES to fix bugs with rendered sprites that should be hidden
#define MAXSPRITES 128 /*4096*/

#define MAXWALLSB (MAXWALLS/4) /*2048*/

#define MAXSTATUS 1024
#define XDIM 320
#define YDIM 200
#define MAXPSKYTILES 256
#define MAXSPRITESONSCREEN (MAXSPRITES/4) /*1024*/

#define CLIPMASK0 (((1L)<<16)+1L)
#define CLIPMASK1 (((256L)<<16)+64L)

#define MAXCLIPNUM 512
#define MAXCLIPDIST 1024

#define MAXYSAVES (XDIM*(MAXSPRITES>>7))


#pragma pack(push,1)

//ceilingstat/floorstat:
//   bit 0: 1 = parallaxing, 0 = not                                 "P"
//   bit 1: 1 = groudraw, 0 = not
//   bit 2: 1 = swap x&y, 0 = not                                    "F"
//   bit 3: 1 = double smooshiness                                   "E"
//   bit 4: 1 = x-flip                                               "F"
//   bit 5: 1 = y-flip                                               "F"
//   bit 6: 1 = Align texture to first wall of sector                "R"
//   bits 7-8:                                                       "T"
//          00 = normal floors
//          01 = masked floors
//          10 = transluscent masked floors
//          11 = reverse transluscent masked floors
//   bits 9-15: reserved

	//40 bytes
typedef struct
{
	int16_t wallptr, wallnum;
	int32_t ceilingz, floorz;
	int16_t ceilingstat, floorstat;
	int16_t ceilingpicnum, ceilingheinum;
	int8_t ceilingshade;
	uint8_t ceilingpal;
	uint8_t ceilingxpanning, ceilingypanning;
	int16_t floorpicnum, floorheinum;
	int8_t floorshade;
	uint8_t floorpal;
	uint8_t floorxpanning, floorypanning;
	uint8_t visibility, filler;
	int16_t lotag, hitag, extra;
} sectortype;
typedef uint8_t assertSizeSectortype[sizeof(sectortype) == 40 ? 1 : -1];

//cstat:
//   bit 0: 1 = Blocking wall (use with clipmove, getzrange)         "B"
//   bit 1: 1 = bottoms of invisible walls swapped, 0 = not          "2"
//   bit 2: 1 = align picture on bottom (for doors), 0 = top         "O"
//   bit 3: 1 = x-flipped, 0 = normal                                "F"
//   bit 4: 1 = masking wall, 0 = not                                "M"
//   bit 5: 1 = 1-way wall, 0 = not                                  "1"
//   bit 6: 1 = Blocking wall (use with hitscan / cliptype 1)        "H"
//   bit 7: 1 = Transluscence, 0 = not                               "T"
//   bit 8: 1 = y-flipped, 0 = normal                                "F"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bits 10-15: reserved

	//32 bytes
typedef struct
{
	int32_t x, y;
	int16_t point2, nextwall, nextsector, cstat;
	int16_t picnum, overpicnum;
	int8_t shade;
	uint8_t pal, xrepeat, yrepeat, xpanning, ypanning;
	int16_t lotag, hitag, extra;
} walltype;
typedef uint8_t assertSizeWalltype[sizeof(walltype) == 32 ? 1 : -1];

//cstat:
//   bit 0: 1 = Blocking sprite (use with clipmove, getzrange)       "B"
//   bit 1: 1 = transluscence, 0 = normal                            "T"
//   bit 2: 1 = x-flipped, 0 = normal                                "F"
//   bit 3: 1 = y-flipped, 0 = normal                                "F"
//   bits 5-4: 00 = FACE sprite (default)                            "R"
//             01 = WALL sprite (like masked walls)
//             10 = FLOOR sprite (parallel to ceilings&floors)
//   bit 6: 1 = 1-sided sprite, 0 = normal                           "1"
//   bit 7: 1 = Real centered centering, 0 = foot center             "C"
//   bit 8: 1 = Blocking sprite (use with hitscan / cliptype 1)      "H"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bits 10-14: reserved
//   bit 15: 1 = Invisible sprite, 0 = not invisible

	//44 bytes
typedef struct
{
	int32_t x, y, z;
	int16_t cstat, picnum;
	int8_t shade;
	uint8_t pal, clipdist, filler;
	uint8_t xrepeat, yrepeat;
	int8_t xoffset, yoffset;
	int16_t sectnum, statnum;
	int16_t ang, owner, xvel, yvel, zvel;
	int16_t lotag, hitag, extra;
} spritetype;
typedef uint8_t assertSizeSpritetype[sizeof(spritetype) == 44 ? 1 : -1];

#pragma pack(pop)


extern sectortype __far sector[MAXSECTORS];
extern walltype __far wall[MAXWALLS];
extern spritetype __far sprite[MAXSPRITES];

extern int32_t spritesortcnt;
extern spritetype __far tsprite[MAXSPRITESONSCREEN];

extern int32_t yxaspect, viewingrange;

extern int32_t numframes;
extern volatile int32_t totalclock;

extern int16_t __far pskyoff[MAXPSKYTILES];
extern int16_t pskybits;

extern int16_t __far clipsectorlist[MAXCLIPNUM];
extern int16_t clipsectnum;

extern int16_t editstatus;

extern int16_t __far headspritesect[MAXSECTORS+1];
extern int16_t __far nextspritesect[MAXSPRITES];

extern int16_t __far uwall[XDIM];
extern int16_t __far dwall[XDIM];

extern int32_t lastx[YDIM];

extern int32_t globalzd;

extern int32_t horizlookup[YDIM * 4];

extern int32_t globalhoriz;

extern int32_t globalxpanning, globalypanning, globalshade;

extern int32_t globalx1, globaly1, globalx2, globaly2;

extern int32_t asm1, asm2;

extern int32_t globalpal;

extern int32_t _a_glogx, _a_glogy;

extern int32_t _a_transmode;

extern int32_t globvis;

extern int32_t globalorientation;

extern int32_t ylookup[YDIM+1];

extern uint8_t __far* _s_screen;

extern int32_t transarea;

extern spritetype __far* tspriteptr[MAXSPRITESONSCREEN];
extern int32_t __far spritesx[MAXSPRITESONSCREEN];
extern int32_t __far spritesy[MAXSPRITESONSCREEN+1];

extern int32_t xdimenscale;

extern int32_t xyaspect;

extern int32_t globalposz;

extern int16_t __far startumost[XDIM];
extern int16_t __far startdmost[XDIM];

extern int16_t __far smost[MAXYSAVES];
extern int16_t __far smoststart[MAXWALLSB];
extern uint8_t __far smostwalltype[MAXWALLSB];
extern int32_t __far smostwall[MAXWALLSB];
extern int32_t smostwallcnt;

extern int32_t __far xb1[MAXWALLSB];
extern int32_t __far yb1[MAXWALLSB];
extern int32_t __far xb2[MAXWALLSB];
extern int32_t __far yb2[MAXWALLSB];
extern int32_t __far rx1[MAXWALLSB];
extern int32_t __far ry1[MAXWALLSB];
extern int32_t __far rx2[MAXWALLSB];
extern int32_t __far ry2[MAXWALLSB];

extern int16_t __far thewall[MAXWALLSB];

extern int16_t searchit;
extern int32_t searchx, searchy;

extern int16_t searchsector, searchwall, searchstat;

extern int32_t globalvisibility;

extern int16_t globalshiftval;

extern int32_t globalyscale;

extern int32_t __far swall[XDIM];
extern int32_t __far lwall[XDIM+4];

extern int32_t xdimscale;

extern int32_t globalposx, globalposy;

extern int32_t cosviewingrangeglobalang, sinviewingrangeglobalang;

extern int32_t cosglobalang, singlobalang;

extern int32_t globaluclip, globaldclip;

extern int16_t globalang;

extern int32_t globalhisibility;

extern int32_t viewingrangerecip;


void drawrooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);
void setvmode(int32_t a);
void setgamemode(void);
void initengine(void);
void nextpage(void);
void drawmasks(void);
void initspritelists(void);
void insertspritesect(int16_t sectnum);
void insertspritestat(int16_t statnum);
int32_t clipmove (int32_t *x, int32_t *y, int32_t *z, int16_t *sectnum, int32_t xvect, int32_t yvect, int32_t walldist, int32_t ceildist, int32_t flordist, uint32_t cliptype);
void updatesector(int32_t x, int32_t y, int16_t *sectnum);
void getzrange(int32_t x, int32_t y, int32_t z, int16_t sectnum, int32_t *ceilz, int32_t *ceilhit, int32_t *florz, int32_t *florhit, int32_t walldist, uint32_t cliptype);
void setaspect(int32_t daxrange, int32_t daaspect);
void rotatesprite (int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, uint8_t dapalnum, uint8_t dastat, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2);

int32_t getceilzofslope(int16_t sectnum, int32_t dax, int32_t day);
int32_t getflorzofslope(int16_t sectnum, int32_t dax, int32_t day);
int32_t clipinsideboxline(int32_t x, int32_t y, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t walldist);
int32_t inside (int32_t x, int32_t y, int16_t sectnum);
void drawsprite (int32_t snum);
int32_t spritewallfront(spritetype __far* s, int32_t w);
void clearlongbuf(int32_t __far* s, int32_t c, size_t n);
void maskwallscan(int32_t x1, int32_t x2, int16_t __far* uwal, int16_t __far* dwal, int32_t __far* swal, int32_t __far* lwal, int16_t globalpicnum);
void transmaskwallscan(int32_t x1, int32_t x2, int16_t globalpicnum);
int32_t krecip(int32_t num);
uint8_t owallmost(int16_t __far* mostbuf, int32_t w, int32_t z);
void clearshortbuf(int16_t __far* s, int16_t c, size_t n);
void qinterpolatedown16short(void __far* bufptr, int32_t num, int32_t val, int32_t add);


/*************************************************************************
POSITION VARIABLES:

		POSX is your x - position ranging from 0 to 65535
		POSY is your y - position ranging from 0 to 65535
			(the length of a side of the grid in EDITBORD would be 1024)
		POSZ is your z - position (height) ranging from 0 to 65535, 0 highest.
		ANG is your angle ranging from 0 to 2047.  Instead of 360 degrees, or
			 2 * PI radians, I use 2048 different angles, so 90 degrees would
			 be 512 in my system.

SPRITE VARIABLES:

	EXTERN int16_t headspritesect[MAXSECTORS+1], headspritestat[MAXSTATUS+1];
	EXTERN int16_t prevspritesect[MAXSPRITES], prevspritestat[MAXSPRITES];
	EXTERN int16_t nextspritesect[MAXSPRITES], nextspritestat[MAXSPRITES];

	Example: if the linked lists look like the following:
		 旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
		 ?     Sector lists:               Status lists:               ?
		 쳐컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴캑
		 ? Sector0:  4, 5, 8             Status0:  2, 0, 8             ?
		 ? Sector1:  16, 2, 0, 7         Status1:  4, 5, 16, 7, 3, 9   ?
		 ? Sector2:  3, 9                                              ?
		 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
	Notice that each number listed above is shown exactly once on both the
		left and right side.  This is because any sprite that exists must
		be in some sector, and must have some kind of status that you define.


Coding example #1:
	To go through all the sprites in sector 1, the code can look like this:

		sectnum = 1;
		i = headspritesect[sectnum];
		while (i != -1)
		{
			nexti = nextspritesect[i];

			//your code goes here
			//ex: printf("Sprite %d is in sector %d\n",i,sectnum);

			i = nexti;
		}

Coding example #2:
	To go through all sprites with status = 1, the code can look like this:

		statnum = 1;        //status 1
		i = headspritestat[statnum];
		while (i != -1)
		{
			nexti = nextspritestat[i];

			//your code goes here
			//ex: printf("Sprite %d has a status of 1 (active)\n",i,statnum);

			i = nexti;
		}

			 insertsprite(int16_t sectnum, int16_t statnum);
			 deletesprite(int16_t spritenum);
			 changespritesect(int16_t spritenum, int16_t newsectnum);
			 changespritestat(int16_t spritenum, int16_t newstatnum);

TIMING VARIABLES:
		TOTALCLOCK - When the engine is initialized, TOTALCLOCK is set to zero.
			From then on, it is incremented 120 times a second by 1.  That
			means that the number of seconds elapsed is totalclock / 120.
		NUMFRAMES - The number of times the draw3dscreen function was called
			since the engine was initialized.  This helps to determine frame
			rate.  (Frame rate = numframes * 120 / totalclock.)

OTHER VARIABLES:

		STARTUMOST[320] is an array of the highest y-coordinates on each column
				that my engine is allowed to write to.  You need to set it only
				once.
		STARTDMOST[320] is an array of the lowest y-coordinates on each column
				that my engine is allowed to write to.  You need to set it only
				once.
***************************************************************************/

#endif
