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

#ifndef SPIRE_INTERFACE_H
#define SPIRE_INTERFACE_H

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "Context.h"
#include "Core/Math.h"  // Necessary in order to communicate vector types.

namespace Spire {

class Hub;
class HubThread;

/// Interface to the renderer.
/// A new interface will need to be created per-context.
/// Spire expects that only one thread will be communicating with it at any
/// given time.
class Interface
{
public:
  /// All possible log outputs from the renderer. Used via the logging function.
  enum LOG_LEVEL
  {
    LOG_DEBUG,    ///< Debug / verbose.
    LOG_MESSAGE,  ///< General message.
    LOG_WARNING,  ///< Warning.
    LOG_ERROR,    ///< Error.
  };

  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// Constructs an interface to the renderer.
  /// \param  shaderDirs    A list of directories to search for shader files.
  /// \param  createThread  If true, then a thread will be created in which the
  ///                       renderer will run. The renderer will do it's best
  ///                       to manage time allocation and only use cycles
  ///                       when there is something to do.
  /// \param  logFP         The logging function to use when logging rendering
  ///                       messages. Remember, this function will be called
  ///                       from the rendering thread; so it will need to 
  ///                       communicate the messages in a thread-safe manner.
  /// @todo Re-enabled shared_ptr context when SCIRun5 adopts C++11.
  //Interface(std::shared_ptr<Context> context, bool createThread, 
  //          LogFunction logFP = LogFunction());
  Interface(std::shared_ptr<Context> context, 
            const std::vector<std::string>& shaderDirs,
            bool createThread, LogFunction logFP = LogFunction());
  virtual ~Interface();


  //============================================================================
  // THREAD SAFE IF ONLY CALLED FROM ONE THREAD
  //============================================================================

  /// Terminates spire. If running 'threaded' then this will join with the 
  /// spire thread before returning. This should be called before the OpenGL
  /// context is destroyed.
  /// There is no mutex lock in this function, it should only be called by one
  /// thread, but it doesn't matter what thread.
  void terminate();

  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================
  // 

  // All functions contained within this section must be called on their
  // appropriate thread. The THREAD parameter indicates which thread the caller
  // is on. These functions send a message to spire and involve no locks.
  // Return is immediate.

  //--------
  // Camera
  //--------
  /// Sets the camera's world-space transformation.
  void cameraSetTransform(const M44& transform);


  //------------------
  // HACKED Interface
  //------------------
  // Everything in this interface will be rendered using the uniform color
  // shader.

  /// HACKED  Uniform color face attributes.
  ///         Once your the buffer is passed into this function, spire assumes
  ///         all ownership of it. It will call delete[] on the buffer.
  ///         Expects that the indexBuffer is an uint16_t buffer.
  void renderHACKSetUCFace(uint8_t* vertexBuffer, size_t vboSize,
                           uint8_t* indexBuffer, size_t iboSize);

  /// HACKED  Uniform face color
  void renderHACKSetUCFaceColor(const V4& color);

  /// HACKED  Uniform color edge attributes.
  ///         Expects that the indexBuffer is an uint16_t buffer.
  void renderHACKSetUCEdge(uint8_t* vertexBuffer, size_t vboSize,
                           uint8_t* indexBuffer, size_t iboSize);

  /// HACKED  Uniform face color
  void renderHACKSetUCEdgeColor(const V4& color);

  /// \todo Construct a *real* interface to spire. Below is a shot of what it
  ///       should look like. I've still got to think about how the uniforms
  ///       will be handled.
  //---------
  // Objects
  //---------
  ///// Removes an object given an identifier.
  //void removeObject(const std::string& object);

  ///// Adds a geometry pass to an object given by the identifier 'object'.
  ///// \param  object    Unique object name.
  ///// \param  passName  Unique name of the pass.
  ///// \param  priority  Priority to use when rendering the pass.
  ///// \param  buffer    Buffer containing attribute data.
  ///// \param  shader    Complete shader program to use when rendering this pass.
  /////                   (see addPersistentShader).
  ///// \todo Figure out what the 'ideal' interface is. This probably is not
  /////       the ideal approach.
  //void addGeomPassToObject(const std::string& object,
  //                         const std::string& passName,
  //                         int priority,
  //                         std::array<uint8_t> buffer, 
  //                         const std::string& shader);
  //void addGeomPassUniform();

  //-----------------
  // Shader Programs
  //-----------------
  /// \todo Create a generic interface so that geometry shaders can be added
  ///       in the future.

  /// Adds a persistent shader under the name 'programName'.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& fragmentShader);

  /// \todo Add 'object'
  /// \todo Remove 'object'
  /// \todo Modify 'object'? -- Maybe

  //============================================================================
  // LOCKING THREAD SAFE
  //============================================================================
  // It doesn't matter where the functions in this section are called as they
  // are all mutex locked. These functions exist solely to retrieve information
  // from Spire in a 'blocking' manner.

  /// \todo Obtain current camera transform.

  /// \todo It is possible to determine where the user 'clicked' in the scene,
  ///       but the question then becomes: where do we perform the spatial
  ///       lookup? In spire or in SCIRun? This will likely be a SCIRun task,
  ///       or at the very least in a 'SCIRun' layer in Spire.
  ///       The SCIRun layer does NOT need to run on the spire thread and can
  ///       interact directly with SCIRun without threading worries.
  ///       The idea is keep spire 'really dumb' with regards to application
  ///       specific needs, but smart enough to provide the application with
  ///       everything it needs.

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================

  /// The following functions are *not* thread safe.
  /// Use these functions only when the renderer is not threaded.
  /// @{
  
  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  /// You must call this function every time you want a new frame to be rendered
  /// unless you are using the threaded renderer. The threaded renderer will 
  /// call doFrame automatically.
  /// \note You must call doFrame on the same thread where makeCurrent issued.
  ///       If this is not the same thread where Interface was created, ensure
  ///       a call to context->makeCurrent() is issued before invoking doFrame
  ///       for the first time.
  void doFrame();

  /// @}


private:

  std::unique_ptr<Hub>                      mHub;           ///< Rendering hub.

};

} // namespace spire

#endif // SPIRE_INTERFACE_H
