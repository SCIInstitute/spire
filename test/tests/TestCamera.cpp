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

#include "GlobalTestEnvironment.h"
#include "TestCamera.h"
#include "TestCommonUniforms.h"

//------------------------------------------------------------------------------
TestCamera::TestCamera() :
    mTrafoSeq(0),
    mPerspective(true),
    mFOV(getDefaultFOVY()),
    mZNear(getDefaultZNear()),
    mZFar(getDefaultZFar())
{
  setAsPerspective();

  // Camera looking down positive Z axis, located at -5.0f z.
  spire::M44 cam;
  cam[3] = spire::V4(0.0f, 0.0f, 5.0f, 1.0f);

  setViewTransform(cam);
}

//------------------------------------------------------------------------------
TestCamera::~TestCamera()
{
}

//------------------------------------------------------------------------------
void TestCamera::setAsPerspective()
{
  mPerspective = true;

  float aspect = static_cast<float>(GlobalTestEnvironment::instance()->getScreenWidth()) / 
                 static_cast<float>(GlobalTestEnvironment::instance()->getScreenHeight());
  mP = glm::perspective(mFOV, aspect, mZNear, mZFar);
}

//------------------------------------------------------------------------------
void TestCamera::setAsOrthographic(float halfWidth, float halfHeight)
{
  mPerspective = false;

	mP = glm::ortho(-halfWidth, halfWidth, 
                  -halfHeight, halfHeight, 
                  mZNear, mZFar);
}

//------------------------------------------------------------------------------
void TestCamera::setViewTransform(const spire::M44& trafo)
{
  ++mTrafoSeq;

  mV    = trafo;
  mIV   = glm::affineInverse(trafo);
  mPIV  = mP * mIV;
}

//------------------------------------------------------------------------------
void TestCamera::setCommonUniforms(std::shared_ptr<spire::Interface> iface)
{
  // Update appropriate uniforms.
  iface->addGlobalUniform(std::get<0>(TestCommonUniforms::getToCameraToProjection()), mPIV);
  iface->addGlobalUniform(std::get<0>(TestCommonUniforms::getToProjection()), mP);
  iface->addGlobalUniform(std::get<0>(TestCommonUniforms::getCameraToWorld()), mV);

  // We project down the negative Z axis. Hence the negation of the camera's
  // 'at' vector.
  iface->addGlobalUniform(std::get<0>(TestCommonUniforms::getCameraViewVec()), -spire::V3(mV[2].xyz()));
  iface->addGlobalUniform(std::get<0>(TestCommonUniforms::getCameraUpVec()), spire::V3(mV[1].xyz()));
}


