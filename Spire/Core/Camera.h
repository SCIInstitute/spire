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

#ifndef SPIRE_HIGH_CAMERA_H
#define SPIRE_HIGH_CAMERA_H

#include "Core/Hub.h"

namespace Spire {

/// Basic camera.
class Camera
{
public:
  Camera(Hub& hub);
  virtual ~Camera();

  // V  = View matrix
  // IV = Inverse view matrix
  // P  = Projection matrix
  // m  = multiplication
  const M44& getWorldToProjection() const     {return mPIV;}      // P * IV
  const M44& getWorldToView() const           {return mIV;}       // IV
  const M44& getViewToProjection() const      {return mP;}        // P

  /// \todo Expose settings for perspective and orthographic camera.

  /// Sets this camera to use a perspective projection transformation.
  void setAsPerspective();

  /// Sets this camera to use an orthographic projection transformation.
  void setAsOrthographic(float halfWidth, float halfHeight);

  /// Sets the current view transform (view to world space).
  void setViewTransform(const M44& view);

  /// Sets the current 'eye' position. This modifies the translation column
  /// of the view matrix. Does not change the last row of the matrix.
  void setEyePosition(const V3& eye);

  /// Retrieves the eye position in world space.
  V3 getEyePosition() const                 {return mV.getTranslation();}

  /// Default camera settings
  /// \todo Switch back to constexpr
  /// @{
  static float getDefaultFOVY()   {return 32.0f * (PI / 18.0f);}
  static float getDefaultZNear()  {return 0.1f;}
  static float getDefaultZFar()   {return 1350.0f;}
  /// @}

private:

  M44                   mPIV;         ///< Projection * Inverse View transformation.
  M44                   mIV;          ///< Inverse view transformation.
  M44                   mV;           ///< View matrix.
  M44                   mP;           ///< Projection transformation.
  size_t                mTrafoSeq;    ///< Current sequence of the view transform.
                                      ///< Helps us determine when a camera is 'dirty'.

  bool                  mPerspective; ///< True if we are using a perspective 
                                      ///< transformation. 
  float                 mFOV;         ///< Field of view.
  float                 mZNear;       ///< Position of near plane along view vec.
  float                 mZFar;        ///< Position of far plane along view vec.

  Hub&                  mHub;
};

} // namespace Spire

#endif
