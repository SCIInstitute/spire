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

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SCIBALL_H
#define SPIRE_APPSPECIFIC_SCIRUN_SCIBALL_H

#include <cstdint>
#include "Core/Math.h"

namespace Spire { 
namespace SCIRun { 

/// A re-implementation of Ken Shoemake's arcball camera. SCIRun 4's camera
/// system is based completely off of Ken's code. The Code appears in
/// Graphics Gems 4, III.1.
/// 
/// If the screenToWorld parameter to the constructor is left as the identity
/// matrix, then all values are given in screen coordinates.
/// Screen coordinates are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is the
/// center of the screen.
/// \todo Extend this class to include Mouse screen coords -> object space
///       calculations. That way we can rotate around a particular object.
///       May also want to visually represent the sphere when we perform this
///       calculation.
class SciBall
{
public:
  /// \param center         Center of the arcball (in screen coordinates if 
  ///                       screenToWorld = identity). Generally this will 
  ///                       always be (0,0,0). But you may move the center
  ///                       in and out of the screen plane to various effect.
  /// \param radius         If in screen coordinates, a good default is 0.75.
  /// \param screenToWorld  Transformation to transform screen coordinates
  ///                       into the appropriate coordinate system in which
  ///                       'center' and 'radius' are given.
  SciBall(const V3& center, float radius, const M44& screenToWorld = M44());
  virtual ~SciBall();
  
  /// Initiate an arc ball drag given the mouse click in screen coordinates.
  void beginDrag(const V2& mouseScreenCoords);

  /// Dragging the mouse.
  void drag(const V2& mouseScreenCoords);

  /// End the arc ball drag given the ending coordinates.
  void endDrag(const V2& mouseScreenCoords);

  /// Retrieves the current rotation.

private:

  /// Calculates our position on the ArcBall from 2D mouse position.
  /// \param mousePos   Screen coordinates of mouse click. Screen coordinates
  ///                   are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is
  ///                   the center of the screen.
  /// \param
  V3 mouseOnSphere(const V2& mouseScreenCoords);

  V3    mCenter;  /// Center of the arcball in target coordinate system.
  float mRadius;  /// Radius of the arcball in target coordinate system.

  /// Transform from screen coordinates to the target coordinate system.
  M44   mScreenTransform;

};

} // namespace SCIRun
} // namespace Spire

#endif 
