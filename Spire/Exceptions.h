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
/// \brief  Defines the exceptions used in the Spire renderer.

#ifndef SPIRE_EXCEPTIONS_H
#define SPIRE_EXCEPTIONS_H

#include <string>
#include <stdexcept>

namespace Spire {

/// TODO: Get rid of these ifdefs when we switch to a visual studio that
///       supports noexcept (VS 2010 and VS 2012 do not).
#ifndef SPIRE_USING_WIN
  #define SPIRE_NOEXCEPT noexcept
#else
  #define SPIRE_NOEXCEPT
#endif

// Basic Spire exception.
class Exception : virtual public std::exception
{
public:
  Exception() {}
  Exception(std::string e) : mError(e) {}
  virtual ~Exception() SPIRE_NOEXCEPT {}
  virtual const char* what() const SPIRE_NOEXCEPT {return this->mError.c_str();}

protected:
  std::string mError;
};

class ThreadException : virtual public Exception
{
public:
  ThreadException() : Exception(std::string("unknown error")) {}
  ThreadException(std::string e) : Exception(e) {}
  virtual ~ThreadException() SPIRE_NOEXCEPT {}
};

class NotFound : virtual public Exception
{
public:
  NotFound() : Exception(std::string("unknown error")) {}
  NotFound(std::string e) : Exception(e) {}
  virtual ~NotFound() SPIRE_NOEXCEPT {}
};

class UnsupportedException : virtual public Exception
{
public:
  UnsupportedException() : Exception(std::string("unknown error")) {}
  UnsupportedException(std::string e) : Exception(e) {}
  virtual ~UnsupportedException() SPIRE_NOEXCEPT {}
};

class GLError : virtual public Exception
{
public:
  GLError() : Exception(std::string("unknown error")) {}
  GLError(std::string e) : Exception(e) {}
  virtual ~GLError() SPIRE_NOEXCEPT {}
};

class ShaderAttributeNotFound : virtual public NotFound
{
public:
  ShaderAttributeNotFound() : NotFound(std::string("unknown error")) {}
  ShaderAttributeNotFound(std::string e) : Exception(e) {}
  virtual ~ShaderAttributeNotFound() SPIRE_NOEXCEPT {}
};

class ShaderUniformNotFound : virtual public NotFound
{
public:
  ShaderUniformNotFound() : NotFound(std::string("unknown error")) {}
  ShaderUniformNotFound(std::string e) : NotFound(e) {}
  virtual ~ShaderUniformNotFound() SPIRE_NOEXCEPT {}
};

class Duplicate : virtual public Exception
{
public:
  Duplicate() : Exception(std::string("unknown error")) {}
  Duplicate(std::string e) : Exception(e) {}
  virtual ~Duplicate() SPIRE_NOEXCEPT {}
};

} // namespace Spire

#endif // SPIRE_EXCEPTIONS_H
