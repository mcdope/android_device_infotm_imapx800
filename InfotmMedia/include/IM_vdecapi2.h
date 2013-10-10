#ifndef _IM_VDEC_
#define _IM_VDEC_
#include "IM_vdeccommon.h"
#ifdef __cplusplus
extern "C"
{
#endif 

#if TARGET_SYSTEM == FS_WINCE
	#ifdef VDEC_EXPORTS		/* DONOT! define it in application */
		#define VDEC_API	__declspec(dllexport)	/* For dll lib */
	#else
		#define VDEC_API	__declspec(dllimport)	/* For application */
	#endif
#else
	#define VDEC_API
#endif

//===================vdec api============================================

/**
 * FUNC: get audio dec version.
 * PARAM: ver_string, save the version string.
 * RETURN: see IM_common defination about version.
 */
VDEC_API IM_UINT32 vdec_version(OUT IM_TCHAR *ver_string);

/**
 * FUNC: create the decoder by intype 
 * PARAM: 
    	vdecCtx INOUT :decoder handle;
		inType  IN    :input type
		flag    IN    : choose proper decoder.
 * RETURN: 
 	IM_RET_VDEC_OK;
	IM_RET_VDEC_FAIL;
 */
VDEC_API IM_RET vdec_create_ctx(INOUT IM_VDEC_CTX *vdecCtx, IN IM_VDEC_INTYPE *inType, IN IM_VDEC_CREATION_FLAG flag);


/**
 * FUNC: get decoder config info 
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecConfig OUT : decoder config info
 * RETURN: 
 	IM_RET_VDEC_OK;
	IM_RET_VDEC_FAIL;
 */
VDEC_API IM_RET vdec_get_config(IN IM_VDEC_CTX vdecCtx, OUT IM_VDEC_CONFIG *vdecConfig);

/**
 * FUNC: set decoder config info 
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecConfig IN : decoder config info
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_UNSUPPORT_INTYPE
	 IM_RET_VDEC_UNSUPPORT_MODE
	 IM_RET_VDEC_UNSUPPORT_OUTTYPE
 */
VDEC_API IM_RET vdec_set_config(IN IM_VDEC_CTX vdecCtx,	IN IM_VDEC_CONFIG *vdecConfig);

/**
 * FUNC: init decoder  
 * PARAM: 
    	vdecCtx IN :decoder handle;
		specData IN : init data needed by decoder
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_INIT_FAIL
*/
VDEC_API IM_RET vdec_init(IN IM_VDEC_CTX vdecCtx, IN IM_VDEC_SPEC_DATA *specData);

/**
 * FUNC: decode input stream and out picture 
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecIn  IN :input stream info
		vdecOut OUT :decode result info
		vdecPicture INOUT: in pic buff info, out pic info,
 * RETURN: 
	 IM_RET_VDEC_HDRS_RDY
	 IM_RET_VDEC_DECINFO_CHANGED
	 IM_RET_VDEC_LACK_OUTBUFF
	 IM_RET_VDEC_MORE_PIC_OUT
	 IM_RET_VDEC_NO_PIC_OUT
	 IM_RET_VDEC_PIC_SKIPPED
	 IM_RET_VDEC_MEM_FAIL
	 IM_RET_VDEC_UNSUPPORT_STREAM
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_decode(IN IM_VDEC_CTX vdecCtx, IN IM_VDEC_IN *vdecIn, OUT IM_VDEC_OUT *vdecOut, INOUT IM_VDEC_PICTURE *vdecPicture);

/**
 * FUNC: get real stream info decoded by decoder 
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecInfo OUT: stream info,
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_get_info(IM_VDEC_CTX vdecCtx, OUT IM_VDEC_INFO *vdecInfo);

/**
 * FUNC: set multibuffers needed by multibuffer mode
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecBuffers IN: 
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_set_multibuffer(IN IM_VDEC_CTX vdecCtx, IN IM_VDEC_BUFFERS *vdecBuffers);

/**
 * FUNC: get out picture 
 * PARAM: 
    	vdecCtx IN :decoder handle;
		vdecPicture INOUT: in pic buff info, out picture,
 * RETURN: 
	 IM_RET_VDEC_MORE_PIC_OUT
	 IM_RET_VDEC_NO_PIC_OUT
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_get_next_picture(IN IM_VDEC_CTX vdecCtx, INOUT IM_VDEC_PICTURE *vdecPicture);

/**
 * FUNC: flush decoder  
 * PARAM: 
    	vdecCtx IN :decoder handle;
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_flush(IN IM_VDEC_CTX vdecCtx);

/**
 * FUNC: deinit decoder  
 * PARAM: 
    	vdecCtx IN :decoder handle;
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_deinit(IN IM_VDEC_CTX vdecCtx);

/**
 * FUNC: destroy decoder  
 * PARAM: 
    	vdecCtx IN :decoder handle;
 * RETURN: 
	 IM_RET_VDEC_OK
	 IM_RET_VDEC_FAIL
*/
VDEC_API IM_RET vdec_destroy(IN IM_VDEC_CTX vdecCtx);

#ifdef __cplusplus
}
#endif
#endif
