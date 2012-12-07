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

#include "High/Hub.h"
#include <sstream>

namespace Spire {

/// Simple interface for stream logging.
/// We override std::stringbuf in order to capture the virtual 'sync' call
/// issued when an std::endl is sent to the stream.
class Log
{
public:
  Log(Hub::LogFunction logFunction);
  virtual ~Log();
  
  /// Retrieves the debug stream.
  std::ostream& getDebugStream()   {return mDebugStream;}

  /// Retrieves the message stream.
  std::ostream& getMessageStream() {return mMessageStream;}
  
  /// Retrieves the warning stream.
  std::ostream& getWarningStream() {return mWarningStream;}

  /// Retrieves the error stream.
  std::ostream& getErrorStream()   {return mErrorStream;}

private:

  /// Our custom stream class.
  class CustomStream : public std::ostream
  {
  public:
    CustomStream(const Hub::LogFunction& fun, Interface::LOG_LEVEL level) :
        std::ostream(&mBuf),
        mBuf(fun, level)
    {}

  private:
    /// This class override's stringbuf's virtual 'sync' method.
    class CustomStringBuf : public std::stringbuf
    {
    public:
      CustomStringBuf(Hub::LogFunction fun, Interface::LOG_LEVEL level) :
          mLogFun(fun),
          mLevel(level)
      {}

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
      Hub::LogFunction      mLogFun;
      Interface::LOG_LEVEL  mLevel;
    };

    CustomStringBuf   mBuf;
  };

  /// The various streams.
  CustomStream     mDebugStream;
  CustomStream     mMessageStream;
  CustomStream     mWarningStream;
  CustomStream     mErrorStream;
};

} // namespace Spire 

#endif 
