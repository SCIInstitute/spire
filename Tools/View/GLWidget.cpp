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

#include "Spire/Core/LambdaInterface.h"
#include "Spire/Core/ObjectLambda.h"

using namespace Spire;
using Spire::V4;
using Spire::V3;
using Spire::V2;
using Spire::M44;

// Simple function to handle object transformations so that the GPU does not
// need to do the same calculation for each vertex.
static void lambdaUniformObjTrafs(ObjectLambdaInterface& iface, 
                                  std::list<Interface::UnsatisfiedUniform>& unsatisfiedUniforms)
{
  // Cache object to world transform.
  M44 objToWorld = iface.getObjectMetadata<M44>("objToWorld");
  std::string objectTrafoName = "uObject";
  std::string objectToViewName = "uViewObject";
  std::string objectToCamProjName = "uProjIVObject";

  // Loop through the unsatisfied uniforms and see if we can provide any.
  for (auto it = unsatisfiedUniforms.begin(); it != unsatisfiedUniforms.end(); /*nothing*/ )
  {
    if (it->uniformName == objectTrafoName)
    {
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToViewName)
    {
      // Grab the inverse view transform.
      M44 inverseView = glm::affineInverse(
          iface.getGlobalUniform<M44>("uView"));
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      M44 inverseViewProjection = iface.getGlobalUniform<M44>("uProjIV");
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseViewProjection * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

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

  buildScene();

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

//------------------------------------------------------------------------------
void GLWidget::buildScene()
{
  // Add shader attributes that we will be using.
  mSpire->addShaderAttribute("aPos",         3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aNormal",      3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColor",       4,  true,   sizeof(char) * 4,   Spire::Interface::TYPE_UBYTE);

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
  Spire::Interface::IBO_TYPE iboType = Spire::Interface::IBO_16BIT;
  
  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Add object
  std::string obj1 = "obj1";
  mSpire->addObject(obj1);

  // Ensure shader is resident.
  std::string shader1 = "UniformColor";
  mSpire->addPersistentShader(
      shader1, 
      { {"UniformColor.vsh", Spire::Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Spire::Interface::FRAGMENT_SHADER},
      });

  // Build the pass (default pass).
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Spire::Interface::TRIANGLE_STRIP);

  M44 xform;
  xform[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);
  mSpire->addObjectPassMetadata(obj1, "objToWorld", xform);

  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));

  mSpire->addLambdaObjectUniforms(obj1, lambdaUniformObjTrafs);

  // Setup camera
  M44 proj = glm::perspective(32.0f * (Spire::PI / 180.0f), 3.0f/2.0f, 0.1f, 1350.0f);
  mSpire->addGlobalUniform("uProjIV", proj);
}

//------------------------------------------------------------------------------
void GLWidget::resizeEvent(QResizeEvent*)
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
  mSpire->ntsDoFrame();
  mContext->swapBuffers();
}

//------------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
  glm::ivec2 thisPos;
  thisPos.x = event->x();
  thisPos.y = event->y();

  glm::ivec2 delta = thisPos - mLastMousePos;

  // Apply this rotation extremelly naively to the camera.
  // divisor is a magic calibration number from pixels to rotation speed.
  const float divisor = 32.0f;
  float rx = static_cast<float>(-delta.y) / divisor;
  float ry = static_cast<float>(delta.x) / divisor;

  M44 tx = glm::rotate(M44(), rx, V3(1.0, 0.0, 0.0));
  M44 ty = glm::rotate(M44(), ry, V3(0.0, 1.0, 0.0));

  // x applied first in object space, then y.
  mCamWorld = mCamWorld * ty * tx;

  mLastMousePos = thisPos;
}

//------------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent* event)
{
  mLastMousePos.x = event->x();
  mLastMousePos.y = event->y();
}

//------------------------------------------------------------------------------
void GLWidget::mouseReleaseEvent(QMouseEvent*)
{
}



