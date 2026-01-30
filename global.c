// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#include "engine.h"

int32_t _a_transmode = 0;
int32_t _a_glogx, _a_glogy;

sectortype __far sector[MAXSECTORS];
walltype __far wall[MAXWALLS];
spritetype __far sprite[MAXSPRITES];

int32_t spritesortcnt;
spritetype __far tsprite[MAXSPRITESONSCREEN];

int32_t ylookup[YDIM+1];
int32_t yxaspect, viewingrange;

volatile int32_t totalclock;
int32_t numframes;

int16_t __far startumost[XDIM];
int16_t __far startdmost[XDIM];

int16_t __far pskyoff[MAXPSKYTILES];
int16_t pskybits;

int16_t __far headspritesect[MAXSECTORS+1];
int16_t __far nextspritesect[MAXSPRITES];

uint8_t __far* _s_screen;

int32_t transarea = 0;

int32_t __far xb1[MAXWALLSB];
int32_t __far yb1[MAXWALLSB];
int32_t __far xb2[MAXWALLSB];
int32_t __far yb2[MAXWALLSB];
int32_t __far rx1[MAXWALLSB];
int32_t __far ry1[MAXWALLSB];
int32_t __far rx2[MAXWALLSB];
int32_t __far ry2[MAXWALLSB];

int16_t __far thewall[MAXWALLSB];

int16_t __far smost[MAXYSAVES];
int16_t __far smoststart[MAXWALLSB];
uint8_t __far smostwalltype[MAXWALLSB];
int32_t __far smostwall[MAXWALLSB];
int32_t smostwallcnt = -1L;

int32_t __far spritesx[MAXSPRITESONSCREEN];
int32_t __far spritesy[MAXSPRITESONSCREEN+1];

spritetype __far* tspriteptr[MAXSPRITESONSCREEN];

int16_t __far uwall[XDIM];
int16_t __far dwall[XDIM];

int32_t __far swall[XDIM];
int32_t __far lwall[XDIM+4];

int32_t xdimenscale;
int32_t xdimscale;

int32_t horizlookup[YDIM * 4];

int32_t globalposx, globalposy;
int32_t globalposz;
int32_t globalhoriz;
int16_t globalang;

int32_t globalpal;
int32_t cosglobalang, singlobalang;
int32_t cosviewingrangeglobalang, sinviewingrangeglobalang;

int32_t globaluclip, globaldclip;
int32_t globvis;
int32_t globalvisibility;
int32_t globalhisibility;

int32_t xyaspect;
int32_t viewingrangerecip;

int32_t asm1, asm2;

int32_t globalxpanning, globalypanning, globalshade;
int16_t globalshiftval;
int32_t globalzd;
int32_t globalyscale;
int32_t globalorientation;
int32_t globalx1, globaly1, globalx2, globaly2;

int32_t lastx[YDIM];

int16_t __far clipsectorlist[MAXCLIPNUM];
int16_t clipsectnum;


int16_t editstatus = 0;
int16_t searchit;
int32_t searchx = -1, searchy;                          //search input
int16_t searchsector, searchwall, searchstat;     //search output
