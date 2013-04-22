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

#include "Common.h"
#include "Exceptions.h"

#include "Core/ShaderUniformMan.h"
#include "Core/MurmurHash3.h"

namespace Spire {

//------------------------------------------------------------------------------
ShaderUniformMan::ShaderUniformMan()
{
  addUniform(getUnknownName(), GL_FLOAT);
}

//------------------------------------------------------------------------------
ShaderUniformMan::~ShaderUniformMan()
{
}

//------------------------------------------------------------------------------
void ShaderUniformMan::addUniform(const std::string& codeName, GLenum type)
{
  std::shared_ptr<UniformState> uniform(new UniformState);
  uniform->codeName  = codeName;
  uniform->type      = type;

  mUniforms.insert(std::make_pair(codeName, uniform));
}

//------------------------------------------------------------------------------
std::shared_ptr<const UniformState>
ShaderUniformMan::getUniformWithName(const std::string& codeName) const
{
  return mUniforms.at(codeName);
}

//------------------------------------------------------------------------------
// Shader Uniform Collection
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void ShaderUniformCollection::addUniform(const std::string& uniformName)
{
  UniformSpecificData uniformData;
  if (getInvalidProgramHandle() != mProgram)
  {
    // Search for the appropriate uniform.
    bool foundActiveUniform = false;
    int total = -1;
    glGetProgramiv( mProgram, GL_ACTIVE_UNIFORMS, &total ); 
    for (int i = 0; i < total; i++)
    {
      // Now query even more data about the uniform.
      const GLsizei nameSize = 128;
      GLsizei bytesWritten = 0;
      char uniformName_ignore[nameSize]; // Name which we already know..
      GL(glGetActiveUniform(mProgram, i,
                            nameSize, &bytesWritten,
                            &uniformData.glSize, &uniformData.glType,
                            uniformName_ignore));
      
      std::string activeUniformName = uniformName_ignore;
      if (activeUniformName == uniformName)
      {
        // If we get here, we have populated glSize and glType with the correct data.
        foundActiveUniform = true;
        break;
      }
    }

    if (foundActiveUniform == false)
    {
      Log::error() << "Could not find active uniform with name: " << uniformName << "!";
      throw GLError("Active uniform not found.");
    }

    // Set gl uniform location (this is NOT the same as the active uniform location!)
    uniformData.glUniformLoc = glGetUniformLocation(mProgram, uniformName.c_str());
    GL_CHECK();

    // std::out_of_range will be thrown here if uniform is not found.
    std::shared_ptr<const UniformState> state;
    try
    {
      state = mUniformMan.getUniformWithName(uniformName);
    }
    catch (std::out_of_range&)
    {
      // By default, add the uniform with the shader's type to the uniform
      // registry.
      mUniformMan.addUniform(uniformName, uniformData.glType);
      state = mUniformMan.getUniformWithName(uniformName);
    }
    uniformData.uniform = state;

    // Perform a type check against uniform type.
    if (state->type != uniformData.glType)
      throw ShaderUniformTypeError("Uniform types do not match!");

    GL_CHECK();
  }
  else
  {
    throw GLError("A valid shader program has not been associated with this "
                  "uniform collection.");
  }
  mUniforms.push_back(uniformData);
}

//------------------------------------------------------------------------------
bool ShaderUniformCollection::hasUniform(const std::string& uniformName) const
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    std::shared_ptr<const UniformState> state = it->uniform;
    if (state->codeName == uniformName)
    {
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------------------
const ShaderUniformCollection::UniformSpecificData&
ShaderUniformCollection::getUniformData(const std::string& uniformName) const
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    std::shared_ptr<const UniformState> state = it->uniform;
    if (state->codeName == uniformName)
    {
      return *it;
    }
  }

  throw std::out_of_range("Unable to find uniform with name specified.");
}

//------------------------------------------------------------------------------
size_t ShaderUniformCollection::getNumUniforms() const
{
  return mUniforms.size();
}

//------------------------------------------------------------------------------
const ShaderUniformCollection::UniformSpecificData&
ShaderUniformCollection::getUniformAtIndex(size_t index) const
{
  return mUniforms[index];
}

} // end of namespace Spire
