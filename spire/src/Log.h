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

#ifndef SPIRE_HIGH_LOG_H
#define SPIRE_HIGH_LOG_H

#include "../Interface.h"
#include <sstream>
#include <fstream>
#ifdef SPIRE_USE_STD_THREADS
#include <atomic>
#include <thread>
// In the future, when there are multiple spire threads, use a mutex to
// protect calls to a vector that stores std::this_thread::get_id()s
// and their associated Logging instances.
#include <mutex>
#include <map>
#endif

namespace CPM_SPIRE_NS {

/// Simple interface for stream logging.
/// We override std::stringbuf in order to capture the virtual 'sync' call
/// issued when an std::endl is sent to the stream.
class Log
{
public:
  Log(const Interface::LogFunction& logFunction);
  virtual ~Log();

  /// Retrieves the respective stream
  /// @{
  std::ostream& getDebugStream()   {return mDebugStream;}
  std::ostream& getMessageStream() {return mMessageStream;}
  std::ostream& getWarningStream() {return mWarningStream;}
  std::ostream& getErrorStream()   {return mErrorStream;}
  /// @}

  /// Retrieves the respective stream using a simple/hacky thread_local
  /// storage mechanism. Currently, logging is supported for only 1 thread.
  /// Will return false if a safe stream does not exist (not paired).
  /// @{
  static std::ostream& debug();
  static std::ostream& message();
  static std::ostream& warning();
  static std::ostream& error();
  /// @}

private:

  /// Our custom stream class.
  class CustomStream : public std::ostream
  {
  public:
    CustomStream(const Interface::LogFunction& fun, Interface::LOG_LEVEL level) :
        std::ostream(&mBuf),
        mBuf(fun, level)
    {}

    void setLogFunction(const Interface::LogFunction& fun)
    {
      mBuf.setLogFunction(fun);
    }

  private:
    /// This class override's stringbuf's virtual 'sync' method.
    class CustomStringBuf : public std::stringbuf
    {
    public:
      CustomStringBuf(Interface::LogFunction fun, Interface::LOG_LEVEL level) :
          mLogFun(fun),
          mLevel(level)
      {}
      
      void setLogFunction(const Interface::LogFunction& fun)
      {
        mLogFun = fun;
      }

    protected:
      virtual int sync()
      {
        if (mLogFun != nullptr)
        {
          std::string outputStr = str();
          mLogFun(outputStr, mLevel);
          str("");
        }
        return 0;
      }

    private:
      Interface::LogFunction      mLogFun;
      Interface::LOG_LEVEL        mLevel;
    };

    CustomStringBuf   mBuf;
  };

  /// Generalized log function which writes output to mOutputFile.
  void logFunction(const std::string& msg, Interface::LOG_LEVEL level);

  /// The various streams.
  CustomStream      mDebugStream;
  CustomStream      mMessageStream;
  CustomStream      mWarningStream;
  CustomStream      mErrorStream;

  static std::ofstream     mOutputFile;     ///< Output file to use when
                                            ///< logging output. Will be closed
                                            ///< when static value is destroyed.

  /// Null output stream should NOT be accessed by multiple different threads.
  static std::ostream                     mCNull;

#ifdef SPIRE_USE_STD_THREADS
  /// Implementation of my own version of thread local storage based on thread
  /// IDs...
  static std::mutex                       mLogLookupLock; ///< Guard for map.
  static std::mutex                       mOutputLock;    ///< Guard for output.
  static std::map<std::thread::id, Log*>  mLogInstances;  ///< Map of log instances.
#else
  static Log*                             mLog;
#endif
};

} // namespace CPM_SPIRE_NS

#endif 
