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


  mGlobalState[name] = item;
}

} // end of namespace Spire
