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
/// \date   October 2012

#ifndef SPIRE_HIGH_HUBTHREAD_H
#define SPIRE_HIGH_HUBTHREAD_H

#include <memory>
#include <thread>
#include <atomic>

namespace Spire {

class Hub;

/// Runs the rendering hub on its own thread. Controlled via Interface.h.
class HubThread
{
public:
  /// @todo Convert Hub into a shared pointer.
  HubThread(Hub* hub);
  virtual ~HubThread();

  // All public functions in HubThread are thread safe.

  /// Terminates the rendering thread. After this call, you will be able to
  /// re-issue context->makeCurrent() and call doFrame manually.
  /// killRendererThread WILL block until the rendering thread has finished.
  /// This is to ensure makeCurrent will not be called again before the thread 
  /// has terminated.
  void killRendererThread();

  /// Creates a rendering thread. 
  /// There must not be a rendering thread already running.
  void createRendererThread();

  /// Returns true if the rendering thread is currently running.
  bool isRendererThreadRunning();

private:

  /// Thread related functions and variables
  /// @{

  /// The hub's thread function.
  void rendererThread();

  std::thread               mThread;        ///< The renderer thread.
  std::atomic<bool>         mThreadKill;    ///< If true, the renderer thread
                                            ///< will attempt to finish what it
                                            ///< is doing and terminate.
  std::atomic<bool>         mThreadRunning; ///< True if the rendering thread
                                            ///< is currently running.
  Hub*                      mHub;           ///< Reference to the hub.
  /// @}

};

} // namespace Spire

#endif // SPIRE_HIGH_HUBTHREAD_H
