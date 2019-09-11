/************************************************************/
/* TITLE       :DWT STILL IMAGE COMPRESSION CODEC HEADER    */
/*              FILE FOR C IMPLEMENTATIONS WITH 16 BIT CPU  */
/*              ARCHITECTURES.                              */
/* VERSION     :2.0                                         */
/* FILE        :WaveletCompress.h                           */
/* DESCRIPTION :Function definitions for implementing a     */
/*              DWT-based still image compression and       */
/*              decompression codec. The input and output   */
/*              image data is in 24bit BGR colour format.   */
/*              The input image is converted to YUV422 and  */
/*              coded into a bit stream (compression) or    */
/*              the bit stream is decoded into a YUV422 and */
/*              converted back into 24bit BGR.              */
/* DATE        :January 2000                                */
/* AUTHOR      :K.L.Ferguson                                */
/************************************************************/
#ifndef _WAVELETCOMPRESS_H
#define _WAVELETCOMPRESS_H

/************************************************************/
/* Public interface to codec functions.                     */
/************************************************************/

extern void InitializeWaveletCompression(void);
extern int OpenWaveletCompression(int Quality,int RGB_Width,int RGB_Height,
                                  unsigned long int BitStreamByteLimit);
extern void CloseWaveletCompression(void);

extern int WaveletCompressionCode(void *pRGB,void *BitStream,
                                  unsigned long int *BitStreamByteSize);
extern int WaveletCompressionDecode(void *BitStream,unsigned long int BitStreamByteSize,
                                    void *pRGB);

extern int WaveletCompressionCodecOpen(void);
extern char *GetWaveletCompressionCodecError(void);

#endif


