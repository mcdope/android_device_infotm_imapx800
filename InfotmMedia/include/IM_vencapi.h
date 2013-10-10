/**
 *
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef __IM_VENCAPI_H__
#define __IM_VENCAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef VENC_EXPORTS
	#define VENC_API		__declspec(dllexport)	/* For dll lib */
#else
	#define VENC_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define VENC_API
#endif	
//#############################################################################

/******************************STREAM TYPE************************************/
#define VENC_H264_BYTE_STREAM	0x10
#define VENC_H264_NAL_UNIT_STREAM	0x11

#define VENC_MPEG4_PLAIN_STREAM	0x20
#define VENC_MPEG4_VP_STREAM	0x21
#define VENC_MPEG4_VP_DP_STREAM	0x22
#define VENC_MPEG4_VP_DP_RVLC_STRM	0x23
#define VENC_MPEG4_SVH_STREAM	0x24
#define VENC_MPEG4_H263_STREAM	0x25

/******************************LEVEL******************************************/
#define VENC_H264_LEVEL_1	0x100
#define VENC_H264_LEVEL_1_B	0x101
#define VENC_H264_LEVEL_1_1	0x102
#define VENC_H264_LEVEL_1_2	0x103
#define VENC_H264_LEVEL_1_3	0x104
#define VENC_H264_LEVEL_2	0x105
#define VENC_H264_LEVEL_2_1	0x106
#define VENC_H264_LEVEL_2_2	0x107
#define VENC_H264_LEVEL_3	0x108
#define VENC_H264_LEVEL_3_1	0x109
#define VENC_H264_LEVEL_3_2	0x10A
#define VENC_H264_LEVEL_4	0x10B
#define VENC_H264_LEVEL_4_1	0x10C


#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_0	0x200
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_0B	0x201
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_1	0x202
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_2	0x203
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_3	0x204
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_4A	0x205
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_5	0x206
#define VENC_MPEG4_SIMPLE_PROFILE_LEVEL_6	0x207
#define VENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_3	0x208
#define VENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_4	0x209
#define VENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_5	0x20A
#define VENC_MPEG4_MAIN_PROFILE_LEVEL_4		0x20B


#define VENC_H263_PROFILE_0_LEVEL_10	0x300
#define VENC_H263_PROFILE_0_LEVEL_20	0x301
#define VENC_H263_PROFILE_0_LEVEL_30	0x302
#define VENC_H263_PROFILE_0_LEVEL_40	0x303
#define VENC_H263_PROFILE_0_LEVEL_50	0x304
#define VENC_H263_PROFILE_0_LEVEL_60	0x305
#define VENC_H263_PROFILE_0_LEVEL_70	0x306


/**********************************************user data type ******************************************/
#define VENC_H264_USER_DATA_SEI	0x1000
#define VENC_H264_USER_DATA_xxx	

#define VENC_MPEG4_USER_DATA_VOS	0x2000
#define VENC_MPEG4_USER_DATA_VO		0x2001
#define VENC_MPEG4_USER_DATA_VOL	0x2002
#define VENC_MPEG4_USER_DATA_GOV	0x2003
/*********************************************coding frame type*****************************************/
#define VENC_I_FRAME	0x10001
#define VENC_P_FRAME	0x10002
#define VENC_B_FRAME	0x10003
#define VENC_NOT_CODED_FRAME	0x10004

/******************************************property key***********************************************/




/*******************************************encode flag**********************************************/
#define VENC_START_OF_STREAM    0x100001
#define VENC_END_OF_STREAM  0x100002




// video encoder context,saved by user
typedef void * VENCCTX;




/*********************************************build struct**************************************/
//video encode lib build infomation,user could know what the encode lib support from 
//this infomation
typedef struct{
	IM_UINT32	maxEncodedWidth;//max EncodedWidth our enc lib support	
	IM_UINT32   maxEncodedHeight;//max EncodedHeight our enc lib support//NOTE: it dose not means support MAXWxMAXH
	IM_UINT32	h264Enabled;	//venc lib support h264 encode
	IM_UINT32 	mpeg4Enabled;	//venc lib support mpeg4 encode
	IM_UINT32	h263Enabled;	//venc lib support h263 encode
	IM_UINT32 	mpeg2Enabled;	//venc_lib support mpeg2 encode
	IM_UINT32	mpeg1Enabled;	//venc_lib support mpeg1 encode
	IM_UINT32 	wmvEnabled;		//venc_lib support rv encode
	IM_UINT32 	vp6Enabled;		//venc_lib support rv encode
	IM_UINT32 	vp8Enabled;		//venc_lib support rv encode
	IM_UINT32	vsEnabled;		//venc lib support video stabilization 
	IM_UINT32	yuvEnabled;		//venc lib support yuv input 
	IM_UINT32	rgbEnabled;		//venc lib support rgb input 
	IM_UINT32   rotationEnabled;	//venc_lib support rotation 
	IM_UINT32 	cropEnabled;//venc_lib support crop 
}VENC_BUILD;


/*******************************************init struct*******************************************/
//this is for the user who do not care the details of the encode params
//they just want a low/main/high quality video stream
//please note that all the quality is dependant on the stream level user set
//main  quality:
typedef enum{
	LOW_QUALITY,
	MAIN_QUALITY,
	HIGH_QUALITY
}VENC_QUALITY;

typedef struct{
	//see IM_STRM_VIDEO_xx
	IM_STREAM_SUBTYPE	encFormat;
	/*
	IM_STREAM_VIDEO_H264:the strmType may be the follow values:
		1,VENC_H264_BYTE_STREAM The byte stream format separates each NAL unit with zero bytes and a start code prefix 
				which makes it easier to separate the NAL units from each other.
		2,VENC_H264_NAL_UNIT_STREAM	the data produced by the encoder does not contain the 4-byte start code 
				at the beginning of each NAL unit.

	IM_STREAM_VIDEO_MPEG4:the strmType may be the follow values:
		1,VENC_MPEG4_PLAIN_STREAM	The encoder will produce plain MPEG-4 stream.
		2,VENC_MPEG4_VP_STRM	The encoder will produce MPEG-4 stream with resynchronization markers,
				The size of a video package can be changed after the initialization of the encoder.
		3,VENC_MPEG4_VP_DP_STREAM	The encoder will produce MPEG-4 stream with resynchronization markers, 
				using data partitioning and reversible VLC (RVLC) code words.
		4,VENC_MPEG4_VP_DP_RVLC_STRM	The encoder will produce MPEG-4 stream with resynchronization markers, using data partitioning 
				and reversible VLC (RVLC) code words
		5,VENC_MPEG4_SVH_STREAM	The encoder will produce MPEG-4 stream that is using short video header (SVH) 
				format for the encoded video frames. Picture resolution has to be one of: sub-QCIF, QCIF, CIF or 4QCIF.
		6,VENC_MPEG4_H263_STREAM	he encoder will produce a stream compatible with the H.263 standard
							...
	*/
	IM_UINT32	strmType;	
	/*
   	IM_STREAM_VIDEO_H264	please see VENC_H264_LEVEL_xxx
	IM_STREAM_VIDEO_MPEG4	please see VENC_MPEG4_LEVEL_xxx
	IM_STREAM_VIDEOH263		please see VENC_H263_LEVEL_xxx
	*/
	IM_UINT32 	level;		 
	/*
	IM_STREAM_VIDEO_H264  	[96, 1920] and the width must be multiple of 4
	IM_STREAM_VIDEO_MPEG4	[96, 1280] for MPEG-4 or [96, 720] for H.263 and the width must be multiple of 4
	xxx		....
	*/
	IM_UINT32	width;
	/*
	IM_STREAM_VIDEO_H264	[96, 1920]	and the height must be multiple of 2
	IM_STREAM_VIDEO_MPEG4	[96, 1024] for MPEG-4 or [96, 720] for H.263 and the height must be multiple of 2;
	xxx			.....
	*/
	IM_UINT32	height;
	/*
	IM_STREAM_VIDEO_H264	Valid value range: [1, 1048575]
	IM_STREAM_VIDEO_MPEG4	Valid value range: [1, 65535]
	*/
	IM_UINT32	frameRateNum;
	/*
	IM_STREAM_VIDEO_H264	Valid value range: [1, frameRateNum]
	IM_STREAM_VIDEO_MPEG4	Valid value range: [1, frameRateNum]
	*/
	IM_UINT32	frameRateDenom;

	VENC_QUALITY	qualityType;
}VENC_CONFIG;



/************************************coding control struct***********************************/
typedef struct{
	/*Valid value range: [0, codingHeight/16>119?119:codingHeight/16]
	 *Sets the size of a slice in macroblock rows(a macroblock height is 16 pixels). 
	 *Zero value disables the use of slices,user can use the slices mode if the input buffer is too big
	 *in this mode,you should notice the size of the last slice to be encoded when the codingHeight is not multiple of 16
	 *This parameter can be updated during the encoding process.
	 * */
	IM_UINT32 sliceSize;
	/*Valid value range: [0, 1],0 disable,1 enable
	 * Enables insertion of picture timing and buffering period SEI messages 
	 * into the stream in the beginning of every encoded frame.Has to be set before starting encoding
	 */
	IM_UINT32 seiMessages;
	/*
	Valid value range: [0, 1]
		0 = Y range in [16, 235], Cb&Cr range in [16, 240]
		1 = Y, Cb and Cr range in [0, 255]
	Has to be set before starting encoding.
	*/
	IM_UINT32 videoFullRange;
	/*
	Valid value range: [0, 1]
	Sets the intra prediction to constrained mode allowing only intra macroblocks to be
	used for prediction. 
	Has to be set before starting the stream.
	*/
	IM_UINT32 constrainedIntraPrediction;
	/*
	Valid value range: [0, 2]
	Sets the mode of the deblocking filter. Value 0 enables filtering on all macroblock
	edges. Value 2 disables filtering on edges that are part of a slice border. Value 1
	disables entirely the deblocking filter. 
	Has to be set before starting the stream.
	*/
	IM_UINT32 disableDeblockingFilter;
	/*
	Valid value range: [0, 65535]
	Horizontal size of the sample aspect ratio (in arbitrary units), 0 for unspecified
	Has to be set before starting the stream.
	*/
	IM_UINT32 sampleAspectRatioWidth;
	/*
	Valid value range: [0, 65535]
	Vertical size of the sample aspect ratio (in arbitrary units), 0 for unspecified. 
	Has to be set before starting the stream
	*/
	IM_UINT32 sampleAspectRatioHeight;
	/*
	Valid value range: [0, 1]
	Enables CABAC for entropy coding. Stream profile will be either Main or High.
	Has to be set before starting the stream
	*/
	IM_UINT32 enableCabac;
	/*
	Valid value range: [0, 2]
	Selects one of the three CABAC initialization tables. 
	Has to be set before starting the stream
	*/
	IM_UINT32 cabacInitIdc;
	/*
	Valid value range: [0, 2].
	Controls the 8x8 transfrom usage. Stream profile will be set to High when enabled.
	Value 0 disables the usage. Value 1 enables the adaptive usage of 8x8 transform(above 720p disabled)
	and value 2 forces the transform to be used always.
	Has to be set before starting the stream
	*/
	IM_UINT32 transform8x8Mode;
	/*
	Valid value range: [0, 2].
	Controls the usage of 1⁄4 pixel motion estimation. When disabled, the motion
	estimation will be done using 1⁄2 pixel precision. 1⁄4 pixel precision improves
	compression efficiency but takes more time. Value 0 disables the usage. Value 1
	enables the adaptive usage (above 720p disabled)based on resolution and value 2 forces the use of 1⁄4
	pixel precision.
	This parameter can be updated during the encoding process.
	*/
	IM_UINT32 quarterPixelMv;
}VENC_H264_CODING_CTRL_CTX;


typedef struct{
	/*
	Valid value range: [0, 1]
		0 = Y range in [16, 235], Cb&Cr range in [16, 240]
		1 = Y, Cb and Cr range in [0, 255]
	Has to be set before starting encoding.
	*/
	IM_UINT32 videoFullRange;
	
	/*
	Valid range: [0, 255]
	Horizontal size of the pixel aspect ratio. A zero value will set the pixel aspect ratio
	to the default square, 1:1 ratio.
	Has to be set before starting the stream
	*/
	IM_UINT32 sampleAspectRatioWidth;
	/*
	Valid range: [0, 255]
	Vertical size of the pixel aspect ratio. A zero value will set the pixel aspect ratio to
	default square, 1:1 ratio.
	Has to be set before starting the stream
	*/
	IM_UINT32 sampleAspectRatioHeight;
	/*
	If different than zero then it will enable the Header Extension Code insertion when
	MPEG-4 video packaged stream is produced
	This parameter can be updated during the encoding process
	*/
	IM_UINT32 insHEC;
	/*
	This a bit mask for inserting Group of Macroblocs (GOB) header into an H.263 or an
	MPEG-4 short video header stream. Bit 0 (LSB) corresponds to GOB number 0, bit
	1 to GOB 1 and so on.
	This parameter can be updated during the encoding process
	*/
	IM_UINT32 insGOB;
	/*
	If different than zero then a Group of VOPs (GOV) header will be inserted in an
	MPEG-4 stream. After a GOV header is inserted the following video frame is INTRA
	coded. GOV is inserted just once
	*/
	IM_UINT32 insGOV;
	/*
	Valid value range: [1, 60000]
	Sets the target size in bits of a video package when MPEG-4 video packaged
	stream is produced. The VP size is limited by standard when the MPEG-4 stream is
	lso data partitioned.
	*/
	IM_UINT32 vpSize;

}VENC_MPEG4_CODING_CTRL_CTX;









/**********************************************rate control struct******************************************/

typedef struct{
	/*
	Enables rate control to adjust QP between frames.
	*/
	IM_UINT32 frameRc;
	/*
	Enables rate control to adjust QP inside frames.
	*/
	IM_UINT32 mbRc;
	/*
	Allow rate control to skip pictures if not enough bits are available.
	*/
	IM_UINT32	pictureSkip;

	/*
	Enable advanced rate control to adjust QP .For h264,decoder lib will use Hypothetical Reference Decoder model
	For mpeg4,decoder lib will use Video Buffering Verifier(VBV) model
	*/
	IM_UINT32  advancedRc;
	/*
	 quantization parameter:
	 Valid value range: -1 or [1, 31] mpeg4/h263
	 Valid value range: -1 or [0, 51] h264
	 The default quantization parameter used by the encoder.
	 If the rate control is enabled then this value is used only at the beginning of the encoding process.
	 When the rate control is disabled then this QP value is used all the time.
	 */
	IM_UINT32	qpHdr;
	/*
	Valid value range: [0, 51] H264,Not recommended to be set lower than 10.
	Valid value range: [1, 31] MPEG4/H263
	The minimum QP that can be set by the RC in the stream.
	*/
	IM_UINT32   qpMax;
	/*
	Valid value range: [qpMin, 51] H264
	Valid value range: [qpMin, 31] Mpeg4/H263
	The maximum QP that can be set by the RC in the stream
	*/
	IM_UINT32 	qpMin;
	/*
	Valid value range: [1, 150] 
	Length of group of pictures. Rate control calculates bit reserve for this GOP length.
	*/
	IM_UINT32	gopLen;
	/*
	The target bit rate in bits per second (bps) when the rate control is enabled.
	*/
	IM_UINT32	bitPerSecond;

	/*
	Valid value range: [-12, 12]	H264,for mpeg4 this value is unuse
	Value added to the Intra frame QP,Min/Max range checking still applies. Can be
	used to lower the Intra picture encoded size (higher QP) or to increase Intra quality
	relative to the Inter pictures (lower QP) to get rid of intra flashing.
	*/
	IM_INT32	intraQpDelta;

	/*
	Valid value range: [0, 51] H264,for mpeg4 this value is unuse
	Use this value for all Intra picture quantization.
	Value 0 disables the feature.Min/Max range checking still applies. intraQpDelta does not apply fixedIntraQp is in use.
	*/
	IM_UINT32 fixedIntraQp;

	/*
	Valid value range: [-8, 7] H264,for mpeg4 this value is unuse
	Enables MAD thresholding for macroblock based QP adjustment,Enabling this will disable MB RC
	*/
	IM_INT32	mbQpAdjustment;

}VENC_RATE_CTRL_CTX;




/**********************************encode preprocess structure**********************/

typedef enum{
	VENC_RGBTOYUV_BT601,
	VENC_RGBTOYUV_BT709,
	VENC_RGBTOYUV_USER_DEFINED
}VENC_COLOR_CONVERT_TYPE;

typedef struct{
	IM_CSC_TYPE type;
	IM_CSC_PARAMS custom_params;
}VENC_COLOR_CONVERTION;

typedef struct{
	//the pre process can do crop if the venc lib is built with this function
	//for the crop function,you'd better get build infomation to see if venc lib support this function
	/*
	IM_STREAM_VIDEO_H264 [96, 4096],the width must multiple of 16. 0 disable the crop 
	IM_STREAM_VIDEO_MPEG4	[96, 1920] the width must multiple of 16
	for the max width,you'd better get the max supportted size from build infomation
	*/
	IM_UINT32 orignalWidth;
	/*
	0 disable the crop
	IM_STREAM_VIDEO_H264	[96, 4096]
	IM_STREAM_VIDEO_MPEG4	[96, 1920]
	*/
	IM_UINT32 orignalHeight;
	/*
	This is the horizontal offset from the top-left corner of the input image to the top-left corner of the encoded image
	IM_STREAM_VIDEO_H264	[0, 4000]
	IM_STREAM_VIDEO_MPEG4	[0, 1824]
	*/
	IM_UINT32 xOffset;
	/*
	This is the vertical offset from the top-left corner of the input image to the top-left corner of the encoded image
	IM_STREAM_VIDEO_H264 	[0, 4000]
	IM_STREAM_VIDEO_MPEG4	[0, 1824]
	*/
	IM_UINT32 yOffset;

	/*
	 curently support YUV420_PLANAR/YUV420_SEMIPLANAR/YUV422_INTERLEAVED_YUYV/INTERLEAVED_UYVY
		RGB565/BGR565/RGB555(16bit)/BGR555(16bit)/RGB444(16bit)/BGR444(16bit)/RGB888(32bit)/BGR888(32bit)/
	   	RGB101010(32bit)/BGR101010(32bit)
		NOTE:you'd better get the build infomation to see if the rgb input is surportted
	
	 */
	IM_IMAGE_TYPE	inputFormat;

	/*
	rgb to yuv color convertion context
	 */
	VENC_COLOR_CONVERTION colorConvert;


	//more detail to see IM_ROTATE_XXX
	IM_ROTATE_TYPE	rotation;

	//if 1 enable video stabilization and scene changed detect function,0 disable it
	//NOTE: you'd better get the build infomation to see if venc lib support this
	IM_UINT32	videoStabilization;

}VENC_PREPROC_CTX;



/**********************************encode input struct**************************/
typedef struct{
	/*luma plane buffer for yuv color space or the whole input buffer for rgb /yuv interleaved */
	IM_Buffer	yBuffer;
	/*cb/cr plane for yuv420sp or cb plane for yuv420p*/
	IM_Buffer	cbBuffer;
	/*cr plane for yuv420p*/
	IM_Buffer	crBuffer;

	/*output encoded stream buffer*/
	IM_Buffer	outBuffer;

	/*the stabilization reference buffer*/
	IM_Buffer	stabBuffer;

	/*indicate the current frame coding type:I,P,B
	 * note: the result may be different the user set,the actual coding type,
	 * you should see the output structure of encoding*/
	IM_UINT32	codingType;
	/*The time stamp of the frame relative to the last encoded frame
	 * This is given in ticks (one tick equals 1/frameRateNum seconds). A zero value is usually used for
	 * the very first frame*/
	IM_UINT32	timeIncrement;

	//encode  flag
	//VENC_START_OF_STREAM,VENC_END_OF_STREAM
	IM_UINT32  flag;

	//the private data is for the special encode type 
	//for example,for h264_nal_stream_type the privateData can be used for store each nal size generated
	//for MPEG4_VP_STRM the privateData can be used for store each MPEG4 PACKAGE size 
	IM_UINT32		privateIn;
   	void *		*privateInSize;	
}VENC_IN;











/****************************************encode output sturcture************************/
typedef struct{
	/*The size of the generated stream in bytes is returned here. If the call was
	 * unsuccessful then this value is not relevant at all and it shall be ignored.
	 * */
	IM_UINT32	dataSize;
	/*The encoding type of the last frame as it was encoded into the stream
	 * NOTE AGAIN:This can be different from the type requested by the user
	 */
	IM_UINT32   codingType;
	void *privateOut;
	IM_UINT32 privateOutSize;

}VENC_OUT;




/*============================Interface API==================================*/
/* 
 * FUNC: get venc version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
VENC_API IM_UINT32 venc_version(OUT IM_TCHAR *ver_string);

/*FUNC:get venc build infomation ,user can use the configuration infomation to check what input our
 * encode lib support.
 *PARAMS:VENC_BUILD *build pointer to the address to save the build infomation
 *RETURN:NULL
 */
VENC_API void venc_build(OUT VENC_BUILD *build);
/* 
 * FUNC: init venc.
 * PARAMS: venc, save venc context.
	Video Encoder stream initialize params 
	encFomat 			IM_STREAM_VIDEO_H264/IM_STREAM_VIDEO_MPEG4/IM_STREAM_VIDEO_H263/IM_STREAM_VIDEO_MPEG2.....
   	strmType				this is closely dependant on encFormat user set.

   level				this is also closely dependant on encFormat user set.

   width				the encoded video width after croping and rotation    
   height 				the encoded video heigth after croping and rotation
   frameRateNum			The numerator part of the input frame rate,it also defines the stream time within a second
   frameRateDenom		The denominator part of the input frame rate,fps = frmRateNum/frmRateDenom
   qualityType		 	the encoder lib will adjust some bitrate control params or coding control params according to the params
   							it is just for the users who do not care the details of the bitrate control.If you want to adjust 
							encoded stream's bitrate accurately,it can refer to API venc_set_bit_control
 * RETURN: IM_RET_OK is successful, else failed.
 */
VENC_API IM_RET venc_init(OUT VENCCTX *venc, VENC_CONFIG *cfg);

/* 
 * FUNC: deinit venc.
 * PARAMS: venc, venc context.
 * RETURN: IM_RET_OK is successful, else failed.
 */
VENC_API void venc_deinit(IN VENCCTX venc);


/*
FUNC:set user data  in a series of pictures,the userData is copied internally ,so user can 
	release the user data safely. If user want disable the user data ,you can set the input params userData
	to NULL or userDataSize to zero.
PARAMS:VENCCTX venc			venc lib handle 
		 IM_VOID *userData	userData pointer,this is also closely dependant on encFormat user set.
		 						IM_STREAM_VIDEO_H264	the userData will be considered as SEI messages,and the userData size is valid in 
														0 or [16, 2048],dataType must be VENC_H264_USER_DATA_SEI
								IM_STREAM_VIDEO_MPEG4	dataType equal VENC_MPEG4_USER_DATA_VOS,the user data will be 
															inserted into the Visual Object Sequence header
														dataType equal VENC_MPEG4_USER_DATA_VO,the user data will be inserted into 
															the Visual Object header	
		 												dataType equal VENC_MPEG4_USER_DATA_VOL,the user data will be inserted into 
															the Video Object Layer header
														dataType equal VENC_MPEG4_USER_DATA_GOV,the user data will be inserted into 
															the Group of Video Object Plane header
								IM_STREAM_VIDEO_XXX		...

		 IM_INT32 userDataSize	how many userData will be set in bytes
		 IM_UINT32 dataType		user data type
RETURN:
*/
VENC_API IM_RET venc_set_user_data(IN VENCCTX venc,IN const void *userData,IN IM_UINT32 userDataSize,IM_UINT32 dataType);


/*
FUNC:Sets the encoder’s coding parameters,some coding params can be alert before every frame encoded,some params can not be
		changed after venc_encode().more details please refer VENC_H264_CODING_CTRL_CTX/VENC_MPEG4_CODING_CTRL_CTX/xxx 
		it is recommended to call venc_get_coding_ctrl() before call this api
PARAMS:
	void *codingData	it is closely dependant on encForamt .for example if the encForamt is IM_STREAM_VIDEO_H264,it it 
							pointed to the address of H264 coding control context and the codingSize must be bigger than
							VENC_H264_CODING_CTRL_CTX size
	IM_UINT32 codingSize  the codingData size
RETURN:
*/
VENC_API IM_RET venc_set_coding_ctrl(IN VENCCTX venc,IN const void *codingData,IN IM_UINT32 codingSize);

/*
FUNC:get current encode control params.
PARAMS:VENCCTX venc		video encode library handle
		see venc_set_coding_ctrl()
RETURN:
*/
VENC_API IM_RET venc_get_coding_ctrl(IN VENCCTX venc,OUT const void *codingData,OUT IM_UINT32 *codingSize);

/*
FUNC:Sets the encoder's rate control params in order to get the target bitrate.Our encode lib support the follow four kinds of
	method to doing the rate control:
		frameRc, encoder lib change the quantize parameters between frames 
		mbRc,encoder lib change the quantize parameters between mbs 
		skipframe,if the user set the target bitrate too small,and encoder lib can not get the target bitrate by adjusting the 
			QP,encoder lib can skip curren input streams and return a value to indicate that.
		advanced rate control,if user use the avanced rate control ,the amount of rate buffer memory required is always less than the
			standard-defined buffer size
	it is recommended to call venc_get_rate_ctrl() before call this api
PARAMS:VENCCTX venc	video encode library handle
		VENC_RATE_CTRL_CTX *rcData		rc structure
		
RETURN:
*/
VENC_API IM_RET venc_set_rate_ctrl(IN VENCCTX venc,IN VENC_RATE_CTRL_CTX *rcData);



/*
FUNC:get current rate control parameter from venc lib
PARAMS:VENC_RATE_CTRL_CTX *rcData		see VENC_RATE_CTRL_CTX
RETURN:
*/
VENC_API IM_RET venc_get_rate_ctrl(IN VENCCTX venc,OUT VENC_RATE_CTRL_CTX *rcData);



/*
FUNC:Get venc lib preprecess configuration
PARAMS:IN VENCCTX venc		venc library handle
		OUT VENC_PREPROC_CTX *ppCfg		venc preprocess infomation return to user
RETURN:
*/
VENC_API IM_RET venc_get_preprocess(IN VENCCTX venc,OUT VENC_PREPROC_CTX *ppCfg);

/*
FUNC:Set venc lib preprecess configuration
PARAMS:IN VENCCTX venc    venc library handle
		IN VENC_PREPROC_CTX *ppCfg		venc preprocess infomation set to venc lib
RETURN:
*/
VENC_API IM_RET venc_set_preprocess(IN VENCCTX venc,IN VENC_PREPROC_CTX *ppCfg);
/* 
 * FUNC: set venc property.
 * PARAMS: venc, venc context.
 * 	key, property key, see VENC_RPOP_KEY_xxx.
 *	value, the key's value.
 *	size, bytes of the value.
 * RETURN: IM_RET_OK is successful, else failed.
 */
VENC_API IM_RET venc_set_property(IN VENCCTX venc, IN IM_UINT32 key, IN void *value, IN IM_UINT32 size);

/* 
 * FUNC: get venc property.
 * PARAMS: venc, venc context.
 * 	key, property key, see VENC_RPOP_KEY_xxx.
 *	value, the key's value.
 *	size, bytes of the value.
 * RETURN: IM_RET_OK is successful, else failed.
 */
VENC_API IM_RET venc_get_property(IN VENCCTX venc, IN IM_UINT32 key, OUT void *value, IN IM_UINT32 size);

/* 
 * FUNC: venc encode.
 * PARAMS: venc, venc context.
 * 		VENC_IN *encin	pointer to the encode input structure
 * 		VENC_OUT *encout pointer to the encode output structure 
 * RETURN: 
 */
VENC_API IM_RET venc_encode(IN VENCCTX venc, IN VENC_IN *encin, OUT VENC_OUT *encout);


//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_VENCAPI_H__

