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
#include "Core/Exceptions.h"
#include "Core/Hub.h"
#include "Core/Log.h"
#include "Core/InterfaceImplementation.h"

using namespace std::placeholders;

namespace Spire {

//------------------------------------------------------------------------------
Interface::Interface(std::shared_ptr<Context> context,
                     const std::vector<std::string>& shaderDirs,
                     bool createThread, LogFunction logFP) :
    mHub(new Hub(context, shaderDirs, logFP, createThread))
{
}

//------------------------------------------------------------------------------
Interface::~Interface()
{
}

//------------------------------------------------------------------------------
void Interface::terminate()
{
  if (mHub->isRendererThreadRunning())
  {
    mHub->killRendererThread();
  }
}

//------------------------------------------------------------------------------
void Interface::ntsDoFrame()
{
  if (mHub->isRendererThreadRunning())
    throw ThreadException("You cannot call doFrame when the renderer is "
                          "running in a separate thread.");

  mHub->doFrame();
}

//------------------------------------------------------------------------------
size_t Interface::ntsGetNumObjects() const
{
  return mHub->getInterfaceImpl()->getNumObjects();
}

//------------------------------------------------------------------------------
std::shared_ptr<const SpireObject> Interface::ntsGetObjectWithName(const std::string& name) const
{
  return mHub->getInterfaceImpl()->getObjectWithName(name);
}

//------------------------------------------------------------------------------
void Interface::ntsClearGLResources()
{
  mHub->getInterfaceImpl()->clearGLResources();
}

//------------------------------------------------------------------------------
bool Interface::ntsIsObjectInPass(const std::string& object, const std::string& pass) const
{
  return mHub->getInterfaceImpl()->isObjectInPass(object, pass);
}

//------------------------------------------------------------------------------
bool Interface::ntsHasPass(const std::string& pass) const
{
  return mHub->getInterfaceImpl()->hasPass(pass);
}

//------------------------------------------------------------------------------
void Interface::addPassToFront(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPassToFront, _1, passName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addPassToBack(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPassToBack, _1, passName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObject(const std::string& objectName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObject, _1, objectName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeObject(const std::string& objectName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeObject, _1, objectName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeAllObjects()
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeAllObjects, _1);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addVBO(const std::string& name,
                       std::shared_ptr<std::vector<uint8_t>> vboData,
                       const std::vector<std::string>& attribNames)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addVBO, _1, name, vboData, attribNames);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeVBO(const std::string& vboName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeVBO, _1, vboName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addIBO(const std::string& name,
                       std::shared_ptr<std::vector<uint8_t>> iboData,
                       IBO_TYPE type)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addIBO, _1, name, iboData, type);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeIBO(const std::string& iboName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeIBO, _1, iboName);
  mHub->addFunctionToThreadQueue(fun);
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
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPassToObject, _1, object, program, 
                vboName, iboName, type, pass, parentPass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removePassFromObject(const std::string& object, const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removePassFromObject, _1, object, pass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObjectPassUniformConcrete(const std::string& object,
                                             const std::string& uniformName,
                                             std::shared_ptr<AbstractUniformStateItem> item,
                                             const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObjectPassUniformConcrete, _1,
                object, uniformName, item, pass);
  mHub->addFunctionToThreadQueue(fun);
}


//------------------------------------------------------------------------------
void Interface::addObjectGlobalUniformConcrete(const std::string& object,
                                               const std::string& uniformName,
                                               std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObjectGlobalUniformConcrete, _1, object, uniformName, item);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addGlobalUniformConcrete(const std::string& uniformName,
                                         std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addGlobalUniformConcrete, _1, uniformName, item);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObjectPassGPUState(const std::string& object, const GPUState& state,
                                      const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObjectPassGPUState, _1, object, state, pass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addShaderAttribute(const std::string& codeName, size_t numComponents,
                                   bool normalize, size_t size, Interface::DATA_TYPES type)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addShaderAttribute, _1, codeName, numComponents, 
                normalize, size, type);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObjectGlobalMetadataConcrete(const std::string& object,
                                                const std::string& attributeName,
                                                std::shared_ptr<AbstractUniformStateItem> item)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObjectGlobalMetadataConcrete, _1, object, attributeName, item);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObjectPassMetadataConcrete(const std::string& object,
                                              const std::string& attributeName,
                                              std::shared_ptr<AbstractUniformStateItem> item,
                                              const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObjectPassMetadataConcrete, _1, object, attributeName, item, passName);
  mHub->addFunctionToThreadQueue(fun);
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
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPersistentShader, _1, programName, shaders);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaBeginAllPasses(const PassLambdaFunction& fp)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaBeginAllPasses, _1, fp);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaEndAllPasses(const PassLambdaFunction& fp)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaEndAllPasses, _1, fp);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaPrePass(const PassLambdaFunction& fp, const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaPrePass, _1, fp, pass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaPostPass(const PassLambdaFunction& fp, const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaPostPass, _1, fp, pass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaObjectRender(const std::string& object, const ObjectLambdaFunction& fp, const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaObjectRender, _1, object ,fp, pass);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addLambdaObjectUniforms(const std::string& object, const ObjectUniformLambdaFunction& fp, const std::string& pass)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addLambdaObjectUniforms, _1, object ,fp, pass);
  mHub->addFunctionToThreadQueue(fun);
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


} // end of namespace Renderer

