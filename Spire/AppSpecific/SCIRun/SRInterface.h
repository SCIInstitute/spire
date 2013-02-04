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

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H
#define SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H

// Be sure to include Spire/Interface.h before including this file!

namespace Spire {
namespace SCIRun {

/// A wrapper around spire that provides higher level functionality required
/// to operate SCIRun.
class SRInterface : public Interface
{
public:
  SRInterface(std::shared_ptr<Context> context,
              const std::vector<std::string>& shaderDirs,
              bool createThread, LogFunction logFP = LogFunction());
  virtual ~SRInterface();

  /// \todo Mouse movement (appropriate camera transform calls here...)

  /// \todo Selecting objects...

  /// \todo Obtaining data from mesh objects in order to spatially partition
  ///       them and provide quick object feedback.

private:

};

} // namespace SCIRun
} // namespace Spire

#endif 
