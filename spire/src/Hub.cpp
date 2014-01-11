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

  // OpenGL initialization
  mContext->makeCurrent();

  // Any GL platform specific initialization.
  CPM_GL_PLATFORM_NS::glPlatformInit();
}

/// \todo Hub::beginFrame needs to be removed in the future.
//------------------------------------------------------------------------------
bool Hub::beginFrame(bool makeContextCurrent)
{
  if (makeContextCurrent == true)
    makeCurrent();

  // Bail if the frame buffer is not complete. One instance where this is a
  // *valid* state is when a window containing the OpenGL scene is hidden.
  // The framebuffer will be invalid in this instance, but we can still render
  // later once the window is unhidden.
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return false;

  /// \todo Move this outside of the interface!
  GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  GL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  ///// \todo Make line width a part of the GPU state.
  //glLineWidth(2.0f);
  ////glEnable(GL_LINE_SMOOTH);

  return true;
}

//------------------------------------------------------------------------------
void Hub::makeCurrent()
{
  mContext->makeCurrent();
}

//------------------------------------------------------------------------------
void Hub::endFrame()
{
  mContext->swapBuffers();
}

} // namespace CPM_SPIRE_NS

