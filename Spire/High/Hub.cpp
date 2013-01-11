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
#include <thread>

#include "Common.h"
#include "Exceptions.h"

#include "High/Hub.h"
#include "High/Log.h"
#include "High/FileUtil.h"

#include "StuPipe/Driver.h"

namespace Spire {

//------------------------------------------------------------------------------
Hub::Hub(Context* context, const std::vector<std::string>& shaderDirs, 
         Interface::LogFunction logFn, bool useThread) :
    mLogFun(logFn),
    mContext(context),
    mShaderMan(*this),
    mThreadKill(false),
    mThreadRunning(false),
    mPixScreenWidth(640),
    mPixScreenHeight(480),
    mShaderProgramMan(*this),
    mShaderDirs(shaderDirs)
{
  // Add default relative shader directory.
  std::string workingDay = getCurrentWorkingDir();
  workingDay += "/Shaders";
  mShaderDirs.push_back(workingDay);

  mCamera = std::shared_ptr<Camera>(new Camera(*this));

  if (useThread)
  {
    createRendererThread();    
  }
  else
  {
    oneTimeInitOnThread();
  }
}

//------------------------------------------------------------------------------
Hub::~Hub()
{
  if (isRendererThreadRunning())
  {
    killRendererThread();
  }
}

//------------------------------------------------------------------------------
void Hub::oneTimeInitOnThread()
{
  // Construct the logger now that we are on the appropriate thread.
  mLog = std::unique_ptr<Log>(new Log(mLogFun));

  // OpenGL initialization
  mContext->makeCurrent();

  // Initialize OpenGL
  glClearColor(0.0, 0.0, 0.0, 1.0);

  const GLubyte* vendor     = glGetString(GL_VENDOR);
  const GLubyte* renderer   = glGetString(GL_RENDERER);
  const GLubyte* versionl   = glGetString(GL_VERSION);

  Log::message() << "OpenGL initialization. Running on a " << vendor << " " 
                 << renderer << " with OpenGL version " << versionl << std::endl
                 << "GL made current on thread " << std::this_thread::get_id()
                 << std::endl;

	GLint tmp;
  Log::debug() << "Hardware specific attributes" << std::endl;
  Log::debug() << "+Programmable:" << std::endl;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
  Log::debug() << "  Texture Units: " << tmp << std::endl;

  Log::debug() << "+Fixed function (transient):" << std::endl;
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &tmp);
  Log::debug() << "  Model view stack depth: " << tmp << std::endl;
	
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &tmp);
  Log::debug() << "  Projection stack depth: " << tmp << std::endl;
	
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &tmp);
  Log::debug() << "  Texture stack depth: " << tmp << std::endl;

  /// TODO: Add GPU memory checks using GL_NVX_gpu_memory_info for NVIDIA
  ///       and GL_ATI_meminfo for ATI.

  // Pulled from Tuvok
  //const bool bOpenGLSO12     = atof((const char*)versionl) >= 1.2;
  //const bool bOpenGLSO20     = atof((const char*)versionl) >= 2.0;

  Log::debug() << "Creating render pipeline." << std::endl;
  mPipe = std::shared_ptr<PipeDriver>(new StuPipe::Driver(*this));
}

//------------------------------------------------------------------------------
void Hub::doFrame()
{
  // When we get here, it has already been determined that a frame needs to be
  // redrawn.

  // Question: How do we split up the pipes? Does everything happen in the
  // stupipe, when we route through the stupipe? It almost appears that it has
  // to. Should we just build the pipe and see where it leads us to?

  // Rudimentary doFrame...
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mPipe->doFrame();

  mContext->swapBuffers();
}

//------------------------------------------------------------------------------
bool Hub::isRendererThreadRunning() const
{
  return mThreadRunning.load();
}

//------------------------------------------------------------------------------
void Hub::createRendererThread()
{
  if (isRendererThreadRunning() == true)
    throw ThreadException("Cannot create new renderer thread; a renderer "
                          "thread is currently running.");

  mThread = std::thread(&Hub::rendererThread, this);
}

//------------------------------------------------------------------------------
void Hub::killRendererThread()
{
  if (isRendererThreadRunning() == false)
    throw ThreadException("Cannot kill renderer thread; the thread is not "
                          "currently running.");

  // The following join statement could possibly throw a system_error if 
  // mThread.get_id() == std::thread::id()
  mThreadKill.store(true);
  mThread.join();
}

//------------------------------------------------------------------------------
void Hub::rendererThread()
{
  mThreadRunning.store(true);

  // Cannot log anything until this is called.
  oneTimeInitOnThread();

  Log::message() << "Started rendering thread" << std::endl;

  while (mThreadKill.load() == false)
  {
    doFrame();
    /// @todo Add logic to determine how long we should sleep for...
    ///       This will be highly dependent on how long it took to render the
    ///       last frame, and if we are in the middle of compositing the final
    ///       frame together.
    std::chrono::milliseconds dur(10);
    std::this_thread::sleep_for(dur);
  }

  Log::message() << "Terminating rendering thread" << std::endl;

  // Hub should really be destroyed here... mHub should be a part of HubThread.
  // Create hub non-threaded class.

  mThreadRunning.store(false);
}

}
