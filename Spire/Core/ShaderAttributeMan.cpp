/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   December 2012

#include <algorithm>

#include "Common.h"
#include "Exceptions.h"

#include "Core/ShaderAttributeMan.h"
#include "Core/MurmurHash3.h"

namespace Spire {

//------------------------------------------------------------------------------
ShaderAttributeMan::ShaderAttributeMan(bool addDefaultAttributes)
{
  // Unknown attribute (attribute at 0 index).
  addAttribute(getUnknownName(), 1, false, sizeof(float), sizeof(short),
               GL_FLOAT, GL_HALF_FLOAT_OES);

  // Add default attributes if requested.
  if (addDefaultAttributes)
  {
    addAttribute("aPos", 3, false, 
                 sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aNormal", 3, false, 
                 sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aTexCoord0", 2, false, 
                 sizeof(float) * 2, sizeof(short) * 2,
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aTexCoord1", 2, false, 
                 sizeof(float) * 2, sizeof(short) * 2,
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aTexCoord2", 2, false, 
                 sizeof(float) * 2, sizeof(short) * 2,
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aTexCoord3", 2, false, 
                 sizeof(float) * 2, sizeof(short) * 2,
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aColor", 4, true, 
                 sizeof(char) * 4, sizeof(char) * 4,
                 GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE);
    addAttribute("aTangent", 3, false, 
                 sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aBinormal", 3, false, 
                 sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
                 GL_FLOAT, GL_HALF_FLOAT_OES);
    addAttribute("aGenPos", 3, false, 
                 sizeof(float) * 3, sizeof(float) * 3,
                 GL_FLOAT, GL_FLOAT);
    addAttribute("aGenNormal", 3, false, 
                 sizeof(float) * 3, sizeof(float) * 3,
                 GL_FLOAT, GL_FLOAT);
    addAttribute("aGenUV", 2, false, 
                 sizeof(float) * 2, sizeof(float) * 2,
                 GL_FLOAT, GL_FLOAT);
    addAttribute("aGenFloat", 1, false, 
                 sizeof(float) * 1, sizeof(float) * 1,
                 GL_FLOAT, GL_FLOAT);
  }
}

//------------------------------------------------------------------------------
ShaderAttributeMan::~ShaderAttributeMan()
{
}

//------------------------------------------------------------------------------
void ShaderAttributeMan::addAttribute(const std::string& codeName,
                                      size_t numComponents, bool normalize,
                                      size_t size, size_t halfFloatSize,
                                      GLenum type, GLenum halfFloatType)
{
  AttribState attrib;
  attrib.index          = mAttributes.size();
  attrib.codeName       = codeName;
  attrib.numComponents  = numComponents;
  attrib.normalize      = normalize;
  attrib.size           = size;
  attrib.halfFloatSize  = halfFloatSize;
  attrib.type           = type;
  attrib.halfFloatType  = halfFloatType;
  attrib.nameHash       = hashString(codeName);

  mAttributes.push_back(attrib);
}

//------------------------------------------------------------------------------
std::tuple<bool, size_t> 
ShaderAttributeMan::findAttributeWithName(const std::string& codeName) const
{
  // Hash the string, search for the hash, then proceed with string compares
  // to check for collisions.
  uint32_t targetHash = hashString(codeName);
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (it->nameHash == targetHash)
    {
      // Check for hash collisions
      if (codeName == it->codeName)
      {
        // We have found the attribute, return it.
        return std::make_tuple(true, it->index);
      }
    }
  }

  return std::make_tuple(false, 0);
}

//------------------------------------------------------------------------------
AttribState 
ShaderAttributeMan::getAttributeWithName(const std::string& codeName) const
{
  std::tuple<bool, size_t> attIndex = findAttributeWithName(codeName);
  if (std::get<0>(attIndex) == false)
    throw NotFound("Unable to find attribute with name.");

  return getAttributeAtIndex(std::get<1>(attIndex));
}

//------------------------------------------------------------------------------
uint32_t ShaderAttributeMan::hashString(const std::string& str)
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
AttribState ShaderAttributeMan::getAttributeAtIndex(size_t index) const
{
  if (index >= mAttributes.size())
    throw std::range_error("Index greater than size of mAttributes.");

  return mAttributes[index];
}

//------------------------------------------------------------------------------
// SHADER ATTRIBUTES
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
ShaderAttributeCollection::AttribSpecificData 
ShaderAttributeCollection::getAttribute(size_t index) const
{
  if (index >= mAttributes.size())
    throw std::range_error("Index greater than size of mAttributes.");

  return mAttributes[index];
}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::getNumAttributes() const
{
  return mAttributes.size();
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::hasAttribute(const std::string& attribName) const
{
  uint32_t hash = ShaderAttributeMan::hashString(attribName);

  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    AttribState state = it->attrib;
    if (state.nameHash == hash)
    {
      // Check for hash collisions
      if (attribName == state.codeName)
      {
        return true;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::doesSatisfyShader(const ShaderAttributeCollection& compare) const
{
  // Not possible to satisfy shader if there are any unknown attributes.
  if (    compare.hasIndex(ShaderAttributeMan::getUnknownAttributeIndex())
      ||  hasIndex(ShaderAttributeMan::getUnknownAttributeIndex()))
    return false;

  // Compare number of common attributes and the size of our attribute array.
  int numCommonAttribs = calculateNumCommonAttributes(compare);
  return (numCommonAttribs == mAttributes.size());
}


//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::calculateStride() const
{
  size_t stride = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    stride += getFullAttributeSize(*it);
  }

  return stride;
}

//------------------------------------------------------------------------------
void ShaderAttributeCollection::addAttribute(const std::string& attribName, 
                                    bool isHalfFloat)
{
  std::tuple<bool,size_t> ret = mAttributeMan.findAttributeWithName(attribName);
  if (std::get<0>(ret))
  {
    AttribSpecificData attribData;
    attribData.attrib = mAttributeMan.getAttributeAtIndex(std::get<1>(ret));
    attribData.isHalfFloat = isHalfFloat;
    mAttributes.push_back(attribData);

    // Re-sort the array.
    std::sort(mAttributes.begin(), mAttributes.end(),
         [] (const AttribSpecificData& a, const AttribSpecificData& b) 
          { return a.attrib.index < b.attrib.index; });
  }
  else
  {
    // We did not find the attribute in the attribute manager.
    throw ShaderAttributeNotFound(attribName);
  }
}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::getFullAttributeSize(const AttribSpecificData& att) const
{
  AttribState state = att.attrib;
  if (!att.isHalfFloat)
  {
    return state.size;
  }
  else
  {
    return state.halfFloatSize;
  }
}

//------------------------------------------------------------------------------
void ShaderAttributeCollection::bindAttributes(GLuint program)
{
  int i = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (it->attrib.index != ShaderAttributeMan::getUnknownAttributeIndex())
    {
      AttribState attrib = it->attrib;
      glBindAttribLocation(program, i, attrib.codeName.c_str());
    }
    ++i;
  }
}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::calculateNumCommonAttributes(const ShaderAttributeCollection& compare) const
{
  int numCommon = 0;

  // This check could be done much faster since both arrays are sorted.
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (compare.hasIndex(it->attrib.index))
      ++numCommon;
  }

  return numCommon;
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::hasIndex(size_t targetIndex) const
{
  // Could perform a binary search here...
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (targetIndex == it->attrib.index)
      return true;
  }

  return false;
}

}

