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
using Spire::V4;
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
  mSpire = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(mContext, shaderSearchDirs, true));
#else
  mSpire = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(mContext, shaderSearchDirs, false));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);
#endif

  // Build and bind StuPipe.
  mStuInterface = std::shared_ptr<Spire::StuInterface>(
      new Spire::StuInterface(mSpire));
  mSpire->pipePushBack(mStuInterface);

  buildScene();

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
void GLWidget::buildScene()
{
  // Simple plane -- complex method of transfering to spire.
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f, -5.0f,
     1.0f,  1.0f, -5.0f,
    -1.0f, -1.0f, -5.0f,
     1.0f, -1.0f, -5.0f
  };
  std::vector<std::string> attribNames = {"aPos"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  Spire::StuInterface::IBO_TYPE iboType = Spire::StuInterface::IBO_16BIT;
  
  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  std::copy(rawBegin, rawBegin + rawSize, rawVBO->begin());

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mStuInterface->addVBO(vbo1, rawVBO, attribNames);
  mStuInterface->addIBO(ibo1, rawIBO, iboType);

  // Add object
  std::string obj1 = "obj1";
  mStuInterface->addObject(obj1);

  // Ensure shader is resident.
  std::string shader1 = "UniformColor";
  mStuInterface->addPersistentShader(
      shader1, 
      { {"UniformColor.vs", Spire::StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", Spire::StuInterface::FRAGMENT_SHADER},
      });

  // Build the pass
  std::string pass1 = "pass1";
  mStuInterface->addPassToObject(obj1, pass1, shader1, vbo1, ibo1, Spire::StuInterface::TRIANGLES);

  // Be sure the global uniform 'uProjIVWorld' is set appropriately...
  mStuInterface->addGlobalUniform("uProjIVWorld", M44());
  mStuInterface->addPassUniform(obj1, pass1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));
}

//------------------------------------------------------------------------------
void GLWidget::resizeEvent(QResizeEvent *evt)
{
  /// @todo Inform the renderer that screen dimensions have changed.
  //mSpire.resizeViewport(evt->size());
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mSpire.reset();
  QGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  mContext->makeCurrent();    // Required on windows...
  mSpire->doFrame();
  mContext->swapBuffers();
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
  mSpire->cameraSetTransform(mCamWorld);

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



