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

#include "Common.h"
#include "StuInterface.h"
#include "StuObject.h"
#include "Core/ShaderProgramMan.h"
#include "Core/Hub.h"

namespace Spire {

//------------------------------------------------------------------------------
StuInterface::StuInterface(Interface& iface) :
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
size_t StuInterface::addIBOToObject(const std::string& object,
                                    std::shared_ptr<std::vector<uint8_t>> iboData,
                                    IBO_TYPE type)
{
  /// \todo Turn into a message (execute call immediately if we are running
  ///       a non-threaded Spire -- very useful if unit testing).
  ///       Need a synchronous test harness. Otherwise things get nasty.
  // The 'at' function will throw std::out_of_range an exception if object
  // doesn't exist.
  StuObject& obj = mObjects.at(object);
  return obj.addIBO(iboData, type);
}

//------------------------------------------------------------------------------
size_t StuInterface::addVBOToObject(const std::string& object,
                                    std::shared_ptr<std::vector<uint8_t>> vboData,
                                    const std::vector<std::string>& attribNames)
{
  /// \todo Turn into a message.
  StuObject& obj = mObjects.at(object);
  return obj.addVBO(vboData, attribNames);
}

//------------------------------------------------------------------------------
void StuInterface::addGeomPassToObject(const std::string& object,
                                       const std::string& pass,
                                       const std::string& program,
                                       size_t vboID,
                                       size_t iboID)
{
  /// \todo Turn into a message.
  StuObject& obj = mObjects.at(object);
  obj.addPass(pass, program, vboID, iboID);
}

//------------------------------------------------------------------------------
void StuInterface::addObject(const std::string& object)
{
  /// \todo Turn into a message.
  if (mObjects.find(object) != mObjects.end())
    throw Duplicate("There already exists an object by that name!");

  StuObject obj;
  mObjects[object] = std::move(obj);
}

//------------------------------------------------------------------------------
void StuInterface::addPassUniformInternal(const std::string& object,
                                          const std::string& pass,
                                          const std::string& uniformName,
                                          std::unique_ptr<AbstractUniformStateItem>&& item)
{
  /// \todo Turn into a message.
  StuObject& obj = mObjects.at(object);
  // Move is not necessary, but makes things more clear.
  obj.addPassUniform(pass, uniformName, std::move(item));
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::string& vertexShader,
                                       const std::string& fragmentShader)
{
  /// \todo Turn into a message.
  std::list<std::tuple<std::string, GLenum>> shaders;
  shaders.push_back(make_tuple(vertexShader, GL_VERTEX_SHADER));
  shaders.push_back(make_tuple(fragmentShader, GL_FRAGMENT_SHADER));
  std::shared_ptr<ShaderProgramAsset> shader = 
      mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders)
{

}

//------------------------------------------------------------------------------
void StuInterface::removeGeomPassFromObject(const std::string& object,
                                            const std::string& pass)
{
}

//------------------------------------------------------------------------------
void StuInterface::removeObject(const std::string& object)
{
}

}
