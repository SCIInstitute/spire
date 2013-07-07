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

#ifndef SPIRE_CORE_SHADERUNIFORMSTATEMANTEMPLATES_H
#define SPIRE_CORE_SHADERUNIFORMSTATEMANTEMPLATES_H

#include <cstddef>
#include <sstream>
#include <vector>
#include "../InterfaceCommon.h"
#include "../Core/Math.h"
#include "../Core/GLMathUtil.h"

namespace Spire {

// A good list of these types are here:
// http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml .
enum UNIFORM_TYPE
{
  UNIFORM_FLOAT,
  UNIFORM_FLOAT_VEC2,
  UNIFORM_FLOAT_VEC3,
  UNIFORM_FLOAT_VEC4,
  UNIFORM_DOUBLE,
  UNIFORM_DOUBLE_VEC2,
  UNIFORM_DOUBLE_VEC3,
  UNIFORM_DOUBLE_VEC4,
  UNIFORM_INT,
  UNIFORM_INT_VEC2,
  UNIFORM_INT_VEC3,
  UNIFORM_INT_VEC4,
  UNIFORM_UNSIGNED_INT,
  UNIFORM_UNSIGNED_INT_VEC2,
  UNIFORM_UNSIGNED_INT_VEC3,
  UNIFORM_UNSIGNED_INT_VEC4,
  UNIFORM_BOOL,
  UNIFORM_BOOL_VEC2,
  UNIFORM_BOOL_VEC3,
  UNIFORM_BOOL_VEC4,
  UNIFORM_FLOAT_MAT2,
  UNIFORM_FLOAT_MAT3,
  UNIFORM_FLOAT_MAT4,
  UNIFORM_FLOAT_MAT2x3,
  UNIFORM_FLOAT_MAT2x4,
  UNIFORM_FLOAT_MAT3x2,
  UNIFORM_FLOAT_MAT3x4,
  UNIFORM_FLOAT_MAT4x2,
  UNIFORM_FLOAT_MAT4x3,
  UNIFORM_DOUBLE_MAT2,
  UNIFORM_DOUBLE_MAT3,
  UNIFORM_DOUBLE_MAT4,
  UNIFORM_DOUBLE_MAT2x3,
  UNIFORM_DOUBLE_MAT2x4,
  UNIFORM_DOUBLE_MAT3x2,
  UNIFORM_DOUBLE_MAT3x4,
  UNIFORM_DOUBLE_MAT4x2,
  UNIFORM_DOUBLE_MAT4x3,
  UNIFORM_SAMPLER_1D,
  UNIFORM_SAMPLER_2D,
  UNIFORM_SAMPLER_3D,
  UNIFORM_SAMPLER_CUBE,
  UNIFORM_SAMPLER_1D_SHADOW,
  UNIFORM_SAMPLER_2D_SHADOW,
  UNIFORM_SAMPLER_1D_ARRAY,
  UNIFORM_SAMPLER_2D_ARRAY,
  UNIFORM_SAMPLER_1D_ARRAY_SHADOW,
  UNIFORM_SAMPLER_2D_ARRAY_SHADOW,
  UNIFORM_SAMPLER_2D_MULTISAMPLE,
  UNIFORM_SAMPLER_2D_MULTISAMPLE_ARRAY,
  UNIFORM_SAMPLER_CUBE_SHADOW,
  UNIFORM_SAMPLER_BUFFER,
  UNIFORM_SAMPLER_2D_RECT,
  UNIFORM_SAMPLER_2D_RECT_SHADOW,
  UNIFORM_INT_SAMPLER_1D,
  UNIFORM_INT_SAMPLER_2D,
  UNIFORM_INT_SAMPLER_3D,
  UNIFORM_INT_SAMPLER_CUBE,
  UNIFORM_INT_SAMPLER_1D_ARRAY,
  UNIFORM_INT_SAMPLER_2D_ARRAY,
  UNIFORM_INT_SAMPLER_2D_MULTISAMPLE,
  UNIFORM_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
  UNIFORM_INT_SAMPLER_BUFFER,
  UNIFORM_INT_SAMPLER_2D_RECT,
  UNIFORM_UNSIGNED_INT_SAMPLER_1D,
  UNIFORM_UNSIGNED_INT_SAMPLER_2D,
  UNIFORM_UNSIGNED_INT_SAMPLER_3D,
  UNIFORM_UNSIGNED_INT_SAMPLER_CUBE,
  UNIFORM_UNSIGNED_INT_SAMPLER_1D_ARRAY,
  UNIFORM_UNSIGNED_INT_SAMPLER_2D_ARRAY,
  UNIFORM_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
  UNIFORM_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
  UNIFORM_UNSIGNED_INT_SAMPLER_BUFFER,
  UNIFORM_UNSIGNED_INT_SAMPLER_2D_RECT,
  UNIFORM_IMAGE_1D,
  UNIFORM_IMAGE_2D,
  UNIFORM_IMAGE_3D,
  UNIFORM_IMAGE_2D_RECT,
  UNIFORM_IMAGE_CUBE,
  UNIFORM_IMAGE_BUFFER,
  UNIFORM_IMAGE_1D_ARRAY,
  UNIFORM_IMAGE_2D_ARRAY,
  UNIFORM_IMAGE_2D_MULTISAMPLE,
  UNIFORM_IMAGE_2D_MULTISAMPLE_ARRAY,
  UNIFORM_INT_IMAGE_1D,
  UNIFORM_INT_IMAGE_2D,
  UNIFORM_INT_IMAGE_3D,
  UNIFORM_INT_IMAGE_2D_RECT,
  UNIFORM_INT_IMAGE_CUBE,
  UNIFORM_INT_IMAGE_BUFFER,
  UNIFORM_INT_IMAGE_1D_ARRAY,
  UNIFORM_INT_IMAGE_2D_ARRAY,
  UNIFORM_INT_IMAGE_2D_MULTISAMPLE,
  UNIFORM_INT_IMAGE_2D_MULTISAMPLE_ARRAY,
  UNIFORM_UNSIGNED_INT_IMAGE_1D,
  UNIFORM_UNSIGNED_INT_IMAGE_2D,
  UNIFORM_UNSIGNED_INT_IMAGE_3D,
  UNIFORM_UNSIGNED_INT_IMAGE_2D_RECT,
  UNIFORM_UNSIGNED_INT_IMAGE_CUBE,
  UNIFORM_UNSIGNED_INT_IMAGE_BUFFER,
  UNIFORM_UNSIGNED_INT_IMAGE_1D_ARRAY,
  UNIFORM_UNSIGNED_INT_IMAGE_2D_ARRAY,
  UNIFORM_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
  UNIFORM_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,
  UNIFORM_UNSIGNED_INT_ATOMIC_COUNTER
};

/// Abstract base class interface for a single uinform state item.
class WIN_DLL AbstractUniformStateItem
{
public:
  AbstractUniformStateItem()            {}
  virtual ~AbstractUniformStateItem()   {}

  /// Applies uniform value.
  virtual void applyUniform(int location) const = 0;

  /// Returns appropriate OpenGL type
  virtual UNIFORM_TYPE getGLType() const = 0;

  /// Retrieve textual representation of uniform.
  virtual std::string asString() const = 0;

  /// Retrieve raw pointer data. Not safe. Use one of the templated versions of
  /// the code below.
  virtual const float* getRawData() const = 0;

  /// Retrieve M44
  template <class T>
  typename std::enable_if<std::is_same<T, M44>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT_MAT4)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type M44.");
    return glm::make_mat4x4(getRawData());
  }

  /// Retrieve M33
  template <class T>
  typename std::enable_if<std::is_same<T, M33>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT_MAT3)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type M33.");
    return glm::make_mat3(getRawData());
  }

  /// Retrieve V4
  template <class T>
  typename std::enable_if<std::is_same<T, V4>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT_VEC4)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type V4.");
    return glm::make_vec4(getRawData());
  }

  /// Retrieve V3
  template <class T>
  typename std::enable_if<std::is_same<T, V3>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT_VEC3)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type V3.");
    return glm::make_vec3(getRawData());
  }

  /// Retrieve V3
  template <class T>
  typename std::enable_if<std::is_same<T, V2>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT_VEC2)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type V2.");
    return glm::make_vec2(getRawData());
  }

  /// Retrieve float
  template <class T>
  typename std::enable_if<std::is_same<T, float>::value, T>::type getData() const
  {
    if (getGLType() != UNIFORM_FLOAT)
      throw std::runtime_error("Mismatched types! Expected uniform to be of type float.");
    return *getRawData();
  }

  /// Series of static utility functions to avoid exposing OpenGL functions
  /// to classes outside of spire.
  ///@{
  static void uniform1f(int location, float v0);
  static void uniform2f(int location, float v0, float v1);
  static void uniform3f(int location, float v0, float v1, float v2);
  static void uniform4f(int location, float v0, float v1, float v2, float v3);
  static void uniformMatrix4fv(int location, size_t count, bool transpose,
                               const float*  value);
  static void uniform3fv(int location, size_t count, const float* value);
  ///@}

};

//------------------------------------------------------------------------------
// Template specializations for types commonly used in shader uniforms.
// This is essentially implementing funcitonal pattern matching.
// When new C++ standard creeps up, use:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3449.pdf instead.
//------------------------------------------------------------------------------

// Type specializations derived off of UniformStateItem.
template <typename T>
class UniformStateItem : public AbstractUniformStateItem
{
public:
  static_assert(true, "There is no valid template specialization of "
                      "UniformStateItem for your type. See ShaderUniformStateMan.h.");
};


//------------------------------------------------------------------------------
// V3
//------------------------------------------------------------------------------
template <>
class UniformStateItem<V3> : public AbstractUniformStateItem
{
public:
  typedef V3 Type;

  UniformStateItem(const Type& in) : mData(in) {}

  void applyUniform(int location) const override
  {
    uniform3f(location, mData.x, mData.y, mData.z);
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT_VEC3;
  }

  std::string asString() const override
  {
    std::stringstream stream;
    stream << "Vec3 - (" << mData.x << ", " << mData.y << ", " << mData.z << ")";
    return stream.str();
  }

  const float* getRawData() const override
  {
    return glm::value_ptr(mData);
  }

private:
  Type mData;
};

/// Vector3 of floats implementation. Avoid using this function as it depends
/// on vectors being tightly packed.
/// \todo Ensure V3's are tightly packed... If we do this, this may hurt 
///       efficiency when performing calculations on the CPU.
template <>
class UniformStateItem<std::vector<V3>> : public AbstractUniformStateItem
{
public:
  typedef V3 Type;
  // Don't technically *need* the std::move in this case, but it makes it more
  // apparent what is going on.
  UniformStateItem(const std::vector<V3>& in) : mData(in)             {}
  UniformStateItem(std::vector<V3>&& in)      : mData(std::move(in))  {}
  
  void applyUniform(int location) const override
  {
    // The standard makes it very clear that vectors will be stored in
    // contiguous memory. This is a *very* dangerous cast that will ONLY work if
    // vectors are tightly packed.
    uniform3fv(location, mData.size(), reinterpret_cast<const float*>(&mData[0]));
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT_VEC3;
  }

  std::string asString() const override
  {
    std::stringstream stream;
    stream << "Vector3 Array - Output not implemented.";
    return stream.str();
  }

  const float* getRawData() const override
  {
    // There is no safe way of returning this data.
    return nullptr;
  }
  

private:
  std::vector<V3>   mData;
};

template <>
class UniformStateItem<V4> : public AbstractUniformStateItem
{
public:
  typedef V4 Type;

  UniformStateItem(const Type& in) : mData(in) {}

  void applyUniform(int location) const override
  {
    uniform4f(location, mData.x, mData.y, mData.z, mData.w);
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT_VEC4;
  }

  std::string asString() const override
  {
    std::stringstream stream;
    stream << "Vec4 - (" << mData.x << ", " << mData.y << ", " << mData.z 
           << ", " << mData.w << ")";
    return stream.str();
  }

  const float* getRawData() const override
  {
    return glm::value_ptr(mData);
  }

private:
  Type mData;
};

template <>
class UniformStateItem<V2> : public AbstractUniformStateItem
{
public:
  typedef V2 Type;

  UniformStateItem(const Type& in) : mData(in) {}

  void applyUniform(int location) const override
  {
    uniform2f(location, mData.x, mData.y);
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT_VEC2;
  }

  std::string asString() const override
  {
    std::stringstream stream;
    stream << "Vec2 - (" << mData.x << ", " << mData.y << ")";
    return stream.str();
  }

  const float* getRawData() const override
  {
    return glm::value_ptr(mData);
  }


private:
  Type mData;
};

template <>
class UniformStateItem<float> : public AbstractUniformStateItem
{
public:
  typedef float Type;

  UniformStateItem(const Type& in) : mData(in) {}

  void applyUniform(int location) const override
  {
    uniform1f(location, mData);
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT;
  }

  std::string asString() const override
  {
    std::stringstream stream;
    stream << "Float - (" << mData << ")";
    return stream.str();
  }

  const float* getRawData() const override
  {
    return &mData;
  }


private:
  Type mData;
};

//------------------------------------------------------------------------------
// M44
//------------------------------------------------------------------------------
template <>
class UniformStateItem<M44> : public AbstractUniformStateItem
{
public:
  typedef M44 Type;
  UniformStateItem(const Type& in)
  {
    // Perform conversion process to float array before applyUniform is ever
    // called.
    M44toArray16(in, glMatrix);
  }

  void applyUniform(int location) const override
  {
    uniformMatrix4fv(location, 1, false, &glMatrix[0]);
  }

  UNIFORM_TYPE getGLType() const override
  {
    return UNIFORM_FLOAT_MAT4;
  }

  std::string asString() const override
  {
    // OpenGL matrices are represented in Column-Major order.
    // We will print off the matrix not as it appears in memory, but its
    // transpose instead (so rows are displayed contiguously).
    std::stringstream stream;
    stream << "Mat4 - (" << glMatrix[0] << " " << glMatrix[4] << " " << glMatrix[8]  << " " << glMatrix[12] << std::endl
           << "        " << glMatrix[1] << " " << glMatrix[5] << " " << glMatrix[9]  << " " << glMatrix[13] << std::endl
           << "        " << glMatrix[2] << " " << glMatrix[6] << " " << glMatrix[10] << " " << glMatrix[14] << std::endl
           << "        " << glMatrix[3] << " " << glMatrix[7] << " " << glMatrix[11] << " " << glMatrix[15];
    return stream.str();
  }

  const float* getRawData() const override
  {
    return glMatrix;
  }

private:
  float glMatrix[16];
};

} // namespace Spire 

#endif 
