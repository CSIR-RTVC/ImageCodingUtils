# CMakeLists.txt in GeneralUtils dir

INCLUDE_DIRECTORIES(
${ImageCodingUtilsIncludes}
)

# Adds a library called Shared (libRtvcGeneralUtils.a under Linux)
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

SET(GENERAL_UTIL_HDRS
../../include/GeneralUtils/MeasurementTable.h
)

SET(GENERAL_UTILS_SRCS
MeasurementTable.cpp
)

ADD_LIBRARY( GeneralUtils STATIC ${GENERAL_UTILS_SRCS} ${GENERAL_UTIL_HDRS})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(GeneralUtils PROPERTIES DEBUG_POSTFIX "D" )
message("GeneralUtils Debug Build")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(GeneralUtils PROPERTIES DEBUG_POSTFIX "")
message("GeneralUtils Release Build")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

