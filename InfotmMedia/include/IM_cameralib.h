/**
 *	
 * Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 * 
 * Description: Header file of Cameralib APIs
 *	
 * Author:
 *     leo zhang<@leo.zhang@infotmic.com.cn>
 *
 * Revision History: 
 * ----------------- 
 * v1.0.1	 leo@2011/4/23 :  first commit
 * v1.0.2	 leo@2011/4/28 : change some interface
 * v1.0.3	 leo@2011/5/30 : add interface for set source
 * v1.1.0	 rane@2012/03/21 : a stable version
 * v2.0.1	 leo@2012/04/13: new interface, it's not compatible with previous version.
 *
 */

#ifndef __IM_CAMERALIB_H__
#define __IM_CAMERALIB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if (TARGET_SYSTEM == FS_WINCE)
#ifdef CAMERALIB_EXPORTS
	#define CAMERALIB_API		__declspec(dllexport)	/* For dll lib */
#else
	#define CAMERALIB_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define CAMERALIB_API
#endif	
//#############################################################################

class IM_CameraLib{
public:
	virtual ~IM_CameraLib(){};
	virtual IM_UINT32 camlib_version(OUT IM_TCHAR *ver_string) = 0;
	virtual IM_RET camlib_get_modules(OUT cam_module_class_t *mcls) = 0;
	virtual IM_RET camlib_open(IN IM_INT32 boxIndex, IN IM_INT32 moduleIndex) = 0;
	virtual IM_RET camlib_close() = 0;
	virtual IM_RET camlib_start() = 0;
	virtual IM_RET camlib_stop() = 0;
	virtual IM_RET camlib_get_preview_configs(OUT cam_preview_config_t *cfg) = 0;
	virtual IM_RET camlib_set_preview_config(IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps) = 0;
	virtual IM_RET camlib_get_codec_configs(OUT cam_codec_config_t *cfg) = 0;
	virtual IM_RET camlib_set_codec_config(IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps) = 0;
	virtual IM_RET camlib_get_picture_configs(OUT cam_picture_config_t *cfg) = 0;
	virtual IM_RET camlib_set_picture_config(IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps) = 0;
	virtual IM_RET camlib_set_property(IN IM_INT32 key, IN void *value, IN IM_INT32 size) = 0;
	virtual IM_RET camlib_get_property(IN IM_INT32 key, OUT void *value, IN IM_INT32 size) = 0;
	virtual IM_RET camlib_assign_buffer(IN IM_INT32 path, IN IM_Buffer *buff) = 0;
	virtual IM_RET camlib_release_buffer(IN IM_INT32 path, IN IM_Buffer *buff) = 0;
	virtual IM_RET camlib_enable_path(IN IM_INT32 path) = 0;
	virtual IM_RET camlib_disable_path(IN IM_INT32 path) = 0;
	virtual IM_RET camlib_prepare_take_picture(IN IM_INT32 buffNum, IN IM_Buffer *buffers) = 0;
	virtual IM_RET camlib_cancel_take_picture() = 0;
	virtual IM_RET camlib_take_picture(OUT cam_frame_t *pic) = 0;
	virtual IM_RET camlib_release_picture(IN IM_Buffer *buffer) = 0;
	virtual IM_RET camlib_wait_frame_ready(OUT IM_INT32 *path, IN IM_INT32 timeout=-1) = 0;
	virtual IM_RET camlib_get_frame(IN IM_INT32 path, OUT cam_frame_t *frame) = 0;
	virtual IM_RET camlib_send_control(IN cam_control_t *ctrl) = 0;
	virtual IM_RET camlib_cancel_control(IN IM_INT32 code, IN IM_INT32 id) = 0;
	virtual IM_RET camlib_wait_control(OUT cam_control_t *ctrl, OUT IM_RET *status, IN IM_INT32 timeout=-1) = 0;
};

typedef IM_RET (*func_camlib_create_t)(OUT IM_CameraLib **camlib);
typedef IM_RET (*func_camlib_destroy_t)(IN IM_CameraLib *camlib);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_CAMERALIB_H__

