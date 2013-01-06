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

#include "Common.h"
#include "High/Hub.h"
#include "High/Log.h"
#include "Camera.h"

namespace Spire {

//------------------------------------------------------------------------------
Camera::Camera(std::shared_ptr<Hub> hub) :
    mTrafoSeq(0),
    mPerspective(true),
    mFOV(getDefaultFOVY()),
    mZNear(getDefaultZNear()),
    mZFar(getDefaultZFar()),
    mHub(hub)
{
  setAsPerspective();

  // Setup default camera to look down the negative Z axis.
  Point3 eye(0.0f, 0.0f, 5.0f);
  Point3 lookAt(0.0f, 0.0f, 0.0f);
  V3 upVec(0.0f, 1.0f, 0.0f);

  // M44::lookAt builds an inverted view matrix that is ready to be multiplied
  // against a projection matrix. For our purposes, we need the *actual* view
  // matrix.
  M44 invCam  = M44::lookAt(eye, lookAt, upVec);
  M44 cam     = Vector::orthoInverse(invCam);
  
  setViewTransform(cam);
}

//------------------------------------------------------------------------------
Camera::~Camera()
{
}

//------------------------------------------------------------------------------
void Camera::setAsPerspective()
{
  mPerspective = true;

  float aspect = static_cast<float>(mHub->getActualScreenWidth()) / 
                 static_cast<float>(mHub->getActualScreenHeight());
  mP = M44::perspective(mFOV, aspect, mZNear, mZFar);

  // Rotate about the Y axis by 180 degrees. Many perspective matrices
  // (see Hughes, et al...) are built looking down negative Z. This is the case
  // with our perspective matrices. As such, we rotate by 180 degrees to re-orient
  // our matrix down positive Z.
  M44 y180 = M44::rotationY(PI);
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void Camera::setAsOrthographic(float halfWidth, float halfHeight)
{
  mPerspective = false;

	mP = M44::orthographic(-halfWidth, halfWidth, 
                         -halfHeight, halfHeight, 
                         mZNear, mZFar);

  // Same reason we rotate the perspective camera by 180 degrees.
  M44 y180 = M44::rotationY(PI);
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void Camera::setEyePosition(const V3& eye)
{
  mV.setTranslation(eye);
  setViewTransform(mV);
}

//------------------------------------------------------------------------------
void Camera::setViewTransform(const M44& trafo)
{
  ++mTrafoSeq;

  mV    = trafo;
  mIV   = Vector::orthoInverse(trafo);
  mPIV  = mP * mIV;
}


} // end of namespace Spire
