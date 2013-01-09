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
/// \date   January 2013
/// \brief  Common GL math utilities.

#include "Common.h"
#include "GLMathUtil.h"

namespace Spire {

void M33toArray9(const M33& in, GLfloat out[9])
{
	// OpenGL is column major.

	// Column 1
	out[0 ] = in.getElem(0,0);
	out[1 ] = in.getElem(0,1);
	out[2 ] = in.getElem(0,2);

	// Column 2
	out[3 ] = in.getElem(1,0);
	out[4 ] = in.getElem(1,1);
	out[5 ] = in.getElem(1,2);

	// Column 3
	out[6 ] = in.getElem(2,0);
	out[7 ] = in.getElem(2,1);
	out[8 ] = in.getElem(2,2);
}

void M44toArray16(const M44& in, GLfloat out[16])
{
	// OpenGL is column major.

	// Column 1
	out[0 ] = in.getElem(0,0);
	out[1 ] = in.getElem(0,1);
	out[2 ] = in.getElem(0,2);
	out[3 ] = in.getElem(0,3);

	// Column 2
	out[4 ] = in.getElem(1,0);
	out[5 ] = in.getElem(1,1);
	out[6 ] = in.getElem(1,2);
	out[7 ] = in.getElem(1,3);

	// Column 3
	out[8 ] = in.getElem(2,0);
	out[9 ] = in.getElem(2,1);
	out[10] = in.getElem(2,2);
	out[11] = in.getElem(2,3);

	// Column 4
	out[12] = in.getElem(3,0);
	out[13] = in.getElem(3,1);
	out[14] = in.getElem(3,2);
	out[15] = in.getElem(3,3);
}

void V3toArray3(const V3& in, GLfloat out[3])
{
	out[0] = in.getX();
	out[1] = in.getY();
	out[2] = in.getZ();
}

void V3toArray4(const V3& in, GLfloat out[4])
{
	V3toArray3(in, out);
	out[3] = 1.0f;
}

void V4toArray3(const V4& in, GLfloat out[3])
{
	out[0] = in.getX();
	out[1] = in.getY();
	out[2] = in.getZ();
}

void V4toArray4(const V4& in, GLfloat out[4])
{
	V4toArray3(in, out);
	out[3] = in.getW();
}

} // end of namespace Spire

