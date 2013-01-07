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

std::atomic<bool>         Log::mHasPairedThread(false);
std::thread::id           Log::mPairedThreadID;
Log*                      Log::mLog(nullptr);
std::ostream              Log::mCNull(0); // See: http://stackoverflow.com/questions/6240950/platform-independent-dev-null-in-c 

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

  // Check to see if any thread has paired.
  if (mHasPairedThread.exchange(true) == false)
  {
    // No thread has paired, we are free to pair.
    mPairedThreadID = std::this_thread::get_id();
    mLog = this;

    message() << "================================================================================" << std::endl;
    message() << "Spire logging - Paired with thread " << std::this_thread::get_id() << std::endl;
    message() << "================================================================================" << std::endl;
  }
  else
  {
    // Do nothing, since another thread has paired.
  }
}

//------------------------------------------------------------------------------
Log::~Log()
{
  message() << "Destroying spire logging class." << std::endl;
  if (mOutputFile.is_open())
    mOutputFile.close();

  if (    mHasPairedThread.load() == true 
      &&  mPairedThreadID == std::this_thread::get_id())
  {
    mHasPairedThread.store(false);
  }
}

//------------------------------------------------------------------------------
std::ostream& Log::debug()
{
  if (    mHasPairedThread.load() == true 
      &&  mPairedThreadID == std::this_thread::get_id())
  {
    return mLog->mDebugStream;
  }
  else
  {
    return mCNull;
  }
}

//------------------------------------------------------------------------------
std::ostream& Log::message()
{
  if (    mHasPairedThread.load() == true 
      &&  mPairedThreadID == std::this_thread::get_id())
  {
    return mLog->mMessageStream;
  }
  else
  {
    return mCNull;
  }
}

//------------------------------------------------------------------------------
std::ostream& Log::warning()
{
  if (    mHasPairedThread.load() == true 
      &&  mPairedThreadID == std::this_thread::get_id())
  {
    return mLog->mWarningStream;
  }
  else
  {
    return mCNull;
  }
}

//------------------------------------------------------------------------------
std::ostream& Log::error()
{
  if (    mHasPairedThread.load() == true 
      &&  mPairedThreadID == std::this_thread::get_id())
  {
    return mLog->mErrorStream;
  }
  else
  {
    return mCNull;
  }
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