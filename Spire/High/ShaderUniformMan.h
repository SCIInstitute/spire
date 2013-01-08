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

namespace Spire
{

class ShaderUniformMan;

/// Holds information on one uniform.
struct UniformState
{
  size_t      index;          ///< Arbitrarily assigned index.
  std::string codeName;       ///< In-shader code name.
  uint32_t    nameHash;       ///< Hash of 'codeName'.
};

class ShaderUniformCollection
{
public:
  ShaderUniformCollection(const ShaderUniformMan& man) :
      mUniformMan(man),
      mProgram(getInvalidProgramHandle())
  {}

  struct UniformSpecificData
  {
    GLuint        uniformLoc; ///< Shader uniform location as returned by
                              ///< glGetUniformLocation.
    UniformState  uniform;    ///< Uniform state.
  };

  static constexpr GLuint getInvalidProgramHandle()  {return static_cast<GLuint>(0);}

  /// Sets the shader program to check the uniforms against.
  void setShaderProgram(GLuint program);

  /// Adds a uniform with the specified name.
  /// Also queries the OpenGL shader program for the position of the uniform.
  void addUniform(const std::string& uniformName);

  /// Retrieves the uniform at 'index' from ShaderUniformMan.
  UniformSpecificData getUniform(size_t index) const;

  /// Retrieves number of uniforms stored in mUniforms.
  size_t getNumUniforms() const;

  /// If 'uniformName' is contained herein, returns true.
  bool hasUniform(const std::string& uniformName) const;

private:

  /// Returns true if the uniform array contains a reference to 'index'.
  /// This is the index into the array in ShaderUniformMan.
  bool hasIndex(size_t targetIndex) const;

  /// Contains indices to uniforms in ShaderUniformMan, sorted (ascending).
  std::vector<UniformSpecificData>  mUniforms;

  /// Reference to the uniform manager.
  /// The uniform manager is queried regarding available uniforms.
  const ShaderUniformMan&           mUniformMan;

  /// GL shader program 
  GLuint                            mProgram;
};

/// Shader uniform manager.
class ShaderUniformMan
{
public:
  ShaderUniformMan(bool addDefaultUniforms = true);
  virtual ~ShaderUniformMan();

  /// Seed value to use when hashing strings for comparison purposes.
  static constexpr uint32_t getMurmurSeedValue()    {return 0x83abd272;}

  /// These two functions represent the unknown's index and name.
  /// @{
  static constexpr size_t getUnknownUniformIndex()  {return 0;}
  static constexpr const char* getUnknownName()     {return "_unknown_";}
  /// @}

  /// Adds a new uniform to the system. Automatically assigns it an internal
  /// index based on when it was added.
  /// A ShaderUniformNotFound exception will be thrown if the uniform is not
  /// found in the ShaderUniformMan.
  /// \param codeName       Name of the uniform in the shader code.
  void addUniform(const std::string& codeName);

  /// Returns the index associated with the uniform whose name is 'codeName'.
  /// \return the first tuple parameter (bool) indicates whether or not an 
  ///         uniform with 'codeName' was found. The second tuple parameter
  ///         returns the index of the uniform, if found. If it is not found
  ///         then size_t will be NULL.
  std::tuple<bool,size_t> findUniformWithName(const std::string& codeName) const;

  /// Same as findUniformWithName, except an exception is thrown instead
  /// of returning false when an uniform with the specified name does not exist.
  UniformState getUniformWithName(const std::string& codeName) const;

  /// Returns the attibute at the specified index.
  /// If no uniform is found at 'index', or if the 'index' is invalid, an
  /// exception is thrown.
  UniformState getUniformAtIndex(size_t index) const;

  /// Returns the number of uniforms currently in the uniform array.
  /// Will always be >= 1, because the unknown uniform is always the first
  /// uniform in the array.
  size_t getNumUniforms() const   {return mUniforms.size();}

  /// Hashes 'str' into a uint32_t using murmur hash.
  /// Uses MURMUR_SEED_VALUE.
  static uint32_t hashString(const std::string& str);

private:

  /// Array of available uniforms.
  std::vector<UniformState>    mUniforms;

};

} // namespace Spire

#endif 

