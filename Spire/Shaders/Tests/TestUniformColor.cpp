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

#include "Common.h"
#include "Exceptions.h"
#include "TestUniformColor.h"

#include "Core/GLMathUtil.h"
#include "Core/Camera.h"

namespace Spire {

//------------------------------------------------------------------------------
TestUniformColor::TestUniformColor(Hub& hub) :
    mHub(hub)
{
  Log::message() << "Testing UniformColor shader." << std::endl;

  // Build shader program using C++11 initializer lists.
  /// \todo Change back to initializer lists once VS supports it.
  std::list<std::tuple<std::string, GLenum>> shaders;
  shaders.push_back(std::make_tuple("UniformColor.vs", GL_VERTEX_SHADER));
  shaders.push_back(std::make_tuple("UniformColor.fs", GL_FRAGMENT_SHADER));
  //  { {"UniformColor.vs", GL_VERTEX_SHADER},
  //    {"UniformColor.fs", GL_FRAGMENT_SHADER} };

  mShader = mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);

  // Create VBO.
  GLfloat vertexData[] = {-1.0f,  1.0f, 10.0f,
                           1.0f,  1.0f, 10.0f,
                          -1.0f, -1.0f, 10.0f,
                           1.0f, -1.0f, 10.0f};
  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

  // Create index buffer.
  uint16_t elementData[] = {0, 1, 2, 3};
  glGenBuffers(1, &mIndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData,
               GL_STATIC_DRAW);
}

//------------------------------------------------------------------------------
TestUniformColor::~TestUniformColor()
{
}

//------------------------------------------------------------------------------
void TestUniformColor::doFrame()
{
  GLuint program = mShader->getProgramID();
  std::shared_ptr<Camera> cam = mHub.getCamera();

  // Obtain the first attribute in the shader (should be position).
  AttribState pos = mHub.getShaderAttributeManager().getAttributeWithName("aPos");

  // Grab pointers to attributes and uniforms.
  const ShaderAttributeCollection& attribs  = mShader->getAttributes();
  //const ShaderUniformCollection&   uniforms = mShader->getUniforms();

  // Sanity check: Ensure the shader knows about our attributes.
  if (attribs.hasAttribute(pos.codeName) == false)
    throw GLError("Unable to find appropriate shader position attribute.");

  // Setup shader.
  glUseProgram(program);

  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

  GLsizei stride = static_cast<GLsizei>(attribs.calculateStride());

  // Manually bind attribute locations (this can also be done through
  // ShaderAttributeCollection::bindAttributes)
  // We can also set the index of the attribute.
  GLuint attribPos = glGetAttribLocation(program, "aPos");
  glEnableVertexAttribArray(attribPos);
  glVertexAttribPointer(attribPos,
                        static_cast<GLint>(pos.numComponents),
                        pos.type, static_cast<GLboolean>(pos.normalize),
                        stride, NULL); 
  // The NULL pointer is critical. Would normally use this pointer as an offset
  // when dealing with multiple attributes in the same buffer.

  GLint unifLoc;
  GLfloat tmpGLMat[16];

  // Projection * Inverse View * World transformation.
  M44 PIV = cam->getWorldToProjection();
  unifLoc = glGetUniformLocation(program, "uProjIVWorld");
  M44toArray16(PIV, tmpGLMat);
  glUniformMatrix4fv(unifLoc, 1, GL_FALSE, tmpGLMat);

  // Color setup
  GLfloat color[4] = {1.0f, 0.0f, 1.0f, 1.0f};
  unifLoc = glGetUniformLocation(program, "uColor");
  glUniform4fv(unifLoc, 1, color);

  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
  //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


  // Now draw the same thing without buffers...
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLfloat vVertices[] = {
    -0.5f,  0.5f,  0.5f,  // TL
     0.5f,  0.5f,  0.5f,  // TR
    -0.5f, -0.5f,  0.5f,  // BL
     0.5f, -0.5f,  0.5f};	// BR

	stride = sizeof(float) * 3;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, vVertices);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // end of namespace Spire

