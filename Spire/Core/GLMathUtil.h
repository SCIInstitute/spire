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
/// \brief  Common GL math utilities.

#ifndef SPIRE_HIGH_GLMATHUTIL_H
#define SPIRE_HIGH_GLMATHUTIL_H

#include "Core/Math.h"

namespace Spire {

//----------------------------------------------------------------------------------------
//
// Internal format conversion to GL representation.
// 
//----------------------------------------------------------------------------------------
void M44toArray16(const M44& in, float* out);
void M33toArray9(const M33& in, float* out);
void V3toArray3(const V3& in, float* out);
void V3toArray4(const V3& in, float* out);
void V4toArray3(const V4& in, float* out);
void V4toArray4(const V4& in, float* out);


} // namespace Spire 

#endif 
