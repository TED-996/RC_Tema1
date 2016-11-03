#ifndef RCT1_DEBUG_H
#define RCT1_DEBUG_H

#ifdef DBG
	#ifndef DBG_DISABLE
		#include <stdio.h>
		#define dbg(...) {printf("D: "); printf(__VA_ARGS__); printf("\n");}
	#endif
#else
	#define dbg(...)
#endif


#endif