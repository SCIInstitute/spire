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

#include <sstream>
#include "Interface.h"
#include "src/Exceptions.h"
#include "src/Hub.h"
#include "src/Log.h"
#include "src/InterfaceImplementation.h"
#include "src/SpireObject.h"

using namespace std::placeholders;

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
Interface::Interface(std::shared_ptr<Context> context,
                     const std::vector<std::string>& shaderDirs,
                     LogFunction logFP) :
    mHub(new Hub(context, shaderDirs, logFP))
{
  mImpl = mHub->getInterfaceImpl();
}

//------------------------------------------------------------------------------
Interface::~Interface()
{
  mImpl.reset();
  mHub.reset();
}

//------------------------------------------------------------------------------
void Interface::terminate()
{
}

//============================================================================
// CONCURRENT INTERFACE
//============================================================================

//------------------------------------------------------------------------------
void Interface::doFrame()
{
  mHub->doFrame();
}

//------------------------------------------------------------------------------
size_t Interface::getNumObjects() const
{
  return mImpl->getNumObjects();
}

//------------------------------------------------------------------------------
std::shared_ptr<SpireObject>
Interface::getObjectWithName(const std::string& name) const
{
  return mImpl->getObjectWithName(name);
}

//------------------------------------------------------------------------------
void Interface::clearGLResources()
{
  mImpl->clearGLResources();
}

//------------------------------------------------------------------------------
bool Interface::isObjectInPass(const std::string& object, const std::string& pass) const
{
  return mImpl->isObjectInPass(object, pass);
}

//------------------------------------------------------------------------------
bool Interface::hasPass(const std::string& pass) const
{
  return mImpl->hasPass(pass);
}


//------------------------------------------------------------------------------
bool Interface::beginFrame(bool makeContextCurrent)
{
  return mHub->beginFrame(makeContextCurrent);
}

//------------------------------------------------------------------------------
void Interface::endFrame()
{
  mHub->endFrame();
}

//------------------------------------------------------------------------------
void Interface::addVBO(const std::string& name,
                       const uint8_t* vboData, size_t vboSize,
                       const std::vector<std::string>& attribNames)
{
  mImpl->addConcurrentVBO(name, vboData, vboSize, attribNames);
}

//------------------------------------------------------------------------------
void Interface::addIBO(const std::string& name,
                       const uint8_t* iboData, size_t iboSize, IBO_TYPE type)
{
  mImpl->addConcurrentIBO(name, iboData, iboSize, type);
}

//------------------------------------------------------------------------------
void Interface::renderObject(const std::string& objectName,
                             const UnsatisfiedUniformCB& cb,
                             const std::string& pass)
{
  std::shared_ptr<SpireObject> obj = getObjectWithName(objectName);
  obj->renderPass(pass, cb);
}

//------------------------------------------------------------------------------
void Interface::makeCurrent()
{
  mHub->makeCurrent();
}

//------------------------------------------------------------------------------
void Interface::addPassToFront(const std::string& passName)
{
  mImpl->addPassToFront(passName);
}

//------------------------------------------------------------------------------
void Interface::addPassToBack(const std::string& passName)
{
  mImpl->addPassToBack(passName);
}

//------------------------------------------------------------------------------
void Interface::addObject(const std::string& objectName)
{
  mImpl->addObject(objectName);
}

//------------------------------------------------------------------------------
void Interface::removeObject(const std::string& objectName)
{
  mImpl->removeObject(objectName);
}

//------------------------------------------------------------------------------
void Interface::removeAllObjects()
{
  mImpl->removeAllObjects();
}

//------------------------------------------------------------------------------
void Interface::addVBO(const std::string& name,
                       std::shared_ptr<std::vector<uint8_t>> vboData,
                       const std::vector<std::string>& attribNames)
{
  mImpl->addVBO(name, vboData, attribNames);
}

//------------------------------------------------------------------------------
void Interface::removeVBO(const std::string& vboName)
{
  mImpl->removeVBO(vboName);
}

//------------------------------------------------------------------------------
void Interface::addIBO(const std::string& name,
                       std::shared_ptr<std::vector<uint8_t>> iboData,
                       IBO_TYPE type)
{
  mImpl->addIBO(name, iboData, type);
}

//------------------------------------------------------------------------------
void Interface::removeIBO(const std::string& iboName)
{
  mImpl->removeIBO(iboName);
}

//------------------------------------------------------------------------------
void Interface::addPassToObject(const std::string& object,
                                const std::string& program,
                                const std::string& vboName,
                                const std::string& iboName,
                                PRIMITIVE_TYPES type,
                                const std::string& pass,
                                const std::string& parentPass)
{
  mImpl->addPassToObject(object, program, vboName, iboName, type, pass, parentPass);
}

//------------------------------------------------------------------------------
void Interface::removePassFromObject(const std::string& object, const std::string& pass)
{
  mImpl->removePassFromObject(object, pass);
}

//------------------------------------------------------------------------------
void Interface::addObjectPassUniformConcrete(const std::string& object,
                                             const std::string& uniformName,
                                             std::shared_ptr<AbstractUniformStateItem> item,
                                             const std::string& pass)
{
  mImpl->addObjectPassUniformConcrete(object, uniformName, item, pass);
}


//------------------------------------------------------------------------------
void Interface::addObjectGlobalUniformConcrete(const std::string& object,
                                               const std::string& uniformName,
                                               std::shared_ptr<AbstractUniformStateItem> item)
{
  mImpl->addObjectGlobalUniformConcrete(object, uniformName, item);
}

//------------------------------------------------------------------------------
void Interface::addGlobalUniformConcrete(const std::string& uniformName,
                                         std::shared_ptr<AbstractUniformStateItem> item)
{
  mImpl->addGlobalUniformConcrete(uniformName, item);
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem>
Interface::getGlobalUniformConcrete(const std::string& uniformName)
{
  return mHub->getGlobalUniformStateMan().getGlobalUniform(uniformName);
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem>
Interface::getObjectPassUniformConcrete(const std::string& object,
                                        const std::string& uniformName,
                                        const std::string& pass)
{
  std::shared_ptr<SpireObject> obj = getObjectWithName(object);
  return obj->getPassUniform(pass, uniformName);
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem>
Interface::getObjectGlobalUniformConcrete(const std::string& object,
                                          const std::string& uniformName)
{
  std::shared_ptr<SpireObject> obj = getObjectWithName(object);
  return obj->getGlobalUniform(uniformName);
}


//------------------------------------------------------------------------------
void Interface::addObjectPassGPUState(const std::string& object, const GPUState& state,
                                      const std::string& pass)
{
  mImpl->addObjectPassGPUState(object, state, pass);
}

//------------------------------------------------------------------------------
void Interface::addShaderAttribute(const std::string& codeName, size_t numComponents,
                                   bool normalize, size_t size, Interface::DATA_TYPES type)
{
  mImpl->addShaderAttribute(codeName, numComponents, normalize, size, type);
}

//------------------------------------------------------------------------------
void Interface::addPersistentShader(const std::string& programName,
                                    const std::string& vertexShader,
                                    const std::string& fragmentShader)
{
  std::vector<std::tuple<std::string, SHADER_TYPES>> shaders;
  shaders.push_back(make_tuple(vertexShader, VERTEX_SHADER));
  shaders.push_back(make_tuple(fragmentShader, FRAGMENT_SHADER));
  addPersistentShader(programName, shaders);
}

//------------------------------------------------------------------------------
void Interface::addPersistentShader(const std::string& programName,
                                    const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders)
{
  mImpl->addPersistentShader(programName, shaders);
}

//------------------------------------------------------------------------------
void Interface::addLambdaBeginAllPasses(const PassLambdaFunction& fp)
{
  mImpl->addLambdaBeginAllPasses(fp);
}

//------------------------------------------------------------------------------
void Interface::addLambdaEndAllPasses(const PassLambdaFunction& fp)
{
  mImpl->addLambdaEndAllPasses(fp);
}

//------------------------------------------------------------------------------
void Interface::addLambdaPrePass(const PassLambdaFunction& fp, const std::string& pass)
{
  mImpl->addLambdaPrePass(fp, pass);
}

//------------------------------------------------------------------------------
void Interface::addLambdaPostPass(const PassLambdaFunction& fp, const std::string& pass)
{
  mImpl->addLambdaPostPass(fp, pass);
}

//------------------------------------------------------------------------------
void Interface::addLambdaObjectRender(const std::string& object, const ObjectLambdaFunction& fp, const std::string& pass)
{
  mImpl->addLambdaObjectRender(object, fp, pass);
}

//------------------------------------------------------------------------------
void Interface::addLambdaObjectUniforms(const std::string& object, const ObjectUniformLambdaFunction& fp, const std::string& pass)
{
  mImpl->addLambdaObjectUniforms(object, fp, pass);
}


//------------------------------------------------------------------------------
size_t Interface::loadProprietarySR5AssetFile(std::istream& stream,
                                              std::vector<uint8_t>& vbo,
                                              std::vector<uint8_t>& ibo)
{
  // Read default SCIRun asset header.
  std::string header = "SCR5";

  char headerStrIn_Raw[5];
  stream.read(headerStrIn_Raw, 4);
  headerStrIn_Raw[4] = '\0';
  std::string headerStrIn = headerStrIn_Raw;

  if (headerStrIn != header)
  {
    /// \todo Use more appropriate I/O exception.
    throw std::invalid_argument("Header does not match asset file.");
  }

  // Read in the number of meshes (this is ignored for now and the first mesh is
  // used).
  uint32_t numMeshes = 0;
  stream.read(reinterpret_cast<char*>(&numMeshes), sizeof(uint32_t));
  if (numMeshes == 0)
  {
    throw std::invalid_argument("Need at least one mesh in asset file.");
  }

  // Read in the first mesh (this is the only mesh we will read in)
  uint32_t numVertices = 0;
  stream.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));

  V3 position;
  V3 normal;

  // Reserve appropriate space in the vbo.
  size_t vboSize = sizeof(float) * 6 * numVertices;
  vbo.resize(vboSize); // linear complexity.
  float* vboPtr = reinterpret_cast<float*>(&vbo[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  for (size_t i = 0; i < numVertices; i++)
  {
    // Read position data
    stream.read(reinterpret_cast<char*>(&position.x), sizeof(float));
    stream.read(reinterpret_cast<char*>(&position.y), sizeof(float));
    stream.read(reinterpret_cast<char*>(&position.z), sizeof(float));
    vboPtr[0] = position.x;
    vboPtr[1] = position.y;
    vboPtr[2] = position.z;
    vboPtr += 3;

    // Read normal data
    stream.read(reinterpret_cast<char*>(&normal.x), sizeof(float));
    stream.read(reinterpret_cast<char*>(&normal.y), sizeof(float));
    stream.read(reinterpret_cast<char*>(&normal.z), sizeof(float));
    vboPtr[0] = normal.x;
    vboPtr[1] = normal.y;
    vboPtr[2] = normal.z;
    vboPtr += 3;
  }

  // Read in the IBO data.
  uint32_t numTriangles = 0;  // Will be counted when loading.
  uint32_t numFaces = 0;
  stream.read(reinterpret_cast<char*>(&numFaces), sizeof(uint32_t));

  // Worst case number of triangles: 2 * numFaces (all quads).
  // The following has pretty harsh algorithmic time -- 3*N. Can easily be sped
  // up by using the preprocessing program to determine how many triangles there
  // are in the object. This would reduce the complexity to 2*N.
  uint32_t numTrianglesWorstCase = numFaces * 2;
  size_t iboWorstCaseSize = numTrianglesWorstCase * sizeof(uint16_t) * 3;
  ibo.resize(iboWorstCaseSize); // linear in complexity
  uint16_t* iboPtr = reinterpret_cast<uint16_t*>(&ibo[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  for (size_t i = 0; i < numFaces; i++)
  {
    uint8_t numIndices;
    stream.read(reinterpret_cast<char*>(&numIndices), sizeof(uint8_t));
    if (numIndices == 3)
    {
      uint16_t index0;
      uint16_t index1;
      uint16_t index2;
      stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
      stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
      stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));
      iboPtr[0] = index0;
      iboPtr[1] = index1;
      iboPtr[2] = index2;

      iboPtr += 3;
      ++numTriangles;
    }
    else if (numIndices == 4)
    {
      // Two triangles
      {
        uint16_t index0;
        uint16_t index1;
        uint16_t index2;
        stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));

        iboPtr[0] = index0;
        iboPtr[1] = index1;
        iboPtr[2] = index2;

        iboPtr += 3;
      }

      {
        uint16_t index0;
        uint16_t index1;
        uint16_t index2;
        stream.read(reinterpret_cast<char*>(&index0), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index1), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(&index2), sizeof(uint16_t));

        iboPtr[0] = index0;
        iboPtr[1] = index1;
        iboPtr[2] = index2;

        iboPtr += 3;
      }

      numTriangles += 2;
    }

  }

  // Resize the IBO appropriately (this is the operation that can be eliminated
  // if we know how many triangles there will be beforehand).
  size_t realIBOSize = numTriangles * sizeof(uint16_t) * 3;
  ibo.resize(realIBOSize); // linear in complexity

  return numTriangles;
}


} // namespace CPM_SPIRE_NS 

