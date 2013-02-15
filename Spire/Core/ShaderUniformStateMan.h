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

#ifndef SPIRE_CORE_SHADERUNIFORMSTATEMAN_H
#define SPIRE_CORE_SHADERUNIFORMSTATEMAN_H

#include <map>

namespace Spire {

/// Abstract base class interface for a single uinform state item.
class UniformStateItem
{
public:
  UniformStateItem()            {}
  virtual ~UniformStateItem()   {}

  /// Applies uniform value.
  virtual void applyUniform() = 0;
};

/// Unform state management. The currently available uniform state can be
/// set and queried from this interface.
class ShaderUniformStateMan
{
public:
  ShaderUniformStateMan();
  virtual ~ShaderUniformStateMan();
  
  /// Reference decay won't work for this because I don't really care about
  /// move semantics in this context. I'm not implementing move constructors 
  /// for any of the specializations of UniformStateItem.
  /// Adds a uniform to the global state.
  void addGlobalUniform(const UniformStateItem& item);

private:

  std::multimap<size_t, UniformStateItem>   mGlobalState;   ///< Global uniform state.
};

// Type specializations derived off of UniformStateItem.

} // namespace Spire 

#endif 
