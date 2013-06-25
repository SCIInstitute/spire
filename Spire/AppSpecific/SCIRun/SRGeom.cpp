/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/// \author James Hughes
/// \date   April 2013

#include "SRGeom.h"
#include "Core/Math.h"

namespace Spire {
namespace SCIRun {

//------------------------------------------------------------------------------
size_t intPow(size_t base, size_t exp)
{
  // Could do this much much quicker by using previous multiplications.
  // For example, if we have already computed 2*2, we can: 2*2 * 2*2 = 2^4
  // Memoization.
  size_t ret = base;
  for (size_t i = 0; i < exp - 1; i++)
  {
    ret *= base;
  }
  return ret;
}

/// This tessellation is for a triangle. We can exploit symmetry and build the
/// entire octahedron at once...
/// Both upperBound and lowerBound refer to some 'n' referencing which tessellation
/// edge we are on.
/// indexOffset is used to offset the indices by some constant amount. This
/// variable remains constant throughout the recursion.
///
/// This function ends up writing over data in the vertex buffer twice.
/// The side effects of this function are stored in the given Vertex / Index
/// buffers.
///
/// Returns the number of elements added to the IBO, and the number of vertices
/// added to the VBO.
///
/// iboOffset is relative to uint16_t.
void triangleTesselateRecurse(size_t lowerBoundN, const V3& lowerCoords, const V3& adjacentLowerCoords,
                              size_t upperBoundN, const V3& upperCoords, const V3& adjacentUpperCoords,
                              std::vector<uint8_t>& vboOut, size_t vboOffset,
                              std::vector<uint16_t>& iboOut, size_t iboOffset,
                              int subdivisionsLeft, int indexOffset)
{
  const int vboStride = sizeof(float) * (3 + 3);

  // Calculate our new position.
  V3 lowerToUpper   = upperCoords - lowerCoords;
  V3 ourPos         = lowerCoords + (lowerToUpper / 2.0f);
  V3 adjacentLowToUp= adjacentUpperCoords - adjacentLowerCoords;
  V3 ourAdjacentPos = adjacentLowerCoords + (adjacentLowToUp / 2.0f);
  size_t ourBoundN  = (upperBoundN - lowerBoundN) / 2.0f;  // Both upperBoundN and lowerBoundN are even (some 2^n).


  if (subdivisionsLeft == 0)
  {
    // Calculate normal common to all faces we are going to construct.
    V3 normal = glm::cross(lowerCoords, adjacentLowerCoords);
    {
      V3 calcUpper = adjacentUpperCoords - upperCoords;
      V3 calcLower = lowerCoords - upperCoords;
      normal = glm::cross(calcUpper, calcLower);
    }

    // We are at one of the leaf elements.


    // Calculate lower indices.
    // This is our offset into the vertex buffer.
    size_t lowerNumVerticesBefore = lowerBoundN * (lowerBoundN + 1) / 2;
    float* vboOffsetLower = reinterpret_cast<float*>(&vboOut[vboStride * lowerNumVerticesBefore] + vboOffset);

    // Calculate upper indices. Calculate number of vertices that must have come
    // before this point and use that as the index offset.
    size_t upperNumVerticesBefore = upperBoundN * (upperBoundN + 1) / 2;  
    float* vboOffsetUpper = reinterpret_cast<float*>(&vboOut[vboStride * upperNumVerticesBefore] + vboOffset);

    // upperNumVerticesBefore is our offset into the vertex buffer.
    // It also determines what vertex index we should use for populating the
    // IBO.

    // Calculate appropriate indices from upperBoundN and lowerBoundN.
    // These numbers represent our location along the tesselation edge, 0..2^n)

    // Construct upper VBO entries.
    // Normals will be reconstructed when tesselation is 'applied'.
    // Smooth normals are simply the normals directed outward from the center
    // of the sphere. Face normals are much simpler than that, but a new
    // vertex buffer would need to be created with distinct vertices for every
    // single triangle element.
    V3 upperVector = adjacentUpperCoords - upperCoords;
    for (size_t i = 0; i <= upperBoundN; i++)
    {
      float fraction = static_cast<float>(i) / static_cast<float>(upperBoundN);
      V3 fromUpperCoord = upperVector * fraction;
      V3 newPos = upperCoords + fromUpperCoord;
      
      float* newOffset = vboOffsetUpper + i * (3 + 3) * sizeof(float);
      newOffset[0] = newPos.x;
      newOffset[1] = newPos.y;
      newOffset[2] = newPos.z;
      newOffset[3] = normal.x;
      newOffset[4] = normal.y;
      newOffset[5] = normal.z;
    }
    
    // Construct lower VBO entries.
    V3 lowerVector = adjacentLowerCoords - lowerCoords;
    for (size_t i = 0; i <= lowerBoundN; i++)
    {
      float fraction = static_cast<float>(i) / static_cast<float>(lowerBoundN);
      V3 fromLowerCoord = lowerVector * fraction;
      V3 newPos = lowerCoords + fromLowerCoord;
      
      float* newOffset = vboOffsetLower + i * (3 + 3) * sizeof(float);
      newOffset[0] = newPos.x;
      newOffset[1] = newPos.y;
      newOffset[2] = newPos.z;
      newOffset[3] = normal.x;
      newOffset[4] = normal.y;
      newOffset[5] = normal.z;
    }

    // The IBO offset is the number of faces that came before the lower offset.
    // Numbre of faces before.
    size_t twoNLower = intPow(2, lowerBoundN);
    size_t numFacesBefore = (twoNLower - 1) * (twoNLower) / 2;
    uint16_t* iboOffsetLower = &iboOut[numFacesBefore * 3 + iboOffset];

    // Construct IBO entries using the lower bound. Every iteration results
    // in a new triangle.
    int priorUpperIndex = upperNumVerticesBefore;
    int priorLowerIndex = lowerNumVerticesBefore;
    for (size_t i = 0; i <= lowerBoundN; i++)
    {
      uint16_t* iboLoc = iboOffsetLower + i * 3;
      if (i % 2 == 0)
      {
        // 1 lower index and 2 upper indices used
        // Use priorLowerIndex and (priorUpperIndex, priorUpperIndex + 1)
        // Counter clockwise winding.
        iboLoc[0] = priorLowerIndex;
        iboLoc[1] = priorUpperIndex;
        iboLoc[2] = priorUpperIndex + 1;

        priorUpperIndex = priorUpperIndex + 1;
      }
      else
      {
        // 2 lower indices and 1 upper index used
        // Counter clockwise winding.
        iboLoc[0] = priorUpperIndex;
        iboLoc[1] = priorLowerIndex + 1;
        iboLoc[2] = priorLowerIndex;

        priorLowerIndex = priorLowerIndex + 1;
      }
    }
  }
  else
  {
    // Recalculate lower and upper bounds for each of our child elements.    

    // Left
    triangleTesselateRecurse(lowerBoundN, lowerCoords, adjacentLowerCoords,
                             ourBoundN, ourPos, ourAdjacentPos,
                             vboOut, vboOffset,
                             iboOut, iboOffset,
                             subdivisionsLeft - 1, indexOffset);

    // Right
    triangleTesselateRecurse(ourBoundN, ourPos, ourAdjacentPos,
                             upperBoundN, upperCoords, adjacentUpperCoords,
                             vboOut, vboOffset,
                             iboOut, iboOffset,
                             subdivisionsLeft - 1, indexOffset);
  }
}

//------------------------------------------------------------------------------
int geomCreateSphere(std::vector<uint8_t>& vboOut, std::vector<uint16_t>& iboOut,
                     float radius, int subdivisionLevel, bool smoothNormals)
{
  size_t n = subdivisionLevel;
  size_t twoN = intPow(2, n);

  // The basic idea for constructing this sphere comes from the accepted answer at:
  // http://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
  // But it leaves out the implementation/tessellation details.

  // Consider one face of the untesselated octahedron. Because of the symmetries
  // of the sphere, constructing one face of the octahedron is sufficient to
  // build the rest of the sphere when transformed appropriately.

  // Because of this we will only consider one face of the octahedron when
  // discussing this implementation.

  // The number of tesselated vertices along one edge of the untesselated 
  // face doubles with each subdivision level. Starting at one of the corners
  // of the to-be-tesselated face (triangle), we run down one of the edges.
  // For every tessellated vertex we run across, the number of vertices along 
  // the line that intersects the adjacent edge and the current vertex increases
  // by one. This explanation is better suited by a drawing.
  
  /// \todo Draw a diagram depicting the tessellation of a triangle.

  // 8 octants compose the sphere.
  // 2^(2n) + 3*2^n + 2
  size_t numOctantVertices      = (twoN + 1) * (twoN + 2) / 2;
  size_t numOctantEdgeVertices  = (twoN + 1);   // One minus this is always even.
  size_t numOctantFaces         = (twoN * twoN);

  /// \todo VBO should not duplicate octahedron's edge vertices.
  
  /// \todo Use this code in the tessellation shader when we want to speed
  ///       this part of the rendering pipeline up.

  size_t numVertices  = numOctantVertices * 8;
  size_t numFaces     = numOctantFaces * 8;

  // Construct 1 octant, and use a transform to build the other octants.
  // Normals on the boundary of each octant are simply the normalized vectors
  // from the boundary vertex to the center of the sphere.
  size_t vboSize = numVertices * ((3 + 3) * sizeof(float));   // Vertex (3) + Normal (3)
  size_t iboSize = numFaces * 3;  // The vector is already a uint16_t.

  V3 upperControl;
  V3 lowerControl;
  V3 adjLowerControl;

  // Control face.
  // We will simply increment the indices of the following faces by a constant
  // factor.
  upperControl    = V3(0.0f, radius, 0.0f);
  lowerControl    = V3(radius, 0.0f, radius);
  adjLowerControl = V3(-radius, 0.0f, radius);
  triangleTesselateRecurse(0, lowerControl, adjLowerControl,
                           twoN, upperControl, upperControl,
                           vboOut, 0,
                           iboOut, 0,
                           subdivisionLevel, 0);

  // Determine control edge for face, and use that as the initial starting points.
  /*
  // There are two ways to build this sphere. One is using this division factor
  // and imperatively running down one edge on one of the faces of the 
  // octahedron. The other is to functionally recurse over the the face
  // and calculate the appropriate IBO indices.
  // The imperative version would be better for a tessellation shader.
  size_t divFactor = numOctantEdgeVertices - 1; // This is always an even number.

  // Imperative method (floating point error may creep in)
  for (int i = 1; i < numOctantEdgeVertices - 1; i++) // Subtract two for each of the edge's endpoints.
  {
    divFactor * i
  }
  */

  return numFaces;
}

//------------------------------------------------------------------------------
void geomCreateCappedCylinder()
{

}

//------------------------------------------------------------------------------
void geomCreateCylinder()
{

}

//------------------------------------------------------------------------------
void geomCreateCapsule()
{
  
}

} } // end of namespace Spire::SCIRun
