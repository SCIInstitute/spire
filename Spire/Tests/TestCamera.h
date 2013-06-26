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
/// \date   March 2013

#ifndef SPIRE_TESTS_TESTCAMERA_H
#define SPIRE_TESTS_TESTCAMERA_H

// Include spire interface so that we have access to the mathematical types.
#include "Spire/Common.h"

/// Basic camera. Pretty much torn straight from Spire when it had a camera
/// class.
class TestCamera
{
public:
  TestCamera();
  virtual ~TestCamera();

  // V  = View matrix
  // IV = Inverse view matrix
  // P  = Projection matrix
  // m  = multiplication
  const Spire::M44& getWorldToProjection() const     {return mPIV;}      // P * IV
  const Spire::M44& getWorldToView() const           {return mIV;}       // IV
  const Spire::M44& getViewToProjection() const      {return mP;}        // P

  /// Sets this camera to use a perspective projection transformation.
  void setAsPerspective();

  /// Sets this camera to use an orthographic projection transformation.
  void setAsOrthographic(float halfWidth, float halfHeight);

  /// Sets the current view transform (view to world space).
  void setViewTransform(const Spire::M44& view);

  /// Default camera settings
  /// @{
  static float getDefaultFOVY()   {return 32.0f * (Spire::PI / 180.0f);}
  static float getDefaultZNear()  {return 0.1f;}
  static float getDefaultZFar()   {return 1350.0f;}
  /// @}

private:

  Spire::M44            mPIV;         ///< Projection * Inverse View transformation.
  Spire::M44            mIV;          ///< Inverse view transformation.
  Spire::M44            mV;           ///< View matrix.
  Spire::M44            mP;           ///< Projection transformation.
  size_t                mTrafoSeq;    ///< Current sequence of the view transform.
                                      ///< Helps us determine when a camera is 'dirty'.

  bool                  mPerspective; ///< True if we are using a perspective 
                                      ///< transformation. 
  float                 mFOV;         ///< Field of view.
  float                 mZNear;       ///< Position of near plane along view vec.
  float                 mZFar;        ///< Position of far plane along view vec.
};

#endif // SPIRE_HIGH_CAMERA_H
