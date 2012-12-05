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

// Only if on a Mac!
#include <OpenGL/gl.h>

#include "../Interface.h"

namespace Spire {

class Log;

/// Central hub for the renderer.
/// Most managers will reference this class in some way.
class Hub
{
public:

  /// @todo This typedef should go in Interface.h.
  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// @todo Make context a shared_ptr
  Hub(Context* context, LogFunction logFn);
  virtual ~Hub();

  /// One-time initialization of the renderer.
  /// Called by the rendering thread, or the thread where this Interface class
  /// was created (called automatically from interface's constructor in the 
  /// latter case).
  void oneTimeGLInit();

  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  void doFrame();

  /// Logging / diagnostic functions.
  /// @{
  std::ostream& logDebug();
  std::ostream& logMessage();
  std::ostream& logWarning();
  std::ostream& logError();
  /// @}

private:

  /// Generalized log function which writes output to mOutputFile.
  void logFunction(const std::string& msg, Interface::LOG_LEVEL level);

  Context*                  mContext;       ///< Context to use when performing
                                            ///< low level ops.
  LogFunction               mLogFP;         ///< Logging function pointer.
  std::ofstream             mOutputFile;    ///< Output file to use when
                                            ///< logging output.

  std::unique_ptr<Log>      mLog;           ///< Class to encapsulate logging
                                            ///< functionality.
};

} // namespace Spire

#endif // SPIRE_HIGH_HUB_H
