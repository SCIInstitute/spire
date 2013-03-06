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

#include "SRCamera.h"
#include "SRCommonUniforms.h"

namespace Spire {
namespace SCIRun {

//------------------------------------------------------------------------------
SRCamera::SRCamera(SRInterface& interface) :
    mTrafoSeq(0),
    mPerspective(true),
    mFOV(getDefaultFOVY()),
    mZNear(getDefaultZNear()),
    mZFar(getDefaultZFar()),
    mInterface(interface)
{
  setAsPerspective();

  // Camera looking down positive Z axis, located at -5.0f z.
  Spire::M44 cam;
  cam.setCol3(Spire::V4(0.0f, 0.0f, -5.0f, 1.0f));
  
  setViewTransform(cam);
}

//------------------------------------------------------------------------------
SRCamera::~SRCamera()
{
}

//------------------------------------------------------------------------------
void SRCamera::setAsPerspective()
{
  mPerspective = true;

  float aspect = static_cast<float>(mInterface.getScreenWidthPixels()) / 
                 static_cast<float>(mInterface.getScreenHeightPixels());
  mP = Spire::M44::perspective(mFOV, aspect, mZNear, mZFar);

  // Rotate about the Y axis by 180 degrees. Many perspective matrices
  // (see Hughes, et al...) are built looking down negative Z. This is the case
  // with our perspective matrices. As such, we rotate by 180 degrees to re-orient
  // our matrix down positive Z.
  Spire::M44 y180 = Spire::M44::rotationY(Spire::PI);
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void SRCamera::setAsOrthographic(float halfWidth, float halfHeight)
{
  mPerspective = false;

	mP = Spire::M44::orthographic(-halfWidth, halfWidth, 
                         -halfHeight, halfHeight, 
                         mZNear, mZFar);

  // Same reason we rotate the perspective camera by 180 degrees.
  Spire::M44 y180 = Spire::M44::rotationY(Spire::PI);
  mP = mP * y180;
}

//------------------------------------------------------------------------------
void SRCamera::setViewTransform(const Spire::M44& trafo)
{
  ++mTrafoSeq;

  mV    = trafo;
  mIV   = Spire::M44::orthoInverse(trafo);
  mPIV  = mP * mIV;

  // Update appropriate uniforms.
  mInterface.getStuPipe()->addGlobalUniform(
      std::get<0>(SRCommonUniforms::getCameraWorldToProjection()), mPIV);

  mInterface.getStuPipe()->addGlobalUniform(
      std::get<0>(SRCommonUniforms::getCameraWorldToView()), mIV);

  mInterface.getStuPipe()->addGlobalUniform(
      std::get<0>(SRCommonUniforms::getCameraProjection()), mP);

  mInterface.getStuPipe()->addGlobalUniform(
      std::get<0>(SRCommonUniforms::getCameraWorld()), mV);
}

} // namespace SCIRun
} // namespace Spire
