#pragma once
#define OCT2D
#ifdef __OPENCL_VERSION__
	#define DIM 2
#else
	#ifdef OCT2D
	#define DIM 2
	#else
	#define DIM 3
	#endif // OCT2D
#endif