# CMakeLists.txt in CodecUtils dir

INCLUDE_DIRECTORIES(
${ImageCodingUtilsIncludes}
)

# Adds a library called Shared (libRtvcCodecUtils.a under Linux)
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

SET(CODEC_UTIL_HDRS
../../include/CodecUtils/BitStreamBase.h
../../include/CodecUtils/BitStreamBaseMSB.h
../../include/CodecUtils/BitStreamReader.h
../../include/CodecUtils/BitStreamReaderMSB.h
../../include/CodecUtils/BitStreamWriter.h
../../include/CodecUtils/BitStreamWriterMSB.h
../../include/CodecUtils/BlockH264.h
../../include/CodecUtils/CAVLCH264Impl.h
../../include/CodecUtils/CAVLCH264Impl2.h
../../include/CodecUtils/CodedBlkPatternH264VlcDecoder.h
../../include/CodecUtils/CodedBlkPatternH264VlcEncoder.h
../../include/CodecUtils/CoeffTokenH264VlcDecoder.h
../../include/CodecUtils/CoeffTokenH264VlcEncoder.h
../../include/CodecUtils/ExpGolombSignedVlcDecoder.h
../../include/CodecUtils/ExpGolombSignedVlcEncoder.h
../../include/CodecUtils/ExpGolombTruncVlcDecoder.h
../../include/CodecUtils/ExpGolombTruncVlcEncoder.h
../../include/CodecUtils/ExpGolombUnsignedVlcDecoder.h
../../include/CodecUtils/ExpGolombUnsignedVlcEncoder.h
../../include/CodecUtils/FastForward4x4ITImpl1.h
../../include/CodecUtils/FastForward4x4ITImpl2.h
../../include/CodecUtils/FastForward4x4On16x16ITImpl1.h
../../include/CodecUtils/FastForwardDC2x2ITImpl1.h
../../include/CodecUtils/FastForwardDC4x4ITImpl1.h
../../include/CodecUtils/FastInverse4x4ITImpl1.h
../../include/CodecUtils/FastInverse4x4On16x16ITImpl1.h
../../include/CodecUtils/FastInverseDC2x2ITImpl1.h
../../include/CodecUtils/FastInverseDC4x4ITImpl1.h
../../include/CodecUtils/H264MotionVectorPredictorImpl1.h
../../include/CodecUtils/IBitStreamReader.h
../../include/CodecUtils/IBitStreamWriter.h
../../include/CodecUtils/ICodecInnerAccess.h
../../include/CodecUtils/ICodecv2.h
../../include/CodecUtils/IContextAwareRunLevelCodec.h
../../include/CodecUtils/IForwardTransform.h
../../include/CodecUtils/IInverseTransform.h
../../include/CodecUtils/IMotionCompensator.h
../../include/CodecUtils/IMotionEstimator.h
../../include/CodecUtils/IMotionVectorPredictor.h
../../include/CodecUtils/IRunLengthCodec.h
../../include/CodecUtils/IStreamHeaderReader.h
../../include/CodecUtils/IVlcDecoder.h
../../include/CodecUtils/IVlcEncoder.h
../../include/CodecUtils/MacroBlockH264.h
../../include/CodecUtils/MotionCompensatorH264ImplStd.h
../../include/CodecUtils/MotionEstimatorH264ImplMultires.h
../../include/CodecUtils/MotionEstimatorH264ImplMultiresCross.h
../../include/CodecUtils/MotionEstimatorH264ImplMultiresCrossVer2.h
../../include/CodecUtils/NalHeaderH264.h
../../include/CodecUtils/PicParamSetH264.h
../../include/CodecUtils/PrefixH264VlcDecoderImpl1.h
../../include/CodecUtils/PrefixH264VlcEncoderImpl1.h
../../include/CodecUtils/RunBeforeH264VlcDecoder.h
../../include/CodecUtils/RunBeforeH264VlcEncoder.h
../../include/CodecUtils/SeqParamSetH264.h
../../include/CodecUtils/SliceHeaderH264.h
../../include/CodecUtils/TotalZeros2x2H264VlcDecoder.h
../../include/CodecUtils/TotalZeros2x2H264VlcEncoder.h
../../include/CodecUtils/TotalZeros2x4H264VlcDecoder.h
../../include/CodecUtils/TotalZeros2x4H264VlcEncoder.h
../../include/CodecUtils/TotalZeros4x4H264VlcDecoder.h
../../include/CodecUtils/TotalZeros4x4H264VlcEncoder.h
../../include/CodecUtils/VectorStructList.h
)

SET(CODEC_UTILS_SRCS
BitStreamReader.cpp
BitStreamReaderMSB.cpp
BitStreamWriter.cpp
BitStreamWriterMSB.cpp
BlockH264.cpp
CAVLCH264Impl.cpp
CAVLCH264Impl2.cpp
CodedBlkPatternH264VlcDecoder.cpp
CodedBlkPatternH264VlcEncoder.cpp
#CodeFragmentForH264BlockLayerTest.cpp
#CodeFragmentForH264MacroBlockLayerTest.cpp
CoeffTokenH264VlcDecoder.cpp
CoeffTokenH264VlcEncoder.cpp
ExpGolombUnsignedVlcDecoder.cpp
ExpGolombUnsignedVlcEncoder.cpp
FastForward4x4ITImpl1.cpp
FastForward4x4ITImpl2.cpp
FastForward4x4On16x16ITImpl1.cpp
FastForwardDC2x2ITImpl1.cpp
FastForwardDC4x4ITImpl1.cpp
FastInverse4x4ITImpl1.cpp
FastInverse4x4On16x16ITImpl1.cpp
FastInverseDC2x2ITImpl1.cpp
FastInverseDC4x4ITImpl1.cpp
MacroBlockH264.cpp
MotionCompensatorH264ImplStd.cpp
MotionEstimatorH264ImplMultires.cpp
MotionEstimatorH264ImplMultiresCross.cpp
MotionEstimatorH264ImplMultiresCrossVer2.cpp
NalHeaderH264.cpp
PicParamSetH264.cpp
RunBeforeH264VlcDecoder.cpp
RunBeforeH264VlcEncoder.cpp
SeqParamSetH264.cpp
SliceHeaderH264.cpp
TotalZeros2x2H264VlcDecoder.cpp
TotalZeros2x2H264VlcEncoder.cpp
TotalZeros2x4H264VlcDecoder.cpp
TotalZeros2x4H264VlcEncoder.cpp
TotalZeros4x4H264VlcDecoder.cpp
TotalZeros4x4H264VlcEncoder.cpp
VectorStructList.cpp
)

ADD_LIBRARY( CodecUtils STATIC ${CODEC_UTILS_SRCS} ${CODEC_UTIL_HDRS})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(CodecUtils PROPERTIES DEBUG_POSTFIX "D" )
message("CodecUtils Debug Build")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
SET_TARGET_PROPERTIES(CodecUtils PROPERTIES DEBUG_POSTFIX "")
message("CodecUtils Release Build")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

