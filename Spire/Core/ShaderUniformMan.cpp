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
std::shared_ptr<const UniformState>
ShaderUniformMan::findUniformWithName(const std::string& codeName) const
{
  auto it = mUniforms.find(codeName);
  if (it != mUniforms.end())
    return (*it).second;
  else
    return std::shared_ptr<const UniformState>();
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

    std::shared_ptr<const UniformState> state = mUniformMan.findUniformWithName(uniformName);
    if (state == nullptr)
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

//------------------------------------------------------------------------------
GLenum ShaderUniformMan::uniformTypeToGL(UNIFORM_TYPE type)
{
  // Vim'd
  switch (type)
  {
    case UNIFORM_FLOAT:
      return GL_FLOAT; 
      break;

    case UNIFORM_FLOAT_VEC2:
      return GL_FLOAT_VEC2; 
      break;

    case UNIFORM_FLOAT_VEC3:
      return GL_FLOAT_VEC3; 
      break;

    case UNIFORM_FLOAT_VEC4:
      return GL_FLOAT_VEC4; 
      break;

    case UNIFORM_DOUBLE:
      return GL_DOUBLE; 
      break;

    case UNIFORM_DOUBLE_VEC2:
      return GL_DOUBLE_VEC2; 
      break;

    case UNIFORM_DOUBLE_VEC3:
      return GL_DOUBLE_VEC3; 
      break;

    case UNIFORM_DOUBLE_VEC4:
      return GL_DOUBLE_VEC4; 
      break;

    case UNIFORM_INT:
      return GL_INT; 
      break;

    case UNIFORM_INT_VEC2:
      return GL_INT_VEC2; 
      break;

    case UNIFORM_INT_VEC3:
      return GL_INT_VEC3; 
      break;

    case UNIFORM_INT_VEC4:
      return GL_INT_VEC4; 
      break;

    case UNIFORM_UNSIGNED_INT:
      return GL_UNSIGNED_INT; 
      break;

    case UNIFORM_UNSIGNED_INT_VEC2:
      return GL_UNSIGNED_INT_VEC2; 
      break;

    case UNIFORM_UNSIGNED_INT_VEC3:
      return GL_UNSIGNED_INT_VEC3; 
      break;

    case UNIFORM_UNSIGNED_INT_VEC4:
      return GL_UNSIGNED_INT_VEC4; 
      break;

    case UNIFORM_BOOL:
      return GL_BOOL; 
      break;

    case UNIFORM_BOOL_VEC2:
      return GL_BOOL_VEC2; 
      break;

    case UNIFORM_BOOL_VEC3:
      return GL_BOOL_VEC3; 
      break;

    case UNIFORM_BOOL_VEC4:
      return GL_BOOL_VEC4; 
      break;

    case UNIFORM_FLOAT_MAT2:
      return GL_FLOAT_MAT2; 
      break;

    case UNIFORM_FLOAT_MAT3:
      return GL_FLOAT_MAT3; 
      break;

    case UNIFORM_FLOAT_MAT4:
      return GL_FLOAT_MAT4; 
      break;

    case UNIFORM_FLOAT_MAT2x3:
      return GL_FLOAT_MAT2x3; 
      break;

    case UNIFORM_FLOAT_MAT2x4:
      return GL_FLOAT_MAT2x4; 
      break;

    case UNIFORM_FLOAT_MAT3x2:
      return GL_FLOAT_MAT3x2; 
      break;

    case UNIFORM_FLOAT_MAT3x4:
      return GL_FLOAT_MAT3x4; 
      break;

    case UNIFORM_FLOAT_MAT4x2:
      return GL_FLOAT_MAT4x2; 
      break;

    case UNIFORM_FLOAT_MAT4x3:
      return GL_FLOAT_MAT4x3; 
      break;

    case UNIFORM_DOUBLE_MAT2:
      return GL_DOUBLE_MAT2; 
      break;

    case UNIFORM_DOUBLE_MAT3:
      return GL_DOUBLE_MAT3; 
      break;

    case UNIFORM_DOUBLE_MAT4:
      return GL_DOUBLE_MAT4; 
      break;

    case UNIFORM_DOUBLE_MAT2x3:
      return GL_DOUBLE_MAT2x3; 
      break;

    case UNIFORM_DOUBLE_MAT2x4:
      return GL_DOUBLE_MAT2x4; 
      break;

    case UNIFORM_DOUBLE_MAT3x2:
      return GL_DOUBLE_MAT3x2; 
      break;

    case UNIFORM_DOUBLE_MAT3x4:
      return GL_DOUBLE_MAT3x4; 
      break;

    case UNIFORM_DOUBLE_MAT4x2:
      return GL_DOUBLE_MAT4x2; 
      break;

    case UNIFORM_DOUBLE_MAT4x3:
      return GL_DOUBLE_MAT4x3; 
      break;

    case UNIFORM_SAMPLER_1D:
      return GL_SAMPLER_1D; 
      break;

    case UNIFORM_SAMPLER_2D:
      return GL_SAMPLER_2D; 
      break;

    case UNIFORM_SAMPLER_3D:
      return GL_SAMPLER_3D; 
      break;

    case UNIFORM_SAMPLER_CUBE:
      return GL_SAMPLER_CUBE; 
      break;

    case UNIFORM_SAMPLER_1D_SHADOW:
      return GL_SAMPLER_1D_SHADOW; 
      break;

    case UNIFORM_SAMPLER_2D_SHADOW:
      return GL_SAMPLER_2D_SHADOW; 
      break;

    case UNIFORM_SAMPLER_1D_ARRAY:
      return GL_SAMPLER_1D_ARRAY; 
      break;

    case UNIFORM_SAMPLER_2D_ARRAY:
      return GL_SAMPLER_2D_ARRAY; 
      break;

    case UNIFORM_SAMPLER_1D_ARRAY_SHADOW:
      return GL_SAMPLER_1D_ARRAY_SHADOW; 
      break;

    case UNIFORM_SAMPLER_2D_ARRAY_SHADOW:
      return GL_SAMPLER_2D_ARRAY_SHADOW; 
      break;

    case UNIFORM_SAMPLER_2D_MULTISAMPLE:
      return GL_SAMPLER_2D_MULTISAMPLE; 
      break;

    case UNIFORM_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return GL_SAMPLER_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_SAMPLER_CUBE_SHADOW:
      return GL_SAMPLER_CUBE_SHADOW; 
      break;

    case UNIFORM_SAMPLER_BUFFER:
      return GL_SAMPLER_BUFFER; 
      break;

    case UNIFORM_SAMPLER_2D_RECT:
      return GL_SAMPLER_2D_RECT; 
      break;

    case UNIFORM_SAMPLER_2D_RECT_SHADOW:
      return GL_SAMPLER_2D_RECT_SHADOW; 
      break;

    case UNIFORM_INT_SAMPLER_1D:
      return GL_INT_SAMPLER_1D; 
      break;

    case UNIFORM_INT_SAMPLER_2D:
      return GL_INT_SAMPLER_2D; 
      break;

    case UNIFORM_INT_SAMPLER_3D:
      return GL_INT_SAMPLER_3D; 
      break;

    case UNIFORM_INT_SAMPLER_CUBE:
      return GL_INT_SAMPLER_CUBE; 
      break;

    case UNIFORM_INT_SAMPLER_1D_ARRAY:
      return GL_INT_SAMPLER_1D_ARRAY; 
      break;

    case UNIFORM_INT_SAMPLER_2D_ARRAY:
      return GL_INT_SAMPLER_2D_ARRAY; 
      break;

    case UNIFORM_INT_SAMPLER_2D_MULTISAMPLE:
      return GL_INT_SAMPLER_2D_MULTISAMPLE; 
      break;

    case UNIFORM_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_INT_SAMPLER_BUFFER:
      return GL_INT_SAMPLER_BUFFER; 
      break;

    case UNIFORM_INT_SAMPLER_2D_RECT:
      return GL_INT_SAMPLER_2D_RECT; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_1D:
      return GL_UNSIGNED_INT_SAMPLER_1D; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_2D:
      return GL_UNSIGNED_INT_SAMPLER_2D; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_3D:
      return GL_UNSIGNED_INT_SAMPLER_3D; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_CUBE:
      return GL_UNSIGNED_INT_SAMPLER_CUBE; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_1D_ARRAY:
      return GL_UNSIGNED_INT_SAMPLER_1D_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
      return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_BUFFER:
      return GL_UNSIGNED_INT_SAMPLER_BUFFER; 
      break;

    case UNIFORM_UNSIGNED_INT_SAMPLER_2D_RECT:
      return GL_UNSIGNED_INT_SAMPLER_2D_RECT; 
      break;

    case UNIFORM_IMAGE_1D:
      return GL_IMAGE_1D; 
      break;

    case UNIFORM_IMAGE_2D:
      return GL_IMAGE_2D; 
      break;

    case UNIFORM_IMAGE_3D:
      return GL_IMAGE_3D; 
      break;

    case UNIFORM_IMAGE_2D_RECT:
      return GL_IMAGE_2D_RECT; 
      break;

    case UNIFORM_IMAGE_CUBE:
      return GL_IMAGE_CUBE; 
      break;

    case UNIFORM_IMAGE_BUFFER:
      return GL_IMAGE_BUFFER; 
      break;

    case UNIFORM_IMAGE_1D_ARRAY:
      return GL_IMAGE_1D_ARRAY; 
      break;

    case UNIFORM_IMAGE_2D_ARRAY:
      return GL_IMAGE_2D_ARRAY; 
      break;

    case UNIFORM_IMAGE_2D_MULTISAMPLE:
      return GL_IMAGE_2D_MULTISAMPLE; 
      break;

    case UNIFORM_IMAGE_2D_MULTISAMPLE_ARRAY:
      return GL_IMAGE_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_INT_IMAGE_1D:
      return GL_INT_IMAGE_1D; 
      break;

    case UNIFORM_INT_IMAGE_2D:
      return GL_INT_IMAGE_2D; 
      break;

    case UNIFORM_INT_IMAGE_3D:
      return GL_INT_IMAGE_3D; 
      break;

    case UNIFORM_INT_IMAGE_2D_RECT:
      return GL_INT_IMAGE_2D_RECT; 
      break;

    case UNIFORM_INT_IMAGE_CUBE:
      return GL_INT_IMAGE_CUBE; 
      break;

    case UNIFORM_INT_IMAGE_BUFFER:
      return GL_INT_IMAGE_BUFFER; 
      break;

    case UNIFORM_INT_IMAGE_1D_ARRAY:
      return GL_INT_IMAGE_1D_ARRAY; 
      break;

    case UNIFORM_INT_IMAGE_2D_ARRAY:
      return GL_INT_IMAGE_2D_ARRAY; 
      break;

    case UNIFORM_INT_IMAGE_2D_MULTISAMPLE:
      return GL_INT_IMAGE_2D_MULTISAMPLE; 
      break;

    case UNIFORM_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_1D:
      return GL_UNSIGNED_INT_IMAGE_1D; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_2D:
      return GL_UNSIGNED_INT_IMAGE_2D; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_3D:
      return GL_UNSIGNED_INT_IMAGE_3D; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_2D_RECT:
      return GL_UNSIGNED_INT_IMAGE_2D_RECT; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_CUBE:
      return GL_UNSIGNED_INT_IMAGE_CUBE; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_BUFFER:
      return GL_UNSIGNED_INT_IMAGE_BUFFER; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_1D_ARRAY:
      return GL_UNSIGNED_INT_IMAGE_1D_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_2D_ARRAY:
      return GL_UNSIGNED_INT_IMAGE_2D_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
      return GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE; 
      break;

    case UNIFORM_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY; 
      break;

    case UNIFORM_UNSIGNED_INT_ATOMIC_COUNTER:
      return GL_UNSIGNED_INT_ATOMIC_COUNTER; 
      break;

    default:
      throw std::invalid_argument("Expect type from UNIFORM_TYPE");
      break;
  }

  // Should neve reach here.
  return GL_FLOAT;
}

} // end of namespace Spire
