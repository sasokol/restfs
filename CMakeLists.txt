cmake_minimum_required(VERSION 2.8)

project(restfs)

SET (CMAKE_VERBOSE_MAKEFILE ON)

IF (NOT DEFINED SYSCONFDIR)
    SET(SYSCONFDIR /etc)
ENDIF(NOT DEFINED SYSCONFDIR)

IF (NOT DEFINED CMAKE_INSTALL_PREFIX)
    SET (CMAKE_INSTALL_PREFIX /usr)
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

#CHECK_INCLUDE_FILES(src/define.h HAVE_STRINGS_H)


SET (CMAKE_CXX_FLAGS "-O0 -Wpedantic -Werror=narrowing -std=c++11")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

SET ( EXE restfsd)

SET (SOURCE_EXE src/${EXE}.cpp)


SET ( CMAKE_EXE_LINKER_FLAGS "")
add_executable(${EXE} ${SOURCE_EXE})

SET (SOURCE_LIBS src/config)
add_library(config STATIC ${SOURCE_LIBS})
target_link_libraries(${EXE} config)


SET (SOURCE_LIBS src/inih/ini.c)
add_library(ini STATIC ${SOURCE_LIBS})
target_link_libraries(${EXE} ini)


SET (SOURCE_LIBS src/error)
add_library(error STATIC ${SOURCE_LIBS})
target_link_libraries(${EXE} error)

SET (SOURCE_LIBS src/router)
add_library(router STATIC ${SOURCE_LIBS})
target_link_libraries(${EXE} router)


SET (SOURCE_LIBS src/object)
add_library(object STATIC ${SOURCE_LIBS})
target_link_libraries(${EXE} object)

target_link_libraries (${EXE} ${CMAKE_THREAD_LIBS_INIT})

#add_subdirectory(src)


#SET (SOURCE_LIBS src/inih/cpp/INIReader)
#add_library(INIReader STATIC ${SOURCE_LIBS})
#target_link_libraries(${EXE} INIReader)


#SET_TARGET_PROPERTIES(${EXE} PROPERTIES LINK_FLAGS ${CMAKE_EXE_LINKER_FLAGS})

find_package (Threads)

FIND_PACKAGE(Boost COMPONENTS thread-mt regex REQUIRED)
if (NOT Boost_FOUND)
    message(SEND_ERROR "Failed to find Boost")
    return()
else()
    INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIR})
    target_link_libraries (${EXE} ${Boost_LIBRARIES})
endif()


find_package(JSONCPP REQUIRED)
if(NOT JSONCPP_FOUND)
    message(SEND_ERROR "Failed to find jsoncpp")
    return()
else()
    #~ include_directories("${JSONCPP_INCLUDE_DIRS}/${JSONCPP_INCLUDE_PREFIX}")
    target_link_libraries (${EXE} "-ljsoncpp")
endif()


find_package(Elliptics REQUIRED)
if(NOT Elliptics_FOUND)
    message(SEND_ERROR "Failed to find Elliptics")
    return()
else()
    include_directories("${Elliptics_INCLUDE_DIR}")
    target_link_libraries (${EXE} "-lelliptics_client")
    target_link_libraries (${EXE} "-lelliptics_cpp")
endif()

find_package(FCGI REQUIRED)
if(NOT FCGI_FOUND)
    message(SEND_ERROR "Failed to find fcgi")
    return()
else()
    include_directories("${FCGI_INCLUDE_DIR}")
    target_link_libraries (${EXE} "-lfcgi++")
    target_link_libraries (${EXE} "-lfcgi")
endif()


find_package(LibPQXX REQUIRED)
if(NOT LIBPQXX_FOUND)
    message(SEND_ERROR "Failed to find LibPQXX")
    return()
else()
    include_directories("${LIBPQXX_INCLUDE_DIRS}")
    target_link_libraries (${EXE} -lpqxx)
endif()

find_package(PostgreSQL REQUIRED)
if(NOT PostgreSQL_FOUND)
    message(SEND_ERROR "Failed to find PostgreSQL")
    return()
else()
    include_directories("${PostgreSQL_INCLUDE_DIRS}")
    target_link_libraries (${EXE} ${PostgreSQL_LIBRARIES})
endif()

find_package(CRYPTOPP REQUIRED)
if(NOT CRYPTOPP_FOUND)
    message(SEND_ERROR "Failed to find cryptopp")
    return()
else()
    include_directories("${CRYPTOPP_INCLUDE_DIR}")
    target_link_libraries (${EXE} "-lcryptopp")
endif()

get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
if ("${LIB64}" STREQUAL "TRUE")
    set(LIBSUFFIX 64)
else()
    set(LIBSUFFIX "")
endif()
set(INSTALL_LIB_DIR     lib${LIBSUFFIX} CACHE PATH "Installation
directory for libraries")
mark_as_advanced(INSTALL_LIB_DIR)


INSTALL(
    TARGETS
        ${EXE}
#        pgsql
    RUNTIME DESTINATION bin COMPONENT ${EXE}
#    LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT pgsql

)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
