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

#include "Core/Log.h"
#include "Exceptions.h"

namespace Spire {

#ifdef SPIRE_USE_STD_THREADS
std::mutex                      Log::mLogLookupLock;
std::mutex                      Log::mOutputLock;
std::map<std::thread::id, Log*> Log::mLogInstances;
#else
Log*                            Log::mLog(nullptr);
#endif

std::ostream                    Log::mCNull(0);
std::ofstream                   Log::mOutputFile;

//------------------------------------------------------------------------------
Log::Log(const Interface::LogFunction& logFunction) :
    mDebugStream(logFunction, Interface::LOG_DEBUG),
    mMessageStream(logFunction, Interface::LOG_MESSAGE),
    mWarningStream(logFunction, Interface::LOG_WARNING),
    mErrorStream(logFunction, Interface::LOG_ERROR)
{
  Interface::LogFunction fun = logFunction;
  if (fun == nullptr)
  {
#ifndef WIN32
    // Scoped lock guard
    {
#ifdef SPIRE_USE_STD_THREADS
      std::lock_guard<std::mutex> lock(mLogLookupLock);
#endif

      if (mOutputFile.is_open() == false)
      {
        // When serializing the log data we will be under the log lookup mutex lock.
        std::stringstream osFilename;
        osFilename << "/tmp/SpireLog";//_" << std::this_thread::get_id();
        mOutputFile.open(osFilename.str());
      }
    }

    fun = std::bind(&Log::logFunction, this,
                    std::placeholders::_1, 
                    std::placeholders::_2);

    // Reset all of the streams' logging functions.
    mDebugStream.setLogFunction(fun);
    mMessageStream.setLogFunction(fun);
    mWarningStream.setLogFunction(fun);
    mErrorStream.setLogFunction(fun);
#endif
  }

#ifdef SPIRE_USE_STD_THREADS
  // Add our thread to the mutex locked map (we will be unique).
  std::thread::id threadID = std::this_thread::get_id();

  // Scope lock the mutex ensuring exception safety with RAII.
  {
    std::lock_guard<std::mutex> lock(mLogLookupLock);
    mLogInstances.insert(make_pair(threadID, this));
  }
#endif
}

//------------------------------------------------------------------------------
Log::~Log()
{
#ifdef SPIRE_USE_STD_THREADS
  std::thread::id threadID = std::this_thread::get_id();
  std::lock_guard<std::mutex> lock(mLogLookupLock);
  mLogInstances.erase(threadID);
#endif
}

//------------------------------------------------------------------------------
std::ostream& Log::debug()
{
#ifdef SPIRE_USE_STD_THREADS
  // Find our logger.
  std::thread::id threadID = std::this_thread::get_id();
  std::lock_guard<std::mutex> lock(mLogLookupLock);
  auto it = mLogInstances.find(threadID);
  if (it != mLogInstances.end())
  {
    return it->second->mDebugStream;
  }
  else
  {
    throw new Exception("Could not find thread in logging map.");
    // Returning this is dangerous in a multithreaded environment. Hence
    // the exception.
    return mCNull;
  }
#else
  // We will always be single threaded.
  if (mLog == nullptr) mLog = new Log(nullptr);
  return mLog->mDebugStream;
#endif
}

//------------------------------------------------------------------------------
std::ostream& Log::message()
{
#ifdef SPIRE_USE_STD_THREADS
  // Find our logger.
  std::thread::id threadID = std::this_thread::get_id();
  std::lock_guard<std::mutex> lock(mLogLookupLock);
  auto it = mLogInstances.find(threadID);
  if (it != mLogInstances.end())
  {
    return it->second->mMessageStream;
  }
  else
  {
    throw new Exception("Could not find thread in logging map.");
    // Returning this is dangerous in a multithreaded environment. Hence
    // the exception.
    return mCNull;
  }
#else
  if (mLog == nullptr) mLog = new Log(nullptr);
  return mLog->mMessageStream;
#endif
}

//------------------------------------------------------------------------------
std::ostream& Log::warning()
{
#ifdef SPIRE_USE_STD_THREADS
  // Find our logger.
  std::thread::id threadID = std::this_thread::get_id();
  std::lock_guard<std::mutex> lock(mLogLookupLock);
  auto it = mLogInstances.find(threadID);
  if (it != mLogInstances.end())
  {
    return it->second->mWarningStream;
  }
  else
  {
    throw new Exception("Could not find thread in logging map.");
    // Returning this is dangerous in a multithreaded environment. Hence
    // the exception.
    return mCNull;
  }
#else
  if (mLog == nullptr) mLog = new Log(nullptr);
  return mLog->mWarningStream;
#endif
}

//------------------------------------------------------------------------------
std::ostream& Log::error()
{
#ifdef SPIRE_USE_STD_THREADS
  // Find our logger.
  std::thread::id threadID = std::this_thread::get_id();
  std::lock_guard<std::mutex> lock(mLogLookupLock);
  auto it = mLogInstances.find(threadID);
  if (it != mLogInstances.end())
  {
    return it->second->mErrorStream;
  }
  else
  {
    throw new Exception("Could not find thread in logging map.");
    // Returning this is dangerous in a multithreaded environment. Hence
    // the exception.
    return mCNull;
  }
#else
  if (mLog == nullptr) mLog = new Log(nullptr);
  return mLog->mErrorStream;
#endif
}

//------------------------------------------------------------------------------
void Log::logFunction(const std::string& msg, Interface::LOG_LEVEL level)
{
#ifdef SPIRE_USE_STD_THREADS
  std::lock_guard<std::mutex> lock(mOutputLock);
  std::thread::id threadID = std::this_thread::get_id();
#else
  int threadID = 0;
#endif

  switch (level)
  {
    case Interface::LOG_DEBUG:
      //mOutputFile << "Debug:   " << msg;
      mOutputFile << "(" << threadID << ") - " << msg;
      break;

    case Interface::LOG_MESSAGE:
      //mOutputFile << "General: " << msg;
      mOutputFile << "(" << threadID << ") - " << msg;
      break;

    case Interface::LOG_WARNING:
      //mOutputFile << "Warning: " << msg;
      mOutputFile << "(" << threadID << ") - " << msg;
      break;

    case Interface::LOG_ERROR:
      //mOutputFile << "Error:   " << msg;
      mOutputFile << "(" << threadID << ") - " << msg;
      break;
  }
  mOutputFile.flush();
}

} // end of namespace Spire
