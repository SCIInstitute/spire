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

#include "WinDLLExport.h"
#include "LambdaInterface.h"

CPM_NAMESPACE
namespace Spire {

class SpireObject;

/// StuPipe interface lambda.
class WIN_DLL ObjectLambdaInterface : public LambdaInterface
{
public:
  ObjectLambdaInterface(Hub& hub, const std::string& pass, const SpireObject& object) :
      LambdaInterface(hub, pass),
      mObject(object)
  {}
  virtual ~ObjectLambdaInterface() {}

  /// Retrieves object spire attribute (as opposed to shader attributes).
  /// Does not depend on the currently active pass.
  template <class T>
  T getObjectMetadata(const std::string& attribName)
  {
    std::shared_ptr<const AbstractUniformStateItem> uniformItem 
        = getObjectMetadata(attribName);
    return uniformItem->getData<T>();
  }

  /// \todo Add pass uniform lookup *if needed*.
  /// getObjectPassUniform

  /// Retrieves the SpireObject.
  const SpireObject& getObject() {return mObject;}
  
private:

  std::shared_ptr<const AbstractUniformStateItem> getObjectMetadata(
      const std::string& attribName);

  const SpireObject&   mObject;
};

} // namespace Spire
CPM_NAMESPACE

#endif 
