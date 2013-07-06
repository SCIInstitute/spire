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
/// \date   July 2013

#ifndef SPIRE_STUINTERFACE_LAMBDA_H
#define SPIRE_STUINTERFACE_LAMBDA_H

#include "../Core/LambdaInterface.h"

namespace Spire 
{

class StuObject;

/// StuPipe interface lambda.
class StuObjectLambda : public LambdaInterface
{
public:
  StuObjectLambda(Hub& hub, const std::string& pass, const StuObject& object) :
      LambdaInterface(hub, pass),
      mObject(object)
  {}
  virtual ~StuObjectLambda() {}

  /// Retrieves object spire attribute (as opposed to shader attributes).
  /// Does not depend on the currently active pass.
  template <class T>
  T getObjectSpireAttribute(const std::string& attribName)
  {
    std::shared_ptr<const AbstractUniformStateItem> uniformItem 
        = getObjectSpireAttribute(attribName);
    return uniformItem->getData<T>();
  }

  /// \todo Add pass uniform lookup *if needed*.
  /// getObjectPassUniform

  /// Retrieves the StuObject.
  const StuObject& getObject() {return mObject;}
  
private:

  std::shared_ptr<const AbstractUniformStateItem> getObjectSpireAttribute(
      const std::string& attribName);

  const StuObject&   mObject;
};

} // namespace 

#endif 
