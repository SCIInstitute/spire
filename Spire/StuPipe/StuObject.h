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
#include "Core/ShaderProgramMan.h"
#include "Core/ShaderUniformStateManTemplates.h"

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
  
  void renderPass();

  const std::string& getName() const    {return mName;}
  int32_t getPassOrder() const          {return mPassOrder;}
  GLenum getPrimitiveType() const       {return mPrimitiveType;}

protected:

  std::string                           mName;      ///< Simple pass name.
  int32_t                               mPassOrder; ///< Pass order.
  GLenum                                mPrimitiveType;

  /// List of uniforms to apply before this shader gets executed.
  std::vector<std::unique_ptr<AbstractUniformStateItem>>  mUniforms;

  /// List of unsatisfied uniforms (the list of uniforms that are not covered
  /// by our mUniforms list).
  /// The set of unsatisfied uniforms should be a subset of the global
  /// uniform state. Otherwise the shader cannot be properly satisfied and a
  /// runtime exception will be thrown.
  std::list<std::string>                mUnsatisfiedUniforms;

  std::shared_ptr<VBOObject>            mVBO;     ///< ID of VBO to use during pass.
  std::shared_ptr<IBOObject>            mIBO;     ///< ID of IBO to use during pass.

  std::shared_ptr<ShaderProgramAsset>   mShader;  ///< Shader to be used when rendering this pass.

  Hub&                                  mHub;
};

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------
class StuObject
{
public:

  StuObject(Hub& hub, const std::string& name, int32_t renderOrder);

  std::string getName() const     {return mName;}
  int32_t getRenderOrder() const  {return mRenderOrder;}

  /// Set new rendering order.
  void setRenderOrder(int32_t renderOrder) {mRenderOrder = renderOrder;}

  /// \note If we add ability to remove IBOs and VBOs, the IBOs and VBOs will
  ///       not be removed until their corresponding passes are removed
  ///       as well due to the shared_ptr.

  /// Adds a geometry pass with the specified index / vertex buffer objects.
  void addPass(const std::string& pass,
               const std::string& program,
               std::shared_ptr<VBOObject> vbo,
               std::shared_ptr<IBOObject> ibo,
               GLenum primType,
               int32_t passOrder);


  /// Removes a geometry pass from the object.
  void removePass(const std::string& pass);

  /// Adds a uniform to the pass.
  void addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item);

  bool hasPassRenderingOrder(const std::vector<std::string>& passes) const;

  /// Renders all passes associated with this object.
  void renderAllPasses();

protected:

  void removePassFromOrderList(const std::string& pass, int32_t passOrder);

  /// Retrieves the pass by name.
  std::shared_ptr<StuPass> getPassByName(const std::string& name);

  /// All registered passes.
  std::unordered_map<std::string, std::shared_ptr<StuPass>>   mPasses;
  std::map<int32_t, std::shared_ptr<StuPass>>                 mPassRenderOrder;

  // These maps may actually be more efficient implemented as an array. The map 
  // sizes are small and cache coherency will be more important. Ignoring for 
  // now until we identify an actual performance bottlenecks.
  // size_t represents a std::hash of a string.
  std::hash<std::string>                        mHashFun;

  std::string                                   mName;
  int32_t                                       mRenderOrder;

  Hub&                                          mHub;
};


} // namespace Spire 

#endif 
