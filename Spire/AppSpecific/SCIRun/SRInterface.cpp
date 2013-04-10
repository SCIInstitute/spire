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
#include "SciBall.h"
#include "SRCommonUniforms.h"
#include "SRCamera.h"

#include "Core/Hub.h"
#include "Core/InterfaceImplementation.h"

using namespace std::placeholders;

namespace Spire {
namespace SCIRun {

//------------------------------------------------------------------------------
SRInterface::SRInterface(std::shared_ptr<Context> context,
                         const std::vector<std::string>& shaderDirs,
                         bool createThread, LogFunction logFP) :
    Interface(context, shaderDirs, createThread, logFP),
    mStuInterface(new StuInterface(*this)),
    mSciBall(new SciBall(V3(0.0f, 0.0f, 0.0f), 1.0f)),
    mCamera(new SRCamera(*this)),
    mCamDistance(7.0f),
    mScreenWidth(640),
    mScreenHeight(480)
{
  // Add stu pipe to the current pipes.
  pipePushBack(mStuInterface);

  // Default camera (to be done AFTER stu pipe is added).
  M44 camRot = mSciBall->getTransformation();
  M44 finalTrafo = camRot * M44::rotationY(PI);
  finalTrafo.setTranslation(camRot.getCol2().xyz() * mCamDistance);
  mCamera->setViewTransform(finalTrafo);
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

  // Ensure glViewport is called appropriately.
  Hub::RemoteFunction resizeFun =
      std::bind(InterfaceImplementation::resize, _1, width, height);
  mHub->addFunctionToThreadQueue(resizeFun);
}

//------------------------------------------------------------------------------
V2 SRInterface::calculateScreenSpaceCoords(const Vector2<int32_t>& mousePos)
{
  float windowOriginX = 0.0f;
  float windowOriginY = 0.0f;

  // Transform incoming mouse coordinates into screen space.
  V2 mouseScreenSpace;
  mouseScreenSpace.x = 2.0f * (static_cast<float>(mousePos.x) - windowOriginX) 
      / static_cast<float>(mScreenWidth) - 1.0f;
  mouseScreenSpace.y = 2.0f * (static_cast<float>(mousePos.y) - windowOriginY)
      / static_cast<float>(mScreenHeight) - 1.0f;

  // Rotation with flipped axes feels much more natural.
  mouseScreenSpace.x = -mouseScreenSpace.x;

  return mouseScreenSpace;
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseDown(const Vector2<int32_t>& pos, MouseButton btn)
{
  if (btn == MOUSE_LEFT)
  {
    V2 mouseScreenSpace = calculateScreenSpaceCoords(pos);
    mSciBall->beginDrag(mouseScreenSpace);
  }
  else if (btn == MOUSE_RIGHT)
  {
    // Store translation starting position.

  }
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseMove(const Vector2<int32_t>& pos, MouseButton btn)
{
  if (btn == MOUSE_LEFT)
  {
    V2 mouseScreenSpace = calculateScreenSpaceCoords(pos);

    mSciBall->drag(mouseScreenSpace);
    M44 camRot = mSciBall->getTransformation();

    // Reorient camera down the Z axis.
    M44 finalTrafo = camRot * M44::rotationY(PI);

    // Add camera dolly.
    finalTrafo.setTranslation(camRot.getCol2().xyz() * mCamDistance);

    mCamera->setViewTransform(finalTrafo);
  }
  else if (btn == MOUSE_RIGHT)
  {
    /// \todo Perform translation by delta.
  }
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseWheel(int32_t delta)
{
  // Reason why we subtract: Feels more natural to me =/.
  mCamDistance -= static_cast<float>(delta) / 100.0f;

  M44 camRot = mSciBall->getTransformation();
  M44 finalTrafo = camRot * M44::rotationY(PI);
  finalTrafo.setTranslation(camRot.getCol2().xyz() * mCamDistance);

  mCamera->setViewTransform(finalTrafo);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseUp(const Vector2<int32_t>& pos, MouseButton btn)
{
  V2 mouseScreenSpace = calculateScreenSpaceCoords(pos);
  mSciBall->endDrag(mouseScreenSpace);
}

} // namespace SCIRun
} // namespace Spire
