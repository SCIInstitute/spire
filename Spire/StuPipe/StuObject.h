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
#include "StuPipe/StuInterface.h"

namespace Spire {

//------------------------------------------------------------------------------
// VBO object
//------------------------------------------------------------------------------
/// Object that encapsulates an OpenGL vertex buffer. The buffer will be
/// automatically deleted by VBOObject's destructor.
class VBOObject
{
public:
  VBOObject(std::shared_ptr<std::vector<uint8_t>> vboData,
            const std::vector<std::string>& attributes);
  ~VBOObject();

  GLuint getGLIndex() const                             {return mGLIndex;}
  const std::vector<std::string>& getAttributes() const {return mAttributes;}

private:
  GLuint                    mGLIndex;    ///< Corresponds to the map index but obtained from OpenGL.
  std::vector<std::string>  mAttributes; ///< Attributes for shader verification.
};

//------------------------------------------------------------------------------
// IBO object
//------------------------------------------------------------------------------
/// Object that encapsulates an OpenGL index buffer. The buffer will be
/// automatically deleted by IBOObject's destructor.
class IBOObject
{
public:
  IBOObject(std::shared_ptr<std::vector<uint8_t>> iboData,
            StuInterface::IBO_TYPE type);
  ~IBOObject();

  GLuint                    glIndex;    ///< Corresponds to the map index but obtained from OpenGL.
  StuInterface::IBO_TYPE    type;       ///< Type of index buffer.
};

//------------------------------------------------------------------------------
// StuPass object
//------------------------------------------------------------------------------
/// Holds all information regarding specific uniforms for use in the pass
/// and also the GL indices of the VBO / IBO to use.
class StuPass
{
public:
  StuPass(const std::string& objectName, const std::string& programName,
            size_t vboID, size_t iboID);
  virtual ~StuPass();
  
protected:

  /// List of uniforms to apply before this shader gets executed.
  std::list<std::unique_ptr<AbstractUniformStateItem>>  mUniforms;

  /// List of unsatisfied uniforms (the list of uniforms that are not covered
  /// by our mUniforms list).
  /// The set of unsatisfied uniforms should be a subset of the global
  /// uniform state. Otherwise the shader cannot be properly satisfied and a
  /// runtime exception will be thrown.
  std::list<std::string>                mUnsatisfiedUniforms;

  const VBOObject&                      mVBO;     ///< VBO reference from the STU object which composites us.
  const IBOObject&                      mIBO;     ///< IBO reference from the STU object which composites us.

  std::shared_ptr<ShaderProgramAsset>   mShader;  ///< Shader to be used when rendering this pass.
};

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------
class StuObject
{
public:

  /// Adds an object specific VBO. See StuInferface.
  size_t addVBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                const std::vector<std::string>& attribNames);

  /// Adds an object specific IBO. See StuInferface.
  size_t addIBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                StuInterface::IBO_TYPE type);

  /// Adds a geometry pass with the specified index / vertex buffer objects.
  void addPass(const std::string& pass,
               const std::string& program,
               size_t vboID,
               size_t iboID);

  /// Removes a geometry pass from the object.
  void removePass(const std::string& pass);

  /// Adds a uniform to the pass.
  void addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::unique_ptr<AbstractUniformStateItem>&& item);

protected:

  /// All registered passes.
  std::unordered_map<std::string, StuPass>  mPasses;

  // These maps may actually be more efficient implemented as an array. The map 
  // sizes are small and cache coherency will be more important. Ignoring for 
  // now until we identify an actual performance bottlenecks.
  std::map<size_t, VBOObject>               mVBOMap;    ///< OpenGL index -> VBOObject map.
  std::map<size_t, IBOObject>               mIBOMap;    ///< OpenGL index -> IBOObject map.
};


} // namespace Spire 

#endif 
