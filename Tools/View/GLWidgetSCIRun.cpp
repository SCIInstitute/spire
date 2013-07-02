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

  // Uniform color shader (nothing that interesting).
  std::string uniformColorShader = "UniformColor";
  stuPipe->addPersistentShader(
      uniformColorShader, 
      { {"UniformColor.vsh", Spire::StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Spire::StuInterface::FRAGMENT_SHADER},
      });

  // Directional gouraud shading.
  std::string dirGouraudShader = "DirGouraud";
  stuPipe->addPersistentShader(
      dirGouraudShader, 
      { {"DirGouraud.vsh", Spire::StuInterface::VERTEX_SHADER}, 
        {"DirGouraud.fsh", Spire::StuInterface::FRAGMENT_SHADER},
      });

  auto loadAsset = [stuPipe](const std::string& assetFileName, 
                             const std::string& shader,
                             const std::string& objectName,
                             const std::string& passName)
  {
    // Load asset data from 'exported assets'.
    std::shared_ptr<std::vector<uint8_t>> vbo(new std::vector<uint8_t>());
    std::shared_ptr<std::vector<uint8_t>> ibo(new std::vector<uint8_t>());

    std::ifstream fstream(assetFileName, std::ifstream::binary);
    if (fstream.fail())
      throw std::runtime_error("Unable to open resource file: " + assetFileName);
    size_t numTriangles = Spire::StuInterface::loadProprietarySR5AssetFile(fstream, *vbo, *ibo);
    fstream.close();

    // loadProprietarySR5ASsetFile always produces positions and normals in the
    // vbo and 16bit index buffers.
    std::vector<std::string> attribNames = {"aPos", "aNormal"};
    Spire::StuInterface::IBO_TYPE iboType = Spire::StuInterface::IBO_16BIT;

    // Add necessary VBO's and IBO's
    std::string vbo1 = objectName + "vbo1";
    std::string ibo1 = objectName + "ibo1";
    stuPipe->addVBO(vbo1, vbo, attribNames);
    stuPipe->addIBO(ibo1, ibo, iboType);

    // Add object
    stuPipe->addObject(objectName);

    // Build the pass
    stuPipe->addPassToObject(objectName, passName, shader, vbo1, ibo1, Spire::StuInterface::TRIANGLES);
  };

  // Directional light in world space.
  stuPipe->addGlobalUniform("uLightDirWorld", V3(1.0f, 0.0f, 0.0f));

  /// \todo Method of setting world transform of object.
  // We need a method of setting the world transform of the object and having
  // it concatenated with uProjIVWorld before sending it to the GPU. Maybe a
  // specialized uniform? Since this is such a specialized case, maybe we can
  // just set the Object -> to world transform and have it be an attribute of
  // the pass?

  // Cylinder
  {
    std::string objName = "cylinder";
    std::string passName = "cylinderPass";

    loadAsset("Assets/CappedCylinder.sp", uniformColorShader, objName, passName);

    stuPipe->addPassUniform(objName, passName, "uColor", V4(0.74f, 0.0f, 0.0f, 1.0f));

    M44 xform;
    xform[3] = V4(1.0f, 0.0f, 0.0f, 1.0f);
    stuPipe->addObjectTransform(objName, xform);
  }

  // Sphere
  {
    std::string objName = "sphere";
    std::string passName = "spherePass";

    loadAsset("Assets/Sphere.sp", dirGouraudShader, objName, passName);

    stuPipe->addPassUniform(objName, passName, "uAmbientColor", V4(0.1f, 0.1f, 0.1f, 1.0f));
    stuPipe->addPassUniform(objName, passName, "uDiffuseColor", V4(0.8f, 0.8f, 0.0f, 1.0f));
    stuPipe->addPassUniform(objName, passName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
    stuPipe->addPassUniform(objName, passName, "uSpecularPower", 32.0f);

    M44 xform;
    xform[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);
    stuPipe->addObjectTransform(objName, xform);
  }

  // UnCapped cylinder
  {
    std::string objName = "uncylinder";
    std::string passName = "uncylinderPass";

    loadAsset("Assets/UncappedCylinder.sp", uniformColorShader, objName, passName);

    stuPipe->addPassUniform(objName, passName, "uColor", V4(0.0f, 0.0f, 0.74f, 1.0f));

    M44 xform;
    xform[3] = V4(-1.0f, 0.0f, 0.0f, 1.0f);
    stuPipe->addObjectTransform(objName, xform);
  }
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

