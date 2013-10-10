#ifndef __IM_VDEC_COMMON_H__
#define __IM_VDEC_COMMON_H__
#include "InfotmMedia.h"

#define IM_VDEC_FLAG_NONE  		(0x0)
#define IM_VDEC_PIC_SKIP 		(0x1)
#define IM_VDEC_PIC_DIRECT_SHOW (0x2)

#define IM_VDEC_ROTATION_NONE     (0)
#define IM_VDEC_ROTATION_RIGHT_90 (1)
#define IM_VDEC_ROTATION_LEFT_90  (2)
#define IM_VDEC_ROTATION_HOR_FLIP (3)
#define IM_VDEC_ROTATION_VER_FLIP (4)
#define IM_VDEC_ROTATION_180      (5)

typedef void *IM_VDEC_CTX;

typedef enum CreationFlags {
	kPreferSoftwareCodecs    = 1,
	kPreferHardwareCodecs    = 2,
	kSoftwareCodecsOnly      = 4,
	kHardwareCodecsOnly      = 8,
}IM_VDEC_CREATION_FLAG;

typedef  enum IM_VDEC_CODEC_LIB
{
	SW_FFMPEG = 0x0,
	SW_VP6    = 0x1,
	SW_INFOTM = 0x2,

	HW_9170   = 0x10,
	HW_G1     = 0x11,

}IM_VDEC_LIB;

typedef   enum IM_VDEC_SUB_CODEC_
{
	NONE,    
	HW_MPEG4 = 0x1,
	HW_H263 = 0x2,

}IM_VDEC_SUB_CODEC;

typedef struct 
{
	IM_VDEC_LIB codecType;        /*indicate the decoder type*/
	IM_VDEC_SUB_CODEC codec;
	IM_BOOL bStandAloneModeSupport; /*decoder and  post process stand alone*/
	IM_BOOL bCombineModeSupport;    /*combine post process and decoder */
	IM_BOOL bMultiBuffModeSupport; 	/*MultiBuffMode pp*/
	IM_BOOL bMMUSupported;
	IM_UINT32 maxOutWidth;
	IM_UINT32 maxOutHeight;
}IM_VDEC_BUILD;

typedef struct
{
	IM_STREAM_SUBTYPE 	codec;
	IM_UINT32   		width;
	IM_UINT32   		height;
}IM_VDEC_INTYPE;

typedef struct
{
	IM_UINT32   		width;
	IM_UINT32   		height;
	IM_UINT32 		    format;
	IM_INT32    		rotation;
}IM_VDEC_OUTTYPE;

typedef struct IM_VDEC_MODE_
{
	IM_BOOL bStandAloneMode;  
	IM_BOOL bCombineMode;
	IM_BOOL bMultiBuffMode;
	IM_BOOL bMMUEnable;
}IM_VDEC_MODE;

typedef struct IM_VDEC_CONFIG_ 
{
	IM_VDEC_BUILD build;
	IM_VDEC_INTYPE inType;
	IM_VDEC_OUTTYPE outType;
	IM_VDEC_MODE mode;
}IM_VDEC_CONFIG;


typedef struct 
{
	void       *pStream;
	IM_UINT32  busAddr;
	IM_INT32   dataLen;
	IM_INT32   flags;
	IM_UINT32  picId;
}IM_VDEC_IN;

typedef struct 
{
	void      *pCurStream;
	IM_UINT32 strmCurrBusAddress;
	IM_INT32  dataLeft;
	IM_UINT32  multiBuffPpSize;
	IM_UINT32  skipPicNum;
}IM_VDEC_OUT;

typedef struct 
{
	IM_UINT32 size;
	IM_UINT8  data[1];
}IM_VDEC_SPEC_DATA;

typedef struct 
{
	void      *virtAddr;
	IM_UINT32 busAddr;
	IM_INT32  size;
}IM_VDEC_BUFFER;

typedef struct 
{
	IM_INT32 num;
	IM_VDEC_BUFFER buffer[1];
}IM_VDEC_BUFFERS;

typedef struct 
{
	IM_VDEC_BUFFER buffer;
	IM_UINT32 dataLen;
    //IM_UINT32 frameWidth;
    //IM_UINT32 frameHeight;
    //IM_UINT32 codedWidth;
    //IM_UINT32 codedHeight;
    IM_UINT32 keyPicture;
    IM_UINT32 picId;
    IM_UINT32 interlaced;
    //IM_UINT32 fieldPicture;
    //IM_UINT32 topField;
    //IM_UINT32 firstField;
    //IM_UINT32 repeatFirstField;
    //IM_UINT32 repeatFrameCount;
    //IM_UINT32 numberOfErrMBs;
    //IM_UINT32 outFormat;
	IM_INT32  cropWidth;
	IM_INT32  cropHeight;
	IM_INT32  skipNum;
}IM_VDEC_PICTURE;

typedef struct
{
	IM_UINT32 frameWidth;
	IM_UINT32 frameHeight;
	IM_UINT32 codedWidth;
	IM_UINT32 codedHeight;
	IM_UINT32 profileAndLevelIndication;
	IM_UINT32 displayAspectRatio;
	IM_UINT32 streamFormat;
	IM_UINT32 videoFormat;
	IM_UINT32 videoRange;      /* ??? only [0-255] */
	IM_UINT32 interlacedSequence;
	IM_UINT32 multiBuffPpSize;
	IM_UINT32 outputFormat;
} IM_VDEC_INFO;


typedef enum{
	IM_DEC_REF_FRM_RASTER_SCAN = 0,
	IM_DEC_REF_FRM_TILED_DEFAULT = 1,
} IM_REF_FRM_FORMAT;

#endif
