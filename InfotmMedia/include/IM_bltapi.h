/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Revision History: 
** ----------------- 
** 1.0.1	leo@2012/05/04: first commit.
**
*****************************************************************************/
 
#ifndef __IM_BLTAPI_H__
#define __IM_BLTAPI_H__


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef BLT_EXPORTS
	#define BLT_API		__declspec(dllexport)	/* For dll lib */
#else
	#define BLT_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define BLT_API
#endif	
//#############################################################################
//
// blt handle(instance).
//
typedef void *	BLTINST;	// the handle of the blt.


//
// picture-block rectangle infomation.
//
typedef struct{
	IM_UINT32	pixfmt;	// IM_PIC_FMT_xxx, see IM_picformat.h.

	//orginal width&height of source whole image.
	IM_INT32	imgWidth;	// pixel unit.
	IM_INT32	imgHeight;	// pixel unit.

	// valid rectangle in this whole picture (crop).
	IM_INT32	width;	// pixel unit.
	IM_INT32	height;	// pixel unit.
	IM_INT32	xOffset; // pixel unit.
	IM_INT32	yOffset; // pixel unit.

	// memory property.
	IM_INT32	strideY; // byte unit, y/yuv/rgb.
	IM_INT32	strideU; // byte unit, u/uv.
	IM_INT32	strideV; // byte unit, v.

	IM_Buffer	bufferY;
	IM_Buffer	bufferU;
	IM_Buffer	bufferV;
}blt_rect_t;

//
// rotation flags(clockwise).
//
#define BLT_ROTATE_0		0x0
#define BLT_ROTATE_90		0x1
#define BLT_ROTATE_180		0x2
#define BLT_ROTATE_270		0x3
#define BLT_ROTATE_FLIP_HOR		0x4
#define BLT_ROTATE_FLIP_VER		0x5

typedef struct{
	IM_UINT32	rotation;	// BLT_ROTATE_xxx.
}blt_rotate_t;

//
// blending.
//
#define BLT_BLEND_MODE_PLANE	0x0
#define BLT_BLEND_MODE_PIXEL	0x1

typedef struct{
	IM_BOOL		enable;
	IM_UINT32	mode;	// BLT_BLEND_MODE_xxx.

	// only valid when mode==BLT_BLEND_MODE_PLANE.
	IM_FLOAT	srcAlpha; // range [1, 0].
	IM_FLOAT	dstAlpha; // range [1, 0].
}blt_blend_t;

//
typedef struct{
	blt_rect_t	dstRect;
	blt_rect_t	srcRect;
	blt_rotate_t	rotate;
	blt_blend_t	blend;
}blt_transfer_t;

//
// APIs.
//
BLT_API IM_UINT32 blt_version(OUT IM_TCHAR *verString);

BLT_API IM_RET blt_open(OUT BLTINST *blt);
BLT_API IM_RET blt_close(IN BLTINST blt);


#define BLT_FUNCMODULE_HW_PP	(0x10001)
#define	BLT_FUNCMODULE_HW_G2D	(0x10002)
#define BLT_FUNCMODULE_SW		(0x10003)

#define BLT_PROPKEY_SET_FUNCMODULE			(0x1) // value type: IM_INT32 *, BLT_FUNCMODULE_XX
#define BLT_PROPKEY_SET_EFFECT_CONTRAST		(0x2) // value type: IM_INT32 *, range [-64, 64], just available for pp
#define BLT_PROPKEY_SET_EFFECT_BRIGHTNESS	(0x3) // value type: IM_INT32 *, range [-128, 127], just available for pp
#define BLT_PROPKEY_SET_EFFECT_SATURATION   (0x4) // value type: IM_INT32 *, range [-64, 128], just available for pp
#define BLT_PROPKEY_SET_EFFECT_EDGESHARP	(0x5) // value type: IM_INT32 *, range [0, 255], just available for g2d
/**
 * function: blt_get_property and blt_set_property
 *		get and set the parameters
 * params:
 *		blt, the instance
 *		key, the specific property
 *		value, the value of the property
 * return value:
 *		IM_RET_OK, if successful
 *		IM_RET_NOTSUPPORT, if failed
 **/
BLT_API IM_RET blt_get_property(IN BLTINST blt, IN IM_UINT32 key, OUT void *value);
BLT_API IM_RET blt_set_property(IN BLTINST blt, IN IM_UINT32 key, IN void *value);

/**
 * function: blt transfer, include scale, color space convert, rotate, alpha-blending, raster.
 * 	if dst valid region size different with src's region, it will do scale.
 * 	if dst pixfmt different with src, it will do color space convert.
 * params:
 * 	blt, the instance.
 * 	trans
 * 		dstRect
 * 			pixfmt, pixel format, see IM_picformat.h.
 * 			width, valid region width.
 * 			height, valid region height.
 * 			xOffset, valid region x offset with whole image.
 * 			yOffset, valid region y offset with whole image.
 * 			strideY, used for RGB, y of yuv-planar, yuv interleaved.
 * 			strideU, used for uv of yuv-semiplanar, u of yuv-planar.
 * 			strideV, used for v of yuv-planar.
 * 			bufferY, used for RGB, y of yuv-planar, yuv interleaved.
 * 				vir_addr, virtual address.
 * 				phy_addr, physical address if flag has IM_BUFFER_FLAG_PHY, devaddr if flag has IM_BUFFER_FLAG_DEVADDR.
 * 				size, buffer size.
 * 				flag, buffer property.
 * 			bufferU, used for uv of yuv-semiplanar, u of yuv-planar.
 * 			bufferV, used for v of yuv-planar.
 * 		srcRect, source block rectangle, same as dstRect.
 * 		rotate
 * 			enable, if or not enable rotate.
 * 			angle, rotate angle.
 * 		blend
 * 			enable, if or not enable alpha-blending.
 * 			mode, blend mode, based pixel or plane.
 * 			srcAlpha, in plane-based mode, source alpha value, range is 0 to 1. 
 * 			dstAlpha, in plane-based mode, destination alpha value, range is 0 to 1. 
 *		raster
 *			enable, if or not enable raster operation.
 * return: IM_RET_OK is succeed, else failed.
 */
BLT_API IM_RET blt_transfer(IN BLTINST blt, IN blt_transfer_t *trans);

/**
 * function: bitblt memcopy.
 * params:
 * 	blt, the instance.
 * 	dst, dst buffer, 
 * 		vir_addr, virtual address.
 * 		phy_addr, physical address if flag has IM_BUFFER_FLAG_PHY, devaddr if flag has IM_BUFFER_FLAG_DEVADDR.
 * 		size, buffer size.
 * 		flag, buffer property.
 * 	src, same as dst buffer.
 * 	size, size need copy.
 * return: IM_RET_OK is succeed, else failed.
 */
BLT_API IM_RET blt_copy(IN BLTINST blt, IN IM_Buffer *dst, IN IM_Buffer *src, IN IM_INT32 size);


//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_BLTAPI_H__


