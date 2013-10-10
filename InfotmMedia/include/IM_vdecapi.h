/**
Desc:

Log:

**/


#ifndef __IM_VDECAPI_H__
#define __IM_VDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(_WINCE_)
#ifdef VDEC_EXPORTS
		#define VDEC_API		__declspec(dllexport)	/* For dll lib */
#else
		#define VDEC_API		__declspec(dllimport)	/* For application */
#endif
#else
		#define VDEC_API
#endif	

//#############################################################################
#include "IM_vdecstruct.h"

/*============================Interface API==================================*/
/* 
 * FUNC: get vdec version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
VDEC_API IM_UINT32 vc_version(OUT IM_TCHAR *ver_string);


/*
func: create Video Codec CTX.
params: 
	codec, [OUT], Save the instance of required Video Codec.
return:
	VC_OK, initialize successfuly. if fail, *codec will be set to NULL.
	VC_INIT_FAIL, 
*/
VDEC_API IM_RET vc_creat_ctx(OUT VCCTX *codec);


/*
func:Initialize Video Codec
params:A :init_package:
	MPEG1/MPEG2/ can be NULL
	MPEG4/DIVX/XVID/H263 can be NULL
	WMV4/WVC1 it must contains init_data which the first byte
			is metadata_size,and the follow segment is metadatabuffer
		example:u8 *init_buffer = (u8 *)init_package;
			metadataSize=init_buffer[0];
			metadataBuffer=init_buffer+1;
	RMVB/RM  it must contais init_data which the first byte is flag indicated the stream type RV30/RV40,and the follow segment is st_rv8_msgs struct 
			(
			typedef struct{
				unsigned int num_sizes;
				unsigned int size[16];	
			}st_rv8_msgs;
			you can get these rv8_msg from parser;
			)
		example:
			u8 *init_buffer=(u8 *)init_package;
			rv_version = init_buffer[0];//rv30(rv_version=3),RV40(rv_version=4)
			memcpy(rv8_msg_buffer,init_buffer+1,sizeof(st_rv8_msg));
     B :VC_EXTRA_DEC *extradecctx
     	if you want that the decode will always success if the video is not supoortted by our hw decode lib, you must pass a sw decode context in as a 
	backup. 

return:
 * */
VDEC_API IM_RET vc_init(IN VCCTX codec,void *init_package,IN VC_EXTRA_DEC *extradecctx);



/*
func:deinit Codec VCCTX
params:
return:
 */

VDEC_API IM_RET vc_deinit(IN VCCTX codec);
/*
func: Deinitialize Video Codec.
params:
	codec, [IN],
return:
	VC_OK,
	VC_NOT_INITIALIZED,
	VC_DEINIT_FAIL,
	...
*/
VDEC_API IM_RET vc_destroy_ctx(IN VCCTX codec);

/*
func: query this codec if support this input video stream.
params: 
	input_type, [IN], required input video stream type.
return:
	VC_OK,
	VC_UNSUPPORT_INPUT_TYPE,
	...
*/
VDEC_API IM_RET vc_check_input_type(IN VCCTX codec,IN VC_INPUT_TYPE *input_type);


/*
func: set the real dec in type
params: 
	input_type, [IN], required input video stream type.
return:
	VC_OK,
	VC_UNSUPPORT_INPUT_TYPE,
	...
*/
VDEC_API IM_RET vc_set_input_type(IN VCCTX codec,IN VC_INPUT_TYPE *input_type);



/*
func: query this codec if support this input video stream.
params: 
	output_type, [IN], required output video stream type.
	suggest_output[OUT] suggest output video type
return:
	VC_OK,
	VC_UNSUPPORT_OUTPUT_TYPE,
	...
*/
VDEC_API IM_RET vc_check_output_type(IN VCCTX codec,IN  VC_OUTPUT_TYPE *output_type,OUT VC_OUTPUT_TYPE *suggest_output);

/*
func: set the real out put type 
params: 
	output_type, [IN], required input video stream type.
return:
	VC_OK,
	VC_UNSUPPORT_OUTPUT_TYPE,
	...
*/
VDEC_API IM_RET vc_set_output_type(IN VCCTX codec,IN  VC_OUTPUT_TYPE *output_type);

/*
func: query this codec if support this input video stream to the output type.
params: 
	input_type, [IN], required input video stream type.
	output_type, [IN], required output video stream type.
return:
	VC_OK,
	VC_UNSUPPORT_TRANSFORM,
	...
*/
VDEC_API IM_RET vc_check_transform(IN VCCTX codec,IN VC_INPUT_TYPE *input_type, IN VC_OUTPUT_TYPE *output_type);

/*
func: inquire codec to get how much buffer will be cost in this video type.
params: 
	codec, [IN],
	buff_property, [OUT], save property.
return:
	AC_OK,
	AC_NOT_INITIALIZED,
*/
VDEC_API IM_RET vc_get_suggested_outbuffer(IN VCCTX codec, OUT BUFF_PROPERTY *buff_property);


/*
func: decode.
param:
	codec, [IN],
	dec_in, [IN], decode input parameters.
	dec_out, [OUT], decode output parameters. 
return:
*/
VDEC_API IM_RET vc_decode(IN VCCTX codec, IN VC_DEC_IN *dec_in, OUT  VC_DEC_OUT *dec_out);


/*
func: set output buffers when codec needs some extra buffer.
param:
	codec, [IN],
return:
*/
VDEC_API IM_RET vc_set_output_buffers(IN VCCTX codec, IN int numbers, IN  DEC_BUFF *buffers);


/*
func: Get decoded frame if has.
param:
	codec, [IN],
	dec_in, [IN], decode input parameters.
	dec_out, [OUT], decode output parameters. 	
return:
*/
VDEC_API IM_RET vc_get_decoded_frame(IN VCCTX codec, IN  VC_DEC_IN *dec_in, OUT  VC_DEC_OUT *dec_out);


/*
func: flush codec, then it can receive new segment and cannot be affect by previous decode.
	return the phyaddr got by decode lib when use multibuffer mode
params:
	codec, [IN],
return:
	VC_OK,
	VC_NOT_INITIALIZED,
	VC_FLUSH_FAIL,
*/
VDEC_API IM_RET vc_flush(IN VCCTX codec,OUT VC_FLUSH_CTX *ctx);



/*
func: 
params:
return:
*/
VDEC_API IM_RET vc_set_property(IN VCCTX codec, IN VC_PROPERTY_PACK *property);


/*
func: 
params:
return:
*/
VDEC_API IM_RET vc_get_property(IN VCCTX codec, OUT VC_PROPERTY_BAG *property);




//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	//_
