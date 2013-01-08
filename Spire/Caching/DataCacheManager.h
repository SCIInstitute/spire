/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   September 2012

#ifndef SPIRE_CACHING_DATACACHEMANAGER_H
#define SPIRE_CACHING_DATACACHEMANAGER_H

namespace Spire {

/// Used for efficient retrieval of objects.
/// Based on the current viewing angle, we can efficiently composite from
/// back to front.
///
/// We can pre-fetch the next 'brick' while we are rendering and compositing
/// the current frame.
/// 
/// Prefetch can work based off the rendering order of the last frame.
///
/// The plan is that the data cache manager will be able to use the metadata
/// from the renderer to make cache requests.
///
/// Much of this may be based on a 128 bit hash.
class DataCacheManager
{
public:
  DataCacheManager();
  virtual ~DataCacheManager();

  /// @todo flesh out this function.
  ///       Possibly record cache misses.
  /// Function blocks until fetched data is ready.
  void fetch();

  /// Attempts to fetch data, but returns immediately if the data is not present
  /// or not immediately accessible.
  void fetchAsync();

  /// The cache manager will attempt to prefetch data as required.
  void prefetch();

  /// Sets the amount of System RAM data cache to be used when fetching data.
  /// Based on this size, data will be paged in and out of memory.

private:
};

} // namespace spire

#endif // SPIRE_CACHING_CACHEMANAGER_H
