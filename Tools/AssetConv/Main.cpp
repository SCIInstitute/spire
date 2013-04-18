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
/// \date   April 2013
/// \brief  Simple asset conversion from a number of different input formats
///         into a simple binary format that is easily loaded into the SCIRun5
///         AppSpecific section of the code.

#include <algorithm>
#include <string>
#include <map>
#include <fstream>

// Include vector library.
#include "../../Spire/Core/Math.h"

// Devil image library.
#include <IL/il.h>
#include <IL/ilu.h>

// TCLAP command line parser
#include "../3rdParty/tclap/include/tclap/CmdLine.h"

// assimp include files. These three are usually needed.
#include <assimp/assimp.hpp>	//OO version Header!
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#include <assimp/DefaultLogger.h>
#include <assimp/LogStream.h>

//------------------------------------------------------------------------------
void createAssimpLogger()
{
  Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
  Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
  //Assimp::DefaultLogger::get()->info("Info-call");
}

//------------------------------------------------------------------------------
void destroyAssimpLogger()
{
  Assimp::DefaultLogger::kill();
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  std::string inFile;
  std::string outFile;
  try
  {
    TCLAP::CmdLine cmd("Asset Converter");
    TCLAP::ValueArg<std::string> input("i", "input", "Input file.", true, "", "String");
    TCLAP::ValueArg<std::string> output("o", "output", "Output file.", true, "", "String");

    //cmd.xorAdd(input, directory);
    cmd.add(input);
    cmd.add(output);
    cmd.parse(argc, argv);

    inFile = input.getValue();
    outFile = output.getValue();
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP exception: " << e.error() << " associated with arg " << e.argId() << "\n";
    return EXIT_FAILURE;
  }

  std::cout << "Input file: " << inFile << std::endl;
  std::cout << "Output file: " << outFile << std::endl;

  // Initialize assimp.
  createAssimpLogger();
  std::cout << "Assimp initialized." << std::endl;

  // Determine IL Version 
  if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
  {
    ILint test = ilGetInteger(IL_VERSION_NUM);
    std::string errMsg = "Wrong DevIL version.";
    char* cErrMsg = (char*)errMsg.c_str();
    std::cout << "IL failure: " << cErrMsg << std::endl;
    return EXIT_FAILURE;
  }

  ilInit();
  
  const aiScene* scene = nullptr;
  Assimp::Importer importer;

  scene = importer.ReadFile(inFile,
    aiProcess_MakeLeftHanded |
    aiProcess_FlipWindingOrder |
    aiProcess_CalcTangentSpace |
    aiProcess_LimitBoneWeights |
    aiProcess_JoinIdenticalVertices |
    aiProcess_ImproveCacheLocality |
    aiProcess_LimitBoneWeights |
    aiProcess_RemoveRedundantMaterials |
    aiProcess_GenUVCoords |
    aiProcess_FindDegenerates |
    aiProcess_FindInvalidData |
    aiProcess_FindInstances |
    aiProcess_ValidateDataStructure |
    aiProcess_OptimizeMeshes); 

  if (!scene)
  {
    std::cout << "Error: " << importer.GetErrorString() << std::endl;
    return EXIT_FAILURE;
  }

  std::string header = "SCR5";

  // Open file for output.
  std::ofstream output(outFile, std::ofstream::binary);

  // Write out the file header.
  output.write(header.c_str(), header.length());

  // Write out number of meshes contained in the file.
  output.write(reinterpret_cast<const char*>(&scene->mNumMeshes), sizeof(uint32_t));

  /// \todo There are a lot of potential errors in the following code.
  ///       Most of the errors come from relying on unsigned int values
  ///       being readily convertable to uint32_t values.

  // Loop through every mesh and write out vertices, normals, and IBO data.
  for (size_t i = 0; i < scene->mNumMeshes; i++)
  {
    const struct aiMesh* mesh = scene->mMeshes[i];
    assert(mesh->mNumVertices > 0);

    // Write out VBO data.
    output.write(reinterpret_cast<const char*>(&mesh->mNumVertices), sizeof(uint32_t));
    for (size_t j = 0; j < mesh->mNumVertices; j++)
    {
      // Position data
      output.write(reinterpret_cast<const char*>(&mesh->mVertices[j].x), sizeof(float));
      output.write(reinterpret_cast<const char*>(&mesh->mVertices[j].y), sizeof(float));
      output.write(reinterpret_cast<const char*>(&mesh->mVertices[j].z), sizeof(float));

      // Normal data
      output.write(reinterpret_cast<const char*>(&mesh->mNormals[j].x), sizeof(float));
      output.write(reinterpret_cast<const char*>(&mesh->mNormals[j].y), sizeof(float));
      output.write(reinterpret_cast<const char*>(&mesh->mNormals[j].z), sizeof(float));
    }

    // Write out IBO data.
    output.write(reinterpret_cast<const char*>(&mesh->mNumFaces), sizeof(uint32_t));
    for (size_t j = 0; j < mesh->mNumFaces; j++)
    {
      if (mesh->mFaces[j].mNumIndices == 3)
      {
        // Handle triangles.

        // The following is a precaution until we can start using 32 bit data.
        // But we still write the indices out using 32 bit data.
        assert(mesh->mFaces[j].mIndices[0] < 65536);
        assert(mesh->mFaces[j].mIndices[1] < 65536);
        assert(mesh->mFaces[j].mIndices[2] < 65536);

        uint16_t index0 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[0]);
        uint16_t index1 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[1]);
        uint16_t index2 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[2]);
        output.write(reinterpret_cast<const char*>(&index0), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index1), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index2), sizeof(uint16_t));
      }
      else if (mesh->mFaces[j].mNumIndices == 4)
      {
        // Handle quads. Need to convert to triangles. Ensure to swap winding
        // order of the quads.
        
        // The following is a precaution until we can start using 32 bit data.
        // But we still write the indices out using 32 bit data.
        assert(mesh->mFaces[j].mIndices[0] < 65536);
        assert(mesh->mFaces[j].mIndices[1] < 65536);
        assert(mesh->mFaces[j].mIndices[2] < 65536);
        assert(mesh->mFaces[j].mIndices[3] < 65536);

        uint16_t index0 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[0]);
        uint16_t index1 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[1]);
        uint16_t index2 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[2]);
        uint16_t index3 = static_cast<uint16_t>(mesh->mFaces[j].mIndices[3]);

        // First triangle.
        output.write(reinterpret_cast<const char*>(&index0), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index1), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index2), sizeof(uint16_t));

        // Second triangle (opposite winding order).
        output.write(reinterpret_cast<const char*>(&index3), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index2), sizeof(uint16_t));
        output.write(reinterpret_cast<const char*>(&index1), sizeof(uint16_t));
      }
    }
  }

  output.close();

  return EXIT_SUCCESS;
}

