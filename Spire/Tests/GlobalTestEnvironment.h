/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
/// \date   February 2013

#ifndef SPIRE_TESTS_GLOBALTESTENVIRONMENT_H
#define SPIRE_TESTS_GLOBALTESTENVIRONMENT_H

#include <exception>

#include "gtest/gtest.h"
#include "../Interface.h"

namespace Spire
{

/// Abstract class expected to be initialized and placed in the google testing
/// environment.
class GlobalTestEnvironment: public ::testing::Environment
{
public:

  static GlobalTestEnvironment* instance()
  {
    if (mInstance == nullptr)
      throw std::runtime_error("Global test environment not initialized!");
    return mInstance;
  }

  virtual std::shared_ptr<Spire::Context>   getContext() = 0;

protected:

  GlobalTestEnvironment()
  {
    if (mInstance != nullptr)
      throw std::runtime_error("Global test environment has already been constructed!");
    mInstance = this;
  }
  
  static GlobalTestEnvironment* mInstance;
};

} // namespace Spire

#endif 
