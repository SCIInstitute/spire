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

#ifndef SPIRE_HIGH_HUB_H
#define SPIRE_HIGH_HUB_H

#include <iostream>
#include <fstream>
#include <functional>
#ifdef SPIRE_USE_STD_THREADS
#include <thread>
#include <atomic>
#endif

#include "Interface.h"
#include "Core/PipeInterface.h"
#include "Core/GPUStateManager.h"
#include "Core/ShaderMan.h"
#include "Core/ShaderProgramMan.h"
#include "Core/ShaderAttributeMan.h"
#include "Core/Camera.h"

namespace Spire {

class Log;
class PipeDriver;
class InterfaceImplementation;
class HackedUCRenderer;

/// Using thread local storage ONLY for logging purposes, nothing else.

/// Central hub for the renderer.
/// Most managers will reference this class in some way.
class Hub
{
public:

  /// @todo Make context a shared_ptr
  Hub(std::shared_ptr<Context> context,
      const std::vector<std::string>& shaderDirs, 
      Interface::LogFunction logFn, bool useThread);
  virtual ~Hub();

  /// Definition of what a remote function should accept.
  typedef std::function<void (Hub& hub)> RemoteFunction;

  /// One-time initialization of the renderer.
  /// Called by the rendering thread, or the thread where this Interface class
  /// was created (called automatically from interface's constructor in the 
  /// latter case).
  void oneTimeInitOnThread();

  /// Returns true if the rendering thread is currently running.
  bool isRendererThreadRunning() const;

  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  void doFrame();

  /// Retrieves the GPU state manager.
  GPUStateManager& getGPUStateManager()           {return mGPUStateManager;}

  /// Retrieves shader manager.
  ShaderMan& getShaderManager()                   {return mShaderMan;}

  /// Retrieves shader attribute manager.
  ShaderAttributeMan& getShaderAttributeManager() {return mShaderAttributes;}

  /// Retrieves shader uniform manager.
  ShaderUniformMan& getShaderUniformManager()     {return mShaderUniforms;}

  /// Retrieves the shader program manager.
  ShaderProgramMan& getShaderProgramManager()     {return mShaderProgramMan;}

  /// Retrieves the default camera.
  std::shared_ptr<Camera> getCamera()             {return mCamera;}

  /// Retrieves the hacked renderer.
  std::shared_ptr<HackedUCRenderer> getHackedRenderer() {return mHackedRenderer;}

  /// Retrieves the actual screen width in pixels.
  size_t getActualScreenWidth() const             {return mPixScreenWidth;}

  /// Retrieves the actual screen width in pixels.
  size_t getActualScreenHeight() const            {return mPixScreenHeight;}

  /// Retrieve list of directories in which to search for shaders.
  const std::vector<std::string>& getShaderDirs() const {return mShaderDirs;}

  /// Adds a pipe to the back of the queue.
  void addPipe(std::shared_ptr<PipeInterface> pipe);

  /// Remove pipe from queue.
  void removePipe(std::shared_ptr<PipeInterface> pipe);

  /// Terminates the rendering thread. After this call, you will be able to
  /// re-issue context->makeCurrent() and call doFrame manually.
  /// killRendererThread WILL block until the rendering thread has finished.
  /// This is to ensure makeCurrent will not be called again before the thread 
  /// has terminated.
  void killRendererThread();

  /// Adds a function to the cross-thread message queue.
  bool addFunctionToThreadQueue(const RemoteFunction& fun);

private:

  Interface::LogFunction      mLogFun;          ///< Log function.
  std::unique_ptr<Log>        mLog;             ///< Spire logging class.
  std::shared_ptr<Context>    mContext;         ///< Rendering context.
  GPUStateManager             mGPUStateManager; ///< GPU state manager.
  ShaderMan                   mShaderMan;       ///< Shader manager.
  ShaderProgramMan            mShaderProgramMan;///< Shader program manager.
  ShaderAttributeMan          mShaderAttributes;///< Shader attribute manager.
  ShaderUniformMan            mShaderUniforms;  ///< Shader attribute manager.
  std::shared_ptr<Camera>     mCamera;          ///< Basic GL Camera 
                                                ///< (this should not be here... move in the future)
  std::vector<std::string>    mShaderDirs;      ///< Shader directories to search.

  std::list<std::shared_ptr<PipeInterface>> mPipes;         ///< Rendering pipes in-order.
  std::shared_ptr<InterfaceImplementation>  mInterfaceImpl; ///< Interface implementation.
  std::shared_ptr<HackedUCRenderer>         mHackedRenderer;///< Hacked uniform color renderer.

  // Threading variables / functions

  /// Creates a rendering thread. 
  /// There must not be a rendering thread already running.
  void createRendererThread();

#ifdef SPIRE_USE_STD_THREADS
  std::thread             mThread;          ///< The renderer thread.
  std::atomic<bool>       mThreadKill;      ///< If true, the renderer thread
                                            ///< will attempt to finish what it
                                            ///< is doing and terminate.
  std::atomic<bool>       mThreadRunning;   ///< True if the rendering thread
                                            ///< is currently running.

  /// Rendering thread
  void rendererThread();
#endif

  size_t                  mPixScreenWidth;  ///< Actual screen width in pxels.
  size_t                  mPixScreenHeight; ///< Actual screen height in pixels.
  
};

} // namespace Spire

#endif // SPIRE_HIGH_HUB_H

