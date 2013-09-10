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

#ifndef SPIRE_HIGH_BASEASSETMAN_H
#define SPIRE_HIGH_BASEASSETMAN_H

#include "Common.h"
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <cstdint>
#include <memory>
#ifndef SPIRE_USING_WIN
#include <chrono>
#else
namespace std {
namespace chrono {
  // Temporary hack until a switch to VS 2012
  typedef uint64_t milliseconds;
} }
#endif

namespace Spire {

/// Derive all of your assets from this class.
class BaseAsset
{
public:
  BaseAsset(const std::string& name);
  virtual ~BaseAsset();

  /// Sets the absolute time at which the reference to this asset will be 
  /// dropped in BaseAssetMan.
  void setAbsTimeToHold(std::chrono::milliseconds holdTime) {mAbsHoldTime = holdTime;}

  /// Retrieves the time at which the reference to this asset will be dropped.
  std::chrono::milliseconds getAbsTimeHeld()                {return mAbsHoldTime;}

  /// Retrieves a hashed representation of the current string.
  static size_t hashString(const std::string& string);

  size_t getNameHash() const                                {return mNameHash;}
  std::string getName() const                               {return mName;}

private:

  size_t                    mNameHash;    ///< Hash representing this name.
  std::string               mName;        ///< Name of the asset.

  std::chrono::milliseconds mAbsHoldTime; ///< Absolute holding time for this object.
                                          ///< Used for sorting in the held assets
                                          ///< priority queue.

};

/// Base asset manager.
/// All asset managers should be derived off of this class.
class BaseAssetMan
{
public:
  BaseAssetMan();
  virtual ~BaseAssetMan();
  
  /// Removes any orphaned assets from the assets array and updates the held
  /// assets priority queue.
  /// \param  absTime     Current absolute time in milliseconds.
  void updateOrphanedAssets(std::chrono::milliseconds absTime);

  /// Holds a reference to an asset for a specified amount of time. 
  /// This helps keep the asset persistent even though the asset may not have
  /// any other references.
  /// \param  asset           Pointer to the asset
  /// \param  absReleaseTime  Absolute time when this asset will be released 
  ///                         in milliseconds.
  void holdAsset(std::shared_ptr<BaseAsset> asset, 
                 std::chrono::milliseconds absReleaseTime);

  /// Clear all held assets.
  void clearHeldAssets();


protected:

  /// Attempts to find the asset with the name given.
  /// If no asset is found a null shared_ptr is returned.
  std::shared_ptr<BaseAsset> findAsset(const std::string& str) const;

  /// Adds an asset onto the asset list.
  /// No reference will be held to the asset -- it will be assigned to a weak
  /// pointer.
  void addAsset(std::shared_ptr<BaseAsset> asset);

private:

  /// A vector of weak_ptrs to the assets. The asset is destroyed when the last
  /// shared_ptr referencing the object is reset. See addAsset.
  std::list<std::weak_ptr<BaseAsset>>               mAssets;

  /// A priority queue containing the assets we are holding a 'reference to'. 
  /// See holdAsset.
  std::priority_queue<std::shared_ptr<BaseAsset>>   mHeldAssets;

};

} // namespace Spire

#endif 
