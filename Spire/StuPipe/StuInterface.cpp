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

#include <functional>
#include <utility>

#include "Common.h"
#include "StuInterface.h"
#include "StuObject.h"
#include "Core/ShaderProgramMan.h"
#include "Core/Hub.h"
#include "Core/CommonUniforms.h"
#include "../Exceptions.h"

using namespace std::placeholders;

namespace Spire {

// Simple static function to convert from PRIMITIVE_TYPES to GL types.
// Not part of the class due to the return type (interface class should have
// nothing GL specific in them).
GLenum getGLPrimitive(StuInterface::PRIMITIVE_TYPES type);


//------------------------------------------------------------------------------
StuInterface::StuInterface(Interface& iface) :
    PipeInterface(iface),
    mCurrentRenderOrder(0),
    mCurrentPassOrder(0)
{
  addPassToBack(SPIRE_DEFAULT_PASS);
}

//------------------------------------------------------------------------------
StuInterface::~StuInterface()
{
}

//------------------------------------------------------------------------------
void StuInterface::ntsInitOnRenderThread()
{
}

//------------------------------------------------------------------------------
void StuInterface::ntsDoAllPasses()
{
  /// \todo Call all passes begin lambdas. Used primarily to setup global
  /// uniforms.

  /// \todo Move this outside of the interface!
  GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  GL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  /// \todo Make line width a part of the GPU state.
  glLineWidth(2.0f);
  //glEnable(GL_LINE_SMOOTH);

  GPUState defaultGPUState;
  mHub.getGPUStateManager().apply(defaultGPUState, true); // true = force application of state.


  // Loop through all of the passes.
  for (auto it = mPasses.begin(); it != mPasses.end(); ++it)
  {
    ntsDoPass((*it)->mName);
  }

  /// \todo Call all passes end lambdas. Used primarily to setup global
  /// uniforms.
}

//------------------------------------------------------------------------------
void StuInterface::ntsDoPass(const std::string& passName)
{
  std::shared_ptr<Pass> pass = mNameToPass.at(passName);

  // Loop over all objects in the pass and render them.
  /// \todo Need to add some way of ordering the rendered objects, whether it be
  /// by another structure built into Spire (not for this at all), or some lambda
  /// callback.
  for (auto it = pass->mNameToObject.begin(); it != pass->mNameToObject.end(); ++it)
  {
    it->second->renderPass(passName);
  }

  ///\todo Call pass begin lambdas. Setup global pass specific uniforms.

  //// Pull the view and the inverse view projection transforms out of the
  //// uniforms. StuInterface expects those to be set.
  //std::shared_ptr<const AbstractUniformStateItem> camToWorldUniform = 
  //    mHub.getShaderUniformStateMan().getGlobalUninform(
  //        std::get<0>(CommonUniforms::getCameraToWorld()));

  //if (camToWorldUniform->getGLType() != UNIFORM_FLOAT_MAT4)
  //{
  //  throw std::runtime_error("-> Camera -> Projection transform is not a 4x4 matrix!");
  //}
  //const float* rawM44 = camToWorldUniform->getRawData();
  //M44 camToWorld = glm::make_mat4x4(rawM44);
  //M44 inverseCamToWorld = glm::affineInverse(camToWorld);

  //std::shared_ptr<const AbstractUniformStateItem> toCamToProjectionUniform =
  //    mHub.getShaderUniformStateMan().getGlobalUninform(
  //        std::get<0>(CommonUniforms::getToCameraToProjection()));

  //if (toCamToProjectionUniform->getGLType() != UNIFORM_FLOAT_MAT4)
  //{
  //  throw std::runtime_error("-> Camera -> Projection transform is not a 4x4 matrix!");
  //}
  //rawM44 = toCamToProjectionUniform->getRawData();
  //M44 toCamToProjection = glm::make_mat4x4(rawM44);

  //for (auto it = mRenderOrderToObjects.begin(); it != mRenderOrderToObjects.end(); ++it)
  //{
  //  //it->second->renderAllPasses();
  //  it->second->renderPass(passName);
  //}

  ///\todo Call pass end lambda.
}

//------------------------------------------------------------------------------
void StuInterface::addPassToFront(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(addPassToBackImpl, _1, this, passName);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addPassToBack(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(addPassToBackImpl, _1, this, passName);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addPassToBackImpl(Hub& hub, StuInterface* iface, std::string passName)
{
  if (iface->ntsHasPass(passName) == true)
    throw std::runtime_error("Pass (" + passName + ") already exists!");

  std::shared_ptr<Pass> pass(new Pass(passName));
  iface->mPasses.push_front(pass);
  iface->mNameToPass[passName] = pass;
}

//------------------------------------------------------------------------------
void StuInterface::addPassToFrontImpl(Hub& hub, StuInterface* iface, std::string passName)
{
  // Verify that there is no pass by that name already.
  if (iface->ntsHasPass(passName) == true)
    throw std::runtime_error("Pass (" + passName + ") already exists!");

  std::shared_ptr<Pass> pass(new Pass(passName));
  iface->mPasses.push_back(pass);
  iface->mNameToPass[passName] = pass;
}

//------------------------------------------------------------------------------
bool StuInterface::ntsHasPass(const std::string& pass) const
{
  return (mNameToPass.find(pass) != mNameToPass.end());
}

//------------------------------------------------------------------------------
bool StuInterface::ntsIsObjectInPass(const std::string& object, const std::string& pass) const
{
  if (ntsHasPass(pass))
  {
    std::shared_ptr<Pass> passPtr = mNameToPass.at(pass);
    return (passPtr->mNameToObject.find(object) != passPtr->mNameToObject.end());
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
void StuInterface::addIBOImpl(Hub& hub, StuInterface* iface,
                              std::string iboName,
                              std::shared_ptr<std::vector<uint8_t>> iboData,
                              StuInterface::IBO_TYPE type)
{
  if (iface->mIBOMap.find(iboName) != iface->mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object.");

  iface->mIBOMap.insert(std::make_pair(
          iboName, std::shared_ptr<IBOObject>(new IBOObject(iboData, type))));
}


//------------------------------------------------------------------------------
void StuInterface::addIBO(const std::string& name,
                          std::shared_ptr<std::vector<uint8_t>> iboData,
                          IBO_TYPE type)
{
  Hub::RemoteFunction fun =
      std::bind(addIBOImpl, _1, this, name, iboData, type);
  mHub.addFunctionToThreadQueue(fun);
}




//------------------------------------------------------------------------------
void StuInterface::addVBOImpl(Hub& hub, StuInterface* iface,
                              std::string vboName,
                              std::shared_ptr<std::vector<uint8_t>> vboData,
                              std::vector<std::string> attribNames)
{
  if (iface->mVBOMap.find(vboName) != iface->mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object.");

  iface->mVBOMap.insert(std::make_pair(
          vboName, std::shared_ptr<VBOObject>(new VBOObject(vboData, attribNames, hub.getShaderAttributeManager()))));
}


//------------------------------------------------------------------------------
void StuInterface::addVBO(const std::string& name,
                          std::shared_ptr<std::vector<uint8_t>> vboData,
                          const std::vector<std::string>& attribNames)
{
  Hub::RemoteFunction fun =
      std::bind(addVBOImpl, _1, this, name, vboData, attribNames);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::removeVBOImpl(Hub& hub, StuInterface* iface,
                                 std::string vboName)
{
  size_t numElementsRemoved = iface->mVBOMap.erase(vboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find VBO to remove.");
}

//------------------------------------------------------------------------------
void StuInterface::removeVBO(const std::string& vboName)
{
  Hub::RemoteFunction fun =
      std::bind(removeVBOImpl, _1, this, vboName);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::removeIBOImpl(Hub& hub, StuInterface* iface,
                                 std::string iboName)
{
  size_t numElementsRemoved = iface->mIBOMap.erase(iboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find IBO to remove.");
}

//------------------------------------------------------------------------------
void StuInterface::removeIBO(const std::string& iboName)
{
  Hub::RemoteFunction fun =
      std::bind(removeIBOImpl, _1, this, iboName);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
size_t StuInterface::loadProprietarySR5AssetFile(std::istream& stream,
                                               std::vector<uint8_t>& vbo,
                                               std::vector<uint8_t>& ibo)
{
  // Read default SCIRun asset header.
  std::string header = "SCR5";

  char headerStrIn_Raw[5];
  stream.read(headerStrIn_Raw, 4);
  headerStrIn_Raw[4] = '\0';
  std::string headerStrIn = headerStrIn_Raw;

  if (headerStrIn != header)
  {
    /// \todo Use more appropriate I/O exception.
    throw std::invalid_argument("Header does not match asset file.");
  }

  // Read in the number of meshes (this is ignored for now and the first mesh is
  // used).
  uint32_t numMeshes = 0;
  stream.read(reinterpret_cast<char*>(&numMeshes), sizeof(uint32_t));
  if (numMeshes == 0)
  {
    throw std::invalid_argument("Need at least one mesh in asset file.");
  }

  // Read in the first mesh (this is the only mesh we will read in)
  uint32_t numVertices = 0;
  stream.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));

  V3 position;
  V3 normal;

  // Reserve appropriate space in the vbo.
  size_t vboSize = sizeof(float) * 6 * numVertices;
  vbo.resize(vboSize); // linear complexity.
  float* vboPtr = reinterpret_cast<float*>(&vbo[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  for (size_t i = 0; i < numVertices; i++)
  {
    // Read position data
    stream.read(reinterpret_cast<char*>(&position.x), sizeof(float));
    stream.read(reinterpret_cast<char*>(&position.y), sizeof(float));
    stream.read(reinterpret_cast<char*>(&position.z), sizeof(float));
    vboPtr[0] = position.x;
    vboPtr[1] = position.y;
    vboPtr[2] = position.z;
    vboPtr += 3;

    // Read normal data
    stream.read(reinterpret_cast<char*>(&normal.x), sizeof(float));
    stream.read(reinterpret_cast<char*>(&normal.y), sizeof(float));
    stream.read(reinterpret_cast<char*>(&normal.z), sizeof(float));
    vboPtr[0] = normal.x;
    vboPtr[1] = normal.y;
    vboPtr[2] = normal.z;
    vboPtr += 3;
  }

  // Read in the IBO data.
  uint32_t numTriangles = 0;  // Will be counted when loading.
  uint32_t numFaces = 0;
  stream.read(reinterpret_cast<char*>(&numFaces), sizeof(uint32_t));

  // Worst case number of triangles: 2 * numFaces (all quads).
  // The following has pretty harsh algorithmic time -- 3*N. Can easily be sped
  // up by using the preprocessing program to determine how many triangles there
  // are in the object. This would reduce the complexity to 2*N.
  uint32_t numTrianglesWorstCase = numFaces * 2;
  size_t iboWorstCaseSize = numTrianglesWorstCase * sizeof(uint16_t) * 3;
  ibo.resize(iboWorstCaseSize); // linear in complexity
  uint16_t* iboPtr = reinterpret_cast<uint16_t*>(&ibo[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  for (size_t i = 0; i < numFaces; i++)
  {
    uint8_t numIndices;
    stream.read(reinterpret_cast<char*>(&numIndices), sizeof(uint8_t));
    if (numIndices == 3)
    {
      uint16_t index0;
      uint16_t index1;
      uint16_t index2;
      stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
      stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
      stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));
      iboPtr[0] = index0;
      iboPtr[1] = index1;
      iboPtr[2] = index2;

      iboPtr += 3;
      ++numTriangles;
    }
    else if (numIndices == 4)
    {
      // Two triangles
      {
        uint16_t index0;
        uint16_t index1;
        uint16_t index2;
        stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));

        iboPtr[0] = index0;
        iboPtr[1] = index1;
        iboPtr[2] = index2;

        iboPtr += 3;
      }

      {
        uint16_t index0;
        uint16_t index1;
        uint16_t index2;
        stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));

        iboPtr[0] = index0;
        iboPtr[1] = index1;
        iboPtr[2] = index2;

        iboPtr += 3;
      }

      numTriangles += 2;
    }

  }

  // Resize the IBO appropriately (this is the operation that can be eliminated
  // if we know how many triangles there will be beforehand).
  size_t realIBOSize = numTriangles * sizeof(uint16_t) * 3;
  ibo.resize(realIBOSize); // linear in complexity

  return numTriangles;
}


//------------------------------------------------------------------------------
void StuInterface::addPassToObjectImpl(Hub& hub, StuInterface* iface,
                                       std::string object,
                                       std::string pass,
                                       std::string program,
                                       std::string vboName,
                                       std::string iboName,
                                       PRIMITIVE_TYPES type,
                                       int32_t passOrder)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  std::shared_ptr<VBOObject> vbo = iface->mVBOMap.at(vboName);
  std::shared_ptr<IBOObject> ibo = iface->mIBOMap.at(iboName);

  // Check to see if the pass exists.
  auto passIt = iface->mNameToPass.find(pass);
  if (passIt == iface->mNameToPass.end())
    throw std::runtime_error("Pass (" + pass + ") does not exist.");

  // Add object to pass if it isn't already part of the pass.
  auto objectInPass = passIt->second->mNameToObject.find(object);
  if (objectInPass == iface->mNameToObject.end())
    passIt->second->mNameToObject[object] = obj;

  obj->addPass(pass, program, vbo, ibo, getGLPrimitive(type), passOrder);
}


//------------------------------------------------------------------------------
void StuInterface::addPassToObject(const std::string& object,
                                   const std::string& program,
                                   const std::string& vboName,
                                   const std::string& iboName,
                                   PRIMITIVE_TYPES type,
                                   const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(addPassToObjectImpl, _1, this, object, pass, program, 
                vboName, iboName, type, mCurrentPassOrder);
  mHub.addFunctionToThreadQueue(fun);
  ++mCurrentPassOrder;
}

//------------------------------------------------------------------------------
void StuInterface::addPassToObject(const std::string& object,
                                   
                                   const std::string& program,
                                   const std::string& vboName,
                                   const std::string& iboName,
                                   PRIMITIVE_TYPES type,
                                   int32_t passOrder,
                                   const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(addPassToObjectImpl, _1, this, object, pass, program, 
                vboName, iboName, type, passOrder);
  mHub.addFunctionToThreadQueue(fun);
}






//------------------------------------------------------------------------------
void StuInterface::removeGeomPassFromObjectImpl(Hub& hub, StuInterface* iface,
                                                std::string object,
                                                std::string pass)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  obj->removePass(pass);
}


//------------------------------------------------------------------------------
void StuInterface::removePassFromObject(const std::string& object,
                                        const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(removeGeomPassFromObjectImpl, _1, this, object, pass);
  mHub.addFunctionToThreadQueue(fun);
}






//------------------------------------------------------------------------------
void StuInterface::addObjectImpl(Hub& hub, StuInterface* iface,
                                 std::string objectName, int32_t renderOrder)
{
  if (iface->mNameToObject.find(objectName) != iface->mNameToObject.end())
    throw Duplicate("There already exists an object by that name!");

  std::shared_ptr<StuObject> obj = std::shared_ptr<StuObject>(
      new StuObject(iface->mHub, objectName, renderOrder));
  iface->mNameToObject[objectName] = obj;

  // Add object to specified rendering order.
  iface->mRenderOrderToObjects.insert(std::make_pair(renderOrder, obj));
}

//------------------------------------------------------------------------------
void StuInterface::addObject(const std::string& object)
{
  // Note: Is is fine to access mCurrentRenderOrder. It is only modified by
  //       the client. Never the render thread.
  Hub::RemoteFunction fun =
      std::bind(addObjectImpl, _1, this, object, mCurrentRenderOrder);
  mHub.addFunctionToThreadQueue(fun);
  ++mCurrentRenderOrder;
}

//------------------------------------------------------------------------------
void StuInterface::addObject(const std::string& object, int32_t renderOrder)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectImpl, _1, this, object, renderOrder);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::removeObjectImpl(Hub& hub, StuInterface* iface,
                                    std::string object)
{
  if (iface->mNameToObject.find(object) == iface->mNameToObject.end())
    throw std::range_error("Object to remove does not exist!");

  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  iface->removeObjectFromOrderList(obj->getName(), obj->getRenderOrder());
  iface->mNameToObject.erase(object);
}

//------------------------------------------------------------------------------
void StuInterface::removeObjectFromOrderList(const std::string& objectName,
                                             int32_t objectOrder)
{
  auto it = mRenderOrderToObjects.find(objectOrder);
  /// \xxx Should we be only iterating to mRenderOrderToObjects.count(objRenderOrder)?
  for (; it != mRenderOrderToObjects.end(); ++it)
  {
    if (it->second->getName() == objectName)
    {
      mRenderOrderToObjects.erase(it);
      return;
    }
  }
  throw std::range_error("Unable to find object to remove in render order map.");
}

//------------------------------------------------------------------------------
void StuInterface::removeObject(const std::string& object)
{
  Hub::RemoteFunction fun =
      std::bind(removeObjectImpl, _1, this, object);
  mHub.addFunctionToThreadQueue(fun);
}




//------------------------------------------------------------------------------
void StuInterface::removeAllObjectsImpl(Hub& hub, StuInterface* iface)
{
  iface->mNameToObject.clear();
  iface->mRenderOrderToObjects.clear();
}

//------------------------------------------------------------------------------
void StuInterface::removeAllObjects()
{
  Hub::RemoteFunction fun = std::bind(removeAllObjectsImpl, _1, this);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::assignRenderOrderImpl(Hub& hub, StuInterface* iface,
                                         std::string objectName, int32_t renderOrder)
{
  if (iface->mNameToObject.find(objectName) == iface->mNameToObject.end())
    throw std::range_error("Object to reassign rendering order to does not exist!");


  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(objectName);
  iface->removeObjectFromOrderList(obj->getName(), obj->getRenderOrder());

  // Re-assign order to object...
  obj->setRenderOrder(renderOrder);

  // Now re-add the object to the render order map.
  iface->mRenderOrderToObjects.insert(std::make_pair(renderOrder, obj));
}

//------------------------------------------------------------------------------
void StuInterface::assignRenderOrder(const std::string& object, int32_t renderOrder)
{
  Hub::RemoteFunction fun =
      std::bind(assignRenderOrderImpl, _1, this, object, renderOrder);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::addObjectPassUniformInternalImpl(Hub& hub, StuInterface* iface,
                                                    std::string object,
                                                    std::string pass,
                                                    std::string uniformName,
                                                    std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  obj->addPassUniform(pass, uniformName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectPassUniformConcrete(const std::string& object,
                                                const std::string& uniformName,
                                                std::shared_ptr<AbstractUniformStateItem> item,
                                                const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectPassUniformInternalImpl, _1, this, object, pass, uniformName, item);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectGlobalUniformInternalImpl(Hub& hub, StuInterface* iface,
                                                      std::string objectName,
                                                      std::string uniformName,
                                                      std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(objectName);
  obj->addGlobalUniform(uniformName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectGlobalUniformConcrete(const std::string& object,
                                                  const std::string& uniformName,
                                                  std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectGlobalUniformInternalImpl, _1, this, object, uniformName, item);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectTransform(const std::string& object,
                                      const M44& transform)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectTransformImpl, _1, this, object, transform);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::addObjectTransformImpl(Hub& hub, StuInterface* iface,
                                          std::string objectName,
                                          M44 transform)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(objectName);
  obj->addObjectTransform(transform);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectPassGPUStateImpl(Hub& hub, StuInterface* iface,
                                             std::string object,
                                             std::string pass,
                                             GPUState state)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  obj->addPassGPUState(pass, state);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectPassGPUState(const std::string& object,
                                         const GPUState& state,
                                         const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectPassGPUStateImpl, _1, this, object, pass, state);
  mHub.addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void StuInterface::addGlobalUniformInternalImpl(Hub& hub, StuInterface* iface,
                                              std::string uniformName,
                                              std::shared_ptr<AbstractUniformStateItem> item)
{
  // Access uniform state manager and apply/update uniform value.
  hub.getGlobalUniformStateMan().updateGlobalUniform(uniformName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addGlobalUniformConcrete(const std::string& uniformName,
                                            std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(addGlobalUniformInternalImpl, _1, this, uniformName, item);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectGlobalSpireAttributeConcrete(const std::string& object,
                                                         const std::string& attributeName,
                                                         std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectGlobalSpireAttributeImpl, _1, this, object, attributeName, item);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectGlobalSpireAttributeImpl(Hub& hub, StuInterface* iface,
                                                     std::string objectName,
                                                     std::string attributeName,
                                                     std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(objectName);
  obj->addObjectGlobalSpireAttribute(attributeName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectPassSpireAttributeConcrete(const std::string& object,
                                                       const std::string& attributeName,
                                                       std::shared_ptr<AbstractUniformStateItem> item,
                                                       const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectPassSpireAttributeImpl, _1, this, object, attributeName, item, passName);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addObjectPassSpireAttributeImpl(Hub& hub, StuInterface* iface,
                                                   std::string objectName,
                                                   std::string attributeName,
                                                   std::shared_ptr<AbstractUniformStateItem> item,
                                                   std::string passName)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(objectName);
  obj->addObjectPassSpireAttribute(passName, attributeName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShaderImpl(
    Hub& hub, StuInterface* iface,
    std::string programName,
    std::vector<std::tuple<std::string, SHADER_TYPES>> tempShaders)
{
  std::list<std::tuple<std::string, GLenum>> shaders;
  for (auto it = tempShaders.begin(); it != tempShaders.end(); ++it)
  {
    GLenum glType = GL_VERTEX_SHADER;
    switch (std::get<1>(*it))
    {
      case VERTEX_SHADER:
        glType = GL_VERTEX_SHADER;
        break;

      case FRAGMENT_SHADER:
        glType = GL_FRAGMENT_SHADER;
        break;

      default:
        throw UnsupportedException("This shader is not supported yet.");
        break;
    }
    shaders.push_back(make_tuple(std::get<0>(*it), glType));
  }

  std::shared_ptr<ShaderProgramAsset> shader = 
      iface->mHub.getShaderProgramManager().loadProgram(programName, shaders);

  // Check to make sure we haven't already added this shader.
  for (auto it = iface->mPersistentShaders.begin();
       it != iface->mPersistentShaders.end(); ++it)
  {
    if (shader == *it)
      throw Duplicate("Attempted to add duplicate shader to persistent shader list");
  }
  iface->mPersistentShaders.push_back(shader);
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::string& vertexShader,
                                       const std::string& fragmentShader)
{
  std::vector<std::tuple<std::string, SHADER_TYPES>> shaders;
  shaders.push_back(make_tuple(vertexShader, VERTEX_SHADER));
  shaders.push_back(make_tuple(fragmentShader, FRAGMENT_SHADER));

  Hub::RemoteFunction fun =
      std::bind(addPersistentShaderImpl, _1, this, programName, shaders);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders)
{
  Hub::RemoteFunction fun =
      std::bind(addPersistentShaderImpl, _1, this, programName, shaders);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
std::shared_ptr<const StuObject> StuInterface::ntsGetObjectWithName(const std::string& name) const
{
  return mNameToObject.at(name);
}

//------------------------------------------------------------------------------
bool StuInterface::ntsHasRenderingOrder(const std::vector<std::string>& renderOrder) const
{
  // Iterate over the map (we will iterate over the map in ascending order).
  // This is the same loop we will be using in the renderer.
  auto itToTest = renderOrder.begin();
  auto itOrder = mRenderOrderToObjects.begin();
  for (; itOrder != mRenderOrderToObjects.end() && itToTest != renderOrder.end();
       ++itOrder, ++itToTest)
  {
    if (itOrder->second->getName() != *itToTest)
      return false;
  }

  if (itToTest == renderOrder.end() && itOrder == mRenderOrderToObjects.end())
    return true;
  else
    return false;
}

//------------------------------------------------------------------------------
void StuInterface::clearGLResources()
{
  mNameToObject.clear();
  mRenderOrderToObjects.clear();
  mPersistentShaders.clear();
  mVBOMap.clear();
  mIBOMap.clear();
}

//------------------------------------------------------------------------------
GLenum getGLPrimitive(StuInterface::PRIMITIVE_TYPES type)
{
  switch (type)
  {
    case StuInterface::POINTS:
      return GL_POINTS;
      break;

    case StuInterface::LINES:
      return GL_LINES;
      break;

    case StuInterface::LINE_LOOP:
      return GL_LINE_LOOP;
      break;

    case StuInterface::LINE_STRIP:
      return GL_LINE_STRIP;
      break;

    case StuInterface::TRIANGLES:
      return GL_TRIANGLES;
      break;

    case StuInterface::TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
      break;

    case StuInterface::TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
      break;

    case StuInterface::LINES_ADJACENCY:
      return GL_LINES_ADJACENCY;
      break;

    case StuInterface::LINE_STRIP_ADJACENCY:
      return GL_LINE_STRIP_ADJACENCY;
      break;

    case StuInterface::TRIANGLES_ADJACENCY:
      return GL_TRIANGLES_ADJACENCY;
      break;

    case StuInterface::TRIANGLE_STRIP_ADJACENCY:
      return GL_TRIANGLE_STRIP_ADJACENCY;
      break;

    default:
      {
        std::stringstream stream;
        stream << "Expected type to be one of PRIMITIVE_TYPES, received " << type;
        throw std::invalid_argument(stream.str());
      }
  }

  return GL_TRIANGLES;
}

}
