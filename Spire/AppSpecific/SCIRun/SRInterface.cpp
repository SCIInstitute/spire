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
/// \date   February 2013

#include "Interface.h"
#include "SRInterface.h"
#include "ArcBall.h"
#include "SRCommonUniforms.h"
#include "SRCamera.h"

namespace Spire {
namespace SCIRun {

//------------------------------------------------------------------------------
SRInterface::SRInterface(std::shared_ptr<Context> context,
                         const std::vector<std::string>& shaderDirs,
                         bool createThread, LogFunction logFP) :
    Interface(context, shaderDirs, createThread, logFP),
    mStuInterface(new StuInterface(*this)),
    mArcBall(new ArcBall),
    mCamera(new SRCamera(*this)),
    mCamDistance(7.0f)
{
  //mCamWorld.setTranslation(V3(0.0f, 0.0f, 5.0f));
  //mArcBall->setUseTranslation(true);
  //mArcBall->setTranslation(mCamWorld);
  mArcBall->setRadius(2.0f);

  // Add stu pipe to the current pipes.
  pipePushBack(mStuInterface);
}

//------------------------------------------------------------------------------
SRInterface::~SRInterface()
{
}

//------------------------------------------------------------------------------
void SRInterface::eventResize(size_t width, size_t height)
{
  mScreenWidth = width;
  mScreenHeight = height; 

  mArcBall->setWindowSize(width, height);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseDown(const Vector2<int32_t>& pos)
{
  /// \todo Only do arc ball if the correct mouse button is down!
  mArcBall->click(pos);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseMove(const Vector2<int32_t>& pos)
{
  /// \todo Change back to old SCI-Run 4.0 camera interaction.

  /// \todo Only do arc ball if the correct mouse button is down!
  Quat q = mArcBall->drag(pos);
  M44 rot = q.computeRotation();
  mCamWorld = mCamWorld * rot;

  mArcBall->click(pos);

  // Make rotation relative to the origin.
  // Super big hack, change when demo is over.
  // Pan is just the distance along the camera's coordinate system's x/y.

  // The y rotation is to re-orient the camera so it is looking down the z axis.
  M44 finalTrafo = mCamWorld * M44::rotationY(PI);
  finalTrafo.setTranslation(mCamWorld.getCol2().xyz() * mCamDistance);

  mCamera->setViewTransform(finalTrafo);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseWheel(int32_t delta)
{
  // Reason why we subtract: Feels more natural to me =/.
  mCamDistance -= static_cast<float>(delta) / 100.0f;

  M44 finalTrafo = mCamWorld * M44::rotationY(PI);
  finalTrafo.setTranslation(mCamWorld.getCol2().xyz() * mCamDistance);

  mCamera->setViewTransform(finalTrafo);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseUp(const Vector2<int32_t>& pos)
{

}

} // namespace SCIRun
} // namespace Spire
