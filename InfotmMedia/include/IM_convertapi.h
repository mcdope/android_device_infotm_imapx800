/**
 * DESC:
 *	
 *
 *
 *
 *
 *
 * LOG:
 *
 *
 */

#ifndef __IM_CONVERTAPI_H__
#define __IM_CONVERTAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef CONVERT_EXPORTS
	#define CONVERT_API		__declspec(dllexport)	/* For dll lib */
#else
	#define CONVERT_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define CONVERT_API
#endif	
//#############################################################################
// convert context type.
typedef void * CVTCTX;

//
//
//
#define CVT_IMAGE_ROTATION_90		(1<<0)
#define CVT_IMAGE_ROTATION_180		(1<<1)
#define CVT_IMAGE_ROTATION_270		(1<<2)
#define CVT_IMAGE_ROTATION_VFLIP	(1<<3)
#define CVT_IMAGE_ROTATION_HFLIP	(1<<4)
#define CVT_IMAGE_ROTATION_0 	    (1<<5)

typedef struct{
	IM_IMAGE_FORMAT srcImg;
	IM_Buffer srcBuf;
	IM_IMAGE_FORMAT dstImg;
	IM_Buffer dstBuf;
	IM_CSC_TYPE cscType;
	IM_CSC_PARAMS cscParams;
	IM_CROP_PARAMS cropParams;
	IM_INT32 flag;	// CVT_IMAGE_XXX
}CVT_IMAGE;

//
//
//
#define CVT_CHECK_FORMAT	(1<<0)
#define CVT_CHECK_SIZE		(1<<1)

typedef struct{
	IM_INT32 what;	// CVT_CHECK_XXX
	void *in;
	IM_INT32 insize;
	void *out;
	IM_INT32 outsize;
}CVT_CHECK;

//
//
//
typedef struct{
}CVT_PCM;

/*============================Interface API==================================*/
/* 
 * FUNC: get convert version.
 * PARAMS: ver_string, save this version string.
 * RETURN: [31:16] major version number, [15:0] minor version number.
 */
CONVERT_API IM_UINT32 cvt_version(OUT IM_TCHAR *ver_string);

/* 
 * FUNC: open converter.
 * PARAMS:
 * 	cvt, save convert context.
 * RETURN: IM_RET_OK is successful, else failed.
 */

CONVERT_API IM_RET cvt_open(OUT CVTCTX *cvt);

/* 
 * FUNC: close convert.
 * PARAMS: cvt, convert context created by cvt_open().
 * RETURN: IM_RET_OK is successful, else failed.
 */
CONVERT_API IM_RET cvt_close(IN CVTCTX cvt);

/**
 * FUNC: convert image from src to dst.
 * PARAMS: 
 * 	cvt, convert context.
 * 	cvtimg, configure of requested image convert.
 * RETURN: IM_RET_OK succeed, else failed.
 */
CONVERT_API IM_RET cvt_image(IN CVTCTX cvt, INOUT CVT_IMAGE *cvtimg);


/*
 * FUNC: check it's capabilities.
 * PARAMS:
 * 	cvt, convert context.
 * 	check, thing of check.
 * RETURN: IM_RET_OK support convert, else don't support.
 */
CONVERT_API IM_RET cvt_check(IN CVTCTX cvt, IN CVT_CHECK *check);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_CONVERTAPI_H__

