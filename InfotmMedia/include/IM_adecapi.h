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

#ifndef __IM_ADECAPI_H__
#define __IM_ADECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
	#ifdef ADEC_EXPORTS		/* DONOT! define it in application */
		#define ADEC_API	__declspec(dllexport)	/* For dll lib */
	#else
		#define ADEC_API	__declspec(dllimport)	/* For application */
	#endif
#else
	#define ADEC_API
#endif
//#############################################################################
//
//
//
typedef void * ADECCTX;

//
//
typedef struct{
	IM_INT32	type;	// see IM_STREAM_SUBTYPE
	
	IM_UINT32	channels;	// Number of channels (e.g. 2 for stereo).
	IM_UINT32	bitspersample;	// Bit per sample.
	IM_UINT32	samplerate;	// Sampling rate of the source data.  Use 0 for variable or unknown sampling rate.
	IM_INT32	bitrate;        // The average bitrate.
	IM_INT32 	blockalign;	// number of bytes per packet if constant and known or 0 Used by some WAV based audio codecs.
	IM_UCHAR  	*extradata;	// some codecs need, like ffmpeg.
	IM_INT32 	extradata_size;
	                       
	IM_UCHAR 	*reserved;	// maybe used in future.
	IM_INT32 	reserved_size;

	void 		*avctx;		//ffmpeg lib used
}ADEC_INPUT_TYPE;

//
// PCM format description
//
typedef struct{ 
	IM_UINT32	channels;		/**< Number of channels (e.g. 2 for stereo) */  
	IM_UINT32	bitspersample;		/**< Bit per sample */ 
	IM_UINT32	samplerate;		/**< Sampling rate of the source data.  Use 0 for variable or unknown sampling rate. */ 
	IM_BOOL		interleaved;		/**< True for normal interleaved data; false for non-interleaved data (e.g. block data) */ 
	IM_UCHAR	*reserved;		/** maybe used in future */
	IM_INT32	reserved_size;
}ADEC_OUTPUT_TYPE;

//
//
//
typedef struct{
	IM_UINT32	size;
	IM_UINT32	prefix;
	IM_UINT32	align;
}ADEC_BUFF_PROPERTY;

//
//
//
#define ADEC_IN_NONBLOCK	(1<<0)
typedef struct{
	IM_Buffer	input_buffer;
	IM_Buffer	output_buffer;
	//...

	IM_INT32	flag;	// ADEC_IN_XXX
}ADEC_IN;

//
//
//
#define ADEC_OUT_ONE_FRAME	(0<<0)
#define ADEC_OUT_NO_FRAME	(1<<0)
#define ADEC_OUT_MORE_FRAME	(2<<0)
typedef struct{
	IM_INT32 	output_len;
	IM_INT32 	bytes_consumed;

	IM_INT32 	flag;	// ADEC_OUT_XXX
}ADEC_OUT;

//
//
//
typedef enum{
	PROPERTY_NONE = 0,
	PROPERTY_CHANNELS
}ADEC_PROPERTY;

typedef struct{
	ADEC_PROPERTY	item;
	void		*pack;
	IM_INT32	pack_size;
}ADEC_PROPERTY_PACK;

/*============================Interface API==================================*/
/**
 * FUNC: get audio dec version.
 * PARAM: ver_string, save the version string.
 * RETURN: see IM_common defination about version.
 */
ADEC_API IM_UINT32 adec_version(OUT IM_TCHAR *ver_string);

/*
func: Initialize Audeo Codec.
params: 
	codec, [OUT], Save the instance of required Audio Codec.
	input_type, [IN], required input audio stream type.
	output_type, [IN], required output aduio type.
return:
	IM_RET_OK, initialize successfuly. if fail, *pCodec will be set to NULL.
	IM_RET_ADEC_INIT_FAIL, 
	IM_RET_ADEC_UNSUPPORT_INPUT_TYPE,
	IM_RET_ADEC_UNSUPPORT_OUTPUT_TYPE,
	...
*/
ADEC_API IM_RET adec_init(OUT ADECCTX *codec, IN ADEC_INPUT_TYPE *input_type, IN ADEC_OUTPUT_TYPE *output_type);


/*
func: query this codec if support this input audio stream.
params: 
	pInputType, [IN], required input audio stream type.
return:
	IM_RET_OK,
	IM_RET_ADEC_UNSUPPORT_INPUT_TYPE,
	...
*/
ADEC_API IM_RET adec_check_input_type(IN ADEC_INPUT_TYPE *input_type);


/*
func: Deinitialize Audio Codec.
params:
	codec, [IN],
return:
	IM_RET_OK,
	IM_RET_ADEC_NOT_INITIALIZED,
	IM_RET_ADEC_DEINIT_FAIL,
	...
*/
ADEC_API IM_RET adec_deinit(IN ADECCTX codec);


/*
func: decode.
param:
	codec, [IN],
	dec_in, [IN], decode input parameters.
	dec_out, [OUT], decode output parameters. 
return:
	
*/
ADEC_API IM_RET adec_decode(IN ADECCTX codec, IN ADEC_IN *dec_in, OUT ADEC_OUT *dec_out);


/*
func: Get decoded frame if has.
param:
	codec, [IN],
	pDecIn, [IN], decode input parameters.
	pDecOut, [OUT], decode output parameters. 	
return:
*/
ADEC_API IM_RET adec_get_decoded_frame(IN ADECCTX codec, IN ADEC_IN *dec_in, OUT ADEC_OUT *dec_out);


/*
func: flush codec, then it can receive new segment and cannot be affect by previous decode.
params:
	codec, [IN],
return:
	IM_RET_OK,
	IM_RET_ADEC_NOT_INITIALIZED,
	IM_RET_ADEC_FLUSH_FAIL,
*/
ADEC_API IM_RET adec_flush(IN ADECCTX codec);


/*
func: inquire codec to get how much buffer will be cost in this audio type.
params: 
	codec, [IN],
	buff_property, [OUT], save property.
return:
	AC_OK,
	AC_NOT_INITIALIZED,
*/
ADEC_API IM_RET adec_get_suggested_outbuffer(IN ADECCTX codec, OUT ADEC_BUFF_PROPERTY *buff_property);


/*
func: 
params:
return:
*/
ADEC_API IM_RET adec_set_property(IN ADECCTX codec, IN ADEC_PROPERTY_PACK *property);


/*
func: 
params:
return:
*/
ADEC_API IM_RET adec_get_property(IN ADECCTX codec, OUT ADEC_PROPERTY_PACK *property);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_ADECAPI_H__


