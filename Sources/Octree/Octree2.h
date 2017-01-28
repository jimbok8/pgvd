#ifndef __OCTREE_2_H__
#define __OCTREE_2_H__

#include "clfw.hpp"
#include "Shaders/Shaders.hpp"
#include "Polylines/Polylines.h"
#include "GLUtilities/gl_utils.h"
#include "Options/Options.h"
#include "Kernels/Kernels.h"
#include "BoundingBox/BoundingBox.h"
#include <glm/glm.hpp>

extern "C" {
#include "OctreeResolution/Resln.h"
#include "Line/Line.h"
}
#include "Octree/OctNode.h"
#include "Quantize/Quantize.h"
#include "GLUtilities/Sketcher.h"

class Quadtree {
private:
  std::vector<OctNode> octree;
  std::vector<floatn> points;
  std::vector<intn> quantized_points;
  std::vector<intn> resolutionPoints;
  std::vector<Line> lines;
  std::vector<Line> orderedLines;
  std::vector<Conflict> conflicts;
  std::vector<Leaf> leaves;
  int initialOctreeSize;
  BoundingBox bb;
  Resln resln;
  int totalPoints;
  int totalLeaves;
  int totalResPoints;

  std::vector<glm::vec3> offsets;
  std::vector<glm::vec3> colors;
  std::vector<float> scales;

  /* GLuint drawVertices_vbo;
   GLuint drawIndices_vbo;
   GLuint vao;*/

  GLuint boxProgram_vao;
  GLuint positions_vbo;
  GLuint position_indices_vbo;
  GLuint instance_vbo;
  cl::Buffer pointsBuffer;
  cl::Buffer qpoints;
  cl::Buffer zpoints;
  cl::Buffer zpointsCopy;
  cl::Buffer linesBuffer;
  cl::Buffer LineLCPs;
  cl::Buffer lineIndices;
  cl::Buffer LCPToOctNode;
  cl::Buffer LCPBounds;
  cl::Buffer resQPoints;
  cl::Buffer leavesBuffer;
  cl::Buffer octreeBuffer;

  void getPoints(const PolyLines *polyLines, vector<floatn> &points, std::vector<Line> &lines);
  void getBoundingBox(const vector<floatn> &points, const int totalPoints, BoundingBox &bb);
  void getQuantizedPoints();
  void getZOrderPoints(cl::Buffer qPoints, cl::Buffer &zpoints, string zPointsName, int totalPoints);
  void getUnorderedBCellFacetPairs();
  void getVertexOctree(cl::Buffer zpoints_i, cl_int numZPoints, cl::Buffer &octree_o, string octreeName, int &octreeSize_o);
  void getConflictCells(cl::Buffer octree_i);
	//void getResPoints(cl::Buffer conflicts_i, int numConflicts, );
  void addResolutionPoints();

public:
  Quadtree();
  void build(const PolyLines* lines);
  typedef struct {
    float offset[3];
    float scale;
    float color[3];
  } Instance;
  std::vector<Instance> gl_instances;

  /* Drawing Methods */
  void draw(const glm::mat4& mvMatrix);

private:
  void clear();
  cl_int placePointsOnCurve(cl::Buffer points_i, int totalPoints, Resln resln, BoundingBox bb, string uniqueString, cl::Buffer &qpoints_o, cl::Buffer &zpoints_o);
  cl_int buildVertexOctree(cl::Buffer points_i, int totalPoints, Resln resln, BoundingBox bb, string uniqueString, cl::Buffer &octree_o, cl_int &totalOctnodes_o, cl::Buffer &leaves_o, cl_int &totalLeaves_o);
  cl_int resolveAmbiguousCells(cl::Buffer octree_i, cl_int totalOctNodes, cl::Buffer leaves_i, cl_int totalLeaves, cl::Buffer lines_i, cl_int totalLines, cl::Buffer qPoints_i, cl_int totalPoints, cl::Buffer orderedLineIndices_i, cl::Buffer firstLastFacetPairs_i);
  /* Drawing Methods */
  void addOctreeNodes();
  void addOctreeNodes(int index, floatn offset, float scale, float3 color);
  void addLeaf(int internalIndex, int leafIndex, float3 color);
  void addConflictCells();
  void drawResolutionPoints();
};


#endif
