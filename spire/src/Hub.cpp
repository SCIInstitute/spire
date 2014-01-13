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

#include <sstream>

#include "Common.h"
#include "Exceptions.h"

#include "Hub.h"
#include "Log.h"
#include "FileUtil.h"
#include "InterfaceImplementation.h"
#include "ShaderMan.h"
#include "ShaderAttributeMan.h"
#include "ShaderProgramMan.h"
#include "ShaderUniformStateMan.h"

#ifdef _WIN32
  // Disable warning: 'this' used in a base member initializer list warning.
  #pragma warning(disable:4355)
#endif

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
Hub::Hub(std::shared_ptr<Context> context,
         const std::vector<std::string>& shaderDirs,
         Interface::LogFunction logFn) :
    mLogFun(logFn),
    mContext(context),
    mShaderMan(new ShaderMan(*this)),
    mShaderAttributes(new ShaderAttributeMan()),
    mShaderProgramMan(new ShaderProgramMan(*this)),
    mShaderUniforms(new ShaderUniformMan()),
    mShaderUniformStateMan(new ShaderUniformStateMan(*this)),
    mPassUniformStateMan(new PassUniformStateMan(*this)),
    mShaderDirs(shaderDirs),
    mInterfaceImpl(new InterfaceImplementation(*this)),
    mPixScreenWidth(640),
    mPixScreenHeight(480)
{
  // Add default relative shader directory.
  std::string workingDir = getCurrentWorkingDir();
  workingDir += "/Shaders";
  mShaderDirs.push_back(workingDir);

  oneTimeInit();
}

//------------------------------------------------------------------------------
Hub::~Hub()
{
}

//------------------------------------------------------------------------------
void Hub::oneTimeInit()
{
  // Construct the logger now that we are on the appropriate thread.
  mLog = std::unique_ptr<Log>(new Log(mLogFun));
}

//------------------------------------------------------------------------------
void Hub::makeCurrent()
{
  mContext->makeCurrent();
}

} // namespace CPM_SPIRE_NS

