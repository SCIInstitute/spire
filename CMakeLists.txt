
set (Renderer_Root_Dir "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(OpenGL REQUIRED)

if (UNIX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  # Super strict settings.
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
endif (UNIX)

if (APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif (APPLE)

# Include OpenGL headers
include_directories(${OPENGL_INCLUDE_DIRS})

# Globbing has some downsides; but the advantages outweigh the
# disadvantages.
file (GLOB Source_Root
  "*.cpp"
  "*.h"
  )

file (GLOB Source_High
  "high/*.cpp"
  "high/*.h"
  )

file (GLOB Source_Low
  "low/*.cpp"
  "low/*.h"
  )

file (GLOB Source_Caching
  "caching/*.cpp"
  "caching/*.h"
  )

add_library(Spire
  ${Source_Root}
  ${Source_High}
  ${Source_Low}
  ${Source_Caching}
  )
target_link_libraries(Spire ${OPENGL_LIBRARIES})

