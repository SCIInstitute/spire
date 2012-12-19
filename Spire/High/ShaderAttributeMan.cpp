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

#include "ShaderAttributeMan.h"
#include "MurmurHash3.h"

namespace Spire {

//------------------------------------------------------------------------------
ShaderAttributeMan::ShaderAttributeMan()
{
  // Unknown attribute (attribute at 0 index).
  addAttribute("_unknown_", 1, false, sizeof(float), sizeof(short), GL_FLOAT,
               GL_HALF_FLOAT_OES);

  // Default attributes.
  addAttribute("a_pos", 3, false, 
               sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_normal", 3, false, 
               sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_texCoord0", 2, false, 
               sizeof(float) * 2, sizeof(short) * 2,
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_texCoord1", 2, false, 
               sizeof(float) * 2, sizeof(short) * 2,
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_texCoord2", 2, false, 
               sizeof(float) * 2, sizeof(short) * 2,
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_texCoord3", 2, false, 
               sizeof(float) * 2, sizeof(short) * 2,
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_color", 4, true, 
               sizeof(char) * 4, sizeof(char) * 4,
               GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE);
  addAttribute("a_tangent", 3, false, 
               sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_binormal", 3, false, 
               sizeof(float) * 3, sizeof(short) * 3 + sizeof(short),
               GL_FLOAT, GL_HALF_FLOAT_OES);
  addAttribute("a_generalPos", 3, false, 
               sizeof(float) * 3, sizeof(float) * 3,
               GL_FLOAT, GL_FLOAT);
  addAttribute("a_generalNormal", 3, false, 
               sizeof(float) * 3, sizeof(float) * 3,
               GL_FLOAT, GL_FLOAT);
  addAttribute("a_generalUV", 2, false, 
               sizeof(float) * 2, sizeof(float) * 2,
               GL_FLOAT, GL_FLOAT);
  addAttribute("a_generalFloat", 1, false, 
               sizeof(float) * 1, sizeof(float) * 1,
               GL_FLOAT, GL_FLOAT);
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
#ifdef OPENGL_ES
  attrib.halfFloatSize  = halfFloatSize;
#else
  attrib.halfFloatSize  = size;
#endif
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
uint32_t ShaderAttributeMan::hashString(const std::string& str)
{
  uint32_t hashOut = 0;
  MurmurHash3_x86_32(
      static_cast<const void*>(str.c_str()),
      static_cast<int>(str.size()),
      MURMUR_SEED_VALUE,
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
AttribState ShaderAttributes::getAttribute(size_t index) const
{
  return mAttributeMan.getAttributeAtIndex(index);
}

//------------------------------------------------------------------------------
size_t ShaderAttributes::getNumAttributes() const
{
  return mAttributes.size();
}

//------------------------------------------------------------------------------
bool ShaderAttributes::hasAttribute(const std::string& attribName) const
{
  uint32_t hash = ShaderAttributeMan::hashString(attribName);

  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    AttribState state = getAttribute(it->index);
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
bool ShaderAttributes::doesSatisfyShader(const ShaderAttributes& compare) const
{
  // Not possible to satisfy shader if there are any unknown attributes.
  if (    compare.hasAttribute(ShaderAttributeMan::UNKNOWN_ATTRIBUTE_INDEX)
      ||  hasAttribute(ShaderAttributeMan::UNKNOWN_ATTRIBUTE_INDEX))
    return false;

  // Compare number of common attributes and the size of our attribute array.
  int numCommonAttribs = calculateNumCommonAttributes(compare);
  return (numCommonAttribs == mAttributes.size());
}


//------------------------------------------------------------------------------
size_t ShaderAttributes::calculateStride() const
{
  size_t stride = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    stride += getFullAttributeSize(*it);
  }

  return stride;
}

//------------------------------------------------------------------------------
void ShaderAttributes::addAttribute(const std::string& attribName, 
                                    bool isHalfFloat)
{
  std::tuple<bool,size_t> ret = mAttributeMan.findAttributeWithName(attribName);
  if (std::get<0>(ret))
  {
    AttribSpecificData attribData;
    attribData.index = std::get<1>(ret);
    attribData.isHalfFloat = isHalfFloat;
    mAttributes.push_back(attribData);

    // Re-sort the array.
    sort(mAttributes.begin(), mAttributes.end(),
         [] (const AttribSpecificData& a, const AttribSpecificData& b) 
          { return a.index < b.index; });
  }
  else
  {
    // We did not find the attribute in the attribute manager.
    throw std::invalid_argument("Unable to find attribute with specified name.");
  }
}

//------------------------------------------------------------------------------
size_t ShaderAttributes::getFullAttributeSize(const AttribSpecificData& attrib) const
{
  AttribState state = mAttributeMan.getAttributeAtIndex(attrib.index);
  if (attrib.isHalfFloat)
  {
    return state.size;
  }
  else
  {
    return state.halfFloatSize;
  }
}

//------------------------------------------------------------------------------
void ShaderAttributes::bindAttributes(GLuint program)
{
  int i = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (it->index != ShaderAttributeMan::UNKNOWN_ATTRIBUTE_INDEX)
    {
      AttribState attrib = getAttribute(it->index);
      glBindAttribLocation(program, i, attrib.codeName.c_str());
    }
    ++i;
  }
}

//------------------------------------------------------------------------------
size_t ShaderAttributes::calculateNumCommonAttributes(const ShaderAttributes& compare) const
{
  int numCommon = 0;

  // This check could be done much faster since both arrays are sorted.
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (compare.hasIndex(it->index))
      ++numCommon;
  }

  return numCommon;
}

//------------------------------------------------------------------------------
bool ShaderAttributes::hasIndex(size_t targetIndex) const
{
  // Could perform a binary search here...
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (targetIndex == it->index)
      return true;
  }

  return false;
}

}

