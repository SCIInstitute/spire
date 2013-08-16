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
/// \date   August 2013

#ifndef SPIREEXT_VISUALIZENORMALS_H
#define SPIREEXT_VISUALIZENORMALS_H

#include <string>
#include <vector>
#include <memory>

#include "WinDLLExport.h"

namespace Spire {
namespace Extensions {

/// \note This class only exists because we do not have access to geometry
///       shaders on all platforms (Mac).

/// \note In order for this class to work as planned, we would need to run it on
//        the graphics thread since we need to grab the VBO using vboName...
//        That means that we would need to script what comes after. This is 
//        where continuation programming would need to come in. If thats really
//        the mechanic that we want to use...

/// Visualize normals using VBO name and the name of the position and normal
/// attributes associated with that VBO.
class WIN_DLL VisualizeNormals
{
public:
  VisualizeNormals(const std::string& vboName, 
                   std::shared_ptr<std::vector<uint8_t>> vboData,
                   const std::string& posAttribName,
                   const std::string& normalAttribName,
                   std::function<> continuation);

  virtual ~VisualizeNormals();
  
};

} // namespace Extensions
} // namespace Spire 

#endif 
