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
/// \date   January 2013

#ifndef SPIRE_HIGH_SHADERUNIFORMMAN_H
#define SPIRE_HIGH_SHADERUNIFORMMAN_H

#include "Common.h"
#include <vector>
#include <string>
#include <unordered_map>

#include "ShaderUniformStateManTemplates.h"

namespace Spire {

class ShaderUniformMan;

/// Holds information on one uniform.
struct UniformState
{
  std::string codeName;       ///< In-shader code name.
  GLenum      type;           ///< Type of the uniform. Used for type checking.
};

class ShaderUniformCollection
{
public:
  ShaderUniformCollection(ShaderUniformMan& man, GLuint program) :
      mUniformMan(man),
      mProgram(program)
  {}

  struct UniformSpecificData
  {
    std::shared_ptr<const UniformState> uniform;      ///< Uniform state.

    /// GL shader specific variables
    /// @{
    GLint     glUniformLoc; ///< Shader uniform location as returned by
                            ///< glGetUniformLocation.
    GLint     glSize;       ///< 'size' of the uniform variable.
    GLenum    glType;       ///< Type of the uniform variable (see: http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml)
                            ///< Should match type of UniformState exactly.
    /// @}
  };

  /// \todo Change back to constexpr
  static GLuint getInvalidProgramHandle()  {return static_cast<GLuint>(0);}

  /// Adds a uniform with the specified name.
  /// Also queries the OpenGL shader program for the position of the uniform.
  void addUniform(const std::string& uniformName);

  /// Retrieves number of uniforms stored in mUniforms.
  size_t getNumUniforms() const;

  /// Retrieves the uniform at 'index'.
  const UniformSpecificData& getUniformAtIndex(size_t index) const;

  /// Throws out_of_range exception if the uniform with 'uniformName' is
  /// not found in the list of uniforms.
  const UniformSpecificData& getUniformData(const std::string& uniformName) const;

  /// If 'uniformName' is contained herein, returns true.
  bool hasUniform(const std::string& uniformName) const;

private:

  /// Contains indices to uniforms in ShaderUniformMan, sorted (ascending).
  std::vector<UniformSpecificData>  mUniforms;

  /// Reference to the uniform manager.
  /// The uniform manager is queried regarding available uniforms.
  ShaderUniformMan&                 mUniformMan;

  /// GL shader program 
  GLuint                            mProgram;
};

/// Shader uniform manager.
class ShaderUniformMan
{
public:
  ShaderUniformMan();
  virtual ~ShaderUniformMan();

  /// These two functions represent the unknown's index and name.
  /// \todo Change back to constexpr
  /// @{
  static const char* getUnknownName()     {return "_unknown_";}
  /// @}

  /// Adds a new uniform to the system. Automatically assigns it an internal
  /// index based on when it was added.
  /// A ShaderUniformNotFound exception will be thrown if the uniform is not
  /// found in the ShaderUniformMan.
  /// \param codeName       Name of the uniform in the shader code.
  /// \param type           For a good description of all available types, see:
  ///                       http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml .
  void addUniform(const std::string& codeName, GLenum type);

  /// Retrieves uniform with name 'codeName'. If codeName is not found,
  /// std::out_of_range is thrown.
  std::shared_ptr<const UniformState> getUniformWithName(const std::string& codeName) const;

  /// Attempts to find the uniform with theh given codeName.
  std::shared_ptr<const UniformState> findUniformWithName(const std::string& codeName) const;

  /// Returns number of uniforms currently registered.
  size_t getNumUniforms()   {return mUniforms.size();}

  // Local function to convert Spire::UNIFORM_TYPE to GLenum.
  // This function does not exist in the header file because we don't want to
  // expose the GLenum type to an interface.
  static GLenum uniformTypeToGL(UNIFORM_TYPE type);

private:

  /// Array of available uniforms.
  std::unordered_map<std::string, std::shared_ptr<UniformState>>   mUniforms;

};

} // namespace Spire

#endif 

