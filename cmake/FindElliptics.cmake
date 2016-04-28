# CMake module to search for Elliptics headers
#
# If it's found it sets Elliptics_FOUND to TRUE
# and following variables are set:
#    Elliptics_INCLUDE_DIR
#    Elliptics_LIBRARY
FIND_PATH(Elliptics_INCLUDE_DIR
  elliptics/session.hpp
  elliptics/logger.hpp
  PATHS
  /usr/include
  /usr/local/include
  #MSVC
  "$ENV{LIB_DIR}/include"
  $ENV{INCLUDE}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(Elliptics_LIBRARY NAMES elliptics_client elliptics_cpp PATHS
  /usr/local/lib 
  /usr/lib 
  #MSVC
  "$ENV{LIB_DIR}/lib"
  $ENV{LIB}
  #mingw
  c:/msys/local/lib
  )

IF (Elliptics_INCLUDE_DIR AND Elliptics_LIBRARY)
   SET(Elliptics_FOUND TRUE)
ENDIF (Elliptics_INCLUDE_DIR AND Elliptics_LIBRARY)

IF (Elliptics_FOUND)
   IF (NOT Elliptics_FIND_QUIETLY)
      MESSAGE(STATUS "Found elliptics: ${Elliptics_LIBRARY}")
   ENDIF (NOT Elliptics_FIND_QUIETLY)
ELSE (Elliptics_FOUND)
   IF (Elliptics_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find elliptics")
   ENDIF (Elliptics_FIND_REQUIRED)
ENDIF (Elliptics_FOUND)
