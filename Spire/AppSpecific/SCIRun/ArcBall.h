/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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

// This code was adapted from ImageVis3D code in Tuvok/Basics which
// was itself adapted from NeHe's tutorial 48.

#ifndef SPIRE_APPSPECIFIC_SCIRUN_ARCBALL_H
#define SPIRE_APPSPECIFIC_SCIRUN_ARCBALL_H

#include "Core/Math.h"

namespace Spire {
namespace SCIRun {

class ArcBall
{
public:
  ArcBall(int32_t iWinWidth=0, int32_t iWinHeight=0,
          int32_t iWinOffsetX=0, int32_t iWinOffsetY=0,
          bool bUseTranslation = false);

  void        setRadius(float fRadius)                {mRadius = fRadius;}
  void        setTranslation(const M44& trans)        {mTranslation = trans;}
  const M44&  getTranslation() const                  {return mTranslation;}
  void        setUseTranslation(bool bUseTranslation) {mUseTranslation = bUseTranslation;}
  bool        getUseTranslation()                     {return mUseTranslation;}

  void setWindowSize(int32_t iWinWidth, int32_t iWinHeight);
  void setWindowOffset(int32_t iWinOffsetX, int32_t iWinOffsetY);
  void click(Vector2<int32_t> vPosition);
  Quat drag(Vector2<int32_t> vPosition);

protected:

  static float msEpsilon;

  V3                mStartDrag;       ///< Saved click vector
  Vector2<int32_t>  mWinDim;          ///< window dimensions
  Vector2<int32_t>  mWinOffsets;      ///< Horizontal/Vertical window offset
  float             mRadius;          ///< radius of the ball
  M44               mTranslation;     ///< translation of the ball
  bool              mUseTranslation;  ///< whether to the translation of the ball

  V3 mapToSphere(Vector2<int32_t> vPosition) const;
};

} // namespace Spire
} // namespace SCIRun

#endif // ARCBALL_H
