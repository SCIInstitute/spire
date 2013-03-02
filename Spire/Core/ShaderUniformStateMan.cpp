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
#include "ShaderUniformStateMan.h"
#include "Hub.h"
#include "Exceptions.h"

namespace Spire {

//------------------------------------------------------------------------------
ShaderUniformStateMan::ShaderUniformStateMan(Hub& hub) :
    mHub(hub)
{
}

//------------------------------------------------------------------------------
ShaderUniformStateMan::~ShaderUniformStateMan()
{
}

//------------------------------------------------------------------------------
void ShaderUniformStateMan::applyUniform(const std::string& name, int location)
{
  // We use mGlobalState.at instead of the [] operator because at throws an
  // exception if the key is not found in the container.
  std::shared_ptr<AbstractUniformStateItem>& ptr = mGlobalState.at(name);
  ptr->applyUniform(location);
}

//------------------------------------------------------------------------------
void ShaderUniformStateMan::updateGlobalUniform(const std::string& name, 
                                                std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<const UniformState> uniform = 
      mHub.getShaderUniformManager().getUniformWithName(name); // std::out_of_range
  
  // Double check that the uniform we are receiving matches types.
  GLenum incomingType = uniformTypeToGL(item->getGLType());
  if (incomingType != uniform->type)
    throw ShaderUniformTypeError("Incoming type does not match type stored in uniform!");

  mGlobalState[name] = item;
}

//------------------------------------------------------------------------------
GLenum ShaderUniformStateMan::uniformTypeToGL(UNIFORM_TYPE type)
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
