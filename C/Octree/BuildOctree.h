#pragma once

#ifdef __OPENCL_VERSION__ 
  #include "./OpenCL/C/BinaryRadixTree/BuildBRT.h"
  #include "./OpenCL/C/Octree/OctNode.h"
  #include "./OpenCL/C/BinaryRadixTree/BrtNode.h"
#else
  #include "../BinaryRadixTree/BuildBRT.h"
  #include "../Octree/OctNode.h"
  #include "../BinaryRadixTree/BrtNode.h"
  #define __local
  #define __global
#endif

int quadrantInLcp(BrtNode* brt_node, const int i);
void ComputeLocalSplits_SerialKernel(__global unsigned int* local_splits, __global BrtNode* I, const int size);
void ComputeLocalSplits(__global unsigned int* local_splits, __global BrtNode* I, const int gid );

void brt2octree_init( const int brt_i, __global OctNode* octree );
void brt2octree( const int brt_i, __global BrtNode* I, __global volatile OctNode* octree, __global unsigned int* local_splits, __global unsigned int* prefix_sums, const int n, const int octree_size);
void brt2octree_kernel(__global BrtNode* I, __global OctNode* octree, __global unsigned int* local_splits, __global unsigned int* prefix_sums, const int n);

#ifndef __OPENCL_VERSION__
  #undef __local
  #undef __global
#endif
