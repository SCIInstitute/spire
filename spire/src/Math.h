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
/// \date   January 2013

#ifndef SPIRE_HIGH_MATH_H
#define SPIRE_HIGH_MATH_H

// Eigen includes.
// A few things to note about eigen:
// 1) Be careful about using it in structures and STL containers. Eigen elements
//    need to be 128bit aligned.
// 2) Secondary utility libraries will need to be constructed in order to fully
//    support eigen for our needs. Most of these are already constructed in GLM.
// 3) Eigen does not by default zero initialize it's matrices. You must define:
//    EIGEN_INITIALIZE_MATRICES_BY_ZERO before eigen headers are included.
// 4) There is no way to initialize to default identity matrix in eigen.
// These 4 items make up the majority of my decision to go with GLM instead 
// of eigen, although eigen may be a tad bit faster in the end. I'm looking more
// for convienence than speed. GLM resolves all of these issues.
// If we have any performance critical code, we can always optimize it away
// using eigen or custom code at a later time.

//#include "Eigen/Dense"
//#include <Eigen/StdVector>
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector3d)
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector4d)
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Quaterniond)
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3d)
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix4d)
//EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector2d)

// GLM
// We want swizzling, but not C++11 swizzling. That type of swizzling messes up
// vector displays in debuggers.
#define GLM_SWIZZLE
#define GLM_FORCE_CXX03
#define GLM_FORCE_RADIANS
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/quaternion.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/matrix_inverse.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

namespace CPM_SPIRE_NS {
namespace spire {

//constexpr double pi() { return std::atan(1)*4; }
/// \TODO: Change const back to constexpr
const double  PI_D  = 3.141592653589793238462;
const float   PI    = 3.14159265358979f;

//// Eigen typedefs
//typedef Eigen::Vector2d    V2;
//typedef Eigen::Vector3d    V3;
//typedef Eigen::Vector4d    V4;
//typedef Eigen::Quaterniond Quat;
//typedef Eigen::Matrix3d    M33;
//typedef Eigen::Matrix4d    M44;

// GLM typedefs
typedef glm::vec2            V2;
typedef glm::vec3            V3;
typedef glm::vec4            V4;
typedef glm::mat3            M33;
typedef glm::mat4            M44;
typedef glm::quat            Quat;

} // namespace spire
} // namespace CPM_SPIRE_NS

#endif 
