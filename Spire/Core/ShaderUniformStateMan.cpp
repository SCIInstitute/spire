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
#include "ShaderUniformMan.h"
#include "Hub.h"
#include "Exceptions.h"

CPM_NAMESPACE
namespace Spire {

//------------------------------------------------------------------------------
ShaderUniformStateMan::ShaderUniformStateMan(Hub& hub) :
    mHub(hub)
{
}

//------------------------------------------------------------------------------
//ShaderUniformStateMan::~ShaderUniformStateMan()
//{
//}

//------------------------------------------------------------------------------
bool ShaderUniformStateMan::applyUniform(const std::string& name, int location)
{
  // We use mGlobalState.at instead of the [] operator because at throws an
  // exception if the key is not found in the container.
  /// \xxx  Possibly detect if the name exists, and if it does not then throw
  ///       a different exception (instead of letting the map throw std::range_error).
  auto it = mGlobalState.find(name);
  if (it != mGlobalState.end())
  {
    std::shared_ptr<AbstractUniformStateItem>& ptr = it->second;
    ShaderUniformMan::applyUniformGLState(ptr, location);
    //std::cout << name << ": " << ptr->asString() << std::endl;
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
void ShaderUniformStateMan::updateGlobalUniform(const std::string& name, 
                                                std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<const UniformState> uniform = mHub.getShaderUniformManager().findUniformWithName(name);
  if (uniform == nullptr)
  {
    // Default to adding the uniform to the uniform manager.
    mHub.getShaderUniformManager().addUniform(name, ShaderUniformMan::uniformTypeToGL(item->getGLType()));
    uniform = mHub.getShaderUniformManager().getUniformWithName(name); // std::out_of_range
  }

  // Double check that the uniform we are receiving matches types.
  GLenum incomingType = ShaderUniformMan::uniformTypeToGL(item->getGLType());
  if (incomingType != uniform->type)
    throw ShaderUniformTypeError("Incoming type does not match type stored in uniform!");

  mGlobalState[name] = item;
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem> ShaderUniformStateMan::getGlobalUninform(const std::string& name)
{
  try
  {
    return mGlobalState.at(name);
  }
  catch (std::exception&)
  {
    throw NotFound("Unable to find uniform at any level: '" + name + "'");
  }
}

//------------------------------------------------------------------------------
std::string ShaderUniformStateMan::uniformAsString(const std::string& name) const
{
  const std::shared_ptr<const AbstractUniformStateItem>& ptr = mGlobalState.at(name);
  return ptr->asString();
}


} // namespace Spire
CPM_NAMESPACE

