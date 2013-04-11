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

//------------------------------------------------------------------------------
int geomCreateSphere(std::vector<uint8_t>& vboOut, std::vector<uint16_t>& iboOut,
                     int subdivisionLevel, bool smoothNormals)
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
  size_t numOctantEdgeVertices  = (twoN + 1);
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

  // There are two ways to build this sphere. One is using this division factor
  // and imperatively running down one edge on one of the faces of the 
  // octahedron. The other is to functionally recurse over the the face
  // and calculate the appropriate IBO indices.
  // The imperative version would be better for a tessellation shader.
  size_t divFactor = numOctantEdgeVertices - 1;

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
