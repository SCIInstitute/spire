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

#include "BaseAssetMan.h"

namespace CPM_SPIRE_NS {
namespace Spire {

//------------------------------------------------------------------------------
BaseAssetMan::BaseAssetMan()
{
}

//------------------------------------------------------------------------------
BaseAssetMan::~BaseAssetMan()
{
}

//------------------------------------------------------------------------------
void BaseAssetMan::addAsset(std::shared_ptr<BaseAsset> asset)
{
  mAssets.push_back(std::weak_ptr<BaseAsset>(asset));
}

//------------------------------------------------------------------------------
void BaseAssetMan::holdAsset(std::shared_ptr<BaseAsset> asset, 
                             std::chrono::milliseconds absTimeToHold)
{
  asset->setAbsTimeToHold(absTimeToHold);
  mHeldAssets.push(asset);
}

//------------------------------------------------------------------------------
void BaseAssetMan::clearHeldAssets()
{
  while (mHeldAssets.empty() == false)
    mHeldAssets.pop();
}

//------------------------------------------------------------------------------
void BaseAssetMan::updateOrphanedAssets(std::chrono::milliseconds absTime)
{
  // Check the first held asset and if it is less than the absolute time,
  // release it and continue.
  while (mHeldAssets.empty() == false)
  {
    if (mHeldAssets.top()->getAbsTimeHeld() < absTime)
      mHeldAssets.pop();
    else 
      break;
  }

  // Iterate over all of the assets and remove expired elements.
  auto it = mAssets.begin();
  while (it != mAssets.end())
  {
    if (it->expired())
    {
      // Remove this element from the list since it has expired.
      it = mAssets.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//------------------------------------------------------------------------------
std::shared_ptr<BaseAsset> 
BaseAssetMan::findAsset(const std::string& str) const
{
  size_t targetHash = BaseAsset::hashString(str);
  for (auto it = mAssets.begin(); it != mAssets.end(); ++it)
  {
    // std::weak_ptr::lock will construct an empty shared_ptr, not throw an
    // exception, if the weak_ptr has expired (a common occurrence within this
    // class...).
    std::shared_ptr<BaseAsset> shader(it->lock());
    if (shader != nullptr)
    {
      if (shader->getNameHash() == targetHash)
      {
        // Now check for hash collision.
        if (shader->getName() == str)
          return it->lock();
      }
    }
  }
  return std::shared_ptr<BaseAsset>(nullptr);
}


//------------------------------------------------------------------------------
// Base Asset
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
BaseAsset::BaseAsset(const std::string& name) :
    mName(name),
    mAbsHoldTime(0)
{
  mNameHash = hashString(name);
}

//------------------------------------------------------------------------------
BaseAsset::~BaseAsset()
{
}

//------------------------------------------------------------------------------
size_t BaseAsset::hashString(const std::string& str)
{
  return std::hash<std::string>()(str);
}

} // namespace Spire
} // namespace CPM_SPIRE_NS

