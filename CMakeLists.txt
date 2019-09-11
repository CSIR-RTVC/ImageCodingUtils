cmake_minimum_required(VERSION 3.11)
project(libvpp VERSION 1.0.0 LANGUAGES CXX)

#Make sure that custom modules like FindRapidJSON are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

##############################################
# Declare dependencies
#find_package(Boost 1.55 REQUIRED COMPONENTS regex)
#find_package(RapidJSON 1.0 REQUIRED MODULE)

##############################################
# Create target and set properties

SET(CODEC_UTIL_HDRS
    ./include/CodecUtils/ICodecv2.h
)
SET(IMAGE_UTIL_HDRS
    ./include/ImageUtils/FastFixedPointRGB24toYUV420Converter.h
)
SET(IMAGE_UTILS_SRCS
    ./src/ImageUtils/FastFixedPointRGB24toYUV420Converter.cpp
)

# interface library for now
add_library(vpp STATIC
    ${IMAGE_UTILS_SRCS} 
    ${IMAGE_UTIL_HDRS} 
    ${CODEC_UTIL_HDRS}
)

#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(Vpp::vpp ALIAS vpp)

#Set target properties
target_include_directories(vpp
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/CodecUtils/>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/GeneralUtils/>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ImageUtils/>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(vpp PRIVATE cxx_auto_type)
target_compile_options(vpp PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

target_link_libraries(vpp
    PUBLIC
    PRIVATE
)

##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Vpp)

install(TARGETS vpp
    EXPORT vpp-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

#This is required so that the exported target has the name Vpp and not vpp
set_target_properties(vpp PROPERTIES EXPORT_NAME Vpp)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#Export the targets to a script
install(EXPORT vpp-targets
  FILE
    VppTargets.cmake
  NAMESPACE
    Vpp::
  DESTINATION
    ${INSTALL_CONFIGDIR}
)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/VppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/VppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/VppConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/VppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/VppConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

##############################################
export(EXPORT vpp-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/VppTargets.cmake NAMESPACE Vpp::)

#Register package in user's package registry
export(PACKAGE Vpp)

#add_subdirectory(test)
