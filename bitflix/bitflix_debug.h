#ifndef __BITFLIX_DEBUG_H__
#define __BITFLIX_DEBUG_H__

#ifdef _LEAK_DEBUG

#define _CRTDBG_MAP_ALLOC 1
#define _CRTDBG_MAP_ALLOC_NEW 1
#include <stdlib.h>
#include <crtdbg.h>

#endif

void debug_begin();
void debug_end();
void debug_dump_blocks_in_heaps();

#define BFTrace printf

#endif
