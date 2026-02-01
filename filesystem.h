// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

// This file IS NOT A PART OF Ken Silverman's original release

#ifndef __FILESYSTEM__
#define __FILESYSTEM__

#include <stdint.h>
#include "compiler.h"

void initgroupfile(char *filename);
void uninitgroupfile(void);
int32_t kopen4load(char *filename, uint8_t searchfirst);
void kread(int32_t handle, void __far* buffer, int32_t leng);
void klseek(int32_t handle, int32_t offset, int32_t whence);
void kclose(int32_t handle);

#endif
