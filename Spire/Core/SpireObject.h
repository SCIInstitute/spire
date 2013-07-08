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

#ifndef SPIRE_STUPIPE_STUOBJECT_H
#define SPIRE_STUPIPE_STUOBJECT_H

#include <string>
#include <list>
#include <map>

#include "Common.h"
#include "ShaderProgramMan.h"
#include "ShaderUniformStateManTemplates.h"

#include "StuVBOObject.h"
#include "StuIBOObject.h"

namespace Spire {

//------------------------------------------------------------------------------
// StuPass object
//------------------------------------------------------------------------------
/// Holds all information regarding specific uniforms for use in the pass
/// and also the GL indices of the VBO / IBO to use.
class StuPass
{
public:
  StuPass(
      Hub& hub,
      const std::string& passName, const std::string& programName, int32_t passOrder,
      std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo, GLenum primitiveType);
  virtual ~StuPass();
  
  void renderPass(ObjectLambdaInterface& lambdaInterface);

  const std::string& getName() const    {return mName;}
  int32_t getPassOrder() const          {return mPassOrder;}
  GLenum getPrimitiveType() const       {return mPrimitiveType;}

  /// Adds a local uniform to the pass.
  /// throws std::out_of_range if 'uniformName' is not found in the shader's
  /// uniform list.
  bool addPassUniform(const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item,
                      bool isObjectGlobalUniform);

  void addGPUState(const GPUState& state);

  /// This function will *not* return true if the uniform was added via the
  /// global object uniforms.
  bool hasPassSpecificUniform(const std::string& uniformName) const;

  /// Unlike the function above, this will return true whether or not object
  /// global uniforms were used to populate the uniform.
  bool hasUniform(const std::string& uniformName) const;

  /// Adds a pass spire attribute.
  void addSpireAttribute(const std::string& attributeName,
                         std::shared_ptr<AbstractUniformStateItem> item);

  /// Retrieves a pass spire attribute.
  /// Returns an empty shared_ptr if no such attribute is found.
  std::shared_ptr<const AbstractUniformStateItem> getSpireAttribute(
      const std::string& attribName) const;

  /// Add render lambda.
  void addRenderLambda(const StuInterface::ObjectLambdaFunction& fp);

  /// Add uniform lambda.
  void addUniformLambda(const StuInterface::ObjectUniformLambdaFunction& fp);

protected:

  struct UniformItem
  {
    UniformItem(const std::string& name,
                std::shared_ptr<AbstractUniformStateItem> uniformItem,
                GLint location, bool passSpecific) :
        uniformName(name),
        item(uniformItem),
        shaderLocation(location),
        passSpecific(passSpecific)
    {}

    std::string                               uniformName;
    std::shared_ptr<AbstractUniformStateItem> item;
    GLint                                     shaderLocation;
    bool                                      passSpecific;   ///< If true, global uniforms do not overwrite.
  };

  struct UnsastisfiedUniformItem
  {
    UnsastisfiedUniformItem(const std::string& name,
                            GLint location,
                            GLenum type) :
        uniformName(name),
        uniformType(type),
        shaderLocation(location)
    {}

    std::string                         uniformName;
    GLenum                              uniformType;
    GLint                               shaderLocation;
  };

//  struct ObjectTransformUniform
//  {
//    enum ObjectTransformType
//    {
//      TRANSFORM_OBJECT,
//      TRANSFORM_OBJECT_TO_CAMERA,
//      TRANSFORM_OBJECT_TO_CAMERA_TO_PROJECTION,
//    };
//
//    ObjectTransformUniform(ObjectTransformType type, GLint shaderVarLocation) :
//        transformType(type),
//        varLocation(shaderVarLocation)  
//    {}
//
//    ObjectTransformType transformType;
//    GLint               varLocation;
//  };


  std::string                           mName;      ///< Simple pass name.
  int32_t                               mPassOrder; ///< Pass order.
  GLenum                                mPrimitiveType;

  /// List of unsatisfied uniforms (the list of uniforms that are not covered
  /// by our mUniforms list).
  /// The set of unsatisfied uniforms should be a subset of the global
  /// uniform state. Otherwise the shader cannot be properly satisfied and a
  /// runtime exception will be thrown.
  /// This list is updated everytime we add or remove elements from mUniforms.
  std::vector<UnsastisfiedUniformItem>  mUnsatisfiedUniforms;
  std::vector<UniformItem>              mUniforms;  ///< Local uniforms
  //std::vector<ObjectTransformUniform>   mObjectTransformUniforms;

  std::shared_ptr<VBOObject>            mVBO;     ///< ID of VBO to use during pass.
  std::shared_ptr<IBOObject>            mIBO;     ///< ID of IBO to use during pass.

  std::shared_ptr<ShaderProgramAsset>   mShader;  ///< Shader to be used when rendering this pass.

  /// \todo I'm using a unique_ptr like 'Maybe' is used in haskell. Look into a
  ///       better representation in C++.
  std::unique_ptr<GPUState>             mGPUState; ///< GPU state to set (if any, default is none).

  std::unordered_map<std::string, std::shared_ptr<AbstractUniformStateItem>> mSpireAttributes;

  Hub&                                  mHub;     ///< Hub.

  /// Lambda callbacks.
  std::vector<StuInterface::ObjectUniformLambdaFunction> mUniformLambdas;
  std::vector<StuInterface::ObjectLambdaFunction>        mRenderLambdas;
};

//------------------------------------------------------------------------------
// SpireObject
//------------------------------------------------------------------------------
class SpireObject
{
public:

  SpireObject(Hub& hub, const std::string& name, int32_t renderOrder);

  std::string getName() const     {return mName;}
  int32_t getRenderOrder() const  {return mRenderOrder;}

  /// Set new rendering order.
  void setRenderOrder(int32_t renderOrder) {mRenderOrder = renderOrder;}

  /// Adds a geometry pass with the specified index / vertex buffer objects.
  void addPass(const std::string& pass,
               const std::string& program,
               std::shared_ptr<VBOObject> vbo,
               std::shared_ptr<IBOObject> ibo,
               GLenum primType,
               int32_t passOrder);

  /// \note If we add ability to remove IBOs and VBOs, the IBOs and VBOs will
  ///       not be removed until their corresponding passes are removed
  ///       as well due to the shared_ptr.

  /// Removes a geometry pass from the object.
  void removePass(const std::string& pass);

  // The precedence for uniforms goes: pass -> uniform -> global.
  // So pass is checked first, then the uniform level of uniforms, then the
  // global level of uniforms.
  // Currently the only 'pass' and 'global' are implemented.

  /// Adds a uniform to the pass.
  void addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item);

  /// Adds a uniform to the pass.
  void addGlobalUniform(const std::string& uniformName,
                        std::shared_ptr<AbstractUniformStateItem> item);

  /// Adds an object attribute to the system. Object attributes do not change
  /// per-pass. Add another function 'addSpireObjectPassAttribute' if that is
  /// really needed.
  void addObjectGlobalSpireAttribute(const std::string& attributeName,
                                     std::shared_ptr<AbstractUniformStateItem> item);

  /// Retrieves a spire attribute. Can be used in the Lambda callbacks for
  /// rendering.
  /// Returns an empty shared_ptr if no such attribute is found.
  std::shared_ptr<const AbstractUniformStateItem> getObjectGlobalSpireAttribute(
      const std::string& attribName) const;

  void addObjectPassSpireAttribute(const std::string& passName,
                                   const std::string& attributeName,
                                   std::shared_ptr<AbstractUniformStateItem> item);

  std::shared_ptr<const AbstractUniformStateItem> getObjectPassSpireAttribute(
      const std::string& passName,
      const std::string& attribName) const;

  /// Adds an object -> world transformation to this object.
  /// This transform is passed to the pass before rendering occurs.
  void addObjectTransform(const M44& transform);

  /// Add GPU state to the pass.
  void addPassGPUState(const std::string& pass,
                       const GPUState& state);

  bool hasPassRenderingOrder(const std::vector<std::string>& passes) const;

  /// Renders all passes associated with this object.
  void renderAllPasses();

  /// \todo Ability to render a single named pass. See github issue #15.
  void renderPass(const std::string& pass);

  /// Returns the associated pass. Otherwise an empty shared_ptr is returned.
  std::shared_ptr<const StuPass> getObjectPassParams(const std::string& passName) const;

  /// Returns the number of registered passes.
  size_t getNumPasses() const {return mPasses.size();}

  /// Returns true if there exists a object global uniform with the name
  /// 'uniformName'.
  bool hasGlobalUniform(const std::string& uniformName) const;

  /// Adds a render lambda to the given pass.
  void addPassRenderLambda(const std::string& pass, const StuInterface::ObjectLambdaFunction& fp);

  /// Adds a uniform lambda to the given pass.
  void addPassUniformLambda(const std::string& pass, const StuInterface::ObjectUniformLambdaFunction& fp);

protected:

  struct ObjectGlobalUniformItem
  {
    ObjectGlobalUniformItem(const std::string& name,
                            std::shared_ptr<AbstractUniformStateItem> uniformItem) :
        uniformName(name),
        item(uniformItem)
    {}

    std::string                               uniformName;
    std::shared_ptr<AbstractUniformStateItem> item;
  };

  void removePassFromOrderList(const std::string& pass, int32_t passOrder);

  /// Retrieves the pass by name.
  std::shared_ptr<StuPass> getPassByName(const std::string& name) const;

  /// All registered passes.
  std::unordered_map<std::string, std::shared_ptr<StuPass>>   mPasses;
  std::map<int32_t, std::shared_ptr<StuPass>>                 mPassRenderOrder;
  std::vector<ObjectGlobalUniformItem>                        mObjectGlobalUniforms;
  std::unordered_map<std::string, std::shared_ptr<AbstractUniformStateItem>> mSpireAttributes;

  // These maps may actually be more efficient implemented as an array. The map 
  // sizes are small and cache coherency will be more important. Ignoring for 
  // now until we identify an actual performance bottlenecks.
  // size_t represents a std::hash of a string.
  std::hash<std::string>                        mHashFun;

  std::string                                   mName;
  int32_t                                       mRenderOrder;

  /// Object -> world transform.
  M44                                           mObjectTransform;

  Hub&                                          mHub;
};


} // namespace Spire 

#endif 
