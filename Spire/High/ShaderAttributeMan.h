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

#ifndef SPIRE_HIGH_SHADERATTRIBUTEMAN_H
#define SPIRE_HIGH_SHADERATTRIBUTEMAN_H

#include "Common.h"
#include <vector>
#include <string>

namespace Spire
{

class ShaderAttributeMan;

/// Holds all information regarding one attribute.
struct AttribState
{
  size_t      index;          ///< Arbitrarily assigned index.
  std::string codeName;       ///< In-shader code name.
  uint32_t    nameHash;       ///< Hash of 'codeName'.
  size_t      numComponents;  ///< Number of attribute components.
  bool        normalize;      ///< True = normalize.
  size_t      size;           ///< Size, in bytes, of all components.
  size_t      halfFloatSize;  ///< Half-float size, in bytes, of all components.
  GLenum      type;           ///< GL-type of each component.
  GLenum      halfFloatType;  ///< Half-float GL-type of each component.
};

/// Shader attrtibutes class used to sort and compare shader input attributes.
/// \todo Should make this mechanism more general and allow arbitrary binding 
///       of shader attributes.
class ShaderAttributes
{
public:
  ShaderAttributes(const ShaderAttributeMan& man) :
      mAttributeMan(man)
  {}

  /// Retrieves the attribute at 'index' from ShaderAttributeMan.
  AttribState getAttribute(size_t index) const;

  /// Retrieves number of attributes stored in mAttributes.
  size_t getNumAttributes() const;

  /// Adds an attribute. The attribute is sorted based on the enumeration
  /// values in SHADER_ATTRIBUTE_TYPE.
  void addAttribute(const std::string& attribName, bool isHalfFloat = false);

  /// If 'attrib' is contained herein, returns true.
  bool hasAttrib(const std::string& attribName) const;

  /// Binds attributes to the shader indicated by parameter 'program'.
  void bindAttributes(GLuint program);

  /// Calculates the stride between vertices based on the attribute sizes
  /// calculated using calculateAttributeSizes.
  size_t calculateStride() const;

  /// Returns the number of common attributes.
  size_t calculateNumCommon(const ShaderAttributes& compare) const;

  /// Returns true if the given ShaderAttributes class would satisfy the 
  /// requirements of the current ShaderAttributes class.
  bool doesSatisfyShader(const ShaderAttributes& compare) const;

private:

  struct AttribSpecificData
  {
    bool    isHalfFloat;    ///< True if we should be using a half-float 
                            ///< representation of the data.
    size_t  index;          ///< Index in mAttributeMan's array.
  };

  /// Retrieves the full (including padding) size of the attribute
  /// in the vertex buffer.
  size_t getFullAttributeSize(const AttribSpecificData& attrib) const;

  /// Returns true if the attribute array contains a reference to 'index'.
  /// This is the index into the array in ShaderAttributeMan.
  bool hasIndex(size_t attributeIndex) const;

  /// Reference to the attribute manager.
  /// The attribute manager is queried regarding available attributes.
  ShaderAttributeMan&               mAttributeMan;

  /// Contains indices to attributes in ShaderAttributeMan, sorted (ascending).
  std::vector<AttribSpecificData>   mAttributes;

};

/// Shader attribute manager.
class ShaderAttributeMan
{
public:
  ShaderAttributeMan();
  virtual ~ShaderAttributeMan();

  /// Seed value to use when hashing strings for comparison purposes.
  const uint32_t MURMUR_SEED_VALUE = 0x9783f23d;

  /// Whenever an attribute has this index, it is not known how to handle it.
  const size_t UNKNOWN_ATTRIBUTE_INDEX = 0;

  /// Adds a new attribute to the system. Automatically assigns it an internal
  /// index based on when it was added.
  /// \param codeName       Name of the attribute in the shader code.
  /// \param numComponents  Number of components associated with this attribute.
  /// \param normalize      If true, the attribute will be normalized.
  /// \param size           Size of the attribute in bytes, including padding.
  /// \param halfFloatSize  Half float size of the attribute, including padding.
  /// \param type           Type of the attribute.
  /// \param halfFloatType  Half float type of the attribute.
  void addAttribute(const std::string& codeName, size_t numComponents,
                    bool normalize, size_t size, size_t halfFloatSize,
                    GLenum type, GLenum halfFloatType);

  /// Returns the index associated with the attribute whose name is 'codeName'.
  /// \return the first tuple parameter (bool) indicates whether or not an 
  ///         attribute with 'codeName' was found. The second tuple parameter
  ///         returns the index of the attribute, if found. If it is not found
  ///         then size_t will be NULL.
  std::tuple<bool,size_t> findAttributeWithName(const std::string& codeName) const;

  /// Returns the attibute at the specified index.
  /// If no attribute is found at 'index', or if the 'index' is invalid, an
  /// exception is thrown.
  AttribState getAttributeAtIndex(size_t index) const;

  /// Hashes 'str' into a uint32_t using murmur hash.
  /// Uses MURMUR_SEED_VALUE.
  static uint32_t hashString(const std::string& str);

private:

  /// Array of available attributes.
  std::vector<AttribState>    mAttributes;

};

} // namespace Spire

#endif 
