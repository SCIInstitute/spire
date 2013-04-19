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
#include <boost/filesystem.hpp>

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

// Forward declarations
int processFile(const std::string& inFile, const std::string& outputDirectory);

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
  std::vector<std::string> inputFiles;
  std::string outputDirectory;
  try
  {
    TCLAP::CmdLine cmd("Asset Converter");

    // Could let both the inputs and the directory options be optional.
    // That would let us add both directories *and* files.
    TCLAP::MultiArg<std::string> inputs("i", "input", "Input file(s).",
                                        true, "Path");
    TCLAP::ValueArg<std::string> directory("d", "directory", "input directory",
                                           true, "", "Path");
    TCLAP::ValueArg<std::string> outputDir("o", "output", "Output directory.",
                                           false, "", "String");

    cmd.xorAdd(inputs, directory);
    cmd.add(outputDir);
    cmd.parse(argc, argv);

    // If inputs have been set, go ahead and add them to the list of inut files.
    if (inputs.isSet())
    {
      inputFiles = inputs.getValue();
    }
    
    if (directory.isSet())
    {
      //using namespace boost::filesystem;

      // Iterate over all files in directory and look for collada file
      // extensions.
      std::string dir = directory.getValue();
      if (!boost::filesystem::exists(dir))
      {
        std::cout << "Unable to find directory: " << dir << std::endl;
        return EXIT_FAILURE;
      }

      if (!boost::filesystem::is_directory(dir))
      {
        std::cout << dir << " is not a directory." << std::endl;
        return EXIT_FAILURE;
      }

      boost::filesystem::recursive_directory_iterator it = 
          boost::filesystem::recursive_directory_iterator(dir);
      boost::filesystem::recursive_directory_iterator end; // Default constructor is the 'end' iterator.

      while (it != end)
      {
        boost::filesystem::path path = *it;
        
        if (boost::filesystem::is_regular_file(path))
        {
          // Ensure the file has the correct extension (.dae)
          std::string extension = path.extension().string();
          std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
          if (extension == ".dae")
            inputFiles.push_back(path.generic_string());
        }

        // Prevent unwanted recursion into symlink directories.
        if (   boost::filesystem::is_directory(*it) 
            && boost::filesystem::is_symlink(*it))
          it.no_push();

        try
        {
          ++it;
        }
        catch (std::exception& e)
        {
          std::cout << e.what() << std::endl;
          it.no_push();
          try { ++it; } catch (...) {std::cout << "!!" << std::endl; return EXIT_FAILURE;}
        }
      }
    }

    outputDirectory = outputDir.getValue();
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP exception: " << e.error() << " associated with arg " << e.argId() << "\n";
    return EXIT_FAILURE;
  }

  std::cout << "Input files:" << std::endl;
  for (auto i : inputFiles)
  {
    std::cout << i << std::endl;
  }

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
  
  int lastExitCode = EXIT_SUCCESS;
  for (auto i : inputFiles)
  {
    lastExitCode = processFile(i, outputDirectory);
  }

  return lastExitCode;
}

int processFile(const std::string& inFile, const std::string& outputDirectory)
{
  std::string outFile;

  if (outputDirectory.length() > 0)
  {
    // Convert the filename to a path so we can extract path information.
    boost::filesystem::path boostPath(inFile);
    boost::filesystem::path outputFilePath(outputDirectory + "/" + boostPath.filename().string());

    outputFilePath.replace_extension(".sp");
    outFile = outputFilePath.string();
  }
  else
  {
    boost::filesystem::path boostPath(inFile);
    boostPath.replace_extension(".sp");
    outFile = boostPath.string();
  }

  std::cout << "Target output file: " << outFile << std::endl;

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


