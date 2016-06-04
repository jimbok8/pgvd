#pragma once
#include "KernelBox_.h"
#include "z_order.h"
namespace KernelBox {
  cl_int PointsToMorton_p(cl_int size, cl_int bits) {
    int globalSize = nextPow2(size);
    if (!isBufferUsable(buffers.bigUnsignedInput, sizeof(BigUnsigned)* (globalSize)))
      createBuffer(buffers.bigUnsignedInput, sizeof(BigUnsigned)* (globalSize));

    const size_t globalWorkSize[] = { globalSize, 0, 0 };
    cl_int error = 0;

    if (!buffers.points) return CL_INVALID_MEM_OBJECT;
    if (!buffers.bigUnsignedInput) return CL_INVALID_MEM_OBJECT;
    cl_mem clBUs = buffers.bigUnsignedInput->getBuffer();
    cl_mem clpoints = buffers.points->getBuffer();
    error |= clSetKernelArg(Kernels.at("PointsToMortonKernel"), 0, sizeof(cl_mem), &clBUs);
    error |= clSetKernelArg(Kernels.at("PointsToMortonKernel"), 1, sizeof(cl_mem), &clpoints);
    error |= clSetKernelArg(Kernels.at("PointsToMortonKernel"), 2, sizeof(cl_int), &size);
    error |= clSetKernelArg(Kernels.at("PointsToMortonKernel"), 3, sizeof(cl_int), &bits);
    error |= clEnqueueNDRangeKernel(CLFW::Queues[0], Kernels.at("PointsToMortonKernel"), 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
    return error;
  };

  cl_int PointsToMorton_s(cl_int size, cl_int bits, cl_int2* points,  BigUnsigned* result) {
    int nextPowerOfTwo = nextPow2(size);
    for (int gid = 0; gid < nextPowerOfTwo; ++gid) {
      if (gid < size) {
        xyz2z(&result[gid], points[gid], bits);
      }
      else {
        initBlkBU(&result[gid], 0);
      }
    }
    return 0;
  }

  /*void BitPredicate(cl_mem input, cl_mem predicate, int &index, unsigned char compared, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  cl_ulong i = index;
  unsigned char c = compared;
  error = clSetKernelArg(bitPredicateKernel, 0, sizeof(cl_mem), &input);
  error = clSetKernelArg(bitPredicateKernel, 1, sizeof(cl_mem), &predicate);
  error = clSetKernelArg(bitPredicateKernel, 2, sizeof(Index), &i);
  error = clSetKernelArg(bitPredicateKernel, 3, sizeof(unsigned char), &c);
  error = clEnqueueNDRangeKernel(queue, bitPredicateKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox predication: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  };
  void UniquePredicate(cl_mem input, cl_mem predicate, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  error = clSetKernelArg(uniquePredicateKernel, 0, sizeof(cl_mem), &input);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox unique predication 1: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  error = clSetKernelArg(uniquePredicateKernel, 1, sizeof(cl_mem), &predicate);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox unique predication 2: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  error = clEnqueueNDRangeKernel(queue, uniquePredicateKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox unique predication: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }
  size_t GetSteamScanWorkGroupSize(size_t globalSize) {
  using namespace std;
  size_t localSize;
  clGetKernelWorkGroupInfo(scanKernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, NULL);
  return min((int)(localSize), (int)globalSize);
  }
  void StreamScan(cl_mem input, cl_mem intermediate, cl_mem intermediateCopy, cl_mem result, size_t globalSize) {
  size_t localSize = getSteamScanWorkGroupSize(globalSize);
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  const size_t localWorkSize[] = { localSize, 0, 0 };
  //std::cout << localSize << std::endl;

  int currentNumWorkgroups = (globalSize / localSize);
  clEnqueueCopyBuffer(queue, intermediateCopy, intermediate, 0, 0, sizeof(Index)* currentNumWorkgroups, 0, NULL, NULL);

  clSetKernelArg(scanKernel, 0, sizeof(cl_mem), &input);
  clSetKernelArg(scanKernel, 1, sizeof(cl_mem), &result);
  clSetKernelArg(scanKernel, 2, sizeof(cl_mem), &intermediate);
  clSetKernelArg(scanKernel, 3, localSize * sizeof(Index), NULL);
  clSetKernelArg(scanKernel, 4, localSize * sizeof(Index), NULL);

  error = clEnqueueNDRangeKernel(queue, scanKernel, 1, 0, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox stream scan: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  lastNumWorkgroups = currentNumWorkgroups;
  };
  void SingleCompact(cl_mem inputBuffer, cl_mem resultBuffer, cl_mem PBuffer, cl_mem ABuffer, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(singleCompactKernel, 0, sizeof(cl_mem), &inputBuffer);
  clSetKernelArg(singleCompactKernel, 1, sizeof(cl_mem), &resultBuffer);
  clSetKernelArg(singleCompactKernel, 2, sizeof(cl_mem), &PBuffer);
  clSetKernelArg(singleCompactKernel, 3, sizeof(cl_mem), &ABuffer);
  error = clEnqueueNDRangeKernel(queue, singleCompactKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }
  void DoubleCompact(cl_mem inputBuffer, cl_mem resultBuffer, cl_mem resultBufferCopy, cl_mem LPBuffer, cl_mem LABuffer, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(doubleCompactKernel, 0, sizeof(cl_mem), &inputBuffer);
  clSetKernelArg(doubleCompactKernel, 1, sizeof(cl_mem), &resultBuffer);
  clSetKernelArg(doubleCompactKernel, 2, sizeof(cl_mem), &LPBuffer);
  clSetKernelArg(doubleCompactKernel, 3, sizeof(cl_mem), &LABuffer);
  clSetKernelArg(doubleCompactKernel, 4, sizeof(Index), &globalSize);

  BigUnsigned zero;
  initBlkBU(&zero, 0);
  //clEnqueueFillBuffer(queue, resultBuffer, &zero, sizeof(BigUnsigned), 0, sizeof(BigUnsigned)* (globalSize), 0, NULL, NULL);
  clEnqueueCopyBuffer(queue, resultBufferCopy, resultBuffer, 0, 0, sizeof(BigUnsigned) * globalSize, 0, NULL, NULL);
  error = clEnqueueNDRangeKernel(queue, doubleCompactKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  };
  void BuildBinaryRadixTree(cl_mem internalNodes, cl_mem leafNodes, cl_mem mpoints, cl_int mbits, cl_int n, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(binaryRadixTreeKernel, 0, sizeof(cl_mem), &internalNodes);
  clSetKernelArg(binaryRadixTreeKernel, 1, sizeof(cl_mem), &leafNodes);
  clSetKernelArg(binaryRadixTreeKernel, 2, sizeof(cl_mem), &mpoints);
  clSetKernelArg(binaryRadixTreeKernel, 3, sizeof(cl_int), &mbits);
  clSetKernelArg(binaryRadixTreeKernel, 4, sizeof(cl_int), &n);

  error = clEnqueueNDRangeKernel(queue, binaryRadixTreeKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }
  void ComputeLocalSplits(cl_mem localSplits, cl_mem localSplitsCopy, cl_mem I, size_t size, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(computeLocalSplitsKernel, 0, sizeof(cl_mem), &localSplits);
  clSetKernelArg(computeLocalSplitsKernel, 1, sizeof(cl_mem), &I);
  clSetKernelArg(computeLocalSplitsKernel, 2, sizeof(cl_mem), &size);

  clEnqueueCopyBuffer(queue, localSplitsCopy, localSplits, 0, 0, sizeof(unsigned int) * globalSize, 0, NULL, NULL);

  error = clEnqueueNDRangeKernel(queue, computeLocalSplitsKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }
  void BRT2Octree(cl_mem I, cl_mem octree, cl_mem local_splits, cl_mem prefix_sums, cl_int n, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(brt2OctreeKernel, 0, sizeof(cl_mem), &I);
  clSetKernelArg(brt2OctreeKernel, 1, sizeof(cl_mem), &octree);
  clSetKernelArg(brt2OctreeKernel, 2, sizeof(cl_mem), &local_splits);
  clSetKernelArg(brt2OctreeKernel, 3, sizeof(cl_mem), &prefix_sums);
  clSetKernelArg(brt2OctreeKernel, 4, sizeof(cl_int), &n);

  error = clEnqueueNDRangeKernel(queue, brt2OctreeKernel, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }
  void BRT2Octree_init(cl_mem I, cl_mem octree, cl_mem local_splits, cl_mem prefix_sums, cl_int n, size_t globalSize) {
  const size_t globalWorkSize[] = { globalSize, 0, 0 };
  clSetKernelArg(brt2OctreeKernel_init, 0, sizeof(cl_mem), &I);
  clSetKernelArg(brt2OctreeKernel_init, 1, sizeof(cl_mem), &octree);
  clSetKernelArg(brt2OctreeKernel_init, 2, sizeof(cl_mem), &local_splits);
  clSetKernelArg(brt2OctreeKernel_init, 3, sizeof(cl_mem), &prefix_sums);
  clSetKernelArg(brt2OctreeKernel_init, 4, sizeof(cl_int), &n);

  error = clEnqueueNDRangeKernel(queue, brt2OctreeKernel_init, 1, 0, globalWorkSize, NULL, 0, nullptr, nullptr);
  if (error != CL_SUCCESS) {
  std::cerr << "KernelBox double compaction: OpenCL call failed with error " << error << std::endl;
  std::getchar();
  std::exit;
  }
  }*/
}