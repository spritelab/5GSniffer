cmake_minimum_required(VERSION 3.12)

include(ExternalProject)

# set the project name
project(5gsniffer)


#######################################################################
# Options
#######################################################################

option(ENABLE_GUI      "Enable GUI"				                 OFF)
option(ENABLE_UHD      "Enable UHD"                         ON)

###########################################################################


#######################################################################
# CMake flags
#######################################################################

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Should be set at some point, but gives too many errors for srsRAN
# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wconversion")

# Warn user if we are in debug mode and add a debug flag
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  message(WARNING "Building in debug mode")
  add_compile_definitions(DEBUG_BUILD=1)
endif()

# Allow debug level messages even in release mode depending on SPDLOG_LEVEL
# At some stage this may be disallowed by moving it into the above if-statement
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=1)

########################################################################
# Dependencies
########################################################################

#FFT

#GUI

# UHD
if(ENABLE_UHD)
  find_package(UHD)
  if(UHD_FOUND)
    include_directories(${UHD_INCLUDE_DIRS})
    link_directories(${UHD_LIBRARY_DIRS})
  endif(UHD_FOUND)
endif(ENABLE_UHD)

# srsRAN
ExternalProject_Add(srsRAN
  # Don't clone the repo because this is done using git submodules
  #GIT_REPOSITORY    git@github.com:NorbLd/srsRANRF
  #GIT_TAG           origin/vanilla
  SOURCE_DIR ../lib/srsRANRF/
  PREFIX lib/srsRANRF
  INSTALL_COMMAND ""
  BUILD_ALWAYS false
)

ExternalProject_Get_Property(srsRAN SOURCE_DIR)
ExternalProject_Get_Property(srsRAN INSTALL_DIR)
include_directories(${SOURCE_DIR}/lib/include)
link_directories("${INSTALL_DIR}/src/srsRAN-build/lib/src/phy")
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/phy/rf)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/rlc)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/mac)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/radio)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/srslog)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/gtpu)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/common)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/asn1)
link_directories(${INSTALL_DIR}/src/srsRAN-build/lib/src/pdcp)

# Logging library
add_compile_definitions(SPDLOG_FMT_EXTERNAL)
find_package(spdlog REQUIRED)

# Add protobuf
find_package(Protobuf REQUIRED)

########################################################################
# Add general includes and dependencies
########################################################################
include_directories(include)
include_directories(src)

########################################################################
# Add the subdirectories
########################################################################
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)  # TODO make external project
