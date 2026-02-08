// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __DRAW__
#define __DRAW__

#include <stdint.h>
#include "compiler.h"

extern uint8_t __far* _s_screen;

void setgamemode(void);
void nextpage(void);

#endif
