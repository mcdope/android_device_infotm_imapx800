/**
 * Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 * 
 * Description: Header file of  Image APIs
 *	
 * Author:
 *     shujin<jimmy.shu@infotmic.com.cn>
 *
 * Revision History: 
 * ----------------- 
 * v1.0.0	Jimmy@2011/05/21: first commit
 */


#ifndef __IM_IMAGEAPI_H__
#define __IM_IMAGEAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if (TARGET_SYSTEM == FS_WINCE)
#ifdef IMG_EXPORTS
	#define IMG_API		__declspec(dllexport)	/* For dll lib */
#else
	#define IMG_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define IMG_API
#endif

//#############################################################################
//
// face detect context type.
//
typedef void * IMGCTX;

#define IMG_MAX_FACE_DETECT_NUM 	20	// max detect face number

//
// property
//
#define IMG_FACEDET_PRO_R_SUPPORT_MAX_DETECT_NUM 	0x00001	// max detect face number supported: (equal to MAX_DETECT_NUM)
#define IMG_FACEDET_PRO_RW_MAX_DETECT_NUM 			0x00002	// max detect face number app need: IM_INT32.

//#############################################################################

/* Image info structure */

typedef struct{
	IM_UINT32		format;//IM_PIC_FMT_XXX	
	IM_UINT32		width;
	IM_UINT32		height;
	IM_Buffer		buffer[3];//buffer[0] for rgb or y, buffer[1/2] for uv of semiplaner/planer, 
}img_face_detect_src_t;

typedef struct{
	IM_UINT32	row_i;
	IM_UINT32	col_j;
	IM_UINT32	width;
	IM_UINT32	height; 
}img_face_detect_pos_t;

typedef struct {
	IM_INT32 num;	//real face number got
	img_face_detect_pos_t pos[IMG_MAX_FACE_DETECT_NUM];
}img_face_detect_des_t;


/*============================Interface API==================================*/
/* 
 * FUNC: get img version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
IMG_API IM_UINT32 img_version(OUT IM_TCHAR *ver_string);

/* 
 * FUNC: init image.
 * PARAMS: img, save instance.
 * RETURN: IM_RET_OK is successful, else failed.
 */
IMG_API IM_RET img_init(OUT IMGCTX *img);

/* 
 * FUNC: deinit image.
 * PARAMS: img, image instance created by img_init().
 * RETURN: IM_RET_OK is successful, else failed.
 */
IMG_API IM_RET img_deinit(IN IMGCTX img);

/*
 * FUNC: set property.
 * PARAMS: 
 *  img, image instance created by img_init().
 * 	pro, property id, see IMG_PRO_XXX.
 * 	value, property value set.
 *  size size of value buffer.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
IMG_API IM_RET img_set_property(IN IMGCTX img, IN IM_INT32 pro, IN void *value,IN IM_INT32 size);

/*
 * FUNC: get property.
 * PARAMS: 
 *  img, image instance created by img_init().
 * 	pro, property id, see IMG_PRO_XXX.
 * 	value, property value got.
 *  size size of value buffer.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
IMG_API IM_RET img_get_property(IN IMGCTX img, IN IM_INT32 pro, OUT void *value,IN IM_INT32 size);

/* 
 * FUNC: face detect exectue.
 * PARAMS: 
 *  img, image instance created by img_init().
 *  src, face parameter for detecting
 *  des, detected result 
 * RETURN: IM_RET_OK is successful, else failed.
 */
IMG_API IM_RET img_facedet_exec(IN IMGCTX img, IN img_face_detect_src_t *src, OUT img_face_detect_des_t *des);

//#############################################################################
#ifdef __cplusplus
}
#endif


#endif	// __IM_IMAGEAPI_H__

