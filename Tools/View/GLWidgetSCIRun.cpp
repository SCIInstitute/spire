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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <QMouseEvent>
#include <QWheelEvent>

#include "GLWidgetSCIRun.h"

using namespace SCIRun::Gui;
using Spire::V4;
using Spire::V3;
using Spire::V2;
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

  buildScene();

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
void GLWidget::buildScene()
{
  std::shared_ptr<Spire::StuInterface> stuPipe = mSpire->getStuPipe();

  // Load asset data from 'exported assets'.
  std::shared_ptr<std::vector<uint8_t>> vbo(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> ibo(new std::vector<uint8_t>());

  std::ifstream fstream("Assets/CappedCylinder.sp", std::ifstream::binary);
  size_t numTriangles = Spire::StuInterface::loadProprietarySR5AssetFile(fstream, *vbo, *ibo);
  fstream.close();

  // loadProprietarySR5ASsetFile always produces positions and normals in the
  // vbo and 16bit index buffers.
  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  Spire::StuInterface::IBO_TYPE iboType = Spire::StuInterface::IBO_16BIT;
  
  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  stuPipe->addVBO(vbo1, vbo, attribNames);
  stuPipe->addIBO(ibo1, ibo, iboType);

  // Add object
  std::string obj1 = "obj1";
  stuPipe->addObject(obj1);

  // Ensure shader is resident.
  std::string shader1 = "UniformColor";
  stuPipe->addPersistentShader(
      shader1, 
      { {"UniformColor.vs", Spire::StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", Spire::StuInterface::FRAGMENT_SHADER},
      });

  // Build the pass
  std::string pass1 = "pass1";
  stuPipe->addPassToObject(obj1, pass1, shader1, vbo1, ibo1, Spire::StuInterface::TRIANGLES);
  stuPipe->addPassUniform(obj1, pass1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));

  // In SCIRun specific renderer, uProjIVworld is already set
  //stuPipe->addGlobalUniform("uProjIVWorld", M44());
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
Spire::SCIRun::SRInterface::MouseButton GLWidget::getSpireButton(QMouseEvent* event)
{
  Spire::SCIRun::SRInterface::MouseButton btn = Spire::SCIRun::SRInterface::MOUSE_NONE;
  if (event->buttons() & Qt::LeftButton)
    btn = Spire::SCIRun::SRInterface::MOUSE_LEFT;
  else if (event->buttons() & Qt::RightButton)
    btn = Spire::SCIRun::SRInterface::MOUSE_RIGHT;
  else if (event->buttons() & Qt::MidButton)
    btn = Spire::SCIRun::SRInterface::MOUSE_MIDDLE;
  
  return btn;
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Extract appropriate key.
  Spire::SCIRun::SRInterface::MouseButton btn = getSpireButton(event);
  mSpire->inputMouseMove(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  Spire::SCIRun::SRInterface::MouseButton btn = getSpireButton(event);
  mSpire->inputMouseDown(glm::ivec2(event->x(), event->y()), btn);
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Spire::SCIRun::SRInterface::MouseButton btn = getSpireButton(event);
  mSpire->inputMouseUp(glm::ivec2(event->x(), event->y()), btn);
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

