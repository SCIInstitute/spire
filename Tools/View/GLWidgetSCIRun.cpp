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

#include "GLWidgetSCIRun.h"

using namespace SCIRun::Gui;
using Spire::Vector2;

//------------------------------------------------------------------------------
GLWidget::GLWidget(const QGLFormat& format) :
    QGLWidget(format),
    mContext(new GLContext(this))
{
  std::vector<std::string> shaderSearchDirs;

  // Create a threaded spire renderer. This should be created at the module
  // level once it has access to the context, should be passed using Transients.
#ifdef SPIRE_USE_STD_THREADS
  mGraphics = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, true));
#else
  mGraphics = std::shared_ptr<Spire::SCIRun::SRInterface>(
      new Spire::SCIRun::SRInterface(
          std::dynamic_pointer_cast<Spire::Context>(mContext),
          shaderSearchDirs, false));
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateRenderer()));
  mTimer->start(35);
#endif

  // Two faces of a cube (the only thing that changes for a cube are the
  // indices). Note: we may be adding normals to the vbo in the future...
  // Actually, since we just want face normals, we'll compute it using
  // the cross product in the vertex shader...
  size_t    vboSize = sizeof(float) * 3 * 8;
  size_t    iboSize = sizeof(uint32_t) * 12;
  float*    vbo = static_cast<float*>(std::malloc(vboSize));
  uint32_t* ibo = static_cast<uint32_t*>(std::malloc(iboSize));

  vbo[0 ] = -1.0f; vbo[1 ] =  1.0f; vbo[2 ] = -1.0f; // 0
  vbo[3 ] =  1.0f; vbo[4 ] =  1.0f; vbo[5 ] = -1.0f; // 1
  vbo[6 ] = -1.0f; vbo[7 ] = -1.0f; vbo[8 ] = -1.0f; // 2
  vbo[9 ] =  1.0f; vbo[10] = -1.0f; vbo[11] = -1.0f; // 3

  vbo[12] = -1.0f; vbo[13] =  1.0f; vbo[14] =  1.0f; // 4
  vbo[15] =  1.0f; vbo[16] =  1.0f; vbo[17] =  1.0f; // 5
  vbo[18] = -1.0f; vbo[19] = -1.0f; vbo[20] =  1.0f; // 6
  vbo[21] =  1.0f; vbo[22] = -1.0f; vbo[23] =  1.0f; // 7

  ibo[0 ] = 0; ibo[1 ] = 1; ibo[2 ] = 2;
  ibo[3 ] = 1; ibo[4 ] = 2; ibo[5 ] = 3;

  ibo[6 ] = 4; ibo[7 ] = 5; ibo[8 ] = 6;
  ibo[9 ] = 5; ibo[10] = 6; ibo[11] = 7;

  mGraphics->renderHACKSetCommonVBO((uint8_t*)(vbo), vboSize);
  mGraphics->renderHACKSetUCFace((uint8_t*)(ibo), iboSize);
  mGraphics->renderHACKSetUCFaceColor(Spire::V4(1.0f, 1.0f, 1.0f, 0.5f));

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
  // Need to inform module that the context is being destroyed.
  mGraphics.reset();
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
  mGraphics->inputMouseMove(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mGraphics->inputMouseDown(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
  /// \todo Include specific button info.
  mGraphics->inputMouseUp(Vector2<int32_t>(event->x(), event->y()));
}

//------------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
  mGraphics->eventResize(static_cast<int32_t>(width),
                         static_cast<int32_t>(height));
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mGraphics->terminate();
  mGraphics.reset();
  //QGLWidget::closeEvent(evt);
}

//------------------------------------------------------------------------------
void GLWidget::updateRenderer()
{
  // Update the renderer.
  mGraphics->doFrame();
}

