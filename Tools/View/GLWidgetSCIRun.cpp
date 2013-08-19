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
#include "Spire/AppSpecific/SCIRun/SRUtil.h"

using namespace SCIRun::Gui;
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
  M44 objToWorld = iface.getObjectMetadata<M44>(
      std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()));

  std::string objectTrafoName = std::get<0>(SRCommonUniforms::getObject());
  std::string objectToViewName = std::get<0>(SRCommonUniforms::getObjectToView());
  std::string objectToCamProjName = std::get<0>(SRCommonUniforms::getObjectToCameraToProjection());

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
          iface.getGlobalUniform<M44>(std::get<0>(SRCommonUniforms::getCameraToWorld())));
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      M44 inverseViewProjection = iface.getGlobalUniform<M44>(
          std::get<0>(SRCommonUniforms::getToCameraToProjection()));
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
  bool buildNormals = true;

  // Add shader attributes that we will be using.
  mSpire->addShaderAttribute("aPos",         3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aNormal",      3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  Spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColor",       4,  true,   sizeof(char) * 4,   Spire::Interface::TYPE_UBYTE);

  // Uniform color shader (nothing that interesting).
  std::string uniformColorShader = "UniformColor";
  mSpire->addPersistentShader(
      uniformColorShader, 
      { std::make_tuple("UniformColor.vsh", Spire::Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Spire::Interface::FRAGMENT_SHADER),
      });

  // Directional gouraud shading.
  std::string dirGouraudShader = "DirGouraud";
  mSpire->addPersistentShader(
      dirGouraudShader, 
      { std::make_tuple("DirGouraud.vsh", Spire::Interface::VERTEX_SHADER), 
        std::make_tuple("DirGouraud.fsh", Spire::Interface::FRAGMENT_SHADER),
      });

  // Directional phong shading.
  std::string dirPhongSphere = "DirPhong";
  mSpire->addPersistentShader(
      dirPhongSphere , 
      { std::make_tuple("DirPhong.vsh", Spire::Interface::VERTEX_SHADER), 
        std::make_tuple("DirPhong.fsh", Spire::Interface::FRAGMENT_SHADER),
      });

  // Varying color shader
  std::string attribColorShader = "AttribColor";
  mSpire->addPersistentShader(
      attribColorShader, 
      { std::make_tuple("Color.vsh", Spire::Interface::VERTEX_SHADER), 
        std::make_tuple("Color.fsh", Spire::Interface::FRAGMENT_SHADER),
      });

  // This load asset function operates only on the default pass, since optional
  // arguments are NOT allowed in lambdas.
  auto loadAsset = [this, buildNormals, uniformColorShader](const std::string& assetFileName, 
                                        const std::string& shader,
                                        const std::string& objectName,
                                        const M44& xform)
  {
    // Load asset data from 'exported assets'.
    std::shared_ptr<std::vector<uint8_t>> vbo(new std::vector<uint8_t>());
    std::shared_ptr<std::vector<uint8_t>> ibo(new std::vector<uint8_t>());

    std::ifstream fstream(assetFileName, std::ifstream::binary);
    if (fstream.fail())
      throw std::runtime_error("Unable to open resource file: " + assetFileName);
    // The number of triangles is returned from loadProprietarySR5AssetFile.
    Spire::Interface::loadProprietarySR5AssetFile(fstream, *vbo, *ibo);
    fstream.close();

    // loadProprietarySR5AssetFile always produces positions and normals in the
    // vbo and 16bit index buffers.
    std::vector<std::string> attribNames = {"aPos", "aNormal"};
    Spire::Interface::IBO_TYPE iboType = Spire::Interface::IBO_16BIT;

    // Add necessary VBO's and IBO's
    std::string vbo1 = objectName + "vbo1";
    std::string ibo1 = objectName + "ibo1";
    mSpire->addVBO(vbo1, vbo, attribNames);
    mSpire->addIBO(ibo1, ibo, iboType);

    mSpire->addObject(objectName);
    mSpire->addPassToObject(objectName, shader, vbo1, ibo1, Spire::Interface::TRIANGLES);
    mSpire->addLambdaObjectUniforms(objectName, lambdaUniformObjTrafs);

    // Apply world transformation.
    mSpire->addObjectPassMetadata(
        objectName, std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()), xform);

    if (buildNormals)
    {
      // Construct a subpass of the default pass that will display the normals
      // associated with the object.
      std::shared_ptr<std::vector<uint8_t>> vboNormals(new std::vector<uint8_t>());
      std::shared_ptr<std::vector<uint8_t>> iboNormals(new std::vector<uint8_t>());
      Spire::SCIRun::buildNormalRenderingForVBO(vbo, 6 * sizeof(float), 0.5f, *vboNormals,
                                                *iboNormals, 0, 3 * sizeof(float));

      std::string normalsPassName= "normals pass";
      std::vector<std::string> normalAttribNames = {"aPos"};
      std::string normalVBO1 = objectName + "normal_vbo1";
      std::string normalIBO1 = objectName + "normal_ibo1";
      mSpire->addVBO(normalVBO1, vboNormals, normalAttribNames);
      mSpire->addIBO(normalIBO1, iboNormals, iboType);
      mSpire->addPassToObject(objectName, uniformColorShader, normalVBO1, normalIBO1, 
                              Spire::Interface::LINES, normalsPassName, SPIRE_DEFAULT_PASS);
      mSpire->addLambdaObjectUniforms(objectName, lambdaUniformObjTrafs, normalsPassName);

      mSpire->addObjectPassUniform(objectName, "uColor", V4(0.74f, 0.0f, 0.0f, 1.0f), 
                                   normalsPassName);
      mSpire->addObjectPassMetadata(
          objectName, std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()),
          xform, normalsPassName);
    }
  };

  // Directional light in world space.
  mSpire->addGlobalUniform("uLightDirWorld", V3(1.0f, 0.0f, 0.0f));

  // Cylinder
  {
    std::string objName = "cylinder";

    M44 xform;
    xform[3] = V4(-2.0f, 0.0f, 0.0f, 1.0f);

    loadAsset("Assets/CappedCylinder.sp", uniformColorShader, objName, xform);
    mSpire->addObjectPassUniform(objName, "uColor", V4(0.74f, 0.0f, 0.0f, 1.0f));
  }

  // Gouraud Sphere
  {
    std::string objName = "sphere";

    M44 xform;
    xform[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);

    loadAsset("Assets/Sphere.sp", dirGouraudShader, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(1.0f, 1.0f, 1.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 32.0f);
  }

  // Gouraud Sphere
  {
    std::string objName = "sphere2";

    M44 xform;
    xform[3] = V4(0.0f, -1.0f, 0.0f, 1.0f);

    loadAsset("Assets/Sphere.sp", dirGouraudShader, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.0f, 0.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 32.0f);
  }

  // Phong Sphere
  {
    std::string objName = "phongSphere";

    M44 xform;
    xform[3] = V4(0.0f, 0.0f, 1.0f, 1.0f);

    loadAsset("Assets/Sphere.sp", dirPhongSphere, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 16.0f);
  }

  // Phong Sphere
  {
    std::string objName = "phongSphere2";

    M44 xform;
    xform[3] = V4(0.0f, -1.0f, 1.0f, 1.0f);

    loadAsset("Assets/Sphere.sp", dirPhongSphere, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.0f, 0.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 16.0f);
  }

  // UnCapped cylinder
  {
    std::string objName = "uncylinder";

    M44 xform;
    xform[3] = V4(-1.0f, 0.0f, 0.0f, 1.0f);

    loadAsset("Assets/UncappedCylinder.sp", uniformColorShader, objName, xform);

    mSpire->addObjectPassUniform(objName, "uColor", V4(0.0f, 0.0f, 0.74f, 1.0f));
  }

  // Coordinate axes.
  // UnitArrow asset is pointing down the Z axis with it's origin at the center
  // of object space.

  V3 coordinateAxesCenter(3.0f, 3.0f, 3.0f);

  // x-axis.
  {
    std::string objName = "xAxis";

    // Rotate by positive 90 degrees about y axis to get arrow pointing down xAxis.
    M44 xform = glm::rotate(M44(), Spire::PI / 2.0f, V3(0.0, 1.0, 0.0));
    xform[3] = V4(coordinateAxesCenter, 1.0f);

    loadAsset("Assets/UnitArrow.sp", dirPhongSphere, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.5f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(1.0f, 0.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 16.0f);
  }

  // y-axis.
  {
    std::string objName = "yAxis";

    // Rotate by positive 90 about x axis to get arrow pointing down yAxis.
    M44 xform = glm::rotate(M44(), -Spire::PI / 2.0f, V3(1.0, 0.0, 0.0));
    xform[3] = V4(coordinateAxesCenter, 1.0f);

    loadAsset("Assets/UnitArrow.sp", dirPhongSphere, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.5f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 1.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 16.0f);
  }

  // z-axis.
  {
    std::string objName = "zAxis";

    // Don't rotate at all, UnitArrow is initially pointing down the z axis.
    M44 xform;
    xform[3] = V4(coordinateAxesCenter, 1.0f);

    loadAsset("Assets/UnitArrow.sp", dirPhongSphere, objName, xform);

    mSpire->addObjectPassUniform(objName, "uAmbientColor", V4(0.01f, 0.01f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uDiffuseColor", V4(0.0f, 0.0f, 1.0f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(objName, "uSpecularPower", 16.0f);
  }


  coordinateAxesCenter = V3(3.0f, 3.0f, -3.0f);
  // Line based coord axes
  {
    std::string objName = "lineCoordAxes";

    // Build 3 lines down each of the coordinate axes and color them
    // appropriately.
    std::vector<float> vboData =
    {
      // X Vector
      0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f, 1.0f,

      // Y Vector
      0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 1.0f,

      // Z Vector
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f
    };
    std::vector<std::string> attribNames = {"aPos", "aColorFloat"};

    std::vector<uint16_t> iboData =
    {
      0, 1,
      2, 3,
      4, 5
    };
    Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

    // This is pretty contorted interface due to the marshalling between
    // std::vector<float> and std::vector<uint8_t>.
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
    std::string vbo1 = "lineCoordAxesVbo";
    std::string ibo1 = "lineCoordAxesIbo";
    mSpire->addVBO(vbo1, rawVBO, attribNames);
    mSpire->addIBO(ibo1, rawIBO, iboType);

    mSpire->addObject(objName);
    mSpire->addPassToObject(objName, attribColorShader, vbo1, ibo1, Interface::LINES);

    mSpire->addLambdaObjectUniforms(objName, lambdaUniformObjTrafs);

    // Don't rotate at all, UnitArrow is initially pointing down the z axis.
    M44 xform;
    xform[3] = V4(coordinateAxesCenter, 1.0f);
    mSpire->addObjectPassMetadata(
        objName, std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()), xform);
  }

  // Another version of the coordinate axes...

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
  mSpire->eventResize(static_cast<size_t>(width),
                      static_cast<size_t>(height));
}

//------------------------------------------------------------------------------
void GLWidget::closeEvent(QCloseEvent* /*evt*/)
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
  mSpire->ntsDoFrame();
  mContext->swapBuffers();
}

