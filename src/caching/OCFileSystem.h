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

#ifndef SPIRE_CACHING_OCFILESYSTEM_H
#define SPIRE_CACHING_OCFILESYSTEM_H

namespace spire {

/// Experimental out-of-core filesystem paging.
/// Uses the filesystem to store object hashes in a hierarchical fashion.
/// Very similar to the way Git stores SHA1 hashes in its .git/objects dir.
/// The plan is to use MurmurHash3 instead of a cryptographic hash (like SHA1),
/// but the hashing function will be interchangable.
///
/// This as opposed to a proprietary out-of-core in-file paging where a 
/// hierarchy is constructed inside of a single file. This approach is the
/// subject of the OCFile class.
///
/// Priority: Low
class OCFileSystem
{
public:
  OCFileSystem();
  virtual ~OCFileSystem();

private:
};

} // namespace spire

#endif // SPIRE_CACHING_OCFILESYSTEM_H
