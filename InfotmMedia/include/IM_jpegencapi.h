/**
 **
 * Copyright (c) 2011~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 *
 * Revision History: 
 * ----------------- 
 * v1.0.0	leo@2011/04/23: first commit.
 * v1.1.0	rane@2012/04/19: a stable version.
 * v1.2.1	leo@2012/05/10: add jenc_get_property() to query capability 
 * 			or property of this encoder.
 */

#ifndef __IM_JPEGENCAPI_H__
#define __IM_JPEGENCAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef JPEGENC_EXPORTS
	#define JPEGENC_API		__declspec(dllexport)	/* For dll lib */
#else
	#define JPEGENC_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define JPEGENC_API
#endif	
//#############################################################################
// jpeg encoder context type.
typedef void * JENCCTX;

//
// property key defination.
//
#define JENC_PROPKEY_R_CAPS		(0x1000)	// value type: IM_UINT32 *, see JENC_CAPS_SUPPORT_xxx.
#define JENC_PROPKEY_R_CAP_ROTATION	(0x1001)	// value type: IM_UINT32 *, see JENC_CAP_ROTATION_xxx.
#define JENC_PROPKEY_R_CAP_INPUT_YUV	(0x1002)	// value type: IM_UINT32 *, see JENC_CAP_INPUT_YUVxxx.
#define JENC_PROPKEY_R_CAP_INPUT_RGB	(0x1003)	// value type: IM_UINT32 *, see JENC_CAP_INPUT_RGBxxx.
#define JENC_PROPKEY_R_CAP_SLICE_ROW	(0x1004)	// value type: IM_INT32 *, rows of slice.

// caps.
#define JENC_CAPS_SUPPORT_ROTATION	(1<<0)
#define JENC_CAPS_SUPPORT_INPUT_YUV	(1<<1)
#define JENC_CAPS_SUPPORT_INPUT_RGB	(1<<2)
#define JENC_CAPS_SUPPORT_SLICE		(1<<3)

// cap_rotation.
#define JENC_CAP_ROTATION_90		(1<<0)
#define JENC_CAP_ROTATION_180		(1<<1)
#define JENC_CAP_ROTATION_270		(1<<2)

// cap_input_yuv.
#define JENC_CAP_INPUT_YUV420P		(1<<0)
#define JENC_CAP_INPUT_YUV420SP		(1<<1)
#define JENC_CAP_INPUT_YUYV422i		(1<<2)
#define JENC_CAP_INPUT_UYVY422i		(1<<3)

// cap_input_rgb.
#define JENC_CAP_INPUT_RGB565		(1<<0)
#define JENC_CAP_INPUT_BGR565		(1<<1)
#define JENC_CAP_INPUT_RGB555		(1<<2)
#define JENC_CAP_INPUT_BGR555		(1<<3)
#define JENC_CAP_INPUT_RGB444		(1<<4)
#define JENC_CAP_INPUT_BGR444		(1<<5)
#define JENC_CAP_INPUT_RGB888		(1<<6)
#define JENC_CAP_INPUT_BGR888		(1<<7)
#define JENC_CAP_INPUT_RGB101010	(1<<8)
#define JENC_CAP_INPUT_BGR101010	(1<<9)


//
// rotation in clockwise
//
#define JENC_ROTATION_0		0
#define JENC_ROTATION_90	1
#define JENC_ROTATION_180	2
#define JENC_ROTATION_270	3
#define JENC_ROTATION_VFLIP	4
#define JENC_ROTATION_HFLIP	5

//
// encode mode
//
#define JENC_ENCMODE_WHOLEFRAME	0	// The whole frame is stored in linear memory
#define JENC_ENCMODE_SLICED	1	// The frame is sliced into restart intervals; Input address is given for each slice

//
// encode units type
//
#define JENC_UNITS_NO			0	// No units, X and Y specify the pixel aspect ratio
#define JENC_UNITS_DOTS_PER_INCH	1	// X and Y are dots per inch
#define JENC_UNITS_DOTS_PER_CM		2	// X and Y are dots per cm

//
// table marker type
//
#define JENC_SINGLE_MARKER	0	// Luma/Chroma tables are written behind own marker/components
#define JENC_MULTI_MARKER	1	// Luma/Chroma tables are written behind one marker/components

//
// jenc config.
//
//
// jpeg thumbnail format
//
//
#define JENC_THUMB_JPEG  0
#define JENC_THUMB_PALETTE_RGB8  1
#define JENC_THUMB_RGB24  2


typedef struct{
    IM_UINT32 format; //JENC_THUMB_XXX
    IM_UINT8 width; //Width in pixels of thumbnail
    IM_UINT8 height; //Height in pixels of thumbnail
    const void* data; //Thumbnail data
    IM_UINT16 dataLength; //Data amount in bytes
}JENC_THUMB;

typedef struct{
	IM_INT32 qLevel;	// Quantization level (0 - 100)
	IM_INT32 inputFormat;	// IM_IMAGE_XXX
	IM_INT32 rotation;	// JENC_ROTATION_XXX
	IM_INT32 unitsType;	// JENC_UNITS_XXX
	IM_INT32 markerType;	// JENC_XXX_MARKER
	IM_INT32 xDensity;	// Horizontal pixel density
	IM_INT32 yDensity;	// Vertical pixel density
	IM_INT32 comLength;	// Length of COM header
	IM_UINT8 *comment;	// Comment header pointer
    JENC_THUMB thumb;   // only for 8270 pp lib
}JENC_CONFIG;

//
// jenc encode
//
typedef struct{
	IM_INT32 inputWidth;	// Number of pixels/line in input image
	IM_INT32 inputHeight;	// Number of lines in input image
	IM_INT32 xOffset;	// Pixels from top-left corner of input image to top-left corner of encoded image
	IM_INT32 yOffset;
	IM_INT32 codingWidth;	// Width of encoded image
	IM_INT32 codingHeight;	// Height of encoded image
	IM_INT32 codingMode;	// JENC_ENCMODE_XXX
	IM_INT32 sliceLines;	// lines in slice mode, must be multiple of 16

	IM_Buffer buffY;	// y , ycbycr, rgb
	IM_Buffer buffCb;	// cb, cbcr
	IM_Buffer buffCr;	// cr
	IM_Buffer buffOut;	// output jpeg
	IM_INT32 jfifSize;	// OUT,Encoded JFIF size (bytes)
}JENC_ENCODE;

/*============================Interface API==================================*/
/* 
 * FUNC: get jpegenc version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
JPEGENC_API IM_UINT32 jenc_version(OUT IM_TCHAR *ver_string);

/* 
 * FUNC: init jenc.
 * PARAMS:
 * 	jenc, save jenc context.
 * RETURN: IM_RET_OK is successful, else failed.
 */

JPEGENC_API IM_RET jenc_init(OUT JENCCTX *jenc);

/* 
 * FUNC: deinit jenc.
 * PARAMS: jenc, jenc context.
 * RETURN: IM_RET_OK is successful, else failed.
 */
JPEGENC_API IM_RET jenc_deinit(IN JENCCTX jenc);

/* 
 * FUNC: configure jenc.
 * PARAMS: jenc, jenc context.
 * 	cfg, confige of jpeg encoding.
 * RETURN: IM_RET_OK is successful, else failed.
 */
JPEGENC_API IM_RET jenc_configure(IN JENCCTX jenc, IN JENC_CONFIG *cfg);

/* 
 * FUNC: jpeg encode.
 * PARAMS: jenc, jenc context.
 * 	jencparam, 
 * RETURN: IM_RET_OK is successful, IM_RET_JENC_SLICED_OK is slice encode ok, 
 * 	else failed.
 */
JPEGENC_API IM_RET jenc_encode(IN JENCCTX jenc, INOUT JENC_ENCODE *encparam);

/*
 * FUNC: get property.
 * PARAMS: jenc, jenc context.
 *	key, the property key.
 *	value, 
 *	size, size of the value buffer.
 * RETURN: IM_RET_OK is successful, IM_RET_NOTSUPPORT or IM_RET_FAILED.
 */
JPEGENC_API IM_RET jenc_get_property(IN JENCCTX jenc, IN IM_UINT32 key, OUT void *value, IN IM_INT32 size);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_JPEGENCAPI_H__

