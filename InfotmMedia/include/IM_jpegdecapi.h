/******************************************************************************
**
** Copyright (c) 2012~2112 ShangHai Inftom Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
**
** Revision History:
** -----------------
** v1.0.1   Karst@2012/04/25: first commit
** 
** v1.0.2   Karst@2012/04/26: add g1 support
**
** v1.0.3	Karst@2012/05/03: add MMU
**
** v1.0.4	Karst@2012/05/09: add MAP
**
** v1.0.5	Karst@2012/05/14: delete crop operation after decode
**
******************************************************************************/


#ifndef __IM_JPEGDECAPI_H__
#define __IM_JPEGDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef JPEGDEC_EXPORTS
	#define JPEGDEC_API		__declspec(dllexport)	/* For dll lib */
#else
	#define JPEGDEC_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define JPEGDEC_API
#endif	
//#############################################################################
//
// jdec context.
//
typedef void * JDECCTX;

#define JDEC_CODINGTYPE_BASELINE	0
#define JDEC_CODINGTYPE_PROGRESSIVE	1
#define JDEC_CODINGTYPE_NONINTERLEAVED	2


#define JDEC_UNITS_NO_UNITS		0	// No units, X and Y specify the pixel aspect ratio.
#define JDEC_UNITS_DOTS_PER_INCH	1	// X and Y are dots per inch.
#define JDEC_UNITS_DOTS_PER_CM 		2 	// X and Y are dots per cm.

#define JDEC_THUMBNAIL_NO		0
#define JDEC_THUMBNAIL_JPEG		1

#define JDEC_ROTATION_NONE	0
#define JDEC_ROTATION_90	1
#define JDEC_ROTATION_180	2
#define JDEC_ROTATION_270	3
#define JDEC_ROTATION_HOR_FLIP 4
#define JDEC_ROTATION_VER_FLIP 5

typedef struct{
	IM_IMAGE_FORMAT image;
	IM_UINT32	units;		// JDEC_UNITS_xxx.
	IM_UINT32	xDensity;
	IM_UINT32	yDensity;
	IM_UINT32	codingMode;	// JDEC_CODINGTYPE_xxx.
	IM_UINT32	thumbnailType;	// JDEC_THUMBNAIL_xxx.
	IM_IMAGE_FORMAT thumbnailImage;
	IM_UINT32	thumbnailCodingMode;	// JDEC_CODINGTYPE_xxx.
}JDEC_IMAGE_INFO;

#define JDEC_DECFLAG_THUMBNAIL		(1<<0)
typedef struct{
	IM_Buffer	buffer;
	IM_UINT32	dataSize;
	IM_UINT32	decFlag;	// JDEC_DECFLAG_xxx.
	//IM_UINT32	sliceMbSet;	// slice mode: mcu rows to decode.
	IM_UINT32	outputFormat;	//IM_PIC_FMT_xxx
	IM_UINT32	outputImageWidth;	//must be mutiple of 8
	IM_UINT32	outputImageHeight;	//must be mutiple of 2
	IM_BOOL		ditheringEnable;	//true means enable
	IM_UINT32	rotation;	//JDEC_ROTATION_xxx
}JDEC_IN;

typedef struct{
	IM_Buffer	buffer;
	IM_Buffer	bufferChroma;
}JDEC_OUT;


/*============================Interface API==================================*/
/* 
 * FUNC: get jdec version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
JPEGDEC_API IM_UINT32 jdec_version(OUT IM_TCHAR *ver_string);

/* 
 * FUNC: init jdec.
 * PARAMS: jdec, save jdec context.
 * RETURN: IM_RET_OK is successful, else failed.
 */

JPEGDEC_API IM_RET jdec_init(OUT JDECCTX *jdec, IM_BOOL MMU);

/* 
 * FUNC: deinit jdec.
 * PARAMS: jdec, jdec context.
 *	MMU, use MMU or not.
 * RETURN: IM_RET_OK is successful, else failed.
 */
JPEGDEC_API IM_RET jdec_deinit(IN JDECCTX jdec);

/* 
 * FUNC: get jpeg's image info.
 * PARAMS: jdec, jdec context.
 * 	jin, jpeg input stream.
 * 	imgInfo, image infomation.
 * RETURN: IM_RET_OK is successful, else failed.
 */
JPEGDEC_API IM_RET jdec_get_image_info(IN JDECCTX jdec, IN JDEC_IN *jin, OUT JDEC_IMAGE_INFO *imgInfo);

/* 
 * FUNC: jpeg decode.
 * PARAMS: jdec, jdec context.
 * 	jin, jpeg input stream.
 * 	jout, jpeg decoded output.
 * RETURN: IM_RET_OK is successful, else failed.
 */
JPEGDEC_API IM_RET jdec_decode(IN JDECCTX jdec, IN JDEC_IN *jin, OUT JDEC_OUT *jout);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_JPEGDECAPI_H__

