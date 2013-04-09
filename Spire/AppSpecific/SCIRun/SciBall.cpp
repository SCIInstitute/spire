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

#include "SciBall.h"

namespace Spire {
namespace SCIRun {

SciBall::SciBall(const V3& center, float radius, const M44& screenToTCS) :
    mCenter(center),
    mRadius(radius),
    mScreenToTCS(screenToTCS)
{
}

SciBall::~SciBall()
{
}

V3 SciBall::mouseOnSphere(const V2& mouseScreenCoords)
{
  return V3();
}

void SciBall::beginDrag(const V2& mouseScreenCoords)
{
}

void SciBall::drag(const V2& mouseScreenCoords)
{
}

void SciBall::endDrag(const V2& mouseScreenCoords)
{
}

M44 SciBall::getTransformation() const
{
  return M44();
}


} // SCIRun
} // SpireSCIRun

