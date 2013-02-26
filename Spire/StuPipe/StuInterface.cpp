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

#include "Common.h"
#include "StuInterface.h"
#include "StuObject.h"
#include "Core/ShaderProgramMan.h"
#include "Core/Hub.h"

using namespace std::placeholders;

namespace Spire {

//------------------------------------------------------------------------------
StuInterface::StuInterface(std::weak_ptr<Interface> iface) :
    PipeInterface(iface)
{
}

//------------------------------------------------------------------------------
StuInterface::~StuInterface()
{
}

//------------------------------------------------------------------------------
void StuInterface::initOnRenderThread()
{
}

//------------------------------------------------------------------------------
void StuInterface::doPass()
{
  
}

//------------------------------------------------------------------------------
void StuInterface::addIBOToObjectImpl(Hub& hub, StuInterface* iface,
                                      std::string object, std::string name,
                                      std::shared_ptr<std::vector<uint8_t>> iboData,
                                      StuInterface::IBO_TYPE type)
{
  // The 'at' function will throw std::out_of_range an exception if object
  // doesn't exist.
  StuObject& obj = iface->mObjects.at(object);
  obj.addIBO(name, iboData, type);
}

//------------------------------------------------------------------------------
void StuInterface::addIBOToObject(const std::string& object,
                                  const std::string& name,
                                  std::shared_ptr<std::vector<uint8_t>> iboData,
                                  IBO_TYPE type)
{
  Hub::RemoteFunction fun =
      std::bind(addIBOToObjectImpl, _1, this, object, name, iboData, type);
  mHub.addFunctionToThreadQueue(fun);
}




//------------------------------------------------------------------------------
void StuInterface::addVBOToObjectImpl(Hub& hub, StuInterface* iface,
                                      std::string object, std::string name,
                                      std::shared_ptr<std::vector<uint8_t>> vboData,
                                      std::vector<std::string> attribNames)
{
  StuObject& obj = iface->mObjects.at(object);
  obj.addVBO(name, vboData, attribNames);
}


//------------------------------------------------------------------------------
void StuInterface::addVBOToObject(const std::string& object,
                                  const std::string& name,
                                  std::shared_ptr<std::vector<uint8_t>> vboData,
                                  const std::vector<std::string>& attribNames)
{
  Hub::RemoteFunction fun =
      std::bind(addVBOToObjectImpl, _1, this, object, name, vboData, attribNames);
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
  /// \todo Turn into a message.
  StuObject& obj = iface->mObjects.at(object);
  obj.addPass(pass, program, vboName, iboName);
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
  StuObject& obj = iface->mObjects.at(object);
  obj.removePass(pass);
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
void StuInterface::addObjectImpl(Hub& hub, StuInterface* iface, std::string object)
{
  if (iface->mObjects.find(object) != iface->mObjects.end())
    throw Duplicate("There already exists an object by that name!");

  StuObject obj;
  iface->mObjects[object] = std::move(obj);
}

//------------------------------------------------------------------------------
void StuInterface::addObject(const std::string& object)
{
  Hub::RemoteFunction fun =
      std::bind(addObjectImpl, _1, this, object);
  mHub.addFunctionToThreadQueue(fun);
}



//------------------------------------------------------------------------------
void StuInterface::removeObjectImpl(Hub& hub, StuInterface* iface,
                                    std::string object)
{
  if (iface->mObjects.find(object) == iface->mObjects.end())
    throw std::range_error("Object to remove does not exist!");

  iface->mObjects.erase(object);
}

//------------------------------------------------------------------------------
void StuInterface::removeObject(const std::string& object)
{
  Hub::RemoteFunction fun =
      std::bind(removeObjectImpl, _1, this, object);
  mHub.addFunctionToThreadQueue(fun);
}





//------------------------------------------------------------------------------
void StuInterface::addPassUniformInternalImpl(Hub& hub, StuInterface* iface,
                                              std::string object,
                                              std::string pass,
                                              std::string uniformName,
                                              std::shared_ptr<AbstractUniformStateItem> item)
{
  StuObject& obj = iface->mObjects.at(object);
  obj.addPassUniform(pass, uniformName, item);
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
void StuInterface::addPersistentShaderImpl(Hub& hub, StuInterface* iface,
                                           std::string programName,
                                           std::vector<std::tuple<std::string, SHADER_TYPES>> tempShaders)
{
  std::list<std::tuple<std::string, GLenum>> shaders;
  for (auto it = shaders.begin(); it != shaders.end(); ++it)
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
      iface->mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);
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


}
