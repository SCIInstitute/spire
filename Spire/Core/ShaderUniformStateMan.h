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
#include <unordered_map>

#include "Core/ShaderUniformStateManTemplates.h"

namespace Spire {

/// Unform state management. The currently available uniform state can be
/// set and queried from this interface.
class ShaderUniformStateMan
{
public:
  ShaderUniformStateMan();
  virtual ~ShaderUniformStateMan();
  
  /// Adds a uniform to the global state.
  /// \param  name    Name, as used in the shader, of the uniform.
  /// \param  data    Data to be associated with the uniform.
  ///                 If you supply an invalid type for data, you will encounter
  ///                 a compile-time error telling you as such. See
  ///                 ShaderUniformStateManTemplates.h for a list of datatypes
  ///                 accepted by this function (look at the specializations).
  template <typename T>
  void addGlobalUniform(const std::string& name, T data)
  {
    /// \todo Want functional pattern matching here.
    ///       See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3449.pdf 
    ///       Stroustrup has built a library that does this already called mach 7.
    ///       https://parasol.tamu.edu/~yuriys/pm/
    ///       This will likely be in the next c++ standard after c++11.

    // Below is an exceptionally verbose implementation of something like 
    // a watered down functional pattern matching using template specialization.
    // A static_assert will be issued if there exists no template specialization
    // for the template type T.

    // Extra parethesis required to avoid most vexing parse.
    std::unique_ptr<AbstractUniformStateItem> ptr((UniformStateItem<T>(data)));
    mGlobalState[name] = std::move(ptr);
  }

  /// Applies the specified uniform to the current shader state.
  /// Throws std::out_of_range if the key was not found in the map.
  void applyUniform(const std::string& name, int location);

private:

  /// Contains all current global uniform state. I would use an ordered map,
  /// but less than is used as the comparison operator. I would need to hash
  /// the strings then insert the hashed value into the map.
  std::unordered_map<std::string, std::unique_ptr<AbstractUniformStateItem>> mGlobalState;
};


} // namespace Spire 

#endif 
