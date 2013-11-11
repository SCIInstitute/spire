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

#ifndef SPIRE_HIGH_INTERFACEIMPLEMENTATION_H
#define SPIRE_HIGH_INTERFACEIMPLEMENTATION_H

#include <memory>
#include <utility>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <map>
#include <tuple>
#include <cstdint>
#include "Common.h"

#include "ObjectLambda.h"
#include "ThreadMessage.h"

#ifdef SPIRE_USE_STD_THREADS
#include "CircFIFOSeqCons.hpp"
#endif

namespace CPM_SPIRE_NS {

class Hub;
class SpireObject;
class ShaderProgramAsset;
class VBOObject;
class IBOObject;

/// Implementation of the functions exposed in Interface.h
/// All functions in this class are not thread safe.
class InterfaceImplementation
{
public:
  InterfaceImplementation(Hub& hub);
  virtual ~InterfaceImplementation()  {}
  
  //============================================================================
  // IMPLEMENTATION
  //============================================================================
  /// Cleans up all GL resources.
  void clearGLResources();

  /// Performs all rendering passes.
  void doAllPasses();

  /// Performs a rendering pass.
  void doPass(const std::string& pass);

  /// Retrieves number of objects.
  size_t getNumObjects()      {return mNameToObject.size();}

  /// Returns true if the pass already exists.
  bool hasPass(const std::string& pass) const;

  /// Retrieves the object with the specified name.
  std::shared_ptr<SpireObject> getObjectWithName(const std::string& name) const;

  /// Returns true if the object is in the given pass.
  bool isObjectInPass(const std::string& object, const std::string& pass) const;

  /// Retrieves appropriate primitive type GLenum from Interface primitives.
  static GLenum getGLPrimitive(Interface::PRIMITIVE_TYPES type);

  /// Retrieve gl type from Interface::DATA_TYPES.
  static GLenum getGLType(Interface::DATA_TYPES type);

  void addConcurrentVBO(const std::string& vboName,
                        const uint8_t* vboData, size_t vboSize,
                        const std::vector<std::string>& attribNames);

  void addConcurrentIBO(const std::string& iboName,
                        const uint8_t* iboData, size_t iboSize,
                        Interface::IBO_TYPE type);

  //============================================================================
  // CALLBACK IMPLEMENTATION -- Called from interface or a derived class.
  //============================================================================

  //--------
  // Passes
  //--------

  void addPassToFront(std::string passName);
  void addPassToBack(std::string passName);

  //---------
  // Objects
  //---------

  void addObject(std::string objectName);
  void removeObject(std::string objectName);
  void removeAllObjects();
  void addVBO(std::string vboName,
              std::shared_ptr<std::vector<uint8_t>> vboData,
              std::vector<std::string> attribNames);
  void removeVBO(std::string vboName);
  void addIBO(std::string iboName,
                     std::shared_ptr<std::vector<uint8_t>> iboData,
                     Interface::IBO_TYPE type);
  void removeIBO(std::string iboName);
  void addPassToObject(std::string object,
                              std::string program, std::string vboName, 
                              std::string iboName, Interface::PRIMITIVE_TYPES type,
                              std::string pass, std::string parentPass);
  void removePassFromObject(std::string object,
                                   std::string pass);

  //----------
  // Uniforms
  //----------
  void addObjectPassUniformConcrete(std::string object, std::string uniformName,
                                    std::shared_ptr<AbstractUniformStateItem> item,
                                    std::string pass);
  void addObjectGlobalUniformConcrete(std::string object,
                                      std::string uniformName,
                                      std::shared_ptr<AbstractUniformStateItem> item);
  void addGlobalUniformConcrete( std::string uniformName,
                                std::shared_ptr<AbstractUniformStateItem> item);
  void addObjectPassGPUState(std::string object,
                             GPUState state, std::string pass);

  //-------------------
  // Shader Attributes
  //-------------------

  // Attributes just as they are in the OpenGL rendering pipeline.
  void addShaderAttribute(std::string codeName, size_t numComponents,
                          bool normalize, size_t size, Interface::DATA_TYPES t);

  //-----------------
  // Object metadata
  //-----------------
  void addObjectGlobalMetadataConcrete(std::string object,
                                       std::string attributeName,
                                       std::shared_ptr<AbstractUniformStateItem> item);
  void addObjectPassMetadataConcrete(std::string object,
                                     std::string attributeName,
                                     std::shared_ptr<AbstractUniformStateItem> item,
                                     std::string passName);
  //-----------------
  // Shader Programs
  //-----------------

  void addPersistentShader(std::string programName,
                           std::vector<std::tuple<std::string, Interface::SHADER_TYPES>> tempShaders);

  //---------
  // Lambdas
  //---------
  void addLambdaBeginAllPasses(Interface::PassLambdaFunction fp);
  void addLambdaEndAllPasses(Interface::PassLambdaFunction fp);
  void addLambdaPrePass(Interface::PassLambdaFunction fp, std::string pass);
  void addLambdaPostPass(Interface::PassLambdaFunction fp, std::string pass);
  void addLambdaObjectRender(std::string object, Interface::ObjectLambdaFunction fp, std::string pass);
  void addLambdaObjectUniforms(std::string object, Interface::ObjectUniformLambdaFunction fp, std::string pass);

private:

  struct Pass
  {
    Pass(const std::string& name) :
        mName(name)
    {}

    std::string                                                     mName;
    std::unordered_map<std::string, std::shared_ptr<SpireObject>>   mNameToObject;

    std::vector<Interface::PassLambdaFunction>                      mPassBeginLambdas;
    std::vector<Interface::PassLambdaFunction>                      mPassEndLambdas;

    /// \todo Rendering order for the objects?
  };

  /// This unordered map is a 1-1 mapping of object names onto objects.
  std::unordered_map<std::string, std::shared_ptr<SpireObject>>   mNameToObject;

  /// List of shaders that are stored persistently by this pipe (will never
  /// be GC'ed unless this pipe is destroyed).
  std::list<std::shared_ptr<ShaderProgramAsset>>                  mPersistentShaders;

  /// VBO names to our representation of a vertex buffer object.
  std::unordered_map<std::string, std::shared_ptr<VBOObject>>     mVBOMap;

  /// IBO names to our representation of an index buffer object.
  std::unordered_map<std::string, std::shared_ptr<IBOObject>>     mIBOMap;

  /// List of passes in the order they are meant to be rendered.
  std::list<std::shared_ptr<Pass>>                                mPasses;
  std::unordered_map<std::string, std::shared_ptr<Pass>>          mNameToPass;

  /// Global begin/end lambdas.
  /// @{
  std::vector<Interface::PassLambdaFunction>                      mGlobalBeginLambdas;
  std::vector<Interface::PassLambdaFunction>                      mGlobalEndLambdas;
  /// @}

private:

  Hub&            mHub;
};

} // namespace CPM_SPIRE_NS

#endif 
