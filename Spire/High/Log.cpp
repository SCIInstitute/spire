/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   November 2012

#include "High/Log.h"

namespace Spire {

//------------------------------------------------------------------------------
Log::Log(const Hub::LogFunction& logFunction) :
    mDebugStream(logFunction, Interface::LOG_DEBUG),
    mMessageStream(logFunction, Interface::LOG_MESSAGE),
    mWarningStream(logFunction, Interface::LOG_WARNING),
    mErrorStream(logFunction, Interface::LOG_ERROR)
{  if (logFunction == nullptr)
  {
#ifndef WIN32
    std::stringstream osFilename;
    osFilename << "/tmp/SpireLog";//_" << std::this_thread::get_id();
    mOutputFile.open(osFilename.str());
    Hub::LogFunction fun = std::bind(&Log::logFunction, this,
                                     std::placeholders::_1, 
                                     std::placeholders::_2);

    // Reset all of the streams' logging functions.
    mDebugStream.setLogFunction(fun);
    mMessageStream.setLogFunction(fun);
    mWarningStream.setLogFunction(fun);
    mErrorStream.setLogFunction(fun);
#endif
  }

}

//------------------------------------------------------------------------------
Log::~Log()
{
  getMessageStream() << "Destroying spire logging class." << std::endl;
  if (mOutputFile.is_open())
    mOutputFile.close();
}

//------------------------------------------------------------------------------
void Log::logFunction(const std::string& msg, Interface::LOG_LEVEL level)
{
  switch (level)
  {
    case Interface::LOG_DEBUG:
      mOutputFile << "Debug:   " << msg;
      break;

    case Interface::LOG_MESSAGE:
      mOutputFile << "General: " << msg;
      break;

    case Interface::LOG_WARNING:
      mOutputFile << "Warning: " << msg;
      break;

    case Interface::LOG_ERROR:
      mOutputFile << "Error:   " << msg;
      break;
  }
  mOutputFile.flush();
}

} // end of namespace Spire
