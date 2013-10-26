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
#include "PassUniformStateMan.h"
#include "ShaderUniformMan.h"
#include "Hub.h"
#include "Exceptions.h"

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
PassUniformStateMan::PassUniformStateMan(Hub& hub) :
    mHub(hub)
{
}

//------------------------------------------------------------------------------
PassUniformStateMan::PassUniforms* PassUniformStateMan::getPass(const std::string& pass)
{
  // Not very efficient -- but still more efficient than other methods when the
  // number of passes is low.
  for (size_t i = 0; i < mPasses.size(); i++)
  {
    if (mPasses[i].passName == pass)
    {
      return &mPasses[i];
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------------
const PassUniformStateMan::PassUniforms* PassUniformStateMan::getPass(const std::string& pass) const
{
  // Not very efficient -- but still more efficient than other methods when the
  // number of passes is low.
  for (size_t i = 0; i < mPasses.size(); i++)
  {
    if (mPasses[i].passName == pass)
    {
      return &mPasses[i];
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------------
PassUniformStateMan::PassUniforms& PassUniformStateMan::getOrCreatePass(const std::string& pass)
{
  PassUniforms* passStruct = getPass(pass);
  if (passStruct == nullptr)
  {
    // Add a new pass, and return that.
    PassUniforms passUniforms;
    passUniforms.passName = pass;
    mPasses.push_back(passUniforms);
    return mPasses.back();
  }
  else
  {
    return *passStruct;
  }
}

//------------------------------------------------------------------------------
bool PassUniformStateMan::tryApplyUniform(const std::string& pass, 
                                          const std::string& name, int location)
{
  // We use mState.at instead of the [] operator because at throws an
  // exception if the key is not found in the container.
  /// \xxx  Possibly detect if the name exists, and if it does not then throw
  ///       a different exception (instead of letting the map throw std::range_error).
  PassUniforms* passStruct = getPass(pass);
  if (passStruct != nullptr)
  {
    std::shared_ptr<AbstractUniformStateItem> ptr = passStruct->uniforms[name];
    if (ptr != nullptr)
    {
      ShaderUniformMan::applyUniformGLState(ptr, location);
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
void PassUniformStateMan::updatePassUniform(const std::string& pass, const std::string& name, 
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

  // Retrieve the appropriate pass and add/update our uniform.
  PassUniforms& passStruct = getOrCreatePass(pass);
  passStruct.uniforms[name] = item;
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem>
PassUniformStateMan::getPassUninform(const std::string& pass, const std::string& name) const
{
  const PassUniforms* passStruct = getPass(pass);
  if (passStruct != nullptr)
  {
    auto it = passStruct->uniforms.find(name);
    if (it != passStruct->uniforms.end())
    {
      return it->second;
    }
    else
    {
      return std::shared_ptr<AbstractUniformStateItem>(); 
    }
  }
  else
  {
    return std::shared_ptr<AbstractUniformStateItem>();
  }
}

//------------------------------------------------------------------------------
std::string PassUniformStateMan::uniformAsString(const std::string& pass, const std::string& name) const
{
  const PassUniforms* passStruct = getPass(pass);
  if (passStruct != nullptr)
  {
    auto it = passStruct->uniforms.find(name);
    if (it != passStruct->uniforms.end())
      return it->second->asString();
    else
      return "";
  }
  else
  {
    return "";
  }
}


} // namespace CPM_SPIRE_NS

