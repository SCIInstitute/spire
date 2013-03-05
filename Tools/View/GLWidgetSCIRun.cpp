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
/// \brief  Not sure this file should go in Modules/Render. But it is an 
///         auxiliary file to the ViewScene render module.

#include <cstdlib>
#include <QMouseEvent>
#include <QWheelEvent>

#include "GLWidgetSCIRun.h"

using namespace SCIRun::Gui;
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

  // Create a threaded spire renderer. This should be created at the module
  // level once it has access to the context, should be passed using Transients.
#ifdef SPIRE_USE_STD_THREADS
  mSpire = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, true));
#else
  mSpire = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, false));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);
#endif

  // Build and bind StuPipe.
  mStuInterface = std::shared_ptr<Spire::StuInterface>(
      new Spire::StuInterface(*mSpire.get()));
  mSpire->pipePushBack(mStuInterface);

  buildScene();

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
void GLWidget::buildScene()
{
  // Two faces of a cube (the only thing that changes for a cube are the
  // indices). Note: we may be adding normals to the vbo in the future...
  // Actually, since we just want face normals, we'll compute it using
  // the cross product in the vertex shader...
  size_t    vboSize = sizeof(float) * 3 * 8;
  size_t    iboSize = sizeof(uint32_t) * 12;
  float*    vbo = static_cast<float*>(std::malloc(vboSize));
  uint32_t* ibo = static_cast<uint32_t*>(std::malloc(iboSize));

  std::vector<float> vboData =
  {
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
  };
  std::vector<std::string> attribNames = {"aPos"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 
    1, 2, 3,

    4, 5, 6,
    5, 6, 7
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
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  mSpire.reset();
}

//------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
  /// \todo Implement this intelligently. This function is called everytime
  ///       there is a new graphics context.
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mSpire->inputMouseMove(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mSpire->inputMouseDown(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mSpire->inputMouseUp(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::wheelEvent(QWheelEvent * event)
{
  mSpire->inputMouseWheel(event->delta());
}

//------------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
  mSpire->eventResize(static_cast<int32_t>(width),
                         static_cast<int32_t>(height));
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mSpire->terminate();
  mSpire.reset();
  //QGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  mContext->makeCurrent();    // Required on windows...
  mSpire->doFrame();
  mContext->swapBuffers();
}

