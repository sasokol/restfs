# CMake module to search for CRYPTOPP headers
#
# If it's found it sets CRYPTOPP_FOUND to TRUE
# and following variables are set:
#    CRYPTOPP_INCLUDE_DIR
#    CRYPTOPP_LIBRARY
FIND_PATH(CRYPTOPP_INCLUDE_DIR
  cryptopp/hex.h
  cryptopp/sha.h
  cryptopp/filters.h
  PATHS
  /usr/include
  /usr/local/include
  #MSVC
  "$ENV{LIB_DIR}/include"
  $ENV{INCLUDE}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(CRYPTOPP_LIBRARY NAMES cryptopp libpryptopp PATHS 
  /usr/local/lib 
  /usr/lib 
  #MSVC
  "$ENV{LIB_DIR}/lib"
  $ENV{LIB}
  #mingw
  c:/msys/local/lib
  )

IF (CRYPTOPP_INCLUDE_DIR AND CRYPTOPP_LIBRARY)
   SET(CRYPTOPP_FOUND TRUE)
ENDIF (CRYPTOPP_INCLUDE_DIR AND CRYPTOPP_LIBRARY)

IF (CRYPTOPP_FOUND)
   IF (NOT CRYPTOPP_FIND_QUIETLY)
      MESSAGE(STATUS "Found cryptopp: ${CRYPTOPP_LIBRARY}")
   ENDIF (NOT CRYPTOPP_FIND_QUIETLY)
ELSE (CRYPTOPP_FOUND)
   IF (CRYPTOPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find cryptopp")
   ENDIF (CRYPTOPP_FIND_REQUIRED)
ENDIF (CRYPTOPP_FOUND)
