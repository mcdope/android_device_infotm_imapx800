/***************************************************************************** 
 ** IMAPxCommonStruct.h
 ** 
 ** Copyright (c) 2010~2015 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** Use of Infotm's code is governed by terms and conditions 
 ** stated in the accompanying licensing statement. 
 ** 
 ** Description: HW DEC INTERFACE 
 **
 ** Author:
 **     Franklin   <Franklin.wang@infotmic.com>
 **      
 ** Revision History: 
 ** ----------------- 
 ** 1.0  04/6/2010 Franklin
 *****************************************************************************/ 
#ifndef __IM_VDECSTRUCT_H__
#define __IM_VDECSTRUCT_H__
//#define PICID_TIMESTAMP



#define VC_DEC_IN_NONBLOCK		(1<<0)
#define VC_DEC_IN_SKIP			(1<<1)
#define VC_DEC_IN_DIRECT_SHOW		(1<<2) 	//for thumbnail and so on

#define VC_DEC_OUT_ONE_FRAME		(1<<3)   //decode out is one frame 
#define VC_DEC_OUT_NO_FRAME		(1<<4)	 //decode success and no frame out 
#define VC_DEC_OUT_MORE_FRAME		(1<<5)	 //decode has a pic and maybe has another picture in dec lib
#define VC_DEC_OUT_INBUF_SKIP		(1<<6)	 //decode heady ready
#define VC_DEC_OUT_HDRS_RDY             (1<<7)   //pp multibuffer mode ,more buffer need to decode
#define VC_DEC_OUT_LACK_OUTBUFF		(1<<8)	 //decode success and save a not display order pic in dec lib
#define VC_DEC_OUT_HIDE_FRAME		(1<<9)

typedef void * VCCTX;

typedef enum {
	VC_EXTRA_DEC_TYPE_NONE = -1, // extra sw dec type is none 


	VC_EXTRA_DEC_TYPE_FFMPEG = 0 // use ffmepg sw dec 
	//VC_EXTRA_DEC_TYPE_XXX;

}VC_EXTRA_DEC_TYPE;

typedef struct{
	VC_EXTRA_DEC_TYPE dec_type;
	void *dec_context;                //sw decode context
	unsigned int dec_context_size;    // sw decode context size
}VC_EXTRA_DEC; 


typedef enum {
	VC_INTYPE_NONE = -1,
		
	VC_INTYPE_MPEG4 =0x0A,
	VC_INTYPE_XVID,
	VC_INTYPE_DIVX3,
	VC_INTYPE_DIVX4,
	VC_INTYPE_DIVX5,
	VC_INTYPE_DIVX6,
	VC_INTYPE_H263,
	VC_INTYPE_SORENSONSPARK,
	VC_INTYPE_MPEG1,
	VC_INTYPE_MPEG2,
	VC_INTYPE_WMV3,
	VC_INTYPE_WVC1,
	VC_INTYPE_VP6,
	VC_INTYPE_VP6F,
	VC_INTYPE_RV30,
	VC_INTYPE_RV40,
	VC_INTYPE_H264 ,
	VC_INTYPE_MJPEG,
	VC_INTYPE_PP,


	//ohter decode lib 
	VC_INTYPE_FFMPEG,
	
	/*VC_INTYPE_XXX*/


}IN_TYPE;

typedef enum{
	VC_OUTTYPE_NONE = -1, //error Format

	VC_OUTTYPE_YCBCR_4_0_0=0x14,

	VC_OUTTYPE_YCBCR_4_2_2_INTERLEAVED,
	VC_OUTTYPE_YCRYCB_4_2_2_INTERLEAVED,
	VC_OUTTYPE_CBYCRY_4_2_2_INTERLEAVED,
	VC_OUTTYPE_CRYCBY_4_2_2_INTERLEAVED,
	VC_OUTTYPE_YCBCR_4_2_2_SEMIPLANAR,

	VC_OUTTYPE_YCBCR_4_4_0,

	VC_OUTTYPE_YCBCR_4_2_0_PLANAR,
	VC_OUTTYPE_YCBCR_4_2_0_SEMIPLANAR,
	VC_OUTTYPE_YCBCR_4_2_0_TILED,

	VC_OUTTYPE_YCBCR_4_1_1_SEMIPLANAR,
	VC_OUTTYPE_YCBCR_4_4_4_SEMIPLANAR,

	VC_OUTTYPE_RGB16_CUSTOM,
	VC_OUTTYPE_RGB16_5_5_5,
	VC_OUTTYPE_RGB16_5_6_5,
	VC_OUTTYPE_BGR16_5_5_5,
	VC_OUTTYPE_BGR16_5_6_5,

	VC_OUTTYPE_RGB32_CUSTOM,
	VC_OUTTYPE_RGB32,
	VC_OUTTYPE_BGR32
}OUT_TYPE;

typedef  enum {
	ROTATION_ERROR=0,
	ROTATION_NONE=0x100,                  
	ROTATION_RIGHT_90 ,  
	ROTATION_LEFT_90 ,                       
	ROTATION_HOR_FLIP,                      
	ROTATION_VER_FLIP ,                     
	ROTATION_180 ,                       
}VC_PP_ROTATION;

typedef struct{
	IN_TYPE	InType;
	unsigned int width;
	unsigned int height;
}VC_INPUT_TYPE;


typedef struct{ 
	OUT_TYPE	OutType;
	unsigned int width;   
	unsigned int height;   
	VC_PP_ROTATION   rotation; 
}VC_OUTPUT_TYPE;


typedef struct{
	IM_BOOL CanbeCombined;
	IM_BOOL CanbeStandAlone;
	IM_BOOL CanUseMultibuffer;
}VC_DEC_CAP;

typedef struct{
	void *virtual_addr;
	unsigned int bus_addr;
	unsigned int size;
}DEC_BUFF;


typedef struct{
	unsigned int	size;
	unsigned int 	prefix;
	unsigned int 	align;

	unsigned int    width;
	unsigned int    height;
	OUT_TYPE        type;
}BUFF_PROPERTY;



typedef struct{
	DEC_BUFF 	input_buffer;
	DEC_BUFF	output_buffer;
#ifdef PICID_TIMESTAMP
	int in_picid;
#endif
	int flag;	// VC_DEC_IN_XXX
}VC_DEC_IN;

typedef struct{
	DEC_BUFF OutUnitbuf;	  
	long DecodeOutSize;     //indicate that outframeSize 

	unsigned int InbufLeft ; // indicate that how much buffer left after decoder    
} VC_DEC_BUFFER_INFO;

typedef struct{
	int 	     enable;//indicate the new segment info is valid or not

	unsigned int width_new;
	unsigned int height_new;
	unsigned int interlaced_new;
	unsigned int multiBuffPpSize_new;
	//xxx
}VC_DEC_NEW_SEGMENT_INFO;
typedef struct{
	VC_DEC_NEW_SEGMENT_INFO  pNewSegmentInfo;//support varible size in a stream.Make api know the revolution changed 
	VC_DEC_BUFFER_INFO pDecBufferInfo ;
	int lack_buffs;
#ifdef PICID_TIMESTAMP
	int out_picid;
#endif
	int flag;	// VC_DEC_OUT_XXX
	int cut_down_num; // notify render how much line should be cut down
    int cut_right_num;//notify render how much line should be discard at the right edge
	int isinterlaced;
	OUT_TYPE outFormat;//really output format
	int skipNum; //the invalid picture stored in decode lib which has been flushed out
}VC_DEC_OUT;


typedef enum{
	PROPERTY_NONE = 0,
	PROPERTY_WITHOUT_PP,
	PROPERTY_WITH_PP,
	PROPERTY_OUTFRAME_DECODE_ORDER,
	PROPERTY_OUTFRAME_DISPLAY_ORDER,
	PROPERTY_WITHOUT_MULTI_BUFFER,
	PROPERTY_WITH_MULTI_BUFFER,
	// ...
}VC_PROPERTY;

typedef struct{
	VC_PROPERTY		item;


	void *			pack;
	int			pack_size;
}VC_PROPERTY_PACK;



typedef struct{
	IM_BOOL 			withpp;//true:with false:without
	IM_BOOL			withmultibuffer;// true:with false:without
	unsigned int 		reorder; //1: display order 0:decode order
}VC_PROPERTY_BAG;

typedef struct{
	int flushlib;	//if 1,we should flush dec9170 lib forcely,0,just get buffer address 
	unsigned int *phyaddr;//phy addr 
	int 	      num;    //hold phyaddr num
	int			  skipNum;
}VC_FLUSH_CTX;


typedef enum{
	VC_OK = IM_RET_VDEC_OK,
	VC_DECODING = IM_RET_VDEC_DECODING,
	VC_PROCESSED = IM_RET_VDEC_PROCESSED,
	VC_LACK_OUTBUFF =IM_RET_VDEC_LACK_OUTBUFF,
	VC_SKIP_FRAME = IM_RET_VDEC_SKIP_FRAME,
	VC_OUTPUT_TYPE_CHANGED = IM_RET_VDEC_OUTPUT_TYPE_CHANGED,
	//cmd set/get property not execute for some reasons
	VC_PROPERTY_CMD_NOT_EXECUTE = IM_RET_VDEC_PROPERTY_CMD_NOT_EXECUTE, 
	//indicate api that we will not use hw decode lib but use some other decode ,api must pass the neccesary param in 
	VC_CHECK_INFOTM_EXTRA_DECODE = IM_RET_VDEC_CHECK_INFOTM_EXTRA_DECODE, 
				     // Init Function
	VC_OUT_BUFFER_NEED_RECONFIG = IM_RET_VDEC_OUT_BUFFER_NEED_RECONFIG, //out buffer is smaller than need,api should check 
	VC_NEW_SEGMENT_READY =IM_RET_VDEC_NEW_SEGMENT_READY,        //varible size stream processed

	VC_FAIL = IM_RET_VDEC_FAIL,
	VC_INIT_FAIL = IM_RET_VDEC_INIT_FAIL,
	VC_DEINIT_FAIL = IM_RET_VDEC_DEINIT_FAIL,
	VC_DEC_FAIL = IM_RET_VDEC_DEC_FAIL,
	VC_FLUSH_FAIL = IM_RET_VDEC_FLUSH_FAIL,
	VC_GET_HWDEC_RESOURCE_FAIL =IM_RET_VDEC_GET_HWDEC_RESOURCE_FAIL,
	VC_RELEASE_HWDEC_RESOURCE_FAIL =IM_RET_VDEC_RELEASE_HWDEC_RESOURCE_FAIL,
	VC_INIT_HWDEC_FAIL =IM_RET_VDEC_INIT_HWDEC_FAIL,
	VC_DEINIT_HWDEC_FAIL = IM_RET_VDEC_DEINIT_HWDEC_FAIL,
	VC_FAIL_FOREVER = IM_RET_VDEC_FAIL_FOREVER,//get a serious error ,api don not need to try next frame
	VC_HW_TIME_OUT= IM_RET_VDEC_HW_TIME_OUT, // hw  time out,do not send this error to api
	
	
	VC_NOT_INITIALIZED = IM_RET_VDEC_NOT_INITIALIZED,
	VC_INVALID_PARAMETERS = IM_RET_VDEC_INVALID_PARAMETERS,
	VC_MEM_FAIL = IM_RET_VDEC_MEM_FAIL,
	VC_RESOURCE_NOT_INIT=IM_RET_VDEC_RESOURCE_NOT_INIT,

	VC_UNSUPPORT_INPUT_TYPE = IM_RET_VDEC_UNSUPPORT_INPUT_TYPE,
	VC_UNSUPPORT_OUTPUT_TYPE = IM_RET_VDEC_UNSUPPORT_OUTPUT_TYPE,
	VC_UNSUPPORT_OUTPUT_SIZE = IM_RET_VDEC_UNSUPPORT_OUTPUT_SIZE,
	VC_UNSUPPORT_TRANSFORM = IM_RET_VDEC_UNSUPPORT_TRANSFORM
}VC_RET;
#define VC_FAILED(ret)	(((ret)<0)?IM_TRUE:IM_FALSE)

#endif
































