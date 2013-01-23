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

#ifndef SPIRE_INTERFACE_H
#define SPIRE_INTERFACE_H

#include <string>
#include <vector>

#include "Context.h"

// Vanilla interface until SCIRun adopts C++11.

namespace Spire {

class Hub;
class HubThread;

/// Interface to the renderer.
/// A new interface will need to be created per-context.
class Interface
{
public:
  /// All possible log outputs from the renderer. Used via the logging function.
  enum LOG_LEVEL
  {
    LOG_DEBUG,    ///< Debug / verbose.
    LOG_MESSAGE,  ///< General message.
    LOG_WARNING,  ///< Warning.
    LOG_ERROR,    ///< Error.
  };

  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// Constructs an interface to the renderer.
  /// \param  shaderDirs    A list of directories to search for shader files.
  /// \param  createThread  If true, then a thread will be created in which the
  ///                       renderer will run. The renderer will do it's best
  ///                       to manage time allocation and only use cycles
  ///                       when there is something to do.
  /// \param  logFP         The logging function to use when logging rendering
  ///                       messages. Remember, this function will be called
  ///                       from the rendering thread; so it will need to 
  ///                       communicate the messages in a thread-safe manner.
  /// @todo Re-enabled shared_ptr context when SCIRun5 adopts C++11.
  //Interface(std::shared_ptr<Context> context, bool createThread, 
  //          LogFunction logFP = LogFunction());
  Interface(Context* context, const std::vector<std::string>& shaderDirs,
            bool createThread, LogFunction logFP = LogFunction());
  virtual ~Interface();


  //============================================================================
  // THREAD SAFE
  //============================================================================

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================

  /// The following functions are *not* thread safe.
  /// Use these functions only when the renderer is not threaded.
  /// @{
  
  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  /// You must call this function every time you want a new frame to be rendered
  /// unless you are using the threaded renderer. The threaded renderer will 
  /// call doFrame automatically.
  /// \note You must call doFrame on the same thread where makeCurrent issued.
  ///       If this is not the same thread where Interface was created, ensure
  ///       a call to context->makeCurrent() is issued before invoking doFrame
  ///       for the first time.
  void doFrame();

  /// @}



private:

  std::unique_ptr<Hub>  mHub;           ///< Rendering hub.
};

} // namespace spire

#endif // SPIRE_INTERFACE_H
