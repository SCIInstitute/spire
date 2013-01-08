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

// Include Sony's vector-math library.
#include "AOS_VectorMath.h"

namespace Spire {

//constexpr double pi() { return std::atan(1)*4; }
constexpr double  PI_D  = 3.141592653589793238462;
constexpr float   PI    = 3.14159265358979f;

// Typedef's for the vector math library.
typedef Vector::Vector3    V3;
typedef Vector::Vector4    V4;
typedef Vector::Quat       Quat;
typedef Vector::Matrix3    M33;
typedef Vector::Matrix4    M44;
typedef Vector::Transform3 Trafo3;
typedef Vector::Point3     Point3;


} // namespace spire

#endif 
