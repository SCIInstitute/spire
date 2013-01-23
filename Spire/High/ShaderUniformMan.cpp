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

#include "High/ShaderUniformMan.h"
#include "High/MurmurHash3.h"

namespace Spire {

//------------------------------------------------------------------------------
ShaderUniformMan::ShaderUniformMan(bool addDefaultUniforms)
{
  addUniform(getUnknownName());

  if (addDefaultUniforms)
  {
    addUniform("uProj");
    addUniform("uProjIV");
    addUniform("uProjIVWorld");
    addUniform("uColor");
    addUniform("uDirLight");
  }
}

//------------------------------------------------------------------------------
ShaderUniformMan::~ShaderUniformMan()
{
}

//------------------------------------------------------------------------------
void ShaderUniformMan::addUniform(const std::string& codeName)
{
  UniformState uniform;
  uniform.index = mUniforms.size();;
  uniform.codeName = codeName;
  uniform.nameHash = hashString(codeName);

  mUniforms.push_back(uniform);
}

//------------------------------------------------------------------------------
std::tuple<bool,size_t>
ShaderUniformMan::findUniformWithName(const std::string& codeName) const
{
  // Hash the string, search for the hash, then proceed with str comparisons.
  uint32_t targetHash = hashString(codeName);
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->nameHash == targetHash)
    {
      if (it->codeName == codeName)
      {
        return std::make_tuple(true, it->index);
      }
    }
  }

  return std::make_tuple(false, 0);
}

//------------------------------------------------------------------------------
UniformState ShaderUniformMan::getUniformAtIndex(size_t index) const
{
  if (index >= mUniforms.size())
    throw std::range_error("Index greater than size of mUniforms.");

  return mUniforms[index];
}

//------------------------------------------------------------------------------
UniformState 
ShaderUniformMan::getUniformWithName(const std::string& codeName) const
{
  std::tuple<bool, size_t> uniformIndex = findUniformWithName(codeName);
  if (std::get<0>(uniformIndex) == false)
    throw NotFound("Unable to find uniform with name.");

  return getUniformAtIndex(std::get<1>(uniformIndex));
}

//------------------------------------------------------------------------------
uint32_t ShaderUniformMan::hashString(const std::string& str)
{
  uint32_t hashOut = 0;
  MurmurHash3_x86_32(
      static_cast<const void*>(str.c_str()),
      static_cast<int>(str.size()),
      getMurmurSeedValue(),
      static_cast<void*>(&hashOut));
  return hashOut;
}

//------------------------------------------------------------------------------
// Shader Uniform Collection
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void ShaderUniformCollection::addUniform(const std::string& uniformName)
{
  std::tuple<bool, size_t> ret = mUniformMan.findUniformWithName(uniformName);
  if (std::get<0>(ret))
  {
    UniformSpecificData uniformData;
    uniformData.uniform = mUniformMan.getUniformAtIndex(std::get<1>(ret));
    // When testing this class, the gl call will always return 0.
    // I thought about writing a mock for the OpenGL interface, but that would
    // likely be a waist of time considering the utilities available on each
    // platform for testing OpenGL.
    if (getInvalidProgramHandle() != mProgram)
      uniformData.uniformLoc = glGetUniformLocation(mProgram, uniformName.c_str());
    else
      uniformData.uniformLoc = 0;
    mUniforms.push_back(uniformData);
  }
  else
  {
    throw ShaderUniformNotFound(uniformName);
  }
}

//------------------------------------------------------------------------------
bool ShaderUniformCollection::hasIndex(size_t targetIndex) const
{
  // Could perform a binary search here...
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (targetIndex == it->uniform.index)
      return true;
  }

  return false;
}

//------------------------------------------------------------------------------
bool ShaderUniformCollection::hasUniform(const std::string& uniformName) const
{
  uint32_t hash = ShaderUniformMan::hashString(uniformName);

  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    UniformState state = it->uniform;
    if (state.nameHash == hash)
    {
      // Check for hash collisions
      if (uniformName == state.codeName)
      {
        return true;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
size_t ShaderUniformCollection::getNumUniforms() const
{
  return mUniforms.size();
}

//------------------------------------------------------------------------------
ShaderUniformCollection::UniformSpecificData 
ShaderUniformCollection::getUniform(size_t index) const
{
  if (index >= mUniforms.size())
    throw std::range_error("Index greater than size of mAttributes.");

  return mUniforms[index];
}


} // end of namespace Spire
