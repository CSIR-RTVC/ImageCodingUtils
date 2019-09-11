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

IF (BUILD_VCODEC)
    SET(VCODEC_SRCS
        ./src/CodecUtils/VectorQuantiserBaseImpl1.cpp
        ./src/CodecUtils/VectorQuantiserCbImpl1.cpp
        ./src/CodecUtils/VectorQuantiserCrImpl1.cpp
        ./src/CodecUtils/VectorQuantiserLumImpl1.cpp
        )
ELSE (BUILD_VCODEC)
    SET(VCODEC_SRCS
        )
ENDIF (BUILD_VCODEC)

SET(CODEC_UTIL_HDRS
    ./include/CodecUtils/AdvancedIntraDctQuantiserImpl.h
    ./include/CodecUtils/AdvancedIntraDctQuantiserImplLookUp.h
    ./include/CodecUtils/AdvancedIntraModeH263VlcDecoder.h
    ./include/CodecUtils/AdvancedIntraModeH263VlcEncoder.h
    ./include/CodecUtils/BitStreamBase.h
    ./include/CodecUtils/BitStreamBaseMSB.h
    ./include/CodecUtils/BitStreamReader.h
    ./include/CodecUtils/BitStreamReaderMSB.h
    ./include/CodecUtils/BitStreamWriter.h
    ./include/CodecUtils/BitStreamWriterMSB.h
    ./include/CodecUtils/BlockH263.h
    ./include/CodecUtils/BlockH264.h
    ./include/CodecUtils/CAVLCH264Impl2.h
    ./include/CodecUtils/CAVLCH264Impl.h
    ./include/CodecUtils/CodecDistortionDef.h
    ./include/CodecUtils/CodecUtilsPch.h
    ./include/CodecUtils/CodedBlkPatternH264VlcDecoder.h
    ./include/CodecUtils/CodedBlkPatternH264VlcEncoder.h
    ./include/CodecUtils/CoeffTokenH264VlcDecoder.h
    ./include/CodecUtils/CoeffTokenH264VlcEncoder.h
    ./include/CodecUtils/ColourPlaneDecoding.h
    ./include/CodecUtils/ColourPlaneEncoding.h
    ./include/CodecUtils/DQuantH263VlcDecoder.h
    ./include/CodecUtils/DQuantH263VlcEncoder.h
    ./include/CodecUtils/DualMotionVectorTypeStruct.h
    ./include/CodecUtils/ExpGolombSignedVlcDecoder.h
    ./include/CodecUtils/ExpGolombSignedVlcEncoder.h
    ./include/CodecUtils/ExpGolombTruncVlcDecoder.h
    ./include/CodecUtils/ExpGolombTruncVlcEncoder.h
    ./include/CodecUtils/ExpGolombUnsignedVlcDecoder.h
    ./include/CodecUtils/ExpGolombUnsignedVlcEncoder.h
    ./include/CodecUtils/FastForward4x4ITImpl1.h
    ./include/CodecUtils/FastForward4x4ITImpl2.h
    ./include/CodecUtils/FastForward4x4On16x16ITImpl1.h
    ./include/CodecUtils/FastForwardDC2x2ITImpl1.h
    ./include/CodecUtils/FastForwardDC4x4ITImpl1.h
    ./include/CodecUtils/FastForwardDctImpl1.h
    ./include/CodecUtils/FastForwardDctImpl2.h
    ./include/CodecUtils/FastInterLastRunLevelH263VlcEncoder.h
    ./include/CodecUtils/FastIntraLastRunLevelH263VlcEncoder.h
    ./include/CodecUtils/FastInverse4x4ITImpl1.h
    ./include/CodecUtils/FastInverse4x4On16x16ITImpl1.h
    ./include/CodecUtils/FastInverseDC2x2ITImpl1.h
    ./include/CodecUtils/FastInverseDC4x4ITImpl1.h
    ./include/CodecUtils/FastInverseDctImpl1.h
    ./include/CodecUtils/FastInverseDctImplZDet2.h
    ./include/CodecUtils/FastInverseDctImplZDet.h
    ./include/CodecUtils/FastMotionVectorVlcDecoderImpl1.h
    ./include/CodecUtils/FastMotionVectorVlcDecoderImpl2.h
    ./include/CodecUtils/FastRunLengthVlcDecoderImpl1.h
    ./include/CodecUtils/FastRunLengthVlcDecoderImpl2.h
    ./include/CodecUtils/FastVectorQuantiserVlcDecoderImpl1.h
    ./include/CodecUtils/FastVectorQuantiserVlcDecoderImpl2.h
    ./include/CodecUtils/H264MbImgCache.h
    ./include/CodecUtils/H263MotionVectorPredictorImpl1.h
    ./include/CodecUtils/H264MotionVectorPredictorImpl1.h
    ./include/CodecUtils/H264RawFileHandler.h
    ./include/CodecUtils/IBitStreamReader.h
    ./include/CodecUtils/IBitStreamWriter.h
    ./include/CodecUtils/ICodecInnerAccess.h
    ./include/CodecUtils/ICodecv2.h
    ./include/CodecUtils/IContextAwareRunLevelCodec.h
    ./include/CodecUtils/IForwardDct.h
    ./include/CodecUtils/IForwardTransform.h
    ./include/CodecUtils/IInverseDct.h
    ./include/CodecUtils/IInverseTransform.h
    ./include/CodecUtils/ImagePlaneDecoder.h
    ./include/CodecUtils/ImagePlaneDecoderIntraImpl.h
    ./include/CodecUtils/ImagePlaneDecoderStdImpl.h
    ./include/CodecUtils/ImagePlaneEncoderConstQImpl.h
    ./include/CodecUtils/ImagePlaneEncoderFastImpl1.h
    ./include/CodecUtils/ImagePlaneEncoderFastImpl2.h
    ./include/CodecUtils/ImagePlaneEncoder.h
    ./include/CodecUtils/ImagePlaneEncoderIntraImpl.h
    ./include/CodecUtils/ImagePlaneEncoderStdImpl.h
    ./include/CodecUtils/IMotionCompensator.h
    ./include/CodecUtils/IMotionEstimator.h
    ./include/CodecUtils/IMotionVectorPredictor.h
    ./include/CodecUtils/InterPCBPYH263VlcDecoder.h
    ./include/CodecUtils/InterPCBPYH263VlcEncoder.h
    ./include/CodecUtils/InterPDctQuantiserImpl.h
    ./include/CodecUtils/InterPDctQuantiserImplLookUp.h
    ./include/CodecUtils/InterPLastRunLevH263Impl.h
    ./include/CodecUtils/InterPMCBPCH263VlcDecoder.h
    ./include/CodecUtils/InterPMCBPCH263VlcEncoder.h
    ./include/CodecUtils/IntraCBPYH263VlcDecoder.h
    ./include/CodecUtils/IntraCBPYH263VlcEncoder.h
    ./include/CodecUtils/IntraDCH263VlcDecoder.h
    ./include/CodecUtils/IntraDCH263VlcEncoder.h
    ./include/CodecUtils/IntraDctQuantiserImpl.h
    ./include/CodecUtils/IntraDctQuantiserImplLookUp.h
    ./include/CodecUtils/IntraLastRunLevelH263VlcDecoder.h
    ./include/CodecUtils/IntraLastRunLevelH263VlcEncoder.h
    ./include/CodecUtils/IntraLastRunLevH263Impl.h
    ./include/CodecUtils/IntraMCBPCH263VlcDecoder.h
    ./include/CodecUtils/IntraMCBPCH263VlcEncoder.h
    ./include/CodecUtils/IRateControl.h
    ./include/CodecUtils/IReEntry.h
    ./include/CodecUtils/IRunLengthCodec.h
    ./include/CodecUtils/IScalarQuantiser.h
    ./include/CodecUtils/IStreamHeader.h
    ./include/CodecUtils/IStreamHeaderReader.h
    ./include/CodecUtils/IVectorQuantiser.h
    ./include/CodecUtils/IVectorQuantiserVer2.h
    ./include/CodecUtils/IVlcDecoder.h
    ./include/CodecUtils/IVlcEncoder.h
    ./include/CodecUtils/IVQCodebookDim16.h
    ./include/CodecUtils/LastRunLevelH263List.h
    ./include/CodecUtils/LastRunLevelH263VlcDecoder.h
    ./include/CodecUtils/LastRunLevelH263VlcEncoder.h
    ./include/CodecUtils/LastRunLevelTypeStruct.h
    ./include/CodecUtils/MacroBlockH263.h
    ./include/CodecUtils/MacroBlockH264.h
    ./include/CodecUtils/ModDQuantH263VlcDecoder.h
    ./include/CodecUtils/ModDQuantH263VlcEncoder.h
    ./include/CodecUtils/ModQuantExtEscLevelH263VlcDecoder.h
    ./include/CodecUtils/ModQuantExtEscLevelH263VlcEncoder.h
    ./include/CodecUtils/MotionCompensatorH263ImplStd.h
    ./include/CodecUtils/MotionCompensatorH263ImplUnres.h
    ./include/CodecUtils/MotionCompensatorH264ImplMultires.h
    ./include/CodecUtils/MotionCompensatorH264ImplStd.h
    ./include/CodecUtils/MotionCompensatorImpl1.h
    ./include/CodecUtils/MotionEstimatorH263ImplE3SS.h
    ./include/CodecUtils/MotionEstimatorH263ImplMultiresCross.h
    ./include/CodecUtils/MotionEstimatorH263ImplMultiresCrossVer2.h
    ./include/CodecUtils/MotionEstimatorH263ImplStd.h
    ./include/CodecUtils/MotionEstimatorH263ImplUnres.h
    ./include/CodecUtils/MotionEstimatorH264ImplMultiresCross.h
    ./include/CodecUtils/MotionEstimatorH264ImplMultiresCrossVer2.h
    ./include/CodecUtils/MotionEstimatorH264ImplMultires.h
    ./include/CodecUtils/MotionEstimatorH264ImplFull.h
    ./include/CodecUtils/MotionEstimatorH264ImplCross.h
    ./include/CodecUtils/MotionEstimatorH264ImplTest.h
    ./include/CodecUtils/MotionEstimatorH264ImplUMHS.h
    ./include/CodecUtils/MotionEstimatorH264ImplFHS.h
    ./include/CodecUtils/MotionEstimatorImpl1.h
    ./include/CodecUtils/MotionEstimatorImpl2.h
    ./include/CodecUtils/MotionEstimatorH264ImplMultiresCrossVer2.h
    ./include/CodecUtils/MotionVectorH263VlcDecoderImplRev.h
    ./include/CodecUtils/MotionVectorH263VlcDecoderImplStd.h
    ./include/CodecUtils/MotionVectorH263VlcEncoderImplRev.h
    ./include/CodecUtils/MotionVectorH263VlcEncoderImplStd.h
    ./include/CodecUtils/MotionVectorVlcDecoder.h
    ./include/CodecUtils/MotionVectorVlcEncoder.h
    ./include/CodecUtils/NalHeaderH264.h
    ./include/CodecUtils/PicParamSetH264.h
    ./include/CodecUtils/PrefixH264VlcDecoderImpl1.h
    ./include/CodecUtils/PrefixH264VlcEncoderImpl1.h
    ./include/CodecUtils/RateControlImpl1.h
    ./include/CodecUtils/RateControlImplPow.h
    ./include/CodecUtils/RateControlImplLog.h
    #./include/CodecUtils/RateControlImplMultiModel.h
    ./include/CodecUtils/RateControlImplQuad.h
    ./include/CodecUtils/RawFileHandlerBase.h
    ./include/CodecUtils/RleMotionVectorCodec.h
    ./include/CodecUtils/RleMotionVectorList.h
    ./include/CodecUtils/RunBeforeH264VlcDecoder.h
    ./include/CodecUtils/RunBeforeH264VlcEncoder.h
    ./include/CodecUtils/RunLengthVlcDecoder.h
    ./include/CodecUtils/RunLengthVlcEncoder.h
    ./include/CodecUtils/SeqParamSetH264.h
    ./include/CodecUtils/SimpleMotionVectorList.h
    ./include/CodecUtils/SliceHeaderH264.h
    ./include/CodecUtils/StreamSwitcher2.h
    ./include/CodecUtils/StreamSwitcher.h
    ./include/CodecUtils/StreamSwitcherVer3.h
    ./include/CodecUtils/SwitchCodecHeaderVer1.h
    ./include/CodecUtils/TotalZeros2x2H264VlcDecoder.h
    ./include/CodecUtils/TotalZeros2x2H264VlcEncoder.h
    ./include/CodecUtils/TotalZeros2x4H264VlcDecoder.h
    ./include/CodecUtils/TotalZeros2x4H264VlcEncoder.h
    ./include/CodecUtils/TotalZeros4x4H264VlcDecoder.h
    ./include/CodecUtils/TotalZeros4x4H264VlcEncoder.h
    ./include/CodecUtils/VectorList.h
    ./include/CodecUtils/VectorQuantiserBaseImpl1.h
    ./include/CodecUtils/VectorQuantiserCbImpl1.h
    ./include/CodecUtils/VectorQuantiserCrImpl1.h
    ./include/CodecUtils/VectorQuantiserDim16Impl1.h
    ./include/CodecUtils/VectorQuantiserDim16Impl2.h
    ./include/CodecUtils/VectorQuantiserDim16Impl3.h
    ./include/CodecUtils/VectorQuantiserLumImpl1.h
    ./include/CodecUtils/VectorQuantiserVer2Dim16Impl1.h
    ./include/CodecUtils/VectorQuantiserVlcDecoder.h
    ./include/CodecUtils/VectorQuantiserVlcEncoder.h
    ./include/CodecUtils/VectorStructList.h
    ./include/CodecUtils/YuvRawFileHandler.h
)
SET(CODEC_UTILS_SRCS
    ./src/CodecUtils/AdvancedIntraDctQuantiserImpl.cpp
    ./src/CodecUtils/AdvancedIntraDctQuantiserImplLookUp.cpp
    ./src/CodecUtils/AdvancedIntraModeH263VlcDecoder.cpp
    ./src/CodecUtils/AdvancedIntraModeH263VlcEncoder.cpp
    ./src/CodecUtils/BitStreamReader.cpp
    ./src/CodecUtils/BitStreamReaderMSB.cpp
    ./src/CodecUtils/BitStreamWriter.cpp
    ./src/CodecUtils/BitStreamWriterMSB.cpp
    ./src/CodecUtils/BlockH263.cpp
    ./src/CodecUtils/BlockH264.cpp
    ./src/CodecUtils/CAVLCH264Impl.cpp
    ./src/CodecUtils/CodecUtilsPch.cpp
    ./src/CodecUtils/CodedBlkPatternH264VlcDecoder.cpp
    ./src/CodecUtils/CodedBlkPatternH264VlcEncoder.cpp
    ./src/CodecUtils/CoeffTokenH264VlcDecoder.cpp
    ./src/CodecUtils/CoeffTokenH264VlcEncoder.cpp
    ./src/CodecUtils/ColourPlaneDecoding.cpp
    ./src/CodecUtils/ColourPlaneEncoding.cpp
    ./src/CodecUtils/DQuantH263VlcDecoder.cpp
    ./src/CodecUtils/DQuantH263VlcEncoder.cpp
    ./src/CodecUtils/ExpGolombUnsignedVlcDecoder.cpp
    ./src/CodecUtils/ExpGolombUnsignedVlcEncoder.cpp
    ./src/CodecUtils/FastForward4x4ITImpl1.cpp
    ./src/CodecUtils/FastForward4x4ITImpl2.cpp
    ./src/CodecUtils/FastForward4x4On16x16ITImpl1.cpp
    ./src/CodecUtils/FastForwardDC2x2ITImpl1.cpp
    ./src/CodecUtils/FastForwardDC4x4ITImpl1.cpp
    ./src/CodecUtils/FastForwardDctImpl1.cpp
    ./src/CodecUtils/FastForwardDctImpl2.cpp
    ./src/CodecUtils/FastInterLastRunLevelH263VlcEncoder.cpp
    ./src/CodecUtils/FastIntraLastRunLevelH263VlcEncoder.cpp
    ./src/CodecUtils/FastInverse4x4ITImpl1.cpp
    ./src/CodecUtils/FastInverse4x4On16x16ITImpl1.cpp
    ./src/CodecUtils/FastInverseDC2x2ITImpl1.cpp
    ./src/CodecUtils/FastInverseDC4x4ITImpl1.cpp
    ./src/CodecUtils/FastInverseDctImpl1.cpp
    ./src/CodecUtils/FastInverseDctImplZDet2.cpp
    ./src/CodecUtils/FastInverseDctImplZDet.cpp
    ./src/CodecUtils/FastMotionVectorVlcDecoderImpl1.cpp
    ./src/CodecUtils/FastMotionVectorVlcDecoderImpl2.cpp
    ./src/CodecUtils/FastRunLengthVlcDecoderImpl1.cpp
    ./src/CodecUtils/FastRunLengthVlcDecoderImpl2.cpp
    ./src/CodecUtils/FastVectorQuantiserVlcDecoderImpl1.cpp
    ./src/CodecUtils/FastVectorQuantiserVlcDecoderImpl2.cpp
    ./src/CodecUtils/H264MbImgCache.cpp
    ./src/CodecUtils/H264RawFileHandler.cpp
    ./src/CodecUtils/ImagePlaneDecoder.cpp
    ./src/CodecUtils/ImagePlaneDecoderIntraImpl.cpp
    ./src/CodecUtils/ImagePlaneDecoderStdImpl.cpp
    ./src/CodecUtils/ImagePlaneEncoderConstQImpl.cpp
    ./src/CodecUtils/ImagePlaneEncoder.cpp
    ./src/CodecUtils/ImagePlaneEncoderFastImpl1.cpp
    ./src/CodecUtils/ImagePlaneEncoderFastImpl2.cpp
    ./src/CodecUtils/ImagePlaneEncoderIntraImpl.cpp
    ./src/CodecUtils/ImagePlaneEncoderStdImpl.cpp
    ./src/CodecUtils/InterPCBPYH263VlcDecoder.cpp
    ./src/CodecUtils/InterPCBPYH263VlcEncoder.cpp
    ./src/CodecUtils/InterPDctQuantiserImpl.cpp
    ./src/CodecUtils/InterPDctQuantiserImplLookUp.cpp
    ./src/CodecUtils/InterPLastRunLevH263Impl.cpp
    ./src/CodecUtils/InterPMCBPCH263VlcDecoder.cpp
    ./src/CodecUtils/InterPMCBPCH263VlcEncoder.cpp
    ./src/CodecUtils/IntraCBPYH263VlcDecoder.cpp
    ./src/CodecUtils/IntraCBPYH263VlcEncoder.cpp
    ./src/CodecUtils/IntraDCH263VlcDecoder.cpp
    ./src/CodecUtils/IntraDCH263VlcEncoder.cpp
    ./src/CodecUtils/IntraDctQuantiserImpl.cpp
    ./src/CodecUtils/IntraDctQuantiserImplLookUp.cpp
    ./src/CodecUtils/IntraLastRunLevelH263VlcDecoder.cpp
    ./src/CodecUtils/IntraLastRunLevelH263VlcEncoder.cpp
    ./src/CodecUtils/IntraLastRunLevH263Impl.cpp
    ./src/CodecUtils/IntraMCBPCH263VlcDecoder.cpp
    ./src/CodecUtils/IntraMCBPCH263VlcEncoder.cpp
    ./src/CodecUtils/LastRunLevelH263List.cpp
    ./src/CodecUtils/LastRunLevelH263VlcDecoder.cpp
    ./src/CodecUtils/LastRunLevelH263VlcEncoder.cpp
    ./src/CodecUtils/MacroBlockH263.cpp
    ./src/CodecUtils/MacroBlockH264.cpp
    ./src/CodecUtils/ModDQuantH263VlcDecoder.cpp
    ./src/CodecUtils/ModDQuantH263VlcEncoder.cpp
    ./src/CodecUtils/ModQuantExtEscLevelH263VlcDecoder.cpp
    ./src/CodecUtils/ModQuantExtEscLevelH263VlcEncoder.cpp
    ./src/CodecUtils/MotionCompensatorH263ImplStd.cpp
    ./src/CodecUtils/MotionCompensatorH263ImplUnres.cpp
    ./src/CodecUtils/MotionCompensatorH264ImplStd.cpp
    ./src/CodecUtils/MotionCompensatorImpl1.cpp
    ./src/CodecUtils/MotionEstimatorH263ImplE3SS.cpp
    ./src/CodecUtils/MotionEstimatorH263ImplMultiresCross.cpp
    ./src/CodecUtils/MotionEstimatorH263ImplStd.cpp
    ./src/CodecUtils/MotionEstimatorH263ImplUnres.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplTest.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplCross.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplFull.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplMultires.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplMultiresCross.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplMultiresCrossVer2.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplUMHS.cpp
    ./src/CodecUtils/MotionEstimatorH264ImplFHS.cpp
    ./src/CodecUtils/MotionEstimatorImpl1.cpp
    ./src/CodecUtils/MotionEstimatorImpl2.cpp
    ./src/CodecUtils/MotionVectorH263VlcDecoderImplRev.cpp
    ./src/CodecUtils/MotionVectorH263VlcDecoderImplStd.cpp
    ./src/CodecUtils/MotionVectorH263VlcEncoderImplRev.cpp
    ./src/CodecUtils/MotionVectorH263VlcEncoderImplStd.cpp
    ./src/CodecUtils/MotionVectorVlcDecoder.cpp
    ./src/CodecUtils/MotionVectorVlcEncoder.cpp
    ./src/CodecUtils/NalHeaderH264.cpp
    ./src/CodecUtils/PicParamSetH264.cpp
    ./src/CodecUtils/RateControlImpl1.cpp
    ./src/CodecUtils/RateControlImplPow.cpp
    ./src/CodecUtils/RateControlImplLog.cpp
    ./src/CodecUtils/RateControlImplMultiModel.cpp
    ./src/CodecUtils/RateControlImplQuad.cpp
    ./src/CodecUtils/RawFileHandlerBase.cpp
    ./src/CodecUtils/RleMotionVectorCodec.cpp
    ./src/CodecUtils/RleMotionVectorList.cpp
    ./src/CodecUtils/RunBeforeH264VlcDecoder.cpp
    ./src/CodecUtils/RunBeforeH264VlcEncoder.cpp
    ./src/CodecUtils/RunLengthVlcDecoder.cpp
    ./src/CodecUtils/RunLengthVlcEncoder.cpp
    ./src/CodecUtils/SeqParamSetH264.cpp
    ./src/CodecUtils/SimpleMotionVectorList.cpp
    ./src/CodecUtils/SliceHeaderH264.cpp
    ./src/CodecUtils/StreamSwitcherVer3.cpp
    ./src/CodecUtils/SwitchCodecHeaderVer1.cpp
    ./src/CodecUtils/TotalZeros2x2H264VlcDecoder.cpp
    ./src/CodecUtils/TotalZeros2x2H264VlcEncoder.cpp
    ./src/CodecUtils/TotalZeros2x4H264VlcDecoder.cpp
    ./src/CodecUtils/TotalZeros2x4H264VlcEncoder.cpp
    ./src/CodecUtils/TotalZeros4x4H264VlcDecoder.cpp
    ./src/CodecUtils/TotalZeros4x4H264VlcEncoder.cpp
    ./src/CodecUtils/VectorList.cpp
    ./src/CodecUtils/VectorQuantiserDim16Impl1.cpp
    ./src/CodecUtils/VectorQuantiserDim16Impl2.cpp
    ./src/CodecUtils/VectorQuantiserDim16Impl3.cpp
    ./src/CodecUtils/VectorQuantiserVlcDecoder.cpp
    ./src/CodecUtils/VectorQuantiserVlcEncoder.cpp
    ./src/CodecUtils/VectorStructList.cpp
    ./src/CodecUtils/H263MotionVectorPredictorImpl1.cpp
    ./src/CodecUtils/MotionEstimatorH263ImplMultiresCrossVer2.cpp
    ./src/CodecUtils/YuvRawFileHandler.cpp
    ${VCODEC_SRCS}
    )
SET(GEN_UTIL_HDRS
    ./include/GeneralUtils/DataProfileImplBase.h
    ./include/GeneralUtils/DataProfileImplCongest.h
    ./include/GeneralUtils/DataProfileImplRandom.h
    ./include/GeneralUtils/DataProfileImplSetQ8Intra.h
    ./include/GeneralUtils/IDataProfile.h
    ./include/GeneralUtils/MeasurementTable.h
    ./include/GeneralUtils/Fifo.h
    ./include/GeneralUtils/LinearModel.h
    )

SET(GEN_UTILS_SRCS
    ./src/GeneralUtils/DataProfileImplBase.cpp
    ./src/GeneralUtils/DataProfileImplCongest.cpp
    ./src/GeneralUtils/DataProfileImplRandom.cpp
    ./src/GeneralUtils/DataProfileImplSetQ8Intra.cpp
    ./src/GeneralUtils/MeasurementTable.cpp
    ./src/GeneralUtils/LinearModel.cpp
    )
# see (http://stackoverflow.com/questions/11580748/using-cmake-for-making-a-project-which-includes-mfc)
# see http://stackoverflow.com/questions/16698882/error-c2039-setdefaultdlldirectoriesis-not-a-member-of-global-namespace
IF (BUILD_CODEC_ANALYSER)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_MFC_IMAGE_UTILS -D_AFXDLL -D_USING_V110_SDK71_")
    SET(MFC_HDRS
	./include/IImageUtils/AutoCrop.h
        ./include/ImageUtils/AviFile.h
        ./include/ImageUtils/AviFileHandler.h
        ./include/ImageUtils/AviFileHandlerUsingCImage.h
        ./include/ImageUtils/Block.h
        ./include/ImageUtils/CImage.h
        ./include/ImageUtils/Graph.h
        ./include/ImageUtils/ImageHandler.h
        ./include/ImageUtils/ImageHandlerNET.h
        )
ELSE (BUILD_CODEC_ANALYSER)
    SET(MFC_HDRS
        )
ENDIF (BUILD_CODEC_ANALYSER)
SET(IMAGE_UTIL_HDRS
    ${MFC_HDRS}
    ./include/ImageUtils/FastFixedPointRGB24toYUV420Converter.h
    #AviFileHandlerNET.h
    #Block.h
    ./include/ImageUtils/BlockMotionVector.h
    ./include/ImageUtils/BlockPatternConstants.h
    ./include/ImageUtils/BlockPatternVlcEncoder.h
    ./include/ImageUtils/Codec.h
    ./include/ImageUtils/Dct2dodd.h
    ./include/ImageUtils/Dwtcodec.h
    ./include/ImageUtils/DwtQnt.h
    ./include/ImageUtils/EncBlockMotionVector.h
    ./include/ImageUtils/EncMotionFactory.h
    ./include/ImageUtils/EncMotionVector.h
    ./include/ImageUtils/EncMotionVectorPattern.h
    ./include/ImageUtils/ExtMem2D.h
    ./include/ImageUtils/ExtMemBlock2D.h
    ./include/ImageUtils/Fast6BitYUV420toRGB24Converter.h
    ./include/ImageUtils/Fast6BitYUV420toRGB24ZoomConverter.h
    ./include/ImageUtils/FastRGB24toYUV420Converter.h
    ./include/ImageUtils/FastYUV420toInvRGB12Converter.h
    ./include/ImageUtils/FastYUV420toInvRGB16Converter.h
    ./include/ImageUtils/FastYUV420toInvRGB24Converter.h
    ./include/ImageUtils/FastYUV420toInvRGB32Converter.h
    ./include/ImageUtils/FastYUV420toInvRGB32Converter_ByteWrite.h
    ./include/ImageUtils/FastYUV420toRGB16Converter.h
    ./include/ImageUtils/FastYUV420toRGB24Converter.h
    ./include/ImageUtils/FastYUV420toRGB24ZoomConverter.h
    ./include/ImageUtils/FastYUV420toRGB32Converter.h
    ./include/ImageUtils/FastYUV420toRGB32Converter_ByteWrite.h
    ./include/ImageUtils/FastYUV420toRGBConverter.h
    ./include/ImageUtils/H263Codec.h
    ./include/ImageUtils/IExistance.h
    ./include/ImageUtils/ImageDefs.h
    ./include/ImageUtils/ImageHandlerV2.h
    ./include/ImageUtils/ImageObjectHandler.h
    ./include/ImageUtils/ImgCodec.h
    ./include/ImageUtils/ImgSmp.h
    ./include/ImageUtils/IMotionFactory.h
    ./include/ImageUtils/Mem2D.h
    ./include/ImageUtils/MemBlock2D.h
    ./include/ImageUtils/MotionVector.h
    ./include/ImageUtils/MotionVectorPattern.h
    ./include/ImageUtils/MotionVectorTree.h
    ./include/ImageUtils/OverlayExtMem2D.h
    ./include/ImageUtils/OverlayExtMem2Dv2.h
    ./include/ImageUtils/OverlayMem2D.h
    ./include/ImageUtils/OverlayMem2Dv2.h
    ./include/ImageUtils/PicConcatBase.h
    ./include/ImageUtils/PicConcatRGB24Impl.h
    ./include/ImageUtils/PicConcatRGB32Impl.h
    ./include/ImageUtils/PicCropperBase.h
    ./include/ImageUtils/PicCropperRGB24Impl.h
    ./include/ImageUtils/PicCropperRGB32Impl.h
    ./include/ImageUtils/PicInPicBase.h
    ./include/ImageUtils/PicInPicRGB24Impl.h
    ./include/ImageUtils/PicInPicRGB32Impl.h
    ./include/ImageUtils/PicRotateBase.h
    ./include/ImageUtils/PicRotateRGB24Impl.h
    ./include/ImageUtils/PicRotateRGB32Impl.h
    ./include/ImageUtils/PicRotateRGBBase.h
    ./include/ImageUtils/PicScalerBase.h
    ./include/ImageUtils/PicScalerRGB24Impl.h
    ./include/ImageUtils/PicScalerYUV420PImpl.h
    ./include/ImageUtils/PsyQnt.h
    ./include/ImageUtils/Real6BitRGB24toYUV420Converter.h
    ./include/ImageUtils/Real6BitYUV420toRGB24Converter.h
    ./include/ImageUtils/RealRGB24toYUV420CCIR601Converter.h
    ./include/ImageUtils/RealRGB24toYUV420CCIR601ConverterVer16.h
    ./include/ImageUtils/RealRGB24toYUV420Converter.h
    ./include/ImageUtils/RealRGB24toYUV420ConverterImpl2.h
    ./include/ImageUtils/RealRGB24toYUV420ConverterImpl2Ver16.h
    ./include/ImageUtils/RealRGB24toYUV420ConverterStl.h
    ./include/ImageUtils/RealRGB32toYUV420Converter.h
    ./include/ImageUtils/RealRGB32toYUV420ConverterStl.h
    ./include/ImageUtils/RealYUV420toRGB24CCIR601Converter.h
    ./include/ImageUtils/RealYUV420toRGB24CCIR601ConverterVer16.h
    ./include/ImageUtils/RealYUV420toRGB24Converter.h
    ./include/ImageUtils/RealYUV420toRGB24ConverterImpl2.h
    ./include/ImageUtils/RealYUV420toRGB24ConverterImpl2Ver16.h
    ./include/ImageUtils/RealYUV420toRGB24ConverterStl.h
    ./include/ImageUtils/RealYUV444toRGB24Converter.h
    ./include/ImageUtils/RGB24toRGB32Converter.h
    ./include/ImageUtils/RGB32toRGB24Converter.h
    ./include/ImageUtils/RGBtoRGBConverter.h
    ./include/ImageUtils/RGBtoYUV420Converter.h
    ./include/ImageUtils/RGBtoYUV420ConverterStl.h
    ./include/ImageUtils/Rlcodec.h
    ./include/ImageUtils/Sampler.h
    ./include/ImageUtils/SampleSet.h
    ./include/ImageUtils/stdafx.h
    ./include/ImageUtils/VectorQuantiser.h
    ./include/ImageUtils/VectorQuantiserVlcEncoder.h
    ./include/ImageUtils/VectorSpace.h
    ./include/ImageUtils/VidCodec.h
    ./include/ImageUtils/VideoCodecVer01.h
    ./include/ImageUtils/VideoDim16VectorQuantiser.h
    ./include/ImageUtils/VQcodec.h
    ./include/ImageUtils/WaveletCompress.h
    ./include/ImageUtils/WImage.h
    ./include/ImageUtils/WImgPce.h
    ./include/ImageUtils/YUV420toRGBConverter.h
    ./include/ImageUtils/YUV420toRGBConverterStl.h
    ./include/ImageUtils/YUV444toRGBConverter.h
)
# for now just check if we're building the codec analyser
# as a check whether to include the MFC-related classes
IF (BUILD_CODEC_ANALYSER)
    SET(MFC_SRCS
        ./src/ImageUtils/AutoCrop.cpp
        ./src/ImageUtils/AviFile.cpp
        ./src/ImageUtils/AviFileHandler.cpp
        ./src/ImageUtils/CImage.cpp
        ./src/ImageUtils/Graph.cpp
        ./src/ImageUtils/ImageHandler.cpp
        )
ELSE (BUILD_CODEC_ANALYSER)
    SET(MFC_SRCS
        )
ENDIF (BUILD_CODEC_ANALYSER)
SET(IMAGE_UTILS_SRCS
    ${MFC_SRCS}
    ./src/ImageUtils/FastFixedPointRGB24toYUV420Converter.cpp
    #AviFileHandlerNET.cpp
    #AviFileHandlerUsingCImage.cpp
    #BlockMotionVector.cpp
    #BlockPatternVlcEncoder.cpp
    #Dct2dodd.cpp
    #Dwtcodec.cpp
    #DwtQnt.cpp
    #EncBlockMotionVector.cpp
    #EncMotionFactory.cpp
    #EncMotionVector.cpp
    #EncMotionVectorPattern.cpp
    #ExtMem2D.cpp
    #ExtMemBlock2D.cpp
    #Fast6BitYUV420toRGB24Converter.cpp
    #Fast6BitYUV420toRGB24ZoomConverter.cpp
    #FastRGB24toYUV420Converter.cpp
    #FastYUV420toInvRGB12Converter.cpp
    #FastYUV420toInvRGB16Converter.cpp
    #FastYUV420toInvRGB24Converter.cpp
    #FastYUV420toInvRGB32Converter_ByteWrite.cpp
    #FastYUV420toInvRGB32Converter.cppadd_definitions(
    #FastYUV420toRGB16Converter.cpp
    #FastYUV420toRGB24Converter.cpp
    #FastYUV420toRGB24ZoomConverter.cpp
    #FastYUV420toRGB32Converter_ByteWrite.cpp
    #FastYUV420toRGB32Converter.cpp
    #H263Codec.cpp
    #ImageHandlerNET.cpp
    #ImageHandlerV2.cpp
    #ImageObjectHandler.cpp
    #ImgCodec.cpp
    #ImgSmp.cpp
    #Mem2D.cpp
    #MemBlock2D.cpp
    #MotionVector.cpp
    #MotionVectorPattern.cpp
    #MotionVectorTree.cpp
    ./src/ImageUtils/OverlayExtMem2D.cpp
    ./src/ImageUtils/OverlayExtMem2Dv2.cpp
    ./src/ImageUtils/OverlayMem2D.cpp
    ./src/ImageUtils/OverlayMem2Dv2.cpp
    ./src/ImageUtils/PicConcatRGB24Impl.cpp
    ./src/ImageUtils/PicConcatRGB32Impl.cpp
    ./src/ImageUtils/PicCropperRGB24Impl.cpp
    ./src/ImageUtils/PicCropperRGB32Impl.cpp
    ./src/ImageUtils/PicInPicRGB24Impl.cpp
    ./src/ImageUtils/PicInPicRGB32Impl.cpp
    ./src/ImageUtils/PicRotateRGB24Impl.cpp
    ./src/ImageUtils/PicRotateRGB32Impl.cpp
    ./src/ImageUtils/PicRotateRGBBase.cpp
    ./src/ImageUtils/PicScalerRGB24Impl.cpp
    ./src/ImageUtils/PicScalerYUV420PImpl.cpp
    #./src/ImageUtils/PsyQnt.cpp
    #./src/ImageUtils/Real6BitRGB24toYUV420Converter.cpp
    #./src/ImageUtils/Real6BitYUV420toRGB24Converter.cpp
    #./src/ImageUtils/RealRGB24toYUV420CCIR601Converter.cpp
    ./src/ImageUtils/RealRGB24toYUV420Converter.cpp
    ./src/ImageUtils/RealRGB24toYUV420ConverterImpl2.cpp
    ./src/ImageUtils/RealRGB24toYUV420ConverterImpl2Ver16.cpp
    ./src/ImageUtils/RealRGB32toYUV420Converter.cpp
    #./src/ImageUtils/RealYUV420toRGB24CCIR601Converter.cpp
    ./src/ImageUtils/RealYUV420toRGB24Converter.cpp
    ./src/ImageUtils/RealYUV420toRGB24ConverterImpl2.cpp
    ./src/ImageUtils/RealYUV420toRGB24ConverterImpl2Ver16.cpp
    ./src/ImageUtils/RealYUV444toRGB24Converter.cpp
    ./src/ImageUtils/RGB24toRGB32Converter.cpp
    ./src/ImageUtils/RGB32toRGB24Converter.cpp
    #Rlcodec.cpp
    #Sampler.cpp
    #SampleSet.cpp
    #VectorQuantiserVlcEncoder.cpp
    #VectorSpace.cpp
    #VidCodec.cpp
    #VideoDim16VectorQuantiser.cpp
    #VQcodec.cpp
    #WaveletCompress.cpp
    #WImage.cpp
    #WImgPce.cpp
)

# interface library for now
add_library(vpp STATIC
    ${CODEC_UTILS_SRCS} 
	${GEN_UTILS_SRCS}
    ${IMAGE_UTILS_SRCS} 
    ${CODEC_UTIL_HDRS}
	${GEN_UTILS_HDRS}
    ${IMAGE_UTIL_HDRS} 
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
