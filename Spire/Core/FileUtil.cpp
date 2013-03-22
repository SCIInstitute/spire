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
/// \brief  These functions exist primarily to locate shader files.
///         Inherits heavily from the SysTools library in IV3D's Tuvok.
///         There is currently no wstring implementation.

#include <vector>
#include <string>
#include <algorithm>

#include "Common.h"
#include "Core/Log.h"
#include "FileUtil.h"

#ifndef SPIRE_USING_WIN
  #include <wchar.h>
  #include <sys/stat.h>
  #include <regex.h>
  #include <dirent.h>
  #include <unistd.h>
  #include <pwd.h>
  #define GetCurrentDir getcwd
  #define LARGE_STAT_BUFFER struct stat
  #define LARGE_STAT(name,buffer) stat(name,buffer)

  typedef wchar_t WCHAR;
  typedef unsigned char CHAR;
#else
  #include <sys/stat.h>
  #include <shlwapi.h>
  #include <windows.h>
  #include <direct.h>
  #define GetCurrentDir _getcwd
  #define LARGE_STAT_BUFFER struct __stat64
  #define LARGE_STAT(name,buffer) _stat64(name,buffer)
#endif

#ifdef SPIRE_USING_OSX
  #include <CoreFoundation/CoreFoundation.h>
#endif

namespace Spire {

// Forward declarations.
bool getFileStats(const std::string& strFileName, LARGE_STAT_BUFFER& stat_buf);
std::vector<std::string> getSubDirList(const std::string& dir);
std::string getFromResourceOnMac(const std::string& strFileName);
std::string findFile(const std::string& file, bool subdirs);

//------------------------------------------------------------------------------
std::string removeExt(const std::string& fileName)
{
  size_t indexDot = fileName.find_last_of(".");
  size_t indexSlash = std::max(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")));
  if (indexDot == std::string::npos ||
      (indexSlash != std::string::npos && indexDot < indexSlash))
    return fileName;
  return fileName.substr(0,indexDot);
}

//------------------------------------------------------------------------------
std::string changeExt(const std::string& fileName, const std::string& newext)
{
  return removeExt(fileName)+ "." + newext;
}

//------------------------------------------------------------------------------
std::string getExt(const std::string& fileName)
{
  size_t indexDot = fileName.find_last_of(".");
  size_t indexSlash = std::max(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")));
  if (indexDot == std::string::npos || (indexSlash != std::string::npos && indexDot < indexSlash)) return "";
  std::string ext = fileName.substr(indexDot+1);
  return ext;
}

//------------------------------------------------------------------------------
std::string findFileInDirs(const std::string& file,
                           const std::vector<std::string>& strDirs,
                           bool subdirs)
{
#ifdef SPIRE_USING_OSX
  if (fileExists(getFromResourceOnMac(file)))
  {
    std::string res = getFromResourceOnMac(file);
    return res;
  }
#endif

  if (fileExists(file)) return file;
  for (auto it = strDirs.begin(); it != strDirs.end(); ++it)
  {
    // Skip non-existant directories.
    if (!fileExists(*it)) continue;

    std::string searchFile = *it + "/" + file;
    if (findFile(searchFile, subdirs) != "")
      return searchFile;
  }
  return "";
}

//------------------------------------------------------------------------------
std::string getFilename(const std::string& fileName)
{
  size_t index = std::max(fileName.find_last_of("\\"), fileName.find_last_of("/"));
  std::string name;
  if (index != std::string::npos)
    name = fileName.substr(index,fileName.length()-index);
  else
    name = fileName;

  return name;
}

//------------------------------------------------------------------------------
std::string findFile(const std::string& file, bool subdirs)
{
  if (!fileExists(file))
  {
    // if it doesn't exist but we allow subdir search, try harder
    if (subdirs)
    {
      std::string programPath = ".";
      std::vector<std::string> dirs = getSubDirList(programPath);
      dirs.push_back(programPath);

      std::string raw_fn = getFilename(file);
      for(std::vector<std::string>::const_iterator d = dirs.begin();
          d != dirs.end(); ++d)
      {
        std::string testfn = *d + "/" + raw_fn;
        if(fileExists(testfn)) {
          return testfn;
        }
      }
    }

    Log::warning() << "Could not find '" << file << "'";
    return "";
  }

  return file;
}

//------------------------------------------------------------------------------
std::vector<std::string> getSubDirList(const std::string& dir)
{
  std::vector<std::string> subDirs;
  std::string rootdir;

#ifdef SPIRE_USING_WIN
  std::stringstream s;
  if (dir == "")
  {
    CHAR path[4096];
    GetCurrentDirectoryA(4096, path);
    s << path << "/";
  }
  else
  {
    s << dir << "/";
  }

  rootdir = s.str();

  WIN32_FIND_DATAA FindFileData;
  HANDLE hFind;

  hFind = FindFirstFileA((rootdir + "*.*").c_str(), &FindFileData);

  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      std::string strFilename = FindFileData.cFileName;
      if(   (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
         && strFilename != "."
         && strFilename != "..")
      {
        subDirs.push_back(strFilename);
      }
    } while ( FindNextFileA(hFind, &FindFileData) );
  }

  FindClose(hFind);
#else
  if (dir == "")
  {
    rootdir = "./";
  }
  else
  {
    rootdir = dir + "/";
  }

  DIR* dirData = opendir(rootdir.c_str());

  if (dirData != NULL)
  {
    struct dirent *inode;

    while ((inode=readdir(dirData)) != NULL)
    {
      std::string strFilenameLocal = inode->d_name;
      std::string strFilename = rootdir + strFilenameLocal;

      struct ::stat st;
      if (::stat(strFilename.c_str(), &st) != -1)
      {
        if (    S_ISDIR(st.st_mode) 
            &&  strFilenameLocal != "." && strFilenameLocal != "..")
        {
          subDirs.push_back(strFilenameLocal);
        }
      }
    }
    closedir(dirData);
  }
#endif

  std::vector<std::string> completeSubDirs(subDirs.size());
  for (size_t i = 0; i < subDirs.size(); i++)
  {
    completeSubDirs.push_back(rootdir + subDirs[i]);
  }
  return completeSubDirs;
}

//------------------------------------------------------------------------------
std::string getFromResourceOnMac(const std::string& strFileName)
{
#ifdef SPIRE_USING_OSX
  CFStringRef cfFilename = CFStringCreateWithCString(
      kCFAllocatorDefault, removeExt(getFilename(strFileName)).c_str(), 
      CFStringGetSystemEncoding());
  CFStringRef cfExt = CFStringCreateWithCString(
      kCFAllocatorDefault, getExt(getFilename(strFileName)).c_str(),
      CFStringGetSystemEncoding());

  CFURLRef imageURL = CFBundleCopyResourceURL(CFBundleGetMainBundle(), 
                                              cfFilename, cfExt, NULL);
  if (imageURL == NULL) return "";
  CFStringRef macPath = CFURLCopyFileSystemPath(imageURL, kCFURLPOSIXPathStyle);
  const char *pathPtr = CFStringGetCStringPtr(macPath, 
                                              CFStringGetSystemEncoding());
  if (macPath != 0 && pathPtr != 0)
  {
    std::string result = pathPtr;
    return result;
  }
  else
  {
    return strFileName;
  }
#else
  return strFileName;
#endif
}

//------------------------------------------------------------------------------
bool getFileStats(const std::string& strFileName, LARGE_STAT_BUFFER& stat_buf)
{
  return (LARGE_STAT(strFileName.c_str(), &stat_buf) >= 0);
}

//------------------------------------------------------------------------------
bool fileExists(const std::string& strFileName)
{
  LARGE_STAT_BUFFER stat_buf;
  return getFileStats(strFileName, stat_buf);
}

//------------------------------------------------------------------------------
std::string getCurrentWorkingDir()
{
  char cCurrentPath[FILENAME_MAX];
  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
  {
    // Should probably throw...
    return "";
  }

  return std::string(cCurrentPath);
} 

//------------------------------------------------------------------------------
std::string getPath(const std::string& fileName)
{
  std::string path = fileName.substr(
      0,std::max(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")))+1);
  if(path.empty()) { path = "./"; }
  return path;
}

//------------------------------------------------------------------------------
bool getTempDirectory(std::string& path)
{
#ifdef DETECTED_OS_WINDOWS
  DWORD result = ::GetTempPathA(0, "");
  if(result == 0) return false;
  std::vector<char> tempPath(result + 1);
  result = ::GetTempPathA(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
  if((result == 0) || (result >= tempPath.size())) return false;
  path = std::string( tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result)  );
  return true;
#else
  char* pointer;
  pointer = tmpnam(NULL);
  path = getPath(std::string(pointer));
  return true;
#endif
}

} // end of names
