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

#include <cstdlib>

#include "Common.h"
#include "Exceptions.h"
#include "HackedUCRenderer.h"

#include "Core/GLMathUtil.h"
#include "Core/Camera.h"

namespace Spire {

//------------------------------------------------------------------------------
HackedUCRenderer::HackedUCRenderer(Hub& hub) :
    mHub(hub),
    mCommonVBO(0),
    mFaceIBO(0),
    mNumFaceElements(0),
    mEdgeIBO(0),
    mNumEdgeElements(0)
{
  Log::message() << "Initializing hacked uniform color renderer." << std::endl;

  // Build shader program using C++11 initializer lists.
  /// \todo Change back to initializer lists once VS supports it.
  std::list<std::tuple<std::string, GLenum>> shaders;
  shaders.push_back(std::make_tuple("UniformColor.vs", GL_VERTEX_SHADER));
  shaders.push_back(std::make_tuple("UniformColor.fs", GL_FRAGMENT_SHADER));
  //  { {"UniformColor.vs", GL_VERTEX_SHADER},
  //    {"UniformColor.fs", GL_FRAGMENT_SHADER} };

  mShader = mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);
}

//------------------------------------------------------------------------------
HackedUCRenderer::~HackedUCRenderer()
{
  /// \todo Cleanup rendering data.
}

//------------------------------------------------------------------------------
void HackedUCRenderer::doFrame()
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

  if (mCommonVBO != 0 && mEdgeIBO != 0)
  {
    glBindBuffer(GL_ARRAY_BUFFER, mCommonVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEdgeIBO);

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
    GLfloat tmpV4[4];
    V4toArray4(mEdgeColor, tmpV4);
    unifLoc = glGetUniformLocation(program, "uColor");
    glUniform4fv(unifLoc, 1, tmpV4);

    // Big INDICES! We should batch together the larger models...
    glDrawElements(GL_LINES, mNumEdgeElements, GL_UNSIGNED_INT, 0);
  }

  if (mCommonVBO != 0 && mFaceIBO != 0)
  {
    glBindBuffer(GL_ARRAY_BUFFER, mCommonVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFaceIBO);

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
    GLfloat tmpV4[4];
    V4toArray4(mFaceColor, tmpV4);
    unifLoc = glGetUniformLocation(program, "uColor");
    glUniform4fv(unifLoc, 1, tmpV4);

    // GL_UNSIGNED_SHORT
    glDrawElements(GL_TRIANGLES, mNumFaceElements, GL_UNSIGNED_INT, 0);
  }

}

//------------------------------------------------------------------------------
void HackedUCRenderer::setCommonVBO(uint8_t* vbo, size_t vboSize)
{
  if (mCommonVBO != 0)
  {
    glDeleteBuffers(1, &mCommonVBO);
  }

  glGenBuffers(1, &mCommonVBO);
  glBindBuffer(GL_ARRAY_BUFFER, mCommonVBO);
  glBufferData(GL_ARRAY_BUFFER, vboSize, vbo, GL_STATIC_DRAW);

  std::free(vbo);
}

//------------------------------------------------------------------------------
void HackedUCRenderer::setEdgeColor(const V4& color)
{
  mEdgeColor = color;
}

//------------------------------------------------------------------------------
void HackedUCRenderer::setEdgeData(uint8_t* ibo, size_t iboSize)
{
  // Delete pre-existing buffers (if any)
  if (mEdgeIBO != 0)
  {
    glDeleteBuffers(1, &mEdgeIBO);
  }

  glGenBuffers(1, &mEdgeIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEdgeIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, ibo, GL_STATIC_DRAW);

  mNumEdgeElements = iboSize / sizeof(uint32_t);

  std::free(ibo);
}

//------------------------------------------------------------------------------
void HackedUCRenderer::setFaceColor(const V4& color)
{
  mFaceColor = color;
}

//------------------------------------------------------------------------------
void HackedUCRenderer::setFaceData(uint8_t* ibo, size_t iboSize)
{
  // Delete pre-existing buffers (if any)
  if (mFaceIBO != 0)
  {
    glDeleteBuffers(1, &mFaceIBO);
  }

  // Build GL buffers.
  glGenBuffers(1, &mFaceIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFaceIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, ibo, GL_STATIC_DRAW);

  mNumFaceElements = iboSize / sizeof(uint32_t);

  std::free(ibo);
}


} // namespace Spire
