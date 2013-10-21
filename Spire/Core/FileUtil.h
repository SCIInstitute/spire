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
/// \brief  These functions exist primarily to locate shader files.
///         Inherits heavily from the SysTools library in IV3D's Tuvok.
///         There is currently no wstring implementation.
///         Really should be replaced with boost::filesystem on appropriate
///         platforms.

#ifndef SPIRE_HIGH_FILEUTIL_H
#define SPIRE_HIGH_FILEUTIL_H

#include <string>
#include <vector>
#include "cpm/cpm.h"

CPM_NAMESPACE
namespace Spire {

std::string findFileInDirs(const std::string& file,
                           const std::vector<std::string>& strDirs,
                           bool subdirs);
bool fileExists(const std::string& strFileName);
std::vector<std::string> getSubDirList(const std::string& dir);
std::string getCurrentWorkingDir();
bool getTempDirectory(std::string& path);

} // namespace Spire
CPM_NAMESPACE

#endif 
