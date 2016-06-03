cmake_minimum_required(VERSION 2.6)

PROJECT( ImageCodingUtils )

message("Source Dir:" ${ImageCodingUtils_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ImageCodingUtils_SOURCE_DIR}/../bin)
SET(LIBRARY_OUTPUT_PATH ${ImageCodingUtils_SOURCE_DIR}/../lib)
SET(RUNTIME_OUTPUT_DIRECTORY ${ImageCodingUtils_SOURCE_DIR}/../bin)

SET(ImageCodingUtils_BIN ${ImageCodingUtils_SOURCE_DIR}/../bin)

message("ImageCodingUtils_SOURCE_DIR directories:" ${ImageCodingUtils_SOURCE_DIR})

IF(WIN32)
IF(MSVC12)
message("Detected MSVC12 compiler")
set(MSVC_VER VC12)
ELSEIF(MSVC11)
message("Detected MSVC11 compiler")
set(MSVC_VER VC11)
ELSEIF(MSVC10)
message("Detected MSVC10 compiler")
set(MSVC_VER VC10)
ELSE(MSVC12)
message("WARNING: Unknown/unsupported MSVC version")
ENDIF(MSVC12)
ENDIF(WIN32)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
# 64bit
  message("Detected 64-bit build - compiling with -fPIC")
  SET(CMAKE_CXX_FLAGS "-fPIC")
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
# 32 bit
message("Detected 32-bit build")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

SET(ImageCodingUtilsIncludes
${ImageCodingUtils_SOURCE_DIR}/../include
${ImageCodingUtils_SOURCE_DIR}/../include/GeneralUtils
${ImageCodingUtils_SOURCE_DIR}/../include/ImageUtils
${ImageCodingUtils_SOURCE_DIR}/../include/CodecUtils
)
message("ImageCodingUtilsIncludes directories:" ${ImageCodingUtilsIncludes})

# lib directories
SET(ImageCodingUtilsLink
${ImageCodingUtils_SOURCE_DIR}/lib
)

message("ImageCodingUtilsLink directories:" ${ImageCodingUtilsLink})

# Include directories
INCLUDE_DIRECTORIES(
${ImageCodingUtils_SOURCE_DIR}/../include
${ImageCodingUtils_SOURCE_DIR}/../include/GeneralUtils
${ImageCodingUtils_SOURCE_DIR}/../include/ImageUtils
${ImageCodingUtils_SOURCE_DIR}/../include/CodecUtils
)

ADD_SUBDIRECTORY(GeneralUtils) 
ADD_SUBDIRECTORY(CodecUtils) 
ADD_SUBDIRECTORY(ImageUtils) 

