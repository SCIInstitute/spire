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

#include "High/GLMathUtil.h"
#include "High/Camera.h"

namespace Spire {

//------------------------------------------------------------------------------
TestUniformColor::TestUniformColor(Hub& hub) :
    mHub(hub)
{
  Log::message() << "Testing UniformColor shader." << std::endl;

  // Create VBO.
  float vertexData[] = {0.0f, 0.0f, 1.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f};
  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

  // Create index buffer.
  uint16_t elementData[] = {0, 1, 2};
  glGenBuffers(1, &mIndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData,
               GL_STATIC_DRAW);

  // Construct a list of appropriate shaders.
  std::list<std::tuple<std::string, GLenum>> shaders = {
    {"UniformColor.vs", GL_VERTEX_SHADER},
    {"UniformColor.fs", GL_FRAGMENT_SHADER} };

  mShader = mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);
}

//------------------------------------------------------------------------------
TestUniformColor::~TestUniformColor()
{
}

//------------------------------------------------------------------------------
void TestUniformColor::doFrame()
{
  GLuint program = mShader->getProgramID();
  Camera cam = mHub.getCamera();

  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

  // Obtain the first attribute in the shader (should be position).
  AttribState pos = mHub.getShaderAttributeManager().getAttributeWithName("aPos");

  // Grab pointers to attributes and uniforms.
  const ShaderAttributeCollection& attribs  = mShader->getAttributes();
  const ShaderUniformCollection&   uniforms = mShader->getUniforms();

  // Sanity check: Ensure the shader knows about our attributes.
  if (attribs.hasAttribute(pos.codeName) == false)
    throw GLError("Unable to find appropriate shader position attribute.");

  GLsizei stride = static_cast<GLsizei>(attribs.calculateStride());
  size_t offset = 0;

  // Manually bind attribute locations (this can also be done through
  // ShaderAttributeCollection::bindAttributes)
  glBindAttribLocation(program, 0, pos.codeName.c_str());
  glVertexAttribPointer(0, 
                        static_cast<GLint>(pos.numComponents), pos.type,
                        static_cast<GLboolean>(pos.normalize), stride,
                        static_cast<const GLvoid*>(&offset));
  offset += pos.size; // This does nothing, but would help if there was another
                      // attribute in the vbo.

  // Setup the program state.
  glUseProgram(program);

  GLint unifLoc;
  GLfloat tmpGLMat[16];

  // Projection * Inverse View * World transformation.
  M44 PIV = cam.getWorldToProjection();
  /// \todo Add a rotation about the y axis.
  unifLoc = glGetUniformLocation(program, "uProjIVWorld");
  M44toArray16(PIV, tmpGLMat);
  glUniformMatrix4fv(unifLoc, 1, GL_FALSE, tmpGLMat);

  // Color setup
  GLfloat color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  unifLoc = glGetUniformLocation(program, "uColor");
  glUniform4fv(unifLoc, 1, color);

  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
}

} // end of namespace Spire

//// The code below was meant to automatically place attributes in the correct
//// positions. We don't need this kind of general mechanism yet.
//  GLsizei stride = static_cast<GLsizei>(attribs.calculateStride());
//  size_t offset = 0;
//
//  uint32_t nameHash = 0;
//  for (size_t i = 0; i < attribs.getNumAttributes(); ++i)
//  {
//    /// \todo Want constexpr string hashing mechanism.
//    ///       That way we can use a case statement.
//    nameHash = attribs.getAttribute(i).attrib.nameHash;
//    if (nameHash == pos.nameHash)
//    {
//      
//    }
//    else
//    {
//
//    }
//  }
