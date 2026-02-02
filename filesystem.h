// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __FILESYSTEM__
#define __FILESYSTEM__

#include <stdint.h>
#include <fcntl.h>
#include "compiler.h"

void initgroupfile(char *filename);
void uninitgroupfile(void);
int kopen4load(char *filename, uint_fast8_t searchfirst);
void kread(int handle, void __far* buffer, size_t leng);
void klseek(int handle, off_t offset, int whence);
void kclose(int handle);

#endif
