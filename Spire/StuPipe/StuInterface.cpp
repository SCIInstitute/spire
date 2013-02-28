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

using namespace std::placeholders;

namespace Spire {

//------------------------------------------------------------------------------
StuInterface::StuInterface(std::weak_ptr<Interface> iface) :
    PipeInterface(iface),
    mCurrentRenderOrder(0)
{
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
void StuInterface::ntsDoPass()
{
  
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
          vboName, std::shared_ptr<VBOObject>(new VBOObject(vboData, attribNames))));
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
void StuInterface::addGeomPassToObjectImpl(Hub& hub, StuInterface* iface,
                                           std::string object,
                                           std::string pass,
                                           std::string program,
                                           std::string vboName,
                                           std::string iboName)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  std::shared_ptr<VBOObject> vbo = iface->mVBOMap.at(vboName);
  std::shared_ptr<IBOObject> ibo = iface->mIBOMap.at(iboName);
  obj->addPass(pass, program, vbo, ibo);
}


//------------------------------------------------------------------------------
void StuInterface::addGeomPassToObject(const std::string& object,
                                       const std::string& pass,
                                       const std::string& program,
                                       const std::string& vboName,
                                       const std::string& iboName)
{
  Hub::RemoteFunction fun =
      std::bind(addGeomPassToObjectImpl, _1, this, object, pass, program, 
                vboName, iboName);
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
void StuInterface::removeGeomPassFromObject(const std::string& object,
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
void StuInterface::addPassUniformInternalImpl(Hub& hub, StuInterface* iface,
                                              std::string object,
                                              std::string pass,
                                              std::string uniformName,
                                              std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<StuObject> obj = iface->mNameToObject.at(object);
  obj->addPassUniform(pass, uniformName, item);
}

//------------------------------------------------------------------------------
void StuInterface::addPassUniformInternal(const std::string& object,
                                          const std::string& pass,
                                          const std::string& uniformName,
                                          std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(addPassUniformInternalImpl, _1, this, object, pass, uniformName, item);
  mHub.addFunctionToThreadQueue(fun);
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

}
