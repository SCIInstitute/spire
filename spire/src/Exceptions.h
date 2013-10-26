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

namespace CPM_SPIRE_NS {

class Exception : public std::runtime_error
{
public:
  Exception(std::string e) : std::runtime_error(e) {}
};

class ThreadException : public Exception
{
public:
  ThreadException(std::string e) : Exception(e) {}
};

class NotFound : public Exception
{
public:
  NotFound(std::string e) : Exception(e) {}
};

class UnsupportedException : public Exception
{
public:
  UnsupportedException(std::string e) : Exception(e) {}
};

class GLError : public Exception
{
public:
  GLError(std::string e) : Exception(e) {}
};

class ShaderAttributeNotFound : public NotFound
{
public:
  ShaderAttributeNotFound(std::string e) : NotFound(e) {}
};

class ShaderUniformNotFound : public NotFound
{
public:
  ShaderUniformNotFound(std::string e) : NotFound(e) {}
};

class ShaderUniformTypeError : public Exception
{
public:
  ShaderUniformTypeError(std::string e) : Exception(e) {}
};

class Duplicate : public Exception
{
public:
  Duplicate(std::string e) : Exception(e) {}
};

} // namespace CPM_SPIRE_NS

#endif // SPIRE_EXCEPTIONS_H
