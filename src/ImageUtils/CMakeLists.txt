# CMakeLists.txt in ImageUtils dir

INCLUDE_DIRECTORIES(
${ImageCodingUtilsIncludes}
)

# Adds a library called Shared (libImageUtils.a under Linux)
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

SET(IMAGE_UTIL_HDRS
../../include/ImageUtils/FastFixedPointRGB24toYUV420Converter.h
#../include/FastGpuRGB24toYUV420Converter.h
../../include/ImageUtils/FastLookupTableRGB24toYUV420Converter.h
../../include/ImageUtils/FastRGB24toYUV420Converter.h
#../include/FastSimdRGB24toYUV420Converter.h
../../include/ImageUtils/ImageHandlerV2.h
../../include/ImageUtils/MtRGB24toYUV420Converter.h
../../include/ImageUtils/OverlayExtMem2Dv2.h
../../include/ImageUtils/PicConcatBase.h
../../include/ImageUtils/PicConcatRGB24Impl.h
../../include/ImageUtils/PicConcatRGB32Impl.h
../../include/ImageUtils/PicCropperBase.h
../../include/ImageUtils/PicCropperRGB24Impl.h
../../include/ImageUtils/PicCropperRGB32Impl.h
../../include/ImageUtils/PicInPicBase.h
../../include/ImageUtils/PicInPicRGB24Impl.h
../../include/ImageUtils/PicInPicRGB32Impl.h
../../include/ImageUtils/PicRotateBase.h
../../include/ImageUtils/PicRotateRGB24Impl.h
../../include/ImageUtils/PicRotateRGB32Impl.h
../../include/ImageUtils/PicRotateRGBBase.h
../../include/ImageUtils/PicScalerBase.h
../../include/ImageUtils/PicScalerRGB24Impl.h
../../include/ImageUtils/PicScalerYUV420PImpl.h
../../include/ImageUtils/RealRGB24toYUV420CCIR601Converter.h
../../include/ImageUtils/RealRGB24toYUV420CCIR601ConverterVer16.h
../../include/ImageUtils/RealRGB24toYUV420Converter.h
../../include/ImageUtils/RealRGB24toYUV420ConverterImpl2.h
../../include/ImageUtils/RealRGB24toYUV420ConverterImpl2Ver16.h
../../include/ImageUtils/RealRGB32toYUV420Converter.h
../../include/ImageUtils/RealYUV420toRGB24CCIR601Converter.h
../../include/ImageUtils/RealYUV420toRGB24CCIR601ConverterVer16.h
../../include/ImageUtils/RealYUV420toRGB24Converter.h
../../include/ImageUtils/RealYUV420toRGB24ConverterImpl2.h
../../include/ImageUtils/RealYUV420toRGB24ConverterImpl2Ver16.h
../../include/ImageUtils/RealYUV444toRGB24Converter.h
../../include/ImageUtils/RGBtoRGBConverter.h
../../include/ImageUtils/RGBtoYUV420Converter.h
../../include/ImageUtils/YUV420toRGBConverter.h
../../include/ImageUtils/YUV444toRGBConverter.h
)

IF(WIN32)
SET(IMAGE_UTILS_HDRS
${IMAGE_UTILS_HDRS}
../../include/ImageUtils/OverlayMem2Dv2.h
)
ENDIF(WIN32)

SET(IMAGE_UTILS_SRCS
FastFixedPointRGB24toYUV420Converter.cpp
#FastGpuRGB24toYUV420Converter.cpp
FastLookupTableRGB24toYUV420Converter.cpp
FastRGB24toYUV420Converter.cpp
#FastSimdRGB24toYUV420Converter.cpp
MtRGB24toYUV420Converter.cpp
OverlayExtMem2Dv2.cpp
OverlayMem2Dv2.cpp
PicConcatRGB24Impl.cpp
PicConcatRGB32Impl.cpp
PicCropperRGB24Impl.cpp
PicCropperRGB32Impl.cpp
PicInPicRGB24Impl.cpp
PicInPicRGB32Impl.cpp
PicRotateRGB24Impl.cpp
PicRotateRGB32Impl.cpp
PicRotateRGBBase.cpp
PicScalerRGB24Impl.cpp
PicScalerYUV420PImpl.cpp
RealRGB24toYUV420CCIR601Converter.cpp
RealRGB24toYUV420CCIR601ConverterVer16.cpp
RealRGB24toYUV420Converter.cpp
RealRGB24toYUV420ConverterImpl2.cpp
RealRGB24toYUV420ConverterImpl2Ver16.cpp
RealRGB32toYUV420Converter.cpp
RealYUV420toRGB24CCIR601Converter.cpp
RealYUV420toRGB24CCIR601ConverterVer16.cpp
RealYUV420toRGB24Converter.cpp
RealYUV420toRGB24ConverterImpl2.cpp
RealYUV420toRGB24ConverterImpl2Ver16.cpp
RealYUV444toRGB24Converter.cpp
)

IF(WIN32)
SET(IMAGE_UTILS_SRC
${IMAGE_UTILS_SRC}
ImageHandlerV2.cpp
)
ENDIF(WIN32)

ADD_LIBRARY( ImageUtils STATIC ${IMAGE_UTILS_SRCS} ${IMAGE_UTIL_HDRS})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(ImageUtils PROPERTIES DEBUG_POSTFIX "D" )
message("ImageUtils Debug Build")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(ImageUtils PROPERTIES DEBUG_POSTFIX "")
message("ImageUtils Release Build")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

