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

#include "StuObject.h"

namespace Spire {

//------------------------------------------------------------------------------
// VBO
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
VBOObject::VBOObject(std::shared_ptr<std::vector<uint8_t>> vboData,
            const std::vector<std::string>& attributes)
{
}

//------------------------------------------------------------------------------
VBOObject::~VBOObject()
{
}

//------------------------------------------------------------------------------
// IBO
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
IBOObject::~IBOObject()
{
}

//------------------------------------------------------------------------------
IBOObject::IBOObject(std::shared_ptr<std::vector<uint8_t>> iboData,
            StuInterface::IBO_TYPE type)
{
}

//------------------------------------------------------------------------------
// StuPass
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuPass::StuPass(const std::string& objectName, const std::string& programName,
                 size_t vboID, size_t iboID) :
    mVBO(vboID),
    mIBO(iboID)
{
  /// \todo Lookup the shader to use
}

//------------------------------------------------------------------------------
StuPass::~StuPass()
{
}

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuObject::StuObject(const std::string& name, int32_t renderOrder) :
    mName(name),
    mRenderOrder(renderOrder)
{

}

//------------------------------------------------------------------------------
void StuObject::addIBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> iboData,
              StuInterface::IBO_TYPE type)
{
  size_t hash = mHashFun(name);
  if (mIBOMap.find(hash) != mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object (possible hash collision?).");

  mIBOMap.emplace(std::make_pair(hash, IBOObject(iboData, type)));
}

//------------------------------------------------------------------------------
void StuObject::addPass(const std::string& pass,
               const std::string& program,
               const std::string& vboName,
               const std::string& iboName)
{
}

//------------------------------------------------------------------------------
void StuObject::addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item)
{
}

//------------------------------------------------------------------------------
void StuObject::addVBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> vboData,
              const std::vector<std::string>& attribNames)
{
  size_t hash = mHashFun(name);
  if (mVBOMap.find(hash) != mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object (possible hash collision?).");

  mVBOMap.emplace(std::make_pair(hash, VBOObject(vboData, attribNames)));
}

//------------------------------------------------------------------------------
void StuObject::removePass(const std::string& pass)
{
}

//------------------------------------------------------------------------------
IBOObject& StuObject::getIBOByName(const std::string& name)
{
  size_t hash = mHashFun(name);
  return mIBOMap.at(hash);
}

//------------------------------------------------------------------------------
VBOObject& StuObject::getVBOByName(const std::string& name)
{
  size_t hash = mHashFun(name);
  return mVBOMap.at(hash);
}


} // end of namespace Spire
