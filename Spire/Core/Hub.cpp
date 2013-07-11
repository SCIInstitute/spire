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

#include "../Common.h"
#include "../Exceptions.h"

#include "Hub.h"
#include "Log.h"
#include "FileUtil.h"
#include "InterfaceImplementation.h"
#include "ShaderMan.h"
#include "ShaderAttributeMan.h"
#include "ShaderProgramMan.h"
#include "ShaderUniformStateMan.h"

#ifdef SPIRE_USING_WIN
  // Disable warning: 'this' used in a base member initializer list warning.
  #pragma warning(disable:4355)
#endif

namespace Spire {

//------------------------------------------------------------------------------
Hub::Hub(std::shared_ptr<Context> context,
         const std::vector<std::string>& shaderDirs,
         Interface::LogFunction logFn, bool useThread) :
    mLogFun(logFn),
    mContext(context),
    mShaderMan(new ShaderMan(*this)),
    mShaderAttributes(new ShaderAttributeMan()),
    mShaderProgramMan(new ShaderProgramMan(*this)),
    mShaderUniforms(new ShaderUniformMan()),
    mShaderUniformStateMan(new ShaderUniformStateMan(*this)),
    mPassUniformStateMan(new PassUniformStateMan(*this)),
    mInterfaceImpl(new InterfaceImplementation(*this)),
    mShaderDirs(shaderDirs),
#ifdef SPIRE_USE_STD_THREADS
    mThreadKill(false),
    mThreadRunning(false),
#endif
    mPixScreenWidth(640),
    mPixScreenHeight(480)
{
  // Add default relative shader directory.
  std::string workingDay = getCurrentWorkingDir();
  workingDay += "/Shaders";
  mShaderDirs.push_back(workingDay);

  if (useThread)
  {
#ifdef SPIRE_USE_STD_THREADS
    createRendererThread();
#else
    throw UnsupportedException("Spire compiled without std threads!");
#endif
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

#ifndef SPIRE_OPENGL_ES_2
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    Log::error() << "GLEW init failed!" << std::endl;
    throw GLError("GLEW failed to initialize!");
  }
#endif

  // Initialize OpenGL
  GL(glClearColor(0.0, 0.0, 0.0, 1.0));

  const GLubyte* vendor     = glGetString(GL_VENDOR);
  const GLubyte* renderer   = glGetString(GL_RENDERER);
  const GLubyte* versionl   = glGetString(GL_VERSION);
  GL_CHECK();

  Log::message() << std::endl << "------------------------------" << std::endl;
  Log::message() << "OpenGL initialization. Running on a " << vendor << " "
                 << renderer << " with OpenGL version " << versionl
#ifdef SPIRE_USE_STD_THREADS
                 << std::endl << "GL made current on thread " << std::this_thread::get_id()
#endif
                 << std::endl;

  GLint tmp;
  Log::debug() << "Hardware specific attributes" << std::endl;
  Log::debug() << "+Programmable:" << std::endl;

#ifdef SPIRE_OPENGL_ES_2
  GL(glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp));
  Log::debug() << "  Vertex texture units: " << tmp << std::endl;

  GL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tmp));
  Log::debug() << "  Fragment texture units: " << tmp << std::endl;

  GL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp));
  Log::debug() << "  Combined texture units: " << tmp << std::endl;
#else
  GL(glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp));
  Log::debug() << "  Texture Units: " << tmp << std::endl;

  Log::debug() << "+Fixed function (transient):" << std::endl;
  GL(glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &tmp));
  Log::debug() << "  Model view stack depth: " << tmp << std::endl;

  GL(glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &tmp));
  Log::debug() << "  Projection stack depth: " << tmp << std::endl;

  GL(glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &tmp));
  Log::debug() << "  Texture stack depth: " << tmp << std::endl;

  const bool openGLSO12     = atof((const char*)versionl) >= 1.2;
  const bool openGLSO20     = atof((const char*)versionl) >= 2.0;
  const bool openGLSO       = glewGetExtension("GL_ARB_shader_objects");
  const bool openGLSL       = glewGetExtension("GL_ARB_shading_language_100");
  const bool openGL3DT      = glewGetExtension("GL_EXT_texture3D");
  const bool openGLFBO      = glewGetExtension("GL_EXT_framebuffer_object");

  const bool openGL32       = atof((const char*)versionl) >= 3.2;

  const bool openGL42       = atof((const char*)versionl) >= 4.2;
  const bool openGLILS_EXT  = glewGetExtension("GL_EXT_shader_image_load_store");
  const bool openGLILS_ARB  = glewGetExtension("GL_ARB_shader_image_load_store");
  const bool openGLCD_ARB   = glewGetExtension("GL_ARB_conservative_depth");
#endif
    
  /// TODO: Add GPU memory checks using GL_NVX_gpu_memory_info for NVIDIA
  ///       and GL_ATI_meminfo for ATI.

  // Pulled from Tuvok
  //const bool bOpenGLSO12     = atof((const char*)versionl) >= 1.2;
  //const bool bOpenGLSO20     = atof((const char*)versionl) >= 2.0;
}

//------------------------------------------------------------------------------
void Hub::doFrame()
{
  // When we get here, it has already been determined that a frame needs to be
  // redrawn.

  // Question: How do we split up the pipes? Does everything happen in the
  // stupipe, when we route through the stupipe? It almost appears that it has
  // to. Should we just build the pipe and see where it leads us to?
  mInterfaceImpl->executeQueue();
  mInterfaceImpl->doAllPasses();
}

//------------------------------------------------------------------------------
bool Hub::isRendererThreadRunning() const
{
#ifdef SPIRE_USE_STD_THREADS
  return mThreadRunning.load();
#else
  return false;
#endif
}

//------------------------------------------------------------------------------
void Hub::createRendererThread()
{
#ifdef SPIRE_USE_STD_THREADS
  if (isRendererThreadRunning() == true)
    throw ThreadException("Cannot create new renderer thread; a renderer "
                          "thread is currently running.");

  mThread = std::thread(&Hub::rendererThread, this);
#else
  throw UnsupportedException("Spire compiled without threading support. "
                             "Cannot create render thread.");
#endif
}

//------------------------------------------------------------------------------
void Hub::killRendererThread()
{
#ifdef SPIRE_USE_STD_THREADS
  if (isRendererThreadRunning() == false)
    throw ThreadException("Cannot kill renderer thread; the thread is not "
                          "currently running.");

  // The following join statement could possibly throw a system_error if
  // mThread.get_id() == std::thread::id()
  mThreadKill.store(true);
  mThread.join();
#endif
}

#ifdef SPIRE_USE_STD_THREADS
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

    // Swap buffers manually.
    mContext->swapBuffers();

    /// @todo Add logic to determine how long we should sleep for...
    ///       This will be highly dependent on how long it took to render the
    ///       last frame, and if we are in the middle of compositing the final
    ///       frame together.
    std::chrono::milliseconds dur(10);
    std::this_thread::sleep_for(dur);
  }

  /// \todo Pass a hub reference into BaseAssetMan and construct an ongoing
  ///       list of base asset managers. That way we can simply loop through
  ///       any assets holding graphics resources.
  mShaderMan->clearHeldAssets();
  mShaderProgramMan->clearHeldAssets();

  mInterfaceImpl->clearGLResources();

  // The following needs to be done whether or not the hub is destroyed in this
  // function.
  for (auto it = mPipes.begin(); it != mPipes.end(); ++it)
  {
    (*it)->clearGLResources();
    it->reset();
  }

  mThreadRunning.store(false);
}
#endif

//------------------------------------------------------------------------------
bool Hub::addFunctionToThreadQueue(const RemoteFunction& fun)
{
  return mInterfaceImpl->addFunctionToQueue(fun);
}

}
