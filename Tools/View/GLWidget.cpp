/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   September 2012

#include <iostream>
#include <QMouseEvent>

#include "GLWidget.h"

// List off the classes we are using.
using Spire::V3;
using Spire::V2;
using Spire::Vector2;
using Spire::M44;

//------------------------------------------------------------------------------
GLWidget::GLWidget(const QGLFormat& format) :
    QGLWidget(format),
    mContext(new GLContext(this))
{
  std::vector<std::string> shaderSearchDirs;
  shaderSearchDirs.push_back("Shaders");

  // Create a threaded spire renderer.
#ifdef SPIRE_USE_STD_THREADS
  mGraphics = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(mContext, shaderSearchDirs, true));
#else
  mGraphics = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(mContext, shaderSearchDirs, false));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);
#endif

  // Setup default camera to look down the negative Z axis.
  V3 eye(0.0f, 0.0f, 5.0f);
  V3 lookAt(0.0f, 0.0f, 0.0f);
  V3 upVec(0.0f, 1.0f, 0.0f);

  // M44::lookAt builds an inverted view matrix that is ready to be multiplied
  // against a projection matrix. For our purposes, we need the *actual* view
  // matrix.
  M44 invCam  = M44::lookAt(eye, lookAt, upVec);
  mCamWorld   = M44::orthoInverse(invCam);
  mGraphics->cameraSetTransform(mCamWorld);

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
void GLWidget::resizeEvent(QResizeEvent *evt)
{
  /// @todo Inform the renderer that screen dimensions have changed.
  //mGraphics.resizeViewport(evt->size());
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mGraphics.reset();
  QGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  // Update the renderer.
  mGraphics->doFrame();
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  Vector2<int> thisPos;
  thisPos.x = event->x();
  thisPos.y = event->y();

  Vector2<int> delta = thisPos - mLastMousePos;

  // Apply this rotation extremelly naively to the camera.
  // divisor is a magic calibration number from pixels to rotation speed.
  const float divisor = 32.0f;
  float rx = static_cast<float>(-delta.y) / divisor;
  float ry = static_cast<float>(delta.x) / divisor;

  M44 tx = M44::rotationX(rx);
  M44 ty = M44::rotationY(ry);

  // x applied first in object space, then y.
  mCamWorld = mCamWorld * ty * tx;

  // Send new camera transform to spire.
  mGraphics->cameraSetTransform(mCamWorld);

  mLastMousePos = thisPos;
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  mLastMousePos.x = event->x();
  mLastMousePos.y = event->y();
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
}



