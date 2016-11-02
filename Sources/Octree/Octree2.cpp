#include <iostream>
#include <fstream>
#include <sstream>

#include <cstdio>

#include "./Octree2.h"

Octree2::Octree2() {
    const int n = 4;
    glm::vec3 drawVertices[n];
    drawVertices[0] = glm::vec3(-0.5, -0.5, 0);
    drawVertices[1] = glm::vec3(0.5, -0.5, 0);
    drawVertices[2] = glm::vec3(0.5, 0.5, 0);
    drawVertices[3] = glm::vec3(-0.5, 0.5, 0);

    glGenBuffers(1, &drawVertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, drawVertices_vbo);
    glBufferData(
        GL_ARRAY_BUFFER, n * sizeof(glm::vec3), drawVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, drawVertices_vbo);

    glGenBuffers(1, &positions_vbo);
    glGenBuffers(1, &instance_vbo);
    glGenBuffers(1, &position_indices_vbo);
    glGenVertexArrays(1, &boxProgram_vao);
    glBindVertexArray(boxProgram_vao);
    glEnableVertexAttribArray(Shaders::boxProgram->position_id);
    glEnableVertexAttribArray(Shaders::boxProgram->offset_id);
    glEnableVertexAttribArray(Shaders::boxProgram->scale_id);
    glEnableVertexAttribArray(Shaders::boxProgram->color_id);
    assert(glGetError() == GL_NO_ERROR);
    float points[] = {
      -.5, -.5, -.5,  -.5, -.5, +.5,
      +.5, -.5, +.5,  +.5, -.5, -.5,
      -.5, +.5, -.5,  -.5, +.5, +.5,
      +.5, +.5, +.5,  +.5, +.5, -.5,
    };
    unsigned char indices[] = {
      0, 1, 1, 2, 2, 3, 3, 0,
      0, 4, 1, 5, 2, 6, 3, 7,
      4, 5, 5, 6, 6, 7, 7, 4,
    };
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    assert(glGetError() == GL_NO_ERROR);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, position_indices_vbo);
    fprintf(stderr, "position_indices_vbo: %d\n", position_indices_vbo);
    assert(glGetError() == GL_NO_ERROR);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribPointer(Shaders::boxProgram->position_id, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribPointer(Shaders::boxProgram->offset_id, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribPointer(Shaders::boxProgram->scale_id, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribPointer(Shaders::boxProgram->color_id, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribDivisor(Shaders::boxProgram->offset_id, 1);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribDivisor(Shaders::boxProgram->scale_id, 1);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribDivisor(Shaders::boxProgram->color_id, 1);
    glBindVertexArray(0);
    assert(glGetError() == GL_NO_ERROR);

    resln = make_resln(1 << 8);
}

inline floatn min_floatn(const floatn a, const floatn b) {
    floatn result;
    for (int i = 0; i < DIM; ++i) {
        result.s[i] = (a.s[i] < b.s[i]) ? a.s[i] : b.s[i];
    }
    return result;
}

inline floatn max_floatn(const floatn a, const floatn b) {
    floatn result;
    for (int i = 0; i < DIM; ++i) {
        result.s[i] = (a.s[i] > b.s[i]) ? a.s[i] : b.s[i];
    }
    return result;
}

void Octree2::generatePoints(const PolyLines *polyLines) {
    const vector<vector<floatn>>& polygons = polyLines->getPolygons();
    lines = polyLines->getLines();

    // Get all vertices into a 1D array (karras_points).
    for (int i = 0; i < polygons.size(); ++i) {
        const vector<floatn>& polygon = polygons[i];
        for (int j = 0; j < polygon.size() - 1; ++j) {
            karras_points.push_back(polygon[j]);
        }
        karras_points.push_back(polygon.back());
    }
    if (polygons.empty()) return;
}

void Octree2::computeBoundingBox() {
    //Probably should be parallelized...
    floatn minimum = karras_points[0];
    floatn maximum = karras_points[0];
    for (int i = 1; i < totalPoints; ++i) {
        minimum = min_floatn(karras_points[i], minimum);
        maximum = max_floatn(karras_points[i], maximum);
    }
    bb = BB_initialize(&minimum, &maximum);
    bb = BB_make_centered_square(&bb);
}

void Octree2::quantizePoints(int numResolutionPoints) {
    Kernels::QuantizePoints_p(totalPoints, karrasPointsBuffer, quantizedPointsBuffer, bb.minimum, resln.width, bb.maxwidth);
}

void Octree2::makeZOrderPoints() {
    Kernels::PointsToMorton_p(quantizedPointsBuffer, zpoints, totalPoints, resln.bits);

}

void Octree2::sortLines() {
    Kernels::SortLinesByLvlThenVal_p(lines, sortedLinesBuffer, zpoints, resln);
}

void Octree2::buildVertexOctree() {
    Kernels::BuildOctree_p(zpoints, totalPoints, octree, resln.bits, resln.mbits);
}

void Octree2::identifyConflictCells() {
    cl::Buffer octreeBuffer = CLFW::Buffers["octree"];

    OctreeData od;
    od.fmin = bb.minimum;
    od.size = octree.size();
    od.qwidth = resln.width;
    od.maxDepth = resln.bits;
    od.fwidth = bb.maxwidth;

    Kernels::FindConflictCells_p(sortedLinesBuffer, lines.size(), octreeBuffer, od, conflictsBuffer, quantizedPointsBuffer);
    Kernels::DownloadConflicts(conflicts, conflictsBuffer, octree.size() * 4);
    assert(CLFW::Queues[0].finish() == CL_SUCCESS);
}

void Octree2::getResolutionPoints() {
    unsigned int totalAdditionalPoints = 0;
    Kernels::DownloadQPoints(quantized_points, CLFW::Buffers["quantizedPoints"], totalPoints);

    Kernels::SampleConflictCounts_s(octree.size(), conflicts.data(), &totalAdditionalPoints, orderedLines.data(),
        quantized_points.data(), resolutionPoints);

    for (int i = 0; i < resolutionPoints.size(); ++i) {
        karras_points.push_back(UnquantizePoint(&resolutionPoints[i], &bb.minimum, resln.width, bb.maxwidth));
    }

    using namespace GLUtilities;
    for (int i = 0; i < resolutionPoints.size(); ++i) {
        intn q = resolutionPoints[i];
        floatn newp = UnquantizePoint(&q, &bb.minimum, resln.width, bb.maxwidth);
        GLUtilities::Point p = { { newp.x, newp.y , 0.0, 1.0 },{ 1.0,0.0,0.0,1.0 } };
        Sketcher::instance()->add(p);
    }
}

void Octree2::insertResolutionPoints() {
    using namespace Kernels;
    if (resolutionPoints.size() != 0) {
        int original = totalPoints;
        int additional = resolutionPoints.size();
        totalPoints += additional;
        cl_int error = 0;

        //If the new resolution points fit inside the existing buffer.
        if (original + additional <= nextPow2(original)) {
            error |= CLFW::DefaultQueue.enqueueWriteBuffer(quantizedPointsBuffer, CL_TRUE, original * sizeof(intn), additional * sizeof(intn), resolutionPoints.data());
        }
        else {
            cl::Buffer oldQPointsBuffer = quantizedPointsBuffer;
            CLFW::Buffers["quantizedPoints"] = cl::Buffer(CLFW::Contexts[0], CL_MEM_READ_WRITE, nextPow2(original + additional) * sizeof(intn));
            quantizedPointsBuffer = CLFW::Buffers["quantizedPoints"];
            error |= CLFW::DefaultQueue.enqueueCopyBuffer(oldQPointsBuffer, quantizedPointsBuffer, 0, 0, original * sizeof(intn));
            error |= CLFW::DefaultQueue.enqueueWriteBuffer(quantizedPointsBuffer, CL_TRUE, original * sizeof(intn), additional * sizeof(intn), resolutionPoints.data());
        }
        assert(error == CL_SUCCESS);
    }
}

void Octree2::build(const PolyLines *polyLines) {
    using namespace std;
    using namespace GLUtilities;
    using namespace Kernels;

    Sketcher::instance()->clear();

    int totalItterations = 0;
    int previousSize;

    karras_points.clear();
    gl_instances.clear();
    octree.clear();
    resolutionPoints.clear();

    /* Quantize the polygon points. */
    generatePoints(polyLines);
    if (karras_points.size() == 0) return;
    totalPoints = karras_points.size();
    UploadKarrasPoints(karras_points, karrasPointsBuffer);
    quantizePoints();
    computeBoundingBox();
    makeZOrderPoints();

    vector<intn> qpoints_vec(karras_points.size());
    CLFW::DefaultQueue.enqueueReadBuffer(CLFW::Buffers["quantizedPoints"], CL_TRUE, 0, sizeof(intn) * karras_points.size(), qpoints_vec.data());

    intn test = QuantizePoint(&karras_points[0], &bb.minimum, resln.width, bb.maxwidth);

    do {
        totalItterations++;
        CLFW::DefaultQueue = CLFW::Queues[0];
        previousSize = octree.size();

        insertResolutionPoints();
        makeZOrderPoints(); //TODO: only z-order the additional points here...
        CLFW::DefaultQueue.finish();

        /*
          On one queue, sort the lines for the ambiguous cell detection.
          On the other, start building the karras octree.
        */
        CLFW::DefaultQueue = CLFW::Queues[1];
        sortLines();

        CLFW::DefaultQueue = CLFW::Queues[0];
        buildVertexOctree();

        CLFW::Queues[0].finish();
        CLFW::Queues[1].finish();

        /* Identify the cells that contain more than one object. */
        if (lines.size() == 0 || lines.size() == 1) break;
        identifyConflictCells();

        /* Resolve one iteration of conflicts. */
        orderedLines.resize(lines.size());
        Kernels::DownloadLines(sortedLinesBuffer, orderedLines, orderedLines.size());
        getResolutionPoints();
    } while (previousSize != octree.size() && resolutionPoints.size() != 0);

    /* Add the octnodes so they'll be rendered with OpenGL. */
    addOctreeNodes();
    /* Add conflict cells so they'll be rendered with OpenGL. */
    addConflictCells();

    cout << "Total itterations: " << totalItterations << endl;
}

/* Drawing Methods */
void Octree2::addOctreeNodes() {
    floatn temp;
    floatn center;

    if (octree.size() == 0) return;
    center = (bb.minimum + bb.maxwidth*.5);
    float3 color = { 1.0, 1.0, 1.0 };
    addOctreeNodes(0, center, bb.maxwidth, color);
}

void Octree2::addOctreeNodes(int index, floatn offset, float scale, float3 color) {
    Instance i = { offset.x, offset.y, 0.0, scale, color.x, color.y, color.z };
    gl_instances.push_back(i);
    if (index != -1) {
        OctNode current = octree[index];
        scale /= 2.0;
        float shift = scale / 2.0;
        addOctreeNodes(current.children[0], { offset.x - shift, offset.y - shift }, scale, color);
        addOctreeNodes(current.children[1], { offset.x + shift, offset.y - shift }, scale, color);
        addOctreeNodes(current.children[2], { offset.x - shift, offset.y + shift }, scale, color);
        addOctreeNodes(current.children[3], { offset.x + shift, offset.y + shift }, scale, color);
    }
}

void Octree2::addLeaf(int internalIndex, int childIndex, float3 color) {
    floatn center = BB_center(&bb);
    float octreeWidth = bb.maxwidth;
    OctNode node = octree[internalIndex];

    //Shift to account for the leaf
    float width = octreeWidth / (1 << (node.level + 1));
    float shift = width / 2.0;
    center.x += (childIndex & 1) ? shift : -shift;
    center.y += (childIndex & 2) ? shift : -shift;

    //Shift for the internal node
    while (node.parent != -1) {
        for (childIndex = 0; childIndex < 4; ++childIndex) {
            if (octree[node.parent].children[childIndex] == internalIndex)
                break;
        }

        shift *= 2.0;
        center.x += (childIndex & 1) ? shift : -shift;
        center.y += (childIndex & 2) ? shift : -shift;

        internalIndex = node.parent;
        node = octree[node.parent];
    }

    Instance i = {
      { center.x, center.y, 0.0 }
      , width
      , { color.x, color.y, color.z }
    };
    gl_instances.push_back(i);
}

void Octree2::addConflictCells() {
    if (conflicts.size() == 0) return;
    for (int i = 0; i < octree.size(); ++i) {
        for (int j = 0; j < 4; j++) {
            if (conflicts[4 * i + j].color == -2) {
                addLeaf(i, j, { 1.0, 0.0, 0.0 });
            }
        }
    }
}

void Octree2::draw() {
    Shaders::boxProgram->use();
    glBindVertexArray(boxProgram_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Instance) * gl_instances.size(), gl_instances.data(), GL_STREAM_DRAW);
    glm::mat4 identity(1.0);
    glUniformMatrix4fv(Shaders::boxProgram->matrix_id, 1, 0, &(identity[0].x)); //glm::value_ptr wont work on identity for some reason...
    glUniform1f(Shaders::boxProgram->pointSize_id, 10.0);
    assert(glGetError() == GL_NO_ERROR);
    glLineWidth(2.0);
    glDrawElementsInstanced(GL_LINES, 12 * 2, GL_UNSIGNED_BYTE, 0, gl_instances.size());
    glBindVertexArray(0);
}