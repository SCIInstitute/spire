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

#include "Hub.h"
#include "InterfaceImplementation.h"
#include "SpireObject.h"
#include "Exceptions.h"
#include "LambdaInterface.h"
#include "ObjectLambda.h"

namespace Spire {

// Simple static function to convert from PRIMITIVE_TYPES to GL types.
// Not part of the class due to the return type (interface class should have
// nothing GL specific in them).
GLenum getGLPrimitive(Interface::PRIMITIVE_TYPES type);

//------------------------------------------------------------------------------
InterfaceImplementation::InterfaceImplementation(Hub& hub, bool threaded) :
    mHub(hub),
    mThreaded(threaded)
{
  addPassToBack(*this, SPIRE_DEFAULT_PASS);
}

//------------------------------------------------------------------------------
bool InterfaceImplementation::addFunctionToQueue(const Hub::RemoteFunction& fun)
{
#ifdef SPIRE_USE_STD_THREADS
  if (mThreaded)
  {
    return mQueue.push(ThreadMessage(fun));
  }
  else
  {
    // Same as the #else clause below.
    fun(*this);
    return true;
  }
#else
  // Call the function immediately. This case (without std threads) will be 
  // used as our synchronized test harness.
  if (!mThreaded)
  {
    fun(*this);
    return true;
  }
  else
  {
    return false;
  }
#endif
}

//------------------------------------------------------------------------------
void InterfaceImplementation::executeQueue()
{
#ifdef SPIRE_USE_STD_THREADS
  if (mThreaded)
  {
    ThreadMessage msg;
    while (mQueue.pop(msg))
    {
      try
      {
        msg.execute(*this);
      }
      catch (std::exception& e)
      {
        Log::error() << "Spire exception thrown: " << e.what() << std::endl;
      }
      msg.clear();
    }
  }
#else
  // The functions were already executed.
#endif
}


//------------------------------------------------------------------------------
void InterfaceImplementation::resize(InterfaceImplementation& /*self*/, size_t width, size_t height)
{
  GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::clearGLResources()
{
  mNameToObject.clear();
  mPersistentShaders.clear();
  mVBOMap.clear();
  mIBOMap.clear();

  // Do we want to clear passes? They don't have any associated GL data.
  //mPasses.clear();
  //mNameToPass.clear();
  //mGlobalBeginLambdas.clear();
}

//------------------------------------------------------------------------------
void InterfaceImplementation::doAllPasses()
{
  /// \todo Call all passes begin lambdas. Used primarily to setup global
  /// uniforms.

  // Do not even attempt to render if the framebuffer is not complete.
  // This can happen when the rendering window is hidden (in SCIRun5 for
  // example);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return;

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
    try
    {
      doPass((*it)->mName);
    }
    catch (std::exception& e)
    {
      Log::error() << "Caught exception when rendering pass: " << (*it)->mName << std::endl;
      Log::error() << "Exception: " << e.what() << std::endl;
    }
  }

  /// \todo Call all passes end lambdas. Used primarily to setup global
  /// uniforms.
}

//------------------------------------------------------------------------------
bool InterfaceImplementation::hasPass(const std::string& pass) const
{
  return (mNameToPass.find(pass) != mNameToPass.end());
}

//------------------------------------------------------------------------------
std::shared_ptr<const SpireObject> InterfaceImplementation::getObjectWithName(const std::string& name) const
{
  return mNameToObject.at(name);
}

//------------------------------------------------------------------------------
bool InterfaceImplementation::isObjectInPass(const std::string& object, const std::string& pass) const
{
  if (hasPass(pass))
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
void InterfaceImplementation::doPass(const std::string& passName)
{
  std::shared_ptr<Pass> pass = mNameToPass.at(passName);

  ///\todo Call pass begin lambdas. Setup global pass specific uniforms.

  // Loop over all objects in the pass and render them.
  /// \todo Need to add some way of ordering the rendered objects, whether it be
  /// by another structure built into Spire (not for this at all), or some lambda
  /// callback.
  for (auto it = pass->mNameToObject.begin(); it != pass->mNameToObject.end(); ++it)
  {
    it->second->renderPass(passName);
  }

  ///\todo Call pass end lambda.
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPassToFront(InterfaceImplementation& self, std::string passName)
{
  // Verify that there is no pass by that name already.
  if (self.hasPass(passName) == true)
    throw std::runtime_error("Pass (" + passName + ") already exists!");

  std::shared_ptr<Pass> pass(new Pass(passName));
  self.mPasses.push_back(pass);
  self.mNameToPass[passName] = pass;
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPassToBack(InterfaceImplementation& self, std::string passName)
{
  if (self.hasPass(passName) == true)
    throw std::runtime_error("Pass (" + passName + ") already exists!");

  std::shared_ptr<Pass> pass(new Pass(passName));
  self.mPasses.push_front(pass);
  self.mNameToPass[passName] = pass;
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObject(InterfaceImplementation& self, std::string objectName)
{
  if (self.mNameToObject.find(objectName) != self.mNameToObject.end())
    throw Duplicate("There already exists an object by that name!");

  std::shared_ptr<SpireObject> obj = std::shared_ptr<SpireObject>(
      new SpireObject(self.mHub, objectName));
  self.mNameToObject[objectName] = obj;
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeObject(InterfaceImplementation& self,
                                           std::string objectName)
{
  if (self.mNameToObject.find(objectName) == self.mNameToObject.end())
    throw std::range_error("Object to remove does not exist!");

  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(objectName);
  self.mNameToObject.erase(objectName);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeAllObjects(InterfaceImplementation& self)
{
  self.mNameToObject.clear();

  // Clear all objects from all passes. This should clean up any lingering
  // ponters to the objects.
  for (auto it = self.mPasses.begin(); it != self.mPasses.end(); ++it)
  {
    (*it)->mNameToObject.clear();
  }
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addVBO(InterfaceImplementation& self, std::string vboName,
                                     std::shared_ptr<std::vector<uint8_t>> vboData,
                                     std::vector<std::string> attribNames)
{
  if (self.mVBOMap.find(vboName) != self.mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object.");

  self.mVBOMap.insert(std::make_pair(
          vboName, std::shared_ptr<VBOObject>(
              new VBOObject(vboData, attribNames, self.mHub.getShaderAttributeManager()))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addConcurrentVBO(
    const std::string& vboName, const uint8_t* vboData, size_t vboSize,
    const std::vector<std::string>& attribNames)
{
  if (mThreaded == true)
  {
    Log::error() << "You cannot call the raw version of addIBO or addVBO "
                 << "when spire is running in its own thread." << std::endl;
    throw std::runtime_error("addVBO: Invalid threaded function call. See Log.");
  }

  if (mVBOMap.find(vboName) != mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object.");

  mVBOMap.insert(std::make_pair(
          vboName, std::shared_ptr<VBOObject>(
              new VBOObject(vboData, vboSize, attribNames, 
                            mHub.getShaderAttributeManager()))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeVBO(InterfaceImplementation& self,
                                        std::string vboName)
{
  size_t numElementsRemoved = self.mVBOMap.erase(vboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find VBO to remove.");
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addIBO(InterfaceImplementation& self, std::string iboName,
                                     std::shared_ptr<std::vector<uint8_t>> iboData,
                                     Interface::IBO_TYPE type)
{
  if (self.mIBOMap.find(iboName) != self.mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object.");

  self.mIBOMap.insert(std::make_pair(
          iboName, std::shared_ptr<IBOObject>(new IBOObject(iboData, type))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addConcurrentIBO(
    const std::string& iboName, const uint8_t* iboData, size_t iboSize,
    Interface::IBO_TYPE type)
{
  if (mThreaded == true)
  {
    Log::error() << "You cannot call the raw version of addIBO or addVBO "
                 << "when spire is running in its own thread." << std::endl;
    throw std::runtime_error("addIBO: Invalid threaded function call. See Log.");
  }

  if (mIBOMap.find(iboName) != mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object.");

  mIBOMap.insert(std::make_pair(
          iboName, std::shared_ptr<IBOObject>(new IBOObject(iboData, iboSize, type))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeIBO(InterfaceImplementation& self, std::string iboName)
{
  size_t numElementsRemoved = self.mIBOMap.erase(iboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find IBO to remove.");
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPassToObject(
    InterfaceImplementation& self, std::string object,
    std::string program, std::string vboName, std::string iboName,
    Interface::PRIMITIVE_TYPES type, std::string pass, std::string parentPass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  std::shared_ptr<VBOObject> vbo = self.mVBOMap.at(vboName);
  std::shared_ptr<IBOObject> ibo = self.mIBOMap.at(iboName);

  // The 'responsiblePass' must exist. It is under this pass in which objects
  // will be rendered.
  std::string responsiblePass = pass;
  if (parentPass.size() > 0)
    responsiblePass = parentPass;

  // Find the responsible pass and add this object to it.
  auto passIt = self.mNameToPass.find(responsiblePass);
  if (passIt != self.mNameToPass.end())
  {
    // Add object to pass if it isn't already part of the pass.
    auto objectInPass = passIt->second->mNameToObject.find(object);
    if (objectInPass == passIt->second->mNameToObject.end())
      passIt->second->mNameToObject[object] = obj;
  }
  else
  {
    // This pass is NOT a subpass, the non-existance of a global pass must
    // be an error.
    Log::error() << "Unable to find global pass and parent pass was not specified.";
    throw std::runtime_error("Global pass (" + pass + ") does not exist.");
  }

  obj->addPass(pass, program, vbo, ibo, getGLPrimitive(type), parentPass);
}


//------------------------------------------------------------------------------
void InterfaceImplementation::removePassFromObject(InterfaceImplementation& self,
                                                   std::string object, std::string pass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  obj->removePass(pass);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectPassUniformConcrete(InterfaceImplementation& self, 
                                                           std::string object, std::string uniformName,
                                                           std::shared_ptr<AbstractUniformStateItem> item,
                                                           std::string pass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  obj->addPassUniform(pass, uniformName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectGlobalUniformConcrete(InterfaceImplementation& self,
                                                             std::string objectName,
                                                             std::string uniformName,
                                                             std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(objectName);
  obj->addGlobalUniform(uniformName, item);
}


//------------------------------------------------------------------------------
void InterfaceImplementation::addGlobalUniformConcrete(InterfaceImplementation& self,
                                                       std::string uniformName,
                                                       std::shared_ptr<AbstractUniformStateItem> item)
{
  // Access uniform state manager and apply/update uniform value.
  self.mHub.getGlobalUniformStateMan().updateGlobalUniform(uniformName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectPassGPUState(InterfaceImplementation& self,
                                                    std::string object,
                                                    GPUState state, 
                                                    std::string pass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  obj->addPassGPUState(pass, state);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addShaderAttribute(InterfaceImplementation& self, std::string codeName,
                                                 size_t numComponents, bool normalize, size_t size,
                                                 Interface::DATA_TYPES t)
{
  self.mHub.getShaderAttributeManager().addAttribute(codeName, numComponents, normalize, size, t);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectGlobalMetadataConcrete(InterfaceImplementation& self,
                                                              std::string objectName,
                                                              std::string attributeName,
                                                              std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(objectName);
  obj->addObjectGlobalMetadata(attributeName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectPassMetadataConcrete(InterfaceImplementation& self, std::string objectName,
                                                            std::string attributeName,
                                                            std::shared_ptr<AbstractUniformStateItem> item,
                                                            std::string passName)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(objectName);
  obj->addObjectPassMetadata(passName, attributeName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPersistentShader(InterfaceImplementation& self,
                                                  std::string programName,
                                                  std::vector<std::tuple<std::string, Interface::SHADER_TYPES>> tempShaders)
{
  std::list<std::tuple<std::string, GLenum>> shaders;
  for (auto it = tempShaders.begin(); it != tempShaders.end(); ++it)
  {
    GLenum glType = GL_VERTEX_SHADER;
    switch (std::get<1>(*it))
    {
      case Interface::VERTEX_SHADER:
        glType = GL_VERTEX_SHADER;
        break;

      case Interface::FRAGMENT_SHADER:
        glType = GL_FRAGMENT_SHADER;
        break;

      default:
        throw UnsupportedException("This shader is not supported yet.");
    }
    shaders.push_back(make_tuple(std::get<0>(*it), glType));
  }

  std::shared_ptr<ShaderProgramAsset> shader = 
      self.mHub.getShaderProgramManager().loadProgram(programName, shaders);

  // Check to make sure we haven't already added this shader.
  for (auto it = self.mPersistentShaders.begin();
       it != self.mPersistentShaders.end(); ++it)
  {
    if (shader == *it)
      throw Duplicate("Attempted to add duplicate shader to persistent shader list");
  }
  self.mPersistentShaders.push_back(shader);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaBeginAllPasses(InterfaceImplementation& self, Interface::PassLambdaFunction fp)
{
  self.mGlobalBeginLambdas.push_back(fp);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaEndAllPasses(InterfaceImplementation& self, Interface::PassLambdaFunction fp)
{
  self.mGlobalEndLambdas.push_back(fp);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaPrePass(InterfaceImplementation& self, Interface::PassLambdaFunction fp, std::string pass)
{
  auto passIt = self.mNameToPass.find(pass);
  if (passIt == self.mNameToPass.end())
    throw std::runtime_error("Pass (" + pass + ") does not exist.");

  passIt->second->mPassBeginLambdas.push_back(fp);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaPostPass(InterfaceImplementation& self, Interface::PassLambdaFunction fp, std::string pass)
{
  auto passIt = self.mNameToPass.find(pass);
  if (passIt == self.mNameToPass.end())
    throw std::runtime_error("Pass (" + pass + ") does not exist.");

  passIt->second->mPassEndLambdas.push_back(fp);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaObjectRender(InterfaceImplementation& self, 
                                                    std::string object, Interface::ObjectLambdaFunction fp, std::string pass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  obj->addPassRenderLambda(pass, fp);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addLambdaObjectUniforms(InterfaceImplementation& self, std::string object, 
                                                      Interface::ObjectUniformLambdaFunction fp, std::string pass)
{
  std::shared_ptr<SpireObject> obj = self.mNameToObject.at(object);
  obj->addPassUniformLambda(pass, fp);
}

//------------------------------------------------------------------------------
GLenum InterfaceImplementation::getGLPrimitive(Interface::PRIMITIVE_TYPES type)
{
  switch (type)
  {
    case Interface::POINTS:                   return GL_POINTS;
    case Interface::LINES:                    return GL_LINES;
    case Interface::LINE_LOOP:                return GL_LINE_LOOP;
    case Interface::LINE_STRIP:               return GL_LINE_STRIP;
    case Interface::TRIANGLES:                return GL_TRIANGLES;
    case Interface::TRIANGLE_STRIP:           return GL_TRIANGLE_STRIP;
    case Interface::TRIANGLE_FAN:             return GL_TRIANGLE_FAN;
    // Adjacency is only supported in OpenGL 4.0 +. Will add when we support
    // Core profiles, starting with core profile 4.1.
#if defined(USE_CORE_PROFILE_4)
    case Interface::LINES_ADJACENCY:          return GL_LINES_ADJACENCY;
    case Interface::LINE_STRIP_ADJACENCY:     return GL_LINE_STRIP_ADJACENCY;
    case Interface::TRIANGLES_ADJACENCY:      return GL_TRIANGLES_ADJACENCY;
    case Interface::TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP_ADJACENCY;
#else
    case Interface::LINES_ADJACENCY:
    case Interface::LINE_STRIP_ADJACENCY:
    case Interface::TRIANGLES_ADJACENCY:
    case Interface::TRIANGLE_STRIP_ADJACENCY:
      Log::error() << "Adjacency primitive types not supported in OpenGL ES 2.0";
      return GL_TRIANGLES;
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    default:
      {
        std::stringstream stream;
        stream << "Expected type to be one of PRIMITIVE_TYPES, received " << type;
        throw std::invalid_argument(stream.str());
      }
#pragma clang diagnostic pop
  }

  return GL_TRIANGLES;
}

//------------------------------------------------------------------------------
GLenum InterfaceImplementation::getGLType(Interface::DATA_TYPES type)
{
  switch (type)
  {
    case Interface::TYPE_BYTE:        return GL_BYTE;
    case Interface::TYPE_UBYTE:       return GL_UNSIGNED_BYTE;
    case Interface::TYPE_SHORT:       return GL_SHORT;
    case Interface::TYPE_USHORT:      return GL_UNSIGNED_SHORT;
    case Interface::TYPE_INT:         return GL_INT;
    case Interface::TYPE_UINT:        return GL_UNSIGNED_INT;
    case Interface::TYPE_FLOAT:       return GL_FLOAT;
#ifdef SPIRE_OPENGL_ES_2
    case Interface::TYPE_HALFFLOAT:   return GL_HALF_FLOAT_OES;
#else
    case Interface::TYPE_HALFFLOAT:
      Log::error() << "Half-float not supported on non-ES platforms.";
      return GL_FLOAT;
#endif

    // Double type not sepported in OpenGL ES 2.0.
#ifndef SPIRE_OPENGL_ES_2
    case Interface::TYPE_DOUBLE:      return GL_DOUBLE;
#else
    case Interface::TYPE_DOUBLE:
      Log::error() << "Double type not supported on ES 2.0 platforms.";
      return GL_FLOAT;
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    default:
    {
      std::stringstream stream;
      stream << "Expected type to be one of Interface::DATA_TYPES, received " << type;
      throw std::invalid_argument(stream.str());
    }
#pragma clang diagnostic pop
  }

  return GL_FLOAT;
}

} // end of namespace Spire

