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

#include "TestCamera.h"
#include "GlobalTestEnvironment.h"
#include "AppSpecific/SCIRun/SRCommonUniforms.h"

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
  Spire::M44 cam;
  cam[3] = Spire::V4(0.0f, 0.0f, -5.0f, 1.0f);

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

  float aspect = static_cast<float>(Spire::GlobalTestEnvironment::instance()->getScreenWidth()) / 
                 static_cast<float>(Spire::GlobalTestEnvironment::instance()->getScreenHeight());
  mP = glm::perspective(mFOV, aspect, mZNear, mZFar);

  // Rotate about the Y axis by 180 degrees. Many perspective matrices
  // (see Hughes, et al...) are built looking down negative Z. This is the case
  // with our perspective matrices. As such, we rotate by 180 degrees to re-orient
  // our matrix down positive Z.
  Spire::M44 y180 = glm::rotate(Spire::M44(), Spire::PI, Spire::V3(0.0, 1.0, 0.0));
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void TestCamera::setAsOrthographic(float halfWidth, float halfHeight)
{
  mPerspective = false;

	mP = glm::ortho(-halfWidth, halfWidth, 
                  -halfHeight, halfHeight, 
                  mZNear, mZFar);

  // Same reason we rotate the perspective camera by 180 degrees.
  Spire::M44 y180 = glm::rotate(Spire::M44(), Spire::PI, Spire::V3(0.0, 1.0, 0.0));
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void TestCamera::setViewTransform(const Spire::M44& trafo)
{
  ++mTrafoSeq;

  mV    = trafo;
  mIV   = glm::affineInverse(trafo);
  mPIV  = mP * mIV;
}

//------------------------------------------------------------------------------
void TestCamera::setSRCommonUniforms(std::shared_ptr<Spire::Interface> iface)
{
  // Update appropriate uniforms.
  iface->addGlobalUniform(std::get<0>(Spire::SRCommonUniforms::getToCameraToProjection()), mPIV);
  iface->addGlobalUniform(std::get<0>(Spire::SRCommonUniforms::getToProjection()), mP);
  iface->addGlobalUniform(std::get<0>(Spire::SRCommonUniforms::getCameraToWorld()), mV);

  // We've modified our projection transform so that the positive Z axis is the
  // axis which our camera will be looking down.
  iface->addGlobalUniform(std::get<0>(Spire::SRCommonUniforms::getCameraViewVec()), Spire::V3(mV[2].xyz()));
  iface->addGlobalUniform(std::get<0>(Spire::SRCommonUniforms::getCameraUpVec()), Spire::V3(mV[1].xyz()));
}

