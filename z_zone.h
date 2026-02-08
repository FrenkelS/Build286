// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2023-2026 by Frenkel Smeijers
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//
//---------------------------------------------------------------------

#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stdint.h>

typedef enum {false, true} boolean;

void Z_Init(void);
void Z_Shutdown(void);
boolean Z_IsEnoughFreeMemory(uint16_t size);
void __far* Z_MallocStatic(uint32_t size);
void __far* Z_MallocStaticWithUser(uint32_t size, void __far*__far* user); 
void Z_ChangeTagToStatic(const void __far* ptr);
void Z_ChangeTagToCache(const void __far* ptr);
void Z_Free(const void __far* ptr);
void Z_CheckHeap(void);

#endif
