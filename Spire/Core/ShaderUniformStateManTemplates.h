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
#include "Core/Math.h"
#include "Core/GLMathUtil.h"

namespace Spire {

/// Abstract base class interface for a single uinform state item.
class AbstractUniformStateItem
{
public:
  AbstractUniformStateItem()            {}
  virtual ~AbstractUniformStateItem()   {}

  /// Applies uniform value.
  virtual void applyUniform(int location) const = 0;

  /// Retrieves uniform's name.

protected:

  /// Series of static utility functions to avoid exposing OpenGL functions
  /// to classes outside of spire.
  ///@{
  static void uniform4f(int location, float v0, float v1, float v2, float v3);
  static void uniform3f(int location, float v0, float v1, float v2);
  static void uniformMatrix4fv(int location, size_t count, bool transpose,
                               const float*  value);
  static void uniform3fv(int location, size_t count, const float* value);
  ///@}

};

//------------------------------------------------------------------------------
// Template specializations for types commonly used in shader uniforms.
// This is essentially implementing funcitonal pattern matching.
// When new C++ standard creeps up, use:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3449.pdf 
// instead.
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

private:
  std::vector<V3>   mData;
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

private:
  float glMatrix[16];
};

} // namespace Spire 

#endif 
