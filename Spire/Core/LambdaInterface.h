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
/// \date   July 2013

#ifndef SPIRE_LAMBDA_INTERFACE_H
#define SPIRE_LAMBDA_INTERFACE_H

#include <string>
#include "../Core/ShaderUniformStateManTemplates.h"
#include "../Core/Hub.h"

namespace Spire
{

/// Class that encapsulates all functionality that an anonymous function passed
/// into an interface would need access to in order to do meaningful work.
class LambdaInterface
{
public:
  LambdaInterface(Hub& hub, const std::string& pass) :
      mHub(hub),
      mPass(pass)
  { }
  virtual ~LambdaInterface() {}
  
  /// Retrieves a global uniform.
  /// The uniform returned depends on the active pass.
  template <class T>
  T getGlobalUniform(const std::string& uniformName)
  {
    // Check the pass uniforms first
    std::shared_ptr<const AbstractUniformStateItem> uniform =
        mHub.getPassUniformStateMan().getPassUninform(mPass, uniformName);
    if (uniform == nullptr)
      return mHub.getGlobalUniformStateMan().getGlobalUninform(uniformName)->getData<T>();
    else
      return uniform->getData<T>();
  }

  // Utility functions to set uniform values.
  template <class T>
  static typename std::enable_if<std::is_same<T, M44>::value, T> setUniform(
      std::string uniformName, int uniformType, int location, const T& val)
  {
    if (uniformType != UNIFORM_FLOAT_MAT4)
      throw std::runtime_error("Mismatched uniform types! Did not expect M44.");
    AbstractUniformStateItem::uniformMatrix4fv(location, 1, false, glm::value_ptr(val));
  }

protected:

  Hub&          mHub;
  std::string   mPass;
};

} // namespace 

#endif 
