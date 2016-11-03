#ifndef RCT1_DEBUG_H
#define RCT1_DEBUG_H

//#define DBG_DISABLE

#ifdef DBG
	#ifndef DBG_DISABLE
		#include <stdio.h>
		#define dbg(...) {printf("D: "); printf(__VA_ARGS__); printf("\n");}
	#else
		#define dbg(...)
	#endif
#else
	#define dbg(...)
#endif


#endif