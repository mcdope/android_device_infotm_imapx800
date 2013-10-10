/* ** ********************************************************
 * **	hardware/infotm/imap210/libcamera/CameraHal.cpp
 * **	
 * **   Description:
 * **       infoTM android Camera Hardware absract layer code
 * **		it's avilable for Camera interface driver only!
 * **	Author:
 * **		Rane  <rane.qiu@infotmic.com.cn>	
 * **	Reversion:
 * **		Rane 1.0  
 *		rane @2012.07.19 2.0 
 *		supprt delive a encode struct to encoder instead of memcpy
 * **
 * ***********************************************************/
#include "CameraHal.h"
#include "Encoder_libjpeg.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define	LOG_TAG "CAMHAL"

//++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@++
#define USE_BLT

//#define CHECK_TTIME_DELAY

//deliver a frame address struct to encoder
#define ENCODE_STRUCT

#if 0
#define LOCK_IN()	LOGE("LOCK+:%d", __LINE__)
#define LOCK_OUT()	LOGE("LOCK-:%d", __LINE__)
#else
#define LOCK_IN()
#define LOCK_OUT()
#endif

//--@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@--
namespace android {

/*
 * for some effet
 * white balance
 * special effect
 * etc
 * */

struct timeval tv1, tv2, tv3;

static Mutex				gPicThrdLock;
static bool					gPicThrdRuning;

struct str_map {            
	const char *const desc; 
	int val;                
};
static const struct str_map wb_map[] = {
	{"auto", CAM_WB_MODE_AUTO},
	{"incandescent", CAM_WB_MODE_INCANDESCENT},
	{"fluorescent", CAM_WB_MODE_FLUORESCENT},
	{"daylight", CAM_WB_MODE_DAYLIGHT},
	{"warm-fluorescent",CAM_WB_MODE_WARM_FLUORECENT},
	{"cloudy-daylight", CAM_WB_MODE_CLOUDY_DAYLIGHT},
	{"twilight",CAM_WB_MODE_TWILIGHT},
	{"shade", CAM_WB_MODE_SHADE},
	{NULL, 0}
};

static const struct str_map se_map[] = {
	{ "none", CAM_SPECIAL_EFFECT_NONE},
	{ "mono", CAM_SPECIAL_EFFECT_MONO },
	{ "negative", CAM_SPECIAL_EFFECT_NEGATIVE },
	{ "solarize", CAM_SPECIAL_EFFECT_SOLARIZE },
	{ "sepia", CAM_SPECIAL_EFFECT_SEPIA },
	{ "posterize", CAM_SPECIAL_EFFECT_POSTERIZE },
	{ "whiteboard", CAM_SPECIAL_EFFECT_WHITEBOARD },
	{ "blackboard", CAM_SPECIAL_EFFECT_BLACKBOARD },
	{ "aqua", CAM_SPECIAL_EFFECT_AQUA },
	{ NULL, 0 }
};

static const struct str_map ab_map[] = {
	{ "auto", CAM_ANTIBANDING_MODE_AUTO},
	{ "50hz", CAM_ANTIBANDING_MODE_50HZ},
	{ "60hz", CAM_ANTIBANDING_MODE_60HZ},
	{ "off", CAM_ANTIBANDING_MODE_OFF},
	{ NULL, 0 }
};

static const struct str_map scn_map[] = {
	{ "auto", CAM_SCENE_MODE_AUTO},
	{ "action", CAM_SCENE_MODE_ACTION},
	{ "portrait", CAM_SCENE_MODE_PORTRAIT},
	{ "landscape", CAM_SCENE_MODE_LANDSCAPE},
	{ "night", CAM_SCENE_MODE_NIGHT},
	{ "night-portrait", CAM_SCENE_MODE_NIGHT_PORTRAIT},
	{ "theatre", CAM_SCENE_MODE_THEATRE},
	{ "beach", CAM_SCENE_MODE_BEACH},
	{ "snow", CAM_SCENE_MODE_SNOW},
	{ "sunset", CAM_SCENE_MODE_SUNSET},
	{ "steadyphoto", CAM_SCENE_MODE_STEADYPHOTO},
	{ "fireworks", CAM_SCENE_MODE_FIREWORKS},
	{ "sports", CAM_SCENE_MODE_SPORTS},
	{ "party", CAM_SCENE_MODE_PARTY},
	{ "candlelight", CAM_SCENE_MODE_CANDLELIGHT},
	{ "barcode", CAM_SCENE_MODE_BARCODE},
	{ NULL, 0 }
};

static const struct str_map fm_map[] = {
	{"off", CAM_FLASH_MODE_OFF},
	{"auto", CAM_FLASH_MODE_AUTO},
	{"on", CAM_FLASH_MODE_ON},
	{"red-eye", CAM_FLASH_MODE_RED_EYE},
	{"torch", CAM_FLASH_MODE_TORCH},
	{NULL, 0}
};

static const int res_size[CAM_RES_ENUM_MAX][3] =
{
	{CAM_RES_QQCIF, 88, 72},
	{CAM_RES_SUB_QCIF, 128, 96},
	{CAM_RES_QQVGA, 160, 120},
	{CAM_RES_QCIF, 176, 144},
	{CAM_RES_QVGA, 320, 240},
	{CAM_RES_CIF, 352, 288},
	{CAM_RES_VGA,  640, 480},
	{CAM_RES_480P, 720, 480},
	{CAM_RES_PAL, 768, 576},
	{CAM_RES_SVGA, 800, 600},
	{CAM_RES_XVGA, 1024, 768},
	{CAM_RES_720P, 1280, 720},
	{CAM_RES_130W, 1280, 960},
	{CAM_RES_SXGA, 1280, 1024},
	{CAM_RES_SXGAPlus, 1400, 1050},
	{CAM_RES_UXGA, 1600, 1200},
	{CAM_RES_1080P, 1920, 1080},
	{CAM_RES_320W, 2048, 1536},
	{CAM_RES_WQXGA, 2560, 1600},
	{CAM_RES_500W, 2592, 1936},
	{CAM_RES_QUXGA, 3200, 2400},
	{CAM_RES_WQXGA_U, 3840, 2400},
	{CAM_RES_12M, 4000, 3000}
};

static int lookup(const struct str_map *const arr, const char *name, int def)
{
	if (name) {
		const struct str_map * trav = arr;
		while (trav->desc) {
			if (!strcmp(trav->desc, name))
				return trav->val;
			trav++;
		}
	}
	return def;
}

static bool calcWidthHeight(int res, int *w, int *h)
{
	if (res == 0) return false;

	for(int i = 0; i < CAM_RES_ENUM_MAX; i++)
	{
		if(res & res_size[i][0]){
			*w = res_size[i][1];
			*h = res_size[i][2];
			return true;
		}
	}

	LOGMSG(DBGERR, "%s calcWidthHeight() failed", ERRHEAD);
	return false;
}

static bool calcRes(int *res, int width, int height)
{
	if ((width < 0)|| (height < 0))return false;

	for(int i = 0; i < CAM_RES_ENUM_MAX; i++)
	{
		if((res_size[i][1] == width)&&(res_size[i][2] == height)){
			*res = res_size[i][0];
			return true;
		}
	}

	LOGMSG(DBGERR, "%s calcRes() failed", ERRHEAD);
	return false;
}

static bool calcSize(int res, char* size_val, int str_size)
{
	IM_Vector vector_size;
	int i;
	for(i = 0; i < CAM_RES_ENUM_MAX; i++)
	{
		if((res & res_size[i][0]) && (!(res_size[i][2] % 16))){
			memset(size_val, 0, str_size);
			sprintf(size_val, "%dx%d", res_size[i][1],  res_size[i][2]);
			if(IM_RET_OK !=	vector_size.append((void *)size_val, strlen(size_val) + 1)){					
				LOGMSG(DBGERR, "%s	vector_size.append() failed", ERRHEAD);
				return false;
			}
		}
	}

	memset(size_val, 0, str_size);
	void* l1;
	char* lsz;
	i = 0;
	int ret = vector_size.begin(&l1);
	while(ret == IM_RET_OK){
		lsz = (char *)l1;
		if(i++ != 0){
			strcat(size_val, ",");
		}
		strcat(size_val, lsz);
		ret = vector_size.next(&l1);
	}

	return true;
}

static bool checkSize(int res, int w, int h)
{
	for(int i = 0; i < CAM_RES_ENUM_MAX; i++)
	{
		if((res_size[i][1] == w) && (res_size[i][2] == h)) 
		{
			if(res & res_size[i][0])
				return true;
			else
				break;
		}
	}
	return false;
}

static bool checkFormat(int fmt, char *format)
{
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_YUV420P))
	{
		if(fmt & CAM_PIXFMT_YUV420P)
			return true;
		else
			return false;
	}
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_YUV420SP))
	{
		if(fmt & CAM_PIXFMT_YUV420SP)
			return true;
		else
			return false;
	}
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_YUV422SP))
	{
		if(fmt & CAM_PIXFMT_YUV422SP)
			return true;
		else
			return false;
	}
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_YUV422I))
	{
		if(fmt & CAM_PIXFMT_YUV422I)
			return true;
		else
			return false;
	}
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_RGB565))
	{
		if(fmt & CAM_PIXFMT_16BPP_RGB565)
			return true;
		else
			return false;
	}
	if (!strcmp(format, CameraParameters::PIXEL_FORMAT_RGBA8888))
	{
		if(fmt & CAM_PIXFMT_32BPP_RGB0888)
			return true;
		else
			return false;
	}
	else
	{
		return false; 
	}
}

static bool calcSingleFormat(int fmt, char* format_val, int str_size)
{
	memset(format_val, 0, str_size);

	if (fmt == 0) return false;

	switch(fmt)
	{
		case CAM_PIXFMT_YUV420P:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_YUV420P);
			return true;
		case CAM_PIXFMT_YUV420SP:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_YUV420SP);
			return true;
		case CAM_PIXFMT_YUV422SP:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_YUV422SP);
			return true;
		case CAM_PIXFMT_YUV422I:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_YUV422I);
			return true;
		case CAM_PIXFMT_16BPP_RGB565:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_RGB565);
			return true;
		case CAM_PIXFMT_32BPP_RGB0888:
			strcpy(format_val, CameraParameters::PIXEL_FORMAT_RGBA8888);
			return true;
		default:
			return false;
	}
}

static bool calcFormat(int fmt, char* format_val, int str_size)
{
	memset(format_val, 0, str_size);

	if (fmt == 0) return false;

	if(fmt & CAM_PIXFMT_YUV420P)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV420P);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV420P);
		}
	}
	if(fmt & CAM_PIXFMT_YUV420SP)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV420SP);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV420SP);
		}
	}
	if(fmt & CAM_PIXFMT_YUV422SP)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV422SP);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV422SP);
		}
	}
	if(fmt & CAM_PIXFMT_YUV422I)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV422I);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_YUV422I);
		}
	}
	if(fmt & CAM_PIXFMT_16BPP_RGB565)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_RGB565);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_RGB565);
		}
	}
	if(fmt & CAM_PIXFMT_32BPP_RGB0888)
	{
		if(strlen(format_val) != 0)
		{
			strcat(format_val, ",");
			strcat(format_val, CameraParameters::PIXEL_FORMAT_RGBA8888);
		}
		else{
			strcat(format_val, CameraParameters::PIXEL_FORMAT_RGBA8888);
		}
	}

	return true;
}

static bool calcBuffersize(int res, int fmt,  int *size)
{
	int val = 1;
	while(res >> 1)
	{
		res = res >> 1;
		val = val << 1;
	}

	switch	(val)
	{
		case CAM_RES_QQCIF:		
			*size =  88*72;
			break;
		case CAM_RES_SUB_QCIF:
			*size =  128*96;
			break;
		case CAM_RES_QQVGA:
			*size =  160*120;
			break;
		case CAM_RES_QCIF:
			*size = 176*144;
			break;
		case CAM_RES_QVGA:
			*size = 320*240;
			break;
		case CAM_RES_CIF:
			*size = 352*288;
			break;
		case CAM_RES_VGA:
			*size =640*480;
			break;
		case CAM_RES_480P:
			*size = 720*480;
			break;
		case CAM_RES_PAL:
			*size = 768*576;
			break;
		case CAM_RES_SVGA:
			*size =800*600; 
			break;
		case CAM_RES_XVGA:
			*size =1024*768;
			break;
		case CAM_RES_720P:
			*size=1280*720;
			break;
		case CAM_RES_130W:
			*size=1280*960;
			break;
		case CAM_RES_SXGAPlus:
			*size =1400*1050;
			break;
		case CAM_RES_UXGA:
			*size = 1600*1200;
			break;
		case CAM_RES_1080P:
			*size = 1920*1080;
			break;
		case CAM_RES_320W:
			*size = 2048*1536;
			break;
		case CAM_RES_WQXGA:
			*size = 2560*1600;
			break;
		case CAM_RES_500W:
			*size = 2592*1936;
			break;
		case CAM_RES_QUXGA:
			*size = 3200*2400;
			break;
		case CAM_RES_WQXGA_U:
			*size = 3840*2400;
			break;
		case CAM_RES_12M:
			*size = 4000*3000;
			break;
		default:
			return false;
	}
	int delta = 1;
	if (fmt & (CAM_PIXFMT_YUV420P | CAM_PIXFMT_YUV420SP))
	{
		delta = 3;
	}
	if (fmt & (CAM_PIXFMT_YUV422SP| CAM_PIXFMT_YUV422I | CAM_PIXFMT_16BPP_RGB565))
	{
		delta = 4;
	}
	if ((fmt & CAM_PIXFMT_32BPP_RGB0888)
		|| (fmt & CAM_PIXFMT_32BPP_BGR0888))
	{
		delta = 8;
	}

	if (delta == 1)
	{
		return false;
	}
	*size  = (*size) * delta /2;

	return true;
}

/*conver camera image struct to blt_retect struct*/
static bool calcBltRect(frame_with_info *inframe, blt_rect_t *blt_rect, bool in)
{
	blt_rect->imgWidth = inframe->image.width;
	blt_rect->imgHeight = inframe->image.height;
	blt_rect->width = inframe->image.width;
	blt_rect->height = inframe->image.height;
	blt_rect->xOffset = inframe->image.xoffset;
	blt_rect->yOffset = inframe->image.yoffset;
	blt_rect->strideY = inframe->image.stride;
	blt_rect->strideU = inframe->image.strideCb;
	blt_rect->strideV = inframe->image.strideCr;

	switch(inframe->image.type)
	{
		case IM_IMAGE_YUV420P:
			blt_rect->pixfmt = IM_PIC_FMT_12BITS_YUV420P;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			blt_rect->bufferY.size = blt_rect->width  * blt_rect->height;
			blt_rect->bufferU.vir_addr = (void*)((int*)inframe->frame.buffer.vir_addr + blt_rect->width  * blt_rect->height);
			blt_rect->bufferU.phy_addr = inframe->frame.buffer.phy_addr + blt_rect->width  * blt_rect->height;
			blt_rect->bufferU.size = blt_rect->width  * blt_rect->height >> 2;
			blt_rect->bufferU.flag = inframe->frame.buffer.flag;
			blt_rect->bufferV.vir_addr = (void*)((int*)inframe->frame.buffer.vir_addr + (blt_rect->width  * blt_rect->height * 5 >> 2));
			blt_rect->bufferV.phy_addr = inframe->frame.buffer.phy_addr + (blt_rect->width  * blt_rect->height * 5 >> 2);
			blt_rect->bufferV.size = blt_rect->width  * blt_rect->height >> 2;
			blt_rect->bufferV.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_YUV420SP:
			blt_rect->pixfmt= IM_PIC_FMT_12BITS_YUV420SP;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = blt_rect->width  * blt_rect->height;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			blt_rect->bufferU.vir_addr = (void*)((int*)inframe->frame.buffer.vir_addr + blt_rect->width  * blt_rect->height);
			blt_rect->bufferU.phy_addr = inframe->frame.buffer.phy_addr + blt_rect->width  * blt_rect->height;
			blt_rect->bufferU.size = blt_rect->width  * blt_rect->height >> 1;
			blt_rect->bufferU.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_YUV422SP:
			blt_rect->pixfmt= IM_PIC_FMT_16BITS_YUV422SP;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = blt_rect->width  * blt_rect->height;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			blt_rect->bufferU.vir_addr = (void*)((int*)inframe->frame.buffer.vir_addr + blt_rect->width  * blt_rect->height);
			blt_rect->bufferU.phy_addr = inframe->frame.buffer.phy_addr + blt_rect->width  * blt_rect->height;
			blt_rect->bufferU.size = blt_rect->width  * blt_rect->height;
			blt_rect->bufferU.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_YUV422I:
			blt_rect->pixfmt= IM_PIC_FMT_16BITS_YUV422I_YUYV;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = inframe->frame.buffer.size;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_RGB565:
			blt_rect->pixfmt= IM_PIC_FMT_16BITS_RGB_565;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = inframe->frame.buffer.size;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_RGB0888:
			//blt_rect->pixfmt= IM_PIC_FMT_32BITS_ARGB_8888;
			blt_rect->pixfmt= IM_PIC_FMT_32BITS_0RGB_8888;
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = inframe->frame.buffer.size;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			break;
		case IM_IMAGE_BGR0888:
			blt_rect->pixfmt= IM_PIC_FMT_32BITS_ABGR_8888;	//here is ABGR888 for PP but 0BGR for 2D CSC in fact
			//blt_rect->pixfmt= IM_PIC_FMT_32BITS_0BGR_8888;//This format not support by g2d, but we need check g2d api later. 
			blt_rect->bufferY.vir_addr = inframe->frame.buffer.vir_addr;
			blt_rect->bufferY.phy_addr = inframe->frame.buffer.phy_addr;
			blt_rect->bufferY.size = inframe->frame.buffer.size;
			blt_rect->bufferY.flag = inframe->frame.buffer.flag;
			break;
		default:
			return false;
	}

	//check input height
	if(in){
		if(blt_rect->imgHeight % 16) {
			blt_rect->imgHeight = blt_rect->imgHeight >> 4 << 4;
			blt_rect->height = blt_rect->height >> 4 << 4;
		}
	}

	return true;
}

static bool calcImage(IM_IMAGE_FORMAT *image, int fmt, int w, int h)
{
	switch(fmt)
	{
		case CAM_PIXFMT_YUV420P:
			image->type = IM_IMAGE_YUV420P;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w;
			image->strideCb = w;
			image->strideCr = w;
			break;
		case CAM_PIXFMT_YUV420SP:
			image->type = IM_IMAGE_YUV420SP;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w;
			image->strideCb = w;
			image->strideCr = 0;
			break;
		case CAM_PIXFMT_YUV422SP:
			image->type = IM_IMAGE_YUV422SP;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w;
			image->strideCb = w;
			image->strideCr = 0;
			break;
		case CAM_PIXFMT_YUV422I:
			image->type = IM_IMAGE_YUV422I;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w * 2;
			image->strideCb = 0;
			image->strideCr = 0;
			break;
		case CAM_PIXFMT_16BPP_RGB565:
			image->type = IM_IMAGE_RGB565;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w * 2;
			image->strideCb = 0;
			image->strideCr = 0;
			break;
		case CAM_PIXFMT_32BPP_RGB0888:
			image->type = IM_IMAGE_RGB0888;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w * 4;
			image->strideCb = 0;
			image->strideCr = 0;
			break;
		case CAM_PIXFMT_32BPP_BGR0888:
			image->type = IM_IMAGE_BGR0888;
			image->width = w;
			image->height = h;
			image->xoffset = 0;
			image->yoffset = 0;
			image->stride = w * 4;
			image->strideCb = 0;
			image->strideCr = 0;
			break;
		default:
			LOGMSG(DBGERR, "%s calcImage() failed", ERRHEAD);
			return false;
	}
	return true;
}

/*
 * rotate yuv420sp format
 * For that pp cant support size large than 2048*2048
 * and G2D cant support yuv420sp as output format, so 
 * we use this func rotate one picture
 * */
static bool  RotateYUV420sp(char *in, char *out, int rotation, int w, int h)
{
	char *pin;
	char *pout;
	int i = 0;
	int j = 0;
	int A = w * (h - 1);
	int B = w * h;
	int C = w * h / 2;
	int D = w * (h /2 -1);
	int E = w / 2;
	int F = h /2;
	if(rotation == 270)
	{
		pout = out;
		pin = in + A;
		for(i =0; i < w; i ++)
		{
			for(j =0; j < h; j++)
			{
				*pout = *pin;
				if( (i < w -1) || (j < h -1))
				{
					pout++;
				}
				if(j < h -1)
				{
					pin -= w;
				}
			}
			if(i < w -1)
			{
				pin++;
				pin += A;
			}
		}

		pout = out + B;
		pin = in + B + D;
		for(i = 0; i < E; i++)
		{
			for(j =0; j < F; j++)
			{
				*pout = *pin;
				*(pout+1) = *(pin + 1);
				if((i < E -1) ||(j < F -1))
				{
					pout += 2;
				}
				if(j < F -1)
				{
					pin -= w;
				}
			}
			if( i < E -1)
			{
				pin += 2;
				pin += D;
			}
		}
	}
	else if(rotation == 180)
	{
		pout = out;
		pin = in + B  -1;
		for(i =0; i < B; i++)
		{
			*pout = *pin;
			if(i < B -1)
			{
				pin--;
				pout++;
			}
		}
		pout = out + B;
		pin = in + B * 3 /2 -1;
		for(i =0; i < C ; i++)
		{
			if(i%2){
				*pout = *(pin - 1);
			}
			else{
				*pout = *(pin + 1);
			}
			if(i < C -1)
			{
				pin--;
				pout++;
			}
		}
	}
	else if(rotation == 90)
	{
		pout = out;
		pin = in + w -1;
		for(i =0; i < w; i ++)
		{
			for(j =0; j < h; j++)
			{
				*pout = *pin;
				if( (i < w -1) || (j < h -1))
				{
					pout++;
				}
				if(j < h -1)
				{
					pin += w;
				}
			}
			if(i < w -1)
			{
				pin = in + w -2 -i;
			}
		}

		pout = out + B;
		pin = in + B + w -2;
		for(i = 0; i < E; i++)
		{
			for(j =0; j < F; j++)
			{
				*pout = *pin;
				*(pout+1) = *(pin + 1);
				if((i < E -1) ||(j < F -1))
				{
					pout += 2;
				}
				if(j < F -1)
				{
					pin += w;
				}
			}
			if( i < E -1)
			{
				pin = in + B + w - (i -2)*2;
			}
		}
	}
	else{
		return false;
	}

	return true;
}

CameraHal::CameraHal(int cameraid) :
	mParameters(),
	mCamctx(NULL),
	mNotifyCb(0),
	mDataCb(0),
	mDataCbTimestamp(0),
	mGet_memory(0),
	mCallbackCookie(NULL),
	mPreviewMemory(0),
	mCurrentPreviewFrame(0),
	mRecordingMemory(0),
	mPreviewStreamOps(0),
	mMsgEnabled(0),
	mFrameSize(0),
	mRawHeap(NULL),
	mRawBuffer(NULL),
	mJpegBuffer(NULL),
	mJencInst(NULL),
	mSupportJpeg(false),
	mRotation(0),
	mNeedreoganize(false),
	mBigMemory(false),
	mPicBufSize(0),
	mPreBufSize(0),
	mRecordEna(false),
	mVideoFrameFormat(CAM_PIXFMT_YUV420SP),
	mUseEncodeSruct(true),
	mBufferCopied(false),
	mNeedResize(false),
	mNeedResizeForVideo(false),
	prePathCounter(0),
	coPathCounter(0),
	mBlt(NULL),
	mNeedTurnOnLight(false),
	mFaceDT(false),
	mImage(NULL),
	mFaceCounter(0),
	mFaceDetecting(false)
{
	cameraID = cameraUID[cameraid];
	mCheckInit = false;

	gPicThrdRuning = false;
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	if(ResourceInit() != true){
		LOGMSG(DBGERR, "%s ResourceInit() failed", ERRHEAD);
		return;
	}
}

bool CameraHal::CheckInit()
{
	return mCheckInit;
}

bool CameraHal::InitDefaultParameters()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	int i= 0;
	int wb_caps = 0;  //white balance caps. 
	int se_caps = 0;  //special effect caps.
	int wb_val = 0;
	int se_val = 0;
	int ab_caps = 0;
	int ab_val = 0;
	int scn_caps = 0;
	int scn_val = 0;
	int zoom_val = 0;
	int max_zoom_val = 0;
	int fm_caps = 0;
	int fm_val = 0;
	int exp_val = 0;
	int exp_max_val = 0;
	int exp_min_val = 0;
	float exp_step = 0;
	char Vals_white_balance[512];
	char Vals_effect[512];
	char Vals_antibanding[512];
	char Vals_scene[512];
	char Vals_flash_mode[512];
	char Vals_zoom_ratios[512];
	char Vals_pre_size[512];
	char Vals_pre_format[512];
	char Vals_pic_size[512];
	char Vals_pic_format[512];
	char pre_format[16];
	char Val_face_number[8];
	int w, h;
	CameraParameters p;
	memset(Vals_white_balance, 0, sizeof(Vals_white_balance));
	memset(Vals_effect, 0, sizeof(Vals_effect));
	memset(Vals_antibanding, 0, sizeof(Vals_antibanding));
	memset(Vals_scene, 0, sizeof(Vals_scene));
	memset(Vals_flash_mode, 0, sizeof(Vals_flash_mode));

	//get white balance(light mode) values 
	if(mCaps&CAM_CAP_WB_MODE_SUPPORT){
		ret = cam_get_property(mCamctx, CAM_KEY_R_SUPPORT_WB_MODE, &wb_caps, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_SUPPORT_WB_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret = cam_get_property(mCamctx, CAM_KEY_RW_WB_MODE, &wb_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_WB_MODE) failed, ret=%d", ERRHEAD, ret);
			return false;
		}


		if(wb_caps != 0){
			for(i = 0; i < (int)(sizeof(wb_map)/sizeof(wb_map[0]) - 1); i++){
				if(wb_caps & wb_map[i].val){
					if(i != 0){
						strcat(Vals_white_balance, ",");
					}
					strcat(Vals_white_balance, wb_map[i].desc);
				}

				if(wb_val & wb_map[i].val)
				{
					p.set(CameraParameters::KEY_WHITE_BALANCE, wb_map[i].desc);
				}
			}
			p.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, Vals_white_balance);
		}
	}

	//get special effect values 
	if(mCaps&CAM_CAP_SPECIAL_EFFECT_SUPPORT){
		ret =  cam_get_property(mCamctx, CAM_KEY_R_SUPPORT_SPECIAL_EFFECT, &se_caps, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_SUPPORT_SPECIAL_EFFECT,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret = cam_get_property(mCamctx, CAM_KEY_RW_SPECIAL_EFFECT, &se_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_SPECIAL_EFFECT,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}

		if(se_caps != 0){
			for(i = 0; i < (int)(sizeof(se_map)/sizeof(se_map[0]) - 1); i++){
				if(se_caps & se_map[i].val){
					if(i != 0){
						strcat(Vals_effect, ",");
					}
					strcat(Vals_effect, se_map[i].desc);
				}

				if(se_val & se_map[i].val)
				{
					p.set(CameraParameters::KEY_EFFECT, se_map[i].desc);
				}
			}
			p.set(CameraParameters::KEY_SUPPORTED_EFFECTS, Vals_effect);
		}
	}

	//get antibanding values
	if(mCaps&CAM_CAP_ANTIBANDING){
		ret =  cam_get_property(mCamctx, CAM_KEY_R_SUPPORT_ANTIBANDING_MODE, &ab_caps, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_SUPPORT_ANTIBANDING_MODE) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret = cam_get_property(mCamctx, CAM_KEY_RW_ANTIBANDING_MODE, &ab_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_ANTIBANDING_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}

		if(ab_caps != 0){
			for(i = 0; i < (int)(sizeof(ab_map)/sizeof(ab_map[0]) - 1); i++){
				if(ab_caps & ab_map[i].val){
					if(i != 0){
						strcat(Vals_antibanding, ",");
					}
					strcat(Vals_antibanding, ab_map[i].desc);
				}

				if(ab_val & ab_map[i].val)
				{
					p.set(CameraParameters::KEY_ANTIBANDING, ab_map[i].desc);
				}
			}
			p.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, Vals_antibanding);
		}
	}

	//get scene mode  values 
	if(mCaps&CAM_CAP_SCENE_MODE_SUPPORT){
		ret =  cam_get_property(mCamctx, CAM_KEY_R_SUPPORT_SCENE_MODE, &scn_caps, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_SUPPORT_SCENE_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret = cam_get_property(mCamctx, CAM_KEY_RW_SCENE_MODE, &scn_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_SCENE_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}

		if(scn_caps != 0){
			for(i = 0; i < (int)(sizeof(scn_map)/sizeof(scn_map[0]) - 1); i++){
				if(scn_caps & scn_map[i].val){
					if(i != 0){
						strcat(Vals_scene, ",");
					}
					strcat(Vals_scene, scn_map[i].desc);
				}

				if(scn_val & scn_map[i].val)
				{
					p.set(CameraParameters::KEY_SCENE_MODE, scn_map[i].desc);
				}
			}
			p.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES, Vals_scene);
		}
	}

	//get zoom valuse
	if(mCaps&CAM_CAP_ZOOM){
		p.set(CameraParameters::KEY_ZOOM_SUPPORTED, "true");
		ret =  cam_get_property(mCamctx, CAM_KEY_R_MAX_ZOOM, &max_zoom_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_MAX_ZOOM,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		p.set(CameraParameters::KEY_MAX_ZOOM, max_zoom_val);
		if(mCaps&CAM_CAP_SMOOTH_ZOOM){
			p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "true");
		}
		mMax_zoom = max_zoom_val;
		int ratio_val = 100;
		char  aNumber[10];
		memset(Vals_zoom_ratios, 0, 512);
		sprintf(aNumber, "%d", ratio_val);
		strcat(Vals_zoom_ratios, aNumber);
		for(i = 1; i <= max_zoom_val; i++)
		{
			int ratio_val = 100 + i * 10;
			sprintf(aNumber, "%d", ratio_val);
			strcat(Vals_zoom_ratios, ",");
			strcat(Vals_zoom_ratios, aNumber);
		}
		p.set(CameraParameters::KEY_ZOOM_RATIOS, Vals_zoom_ratios);
		ret =  cam_get_property(mCamctx, CAM_KEY_RW_ZOOM, &zoom_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_ZOOM) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		p.set(CameraParameters::KEY_ZOOM, zoom_val);
	}

	//get exposure values
	if(mCaps&CAM_CAP_EXPOSURE){
		ret =  cam_get_property(mCamctx, CAM_KEY_R_MAX_EXPOSURE_COMPENSATION, &exp_max_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_MAX_EXPOSURE_COMPENSATION) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret =  cam_get_property(mCamctx, CAM_KEY_R_MIN_EXPOSURE_COMPENSATION, &exp_min_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_MIN_EXPOSURE_COMPENSATION) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret =  cam_get_property(mCamctx, CAM_KEY_R_EXPOSURE_COMPENSATION_STEP, &exp_step, sizeof(float));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_EXPOSURE_COMPENSATION_STEP) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret =  cam_get_property(mCamctx, CAM_KEY_RW_EXPOSURE_COMPENSATION, &exp_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_EXPOSURE_COMPENSATION) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		p.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, exp_val);
		p.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, exp_max_val);
		p.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, exp_min_val);
		p.setFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, exp_step);
		p.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, "true");
	}
	else
	{
		p.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, 0);
		p.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, 0);
		p.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, 0);
		p.setFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, 0);
	}

	//get flash mode values
	if(mCaps&CAM_CAP_FLASH_MODE_SUPPORT){
		ret =  cam_get_property(mCamctx, CAM_KEY_R_SUPPORT_FLASH_MODE, &fm_caps, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_SUPPORT_FLASH_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}
		ret = cam_get_property(mCamctx, CAM_KEY_RW_FLASH_MODE, &fm_val, sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_RW_FLASH_MODE,) failed, ret=%d", ERRHEAD, ret);
			return false;
		}

		if(fm_caps != 0){
			for(i = 0; i < (int)(sizeof(fm_map)/sizeof(fm_map[0]) - 1); i++){
				if(fm_caps & fm_map[i].val){
					if(i != 0){
						strcat(Vals_flash_mode, ",");
					}
					strcat(Vals_flash_mode, fm_map[i].desc);
				}

				if(fm_val & fm_map[i].val)
				{
					p.set(CameraParameters::KEY_FLASH_MODE, fm_map[i].desc);
				}
			}
			p.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, Vals_flash_mode);
		}
	}

	//get preview/picture configs
	ret = cam_get_preview_configs(mCamctx, &mPreviewCfg);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_get_preview_configs() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	//init the r_pre_fmt
	r_pre_fmt = mPreviewCfg.fmt;

	ret = redir_cam_get_picture_configs(mCamctx, &mPictureCfg);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_get_picture_configs() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	//init the r_pic_fmt
	r_pic_fmt = mPictureCfg.fmt;

	//alloc buffer for hal
	if(mBuffAlloc != IM_NULL){
		memset(&mJpegBuff, 0, sizeof(mJpegBuff));
		memset(&mCodecBuff, 0, sizeof(mJpegOutBuff));
		memset(&mPreviewBuff, 0, sizeof(mPreviewBuff));
		memset(&mFaceBuff, 0, sizeof(mFaceBuff));

		int flag = ALC_FLAG_PHY_MUST | ALC_FLAG_ALIGN_32BYTES;
		int flag2 = 0;
		int allocsize;
		//check allocsize. if > 1 M use big memory
		if(!calcBuffersize(mPictureCfg.resBits, (mPreviewCfg.fmtBits & ~CAM_PIXFMT_32BPP_RGB0888), &allocsize)){ 
			LOGMSG(DBGERR, "%s calcBuffersize(0x%x) failed, line=%d", ERRHEAD, mPictureCfg.resBits, __LINE__);
			return false;
		}
		if(allocsize >400000){
			switch(GetRealPath(CAM_PATH_PICTURE)){
				case CAM_PATH_PREVIEW:
					ret = cam_get_property(mCamctx, CAM_KEY_R_PREVIEW_BUFFER_REQUIREMENT, (void *)&pre_buffer_property, sizeof(cam_buffer_property_t));
					if(ret != IM_RET_OK){
						LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_PREVIEW_BUFFER_MIN) failed, ret=%d", ERRHEAD, ret);
						return false;
					}
				case CAM_PATH_PICTURE:
					//check later
					break;
				default:
					return false;
			}
			ret = alc_init_bigmem(mBuffAlloc, ((((allocsize-1)>>12) + 1)<<12), (pre_buffer_property.minNumber + 3), IM_FALSE);
			if(ret != IM_RET_OK){
				mBigMemory = false;
				LOGMSG(DBGERR, "%s alc_init_bigmem() failed, ret=%d", ERRHEAD, ret);
			}
			else{
				mBigMemory = true;
				flag2 = ALC_FLAG_BIGMEM;
			}
		}

		if(allocsize > 1000000)
		{
			mOutbufsize = allocsize ;
		}
		else{
			mOutbufsize = 1000000;//1M
		}	
		ret = alc_alloc(mBuffAlloc, allocsize,&mJpegBuff, flag|flag2);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
		}
		ret = alc_alloc(mBuffAlloc, mOutbufsize , &mJpegOutBuff, flag/*|flag2*/);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
		}
		if(!calcBuffersize(mPreviewCfg.resBits, CAM_PIXFMT_32BPP_RGB0888, &allocsize))
		{
			LOGMSG(DBGERR, "%s calcBuffersize(0x%x) failed, line=%d", ERRHEAD, mPreviewCfg.resBits, __LINE__);
			return false;
		}

		//TODO: check each preview fps,  and remove some resolution
		if (mPreviewCfg.resBits & CAM_RES_1080P){
			ret = alc_alloc(mBuffAlloc, 1920 * 1080 * 2,&mPreviewBuff, flag);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
			}
		}
		else if (mPreviewCfg.resBits & CAM_RES_720P){
			ret = alc_alloc(mBuffAlloc, 1280 * 720 * 2,&mPreviewBuff, flag);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
			}
		}
		else{
			ret = alc_alloc(mBuffAlloc, 640 * 480 * 2,&mPreviewBuff, flag);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
			}
		}
		ret = alc_alloc(mBuffAlloc, 320*240*2,&mFaceBuff, flag);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
		}

		//Thumbnail Header
		ret = alc_alloc(mBuffAlloc, 128*96*2,&mThumbBuff, flag);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
		}

		for(i = 0; i < mRecordingBufferNumber; i++)
		{
			memset(&mVideoResize[i], 0, sizeof(IM_Buffer));
		}
		r_thumb_width = 128;
		r_thumb_height = 96;
	}

	//get supported size and format
	if (!calcSize(mPreviewCfg.resBits, Vals_pre_size, 512))
	{ 
		LOGMSG(DBGERR, "%s calcSize() failed, mPreviewCfg.resBits=0x%x, line = %d", ERRHEAD, mPreviewCfg.resBits, __LINE__);
		return false;
	}
	//if (!calcSize(mPreviewCfg.resBits, Vals_pic_size, 512)) 	return false;
	if (!calcSize(mPictureCfg.resBits, Vals_pic_size, 512))
	{ 
		LOGMSG(DBGERR, "%s calcSize() failed, mPictureCfg.resBits=0x%x, line = %d", ERRHEAD, mPictureCfg.resBits, __LINE__);
		return false;
	}
	if (!calcFormat(mPreviewCfg.fmtBits, Vals_pre_format, 512))
	{ 
		LOGMSG(DBGERR, "%s calcFormat() failed, mPreviewCfg.fmtBits=0x%x, line = %d", ERRHEAD, mPreviewCfg.fmtBits, __LINE__);
		return false;
	}
	if (!calcFormat(mPictureCfg.fmtBits, Vals_pic_format, 512))
	{
		LOGMSG(DBGERR, "%s calcFormat(0x%x) failed, line=%d", ERRHEAD, mPictureCfg.fmtBits, __LINE__);
		return false;
	}
	strcat(Vals_pic_format, ",");
	strcat(Vals_pic_format, CameraParameters::PIXEL_FORMAT_JPEG);

	p.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS, Vals_pic_format);
	p.set(CameraParameters::KEY_PICTURE_FORMAT, CameraParameters::PIXEL_FORMAT_JPEG);
	p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, Vals_pic_size);
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS, Vals_pre_format);
	//320X240, 352X288 and 176x144 is the res maybe QQ, skype or the other third party apk need,
	//so here we must support, if camera sensor not support, we must resize the res from another supported res. 
	if (mPreviewCfg.resBits & CAM_RES_1080P)
	{
		p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "1920x1080,1280x720,640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, "1920x1080,1280x720,640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, "1920x1080");
		p.setVideoSize(1920,1080);
	}
	else if (mPreviewCfg.resBits & CAM_RES_720P)
	{
		p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "1280x720,640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, "1280x720,640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, "1280x720");
		p.setVideoSize(1280,720);
	}
	else
	{
		p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, "640x480,320x240,352x288,176x144");
		p.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, "640x480");
		p.setVideoSize(640,480);    
	}

	if(!calcWidthHeight(mPictureCfg.resBits, &w, &h))
	{
		LOGMSG(DBGERR, "%s calcWidthHeight(0x%x) failed, line=%d", ERRHEAD, mPictureCfg.resBits, __LINE__);
		return false;
	}
	//set the default value
	p.setPictureSize(w, h);
	r_pic_width = w;
	r_pic_height = h;

	if(!calcWidthHeight(mPreviewCfg.resBits, &w, &h))
	{
		LOGMSG(DBGERR, "%s calcWidthHeight(0x%x) failed, line=%d", ERRHEAD, mPreviewCfg.resBits, __LINE__);
		return false;
	}
	//set the default value
	r_pre_width = w;
	r_pre_height = h;
	r_vid_width = w;
	r_vid_height = h;

	p.setPreviewSize(w, h);
	if(!calcSingleFormat(mPreviewCfg.fmt, pre_format, 16))
	{
		LOGMSG(DBGERR, "%s calcSingleFormat(0x%x) failed, line=%d", ERRHEAD, mPreviewCfg.fmt, __LINE__);
		return false;
	}
	p.set(CameraParameters::KEY_PREVIEW_FORMAT, pre_format);
	//setting  default init preview format as video frame format
	p.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, pre_format);
	mVideoFrameFormat = mPreviewCfg.fmt;
	//p.set(CameraParameters::KEY_PREVIEW_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);
	//p.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);
	//set for app; in fact it may depend on different sensors, and we dont care it
	//TODO: we need use data get from sensors
	p.set(CameraParameters::KEY_PREVIEW_FRAME_RATE, 30); 
	//p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "30,29,26,25,24,22,20,18,15,12,10,7,5"); 
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "30,20,18,15"); 
	p.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000"); 
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,30000)"); 
	//set focus mode
	p.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_AUTO);
	p.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, CameraParameters::FOCUS_MODE_AUTO);
	p.set(CameraParameters::KEY_FOCUS_DISTANCES, "Infinity,Infinity,Infinity");
	p.set(CameraParameters::KEY_FOCAL_LENGTH, "4");
	//angle
	p.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, "360");
	p.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, "360");
	//face detect
	sprintf(Val_face_number, "%d", IMG_MAX_FACE_DETECT_NUM);
	p.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, Val_face_number);
	//jpeg
	p.set(CameraParameters::KEY_JPEG_QUALITY, 90);
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, 90);
	p.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, "128x96,0x0");
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, 128);
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, 96);
	//GPS
#if 0  
	p.set(CameraParameters::KEY_GPS_LATITUDE, 37.736071);
	p.set(CameraParameters::KEY_GPS_LONGITUDE, -122.441983);
	p.set(CameraParameters::KEY_GPS_ALTITUDE, 21);
	p.set(CameraParameters::KEY_GPS_TIMESTAMP, 1199145601);
	p.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, "GPS");
#endif	

	mParameters = p;
	String8 strparams(p.flatten());
	LOGMSG(DBGINFO, "%s init params=%s", INFOHEAD, strparams.string());
	LOGMSG(DBGINFO, "%s Vals_pre_size=%s", INFOHEAD, Vals_pre_size);
	LOGMSG(DBGINFO, "%s Vals_pre_format=%s", INFOHEAD, Vals_pre_format);
	LOGMSG(DBGINFO, "%s Vals_pic_size=%s", INFOHEAD, Vals_pic_size);
	LOGMSG(DBGINFO, "%s Vals_pic_format=%s", INFOHEAD, Vals_pic_format);
	LOGMSG(DBGINFO, "%s pre_format=%s", INFOHEAD, pre_format);
	LOGMSG(DBGINFO, "%s Vals_white_balance=%s", INFOHEAD, Vals_white_balance);
	LOGMSG(DBGINFO, "%s Vals_effect=%s", INFOHEAD, Vals_effect);
	LOGMSG(DBGINFO, "%s Vals_antibanding=%s", INFOHEAD, Vals_antibanding);
	LOGMSG(DBGINFO, "%s Vals_scene=%s", INFOHEAD, Vals_scene);
	LOGMSG(DBGINFO, "%s zoom_val=%d", INFOHEAD, zoom_val);
	LOGMSG(DBGINFO, "%s max_zoom_val=%d", INFOHEAD, max_zoom_val);
	LOGMSG(DBGINFO, "%s Val_face_number=%s", INFOHEAD, Val_face_number);

	return true;
}

CameraHal::~CameraHal()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();
	//TODO:
	//maybe we should free the mRecordingMemory shared buffer when we stop recording
	//but we should note the stopRecording thread sync with the preview thread
	if(mRecordingMemory != NULL){
		mRecordingMemory->release(mRecordingMemory);
		mRecordingMemory = NULL;
	}

	cancelPicture();
}

/*sp<IMemoryHeap> CameraHal::getPreviewHeap() const
  {
  LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
  return mPreviewHeap;
  }*/

/*sp<IMemoryHeap> CameraHal::getRawHeap() const
  {
  LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
  return mRawHeap;
  }*/

void CameraHal::setCallbacks(camera_notify_callback notify_cb,
		camera_data_callback data_cb,
		camera_data_timestamp_callback data_cb_timestamp,
		camera_request_memory get_memory,
		void *user)
{
	LOGMSG(DBGINFO, "%s %s(notify_cb=0x%x, data_cb=0x%x, data_cb_timestamp=0x%x, get_memory=0x%x, user=0x%x, )", INFOHEAD, __FUNCTION__, (int)notify_cb, (int)data_cb, (int)data_cb_timestamp, (int)get_memory, (int)user);
	Mutex::Autolock lock(mLock);
	mNotifyCb = notify_cb;
	mDataCb = data_cb;
	mDataCbTimestamp = data_cb_timestamp;
	mGet_memory = get_memory;
	mCallbackCookie = user;
}

void CameraHal::enableMsgType(int32_t msgType)
{
	LOGMSG(DBGINFO, "%s %s(msgType=0x%x)", INFOHEAD, __FUNCTION__, msgType);
	Mutex::Autolock lock(mLock);
	mMsgEnabled |= msgType;
}

void CameraHal::disableMsgType(int32_t msgType)
{
	LOGMSG(DBGINFO, "%s %s(msgType=0x%x)++", INFOHEAD, __FUNCTION__, msgType);
	Mutex::Autolock lock(mLock);
	mMsgEnabled &= ~msgType;
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
}

bool CameraHal::msgTypeEnabled(int32_t msgType)
{
	LOGMSG(DBGINFO, "%s %s(msgType=0x%x)", INFOHEAD, __FUNCTION__, msgType);
	Mutex::Autolock lock(mLock);
	return (mMsgEnabled & msgType);
}

/*
 * convert other formats to yuv420sp
 * cause render & codec & jpeg need this format
 *
 * 20111/10/9: now thisfunction is useless 
 * */
/*bool CameraHal::ConvertFormat(CAM_FRAME *inframe, CAM_FRAME *outframe)
  {
  LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
  Mutex::Autolock lock(mCvtLock);

  IM_RET ret;

  if ((inframe == NULL) || (outframe == NULL))
  {
  LOGMSG(DBGERR, "%s parameters is null", ERRHEAD);
  return false;
  }

  memcpy(&mCvtimg.srcImg, &inframe->image, sizeof(inframe->image));
  mCvtimg.srcBuf.vir_addr = inframe->buff.vir_addr;
  mCvtimg.srcBuf.phy_addr = inframe->buff.phy_addr;
  mCvtimg.srcBuf.size = inframe->buff.size;

  mCvtimg.dstImg.type = CAM_PIXFMT_YUV420SP;
  mCvtimg.dstImg.height = mCvtimg.srcImg.height;
  mCvtimg.dstImg.width = mCvtimg.srcImg.width;
  mCvtimg.flag = 0;

  mCvtimg.dstImg.xoffset = 0;
  mCvtimg.dstImg.yoffset = 0;
  mCvtimg.dstImg.stride = mCvtimg.dstImg.width;
  mCvtimg.dstImg.strideCb = mCvtimg.dstImg.width;

  mCvtimg.dstBuf.vir_addr = outframe->buff.vir_addr;
  mCvtimg.dstBuf.phy_addr = outframe->buff.phy_addr;
  mCvtimg.dstBuf.size = outframe->buff.size;

  memcpy(&outframe->image, &mCvtimg.dstImg, sizeof(mCvtimg.dstImg));

  ret = cvt_image(mConvert, &mCvtimg);
  if(ret != IM_RET_OK){
  LOGMSG(DBGERR, "%s cvt_image() failed, ret=%d", ERRHEAD, ret);
  return false;
  }

  return true;
  }
  */

/* rotation: it should be multiple of 90
 * usage
 * 0: for display
 * 1: for preview datacallback
 * 2: for picture*/
bool CameraHal::ConvertFrame(frame_with_info *inframe, frame_with_info *outframe, int32_t rotation, int32_t usage)
{
	//	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;

	if ((inframe == NULL) || (outframe == NULL))
	{
		LOGMSG(DBGERR, "%s parameters is null", ERRHEAD);
		return false;
	}

#ifdef USE_BLT
	if(!calcBltRect(inframe, &mBlt_tsf.srcRect, true))
	{
		LOGMSG(DBGERR, "%s calcBltRect() failed line = %d", ERRHEAD, __LINE__);
		return false;
	}
	if(!calcBltRect(outframe, &mBlt_tsf.dstRect, false))
	{
		LOGMSG(DBGERR, "%s calcBltRect() failed line = %d", ERRHEAD, __LINE__);
		return false;
	}
	if(facing == CAM_MODULE_FACING_BACK){
		switch(rotation % 360)
		{
			case 0:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_0;	
				break;
			case 90:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_270;	
				break;
			case 180:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_180;	
				break;
			case 270:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_90;	
				break;
			default:
				LOGMSG(DBGERR, "%s a invalid rotation line = %d", ERRHEAD, __LINE__);
				return false;
		}
	}
	else{
		switch(rotation % 360)
		{
			case 0:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_0;	
				break;
			case 90:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_90;	
				break;
			case 180:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_180;	
				break;
			case 270:
				mBlt_tsf.rotate.rotation = BLT_ROTATE_270;	
				break;
			default:
				LOGMSG(DBGERR, "%s a invalid rotation line = %d", ERRHEAD, __LINE__);
				return false;
		}
	}


	if((usage == 0) || (usage == 1) || (usage == 2))
	{
		if(facing == CAM_MODULE_FACING_FRONT)
		{
			mBlt_tsf.rotate.rotation = BLT_ROTATE_0;	
		}
		else{
			mBlt_tsf.rotate.rotation = BLT_ROTATE_0;	
		}
	}

	ret = blt_transfer(mBlt, &mBlt_tsf);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s blt_transfer() failed, ret=%d", ERRHEAD, ret);
		return false;
	}


#else
	memcpy(&mCvtimg.srcImg, &inframe->image, sizeof(inframe->image));
	mCvtimg.srcBuf.vir_addr = inframe->frame.buffer.vir_addr;
	mCvtimg.srcBuf.phy_addr = inframe->frame.buffer.phy_addr;
	mCvtimg.srcBuf.size = inframe->frame.buffer.size;

	if(facing == CAM_MODULE_FACING_BACK)
	{
		if ((rotation % 360) == 90){
			mCvtimg.flag = CVT_IMAGE_ROTATION_90;
		}
		else if ((rotation % 360) == 180){
			mCvtimg.flag = CVT_IMAGE_ROTATION_180;
		}
		else if ((rotation % 360) == 270){
			mCvtimg.flag = CVT_IMAGE_ROTATION_270;
		}
		else if((rotation % 360) == 0)
		{
			mCvtimg.flag = CVT_IMAGE_ROTATION_0;
		}
	}
	else{
		if ((rotation % 360) == 90){
			mCvtimg.flag = CVT_IMAGE_ROTATION_270;
		}
		else if ((rotation % 360) == 180){
			mCvtimg.flag = CVT_IMAGE_ROTATION_180;
		}
		else if ((rotation % 360) == 270){
			mCvtimg.flag = CVT_IMAGE_ROTATION_90;
		}
		else if((rotation % 360) == 0)
		{
			mCvtimg.flag = CVT_IMAGE_ROTATION_0;
		}
	}

	if(usage == 0)
	{
		if(facing == CAM_MODULE_FACING_FRONT)
		{
			//camera service will do HFLIP self
			mCvtimg.flag = CVT_IMAGE_ROTATION_HFLIP;
		}
		else{
			mCvtimg.flag = CVT_IMAGE_ROTATION_0;
		}
	}

	mCvtimg.cropParams.enable = 0;

	memcpy(&mCvtimg.dstImg, &outframe->image, sizeof(outframe->image));
	mCvtimg.dstBuf.vir_addr = outframe->frame.buffer.vir_addr;
	mCvtimg.dstBuf.phy_addr = outframe->frame.buffer.phy_addr;
	mCvtimg.dstBuf.size = outframe->frame.buffer.size;

	ret = cvt_image(mConvert, &mCvtimg);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cvt_image() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
#endif

	return true;
}

/*
 * This fuction can convert yuv420sp to yuv422sp, some app may need yuv422sp format.
 * In fact, it is useless now
 * */
bool CameraHal::Convert420spto422sp(uint8_t* in, uint8_t* out, int32_t width, int32_t height)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	if((in == NULL) || (out == NULL)) return false;
	int frameSize = width * height;
	memcpy(out, in, frameSize);
	for(int i = 0; i < height/2; i++)
	{
		memcpy(out + width *(height + i * 2), in + width *(height + i), width);
		if(i < height/2 - 1){
			for(int j = 0; j < width; j++)
			{
				*(out +  width *(height + i * 2 + 1) + j) = (*(in + width * (height + i) + j) >> 1 ) + (*(in + width * (height + i + 1) + j) >> 1);			
			}
		}
		else{
			for(int k = 0; k < width; k++)
			{
				*(out +  width *(height + i * 2 + 1) + k) = (*(in + width * (height + i) + k) - (*(in + width * (height + i - 1) + k) >> 1)) << 1;			
			}
		}
	}
	return true;
}

//convert YUV420SP_NV12 to YUV420SP_NV21
bool CameraHal::ConvertNV12toNV21(uint8_t* in, uint8_t* out, int32_t width, int32_t height)
{
	//LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	if(in == NULL) return false;

	const int y_size = width * height;
	const int uv_size = (width * height) >> 1;
	char *vu;
	char *u;
	char *v;
	char temp;
	int h = 0;
	
	if(out != NULL)
	{
		memcpy(out, in, y_size);
		vu = ((char *)out) + width * height;
		u = ((char *)in) + width * height;
		v = u + 1;

		while (h < width * height / 4) { 
			*vu++ = *v;
			v = v + 2;
			*vu++ = *u;
			u = u + 2;
			++h; 
		}    
	}
	else
	{
		vu = ((char *)in) + width * height;
		while (h < width * height / 4) { 
			temp = *vu;
			*vu = *(vu + 1);
			vu++;
			*vu++ = temp;
			++h; 
		}
	}

	//LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return true;
}

/*
 * This funtion swap yuv420sp format's R and B. (If RB is not right, your face may look like Avender)
 * Cause Infotm gpu solution use reverse RB 
 * Our system funtion for convert yuv to RGB make a reverse.
 * QQ video call might use our system funtion converting color format , so we cant get
 * right color on pc or other pad.
 *
 * This is a work round solution.
 * */
bool CameraHal::Yuv420spRBswap( uint8_t* in, uint8_t* out, int32_t width, int32_t height)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	/*
	 * here is  the transfer matrix 
	 * |1, -0.25,    0.315|   |inY1    |   |outY2    |
	 * |0,  0.183,   1.084| * |inU1-128| = |outU2-128|
	 * |0, 0.936,   -0.182|   |inV1-128|   |outV2-128| 
	 * so we use it swap RB for yuv420sp color format*/
	if((in == NULL) || (out == NULL)) return false;

	long iny,iny1,inyy,inyy1, inu,inv ,outy, outu, outv , temp;//

	uint32_t wi2,j2,hw ,wi ,j2p1;//width*i*2,j*2,height*width,width*i,j*2 + 1

	int temp1 = 0;//for overflow protect.

	hw = height*width;

	for(int i=0; i < height/2 ;i++)
	{
		wi2 = width*i*2;

		wi = width*i;
		for(int j=0; j < width/2;j++)
		{
			j2 = j*2;
			j2p1 = j2+1;

			iny = in[wi2 + j2];
			iny1 = in[wi2 + j2p1];
			inyy = in[wi2+j2+width];
			inyy1 = in[wi2 +j2p1+width];

			inu = in[hw + wi + j2];
			inv = in[hw + wi + j2p1];

			temp = inv*323 - inu*256 - 8520;

			temp1 = ((iny<<10) + temp)>>10;
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[wi2 + j2] = (uint8_t)temp1;

			temp1 = ((iny1<<10) + temp)>>10;
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[wi2 + j2p1] = (uint8_t)temp1;

			temp1 = ((inyy<<10) + temp)>>10;
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[wi2 + j2+width] = (uint8_t)temp1;

			temp1 = ((inyy1<<10) + temp)>>10;
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[wi2 + j2p1 +width] = (uint8_t)temp1;


			temp1 = ((inu*187 + inv*1059 - 28443)>>10);
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[hw + wi + j2] = (uint8_t)temp1;

			temp1 = ((inu*958 - inv*186 +32244)>>10);
			if(temp1>255) temp1=255;
			if(temp1<0) temp1=0;
			out[hw + wi +j2p1] = (uint8_t)temp1;
		}
	}
	return true;
}

bool CameraHal::getBuffersFromWindow()
{
	int undequeued = 0;
	status_t Err = NO_ERROR;
	// Set gralloc usage bits for window.
	Err = mPreviewStreamOps->set_usage(mPreviewStreamOps, CAMHAL_GRALLOC_USAGE | GRALLOC_USAGE_INFOTM_LINEAR);
	if(Err != 0){
		ALOGE("set_usage Err = %d", Err);
		if ( ENODEV == Err ) {
			ALOGE("Preview surface abandoned!");
			mPreviewStreamOps = NULL;
		}
		return false;
	}

	///Set the number of buffers needed for camera preview
	Err = mPreviewStreamOps->set_buffer_count(mPreviewStreamOps, mBufferCount);
	if(Err != 0){
		ALOGE("set_buffer_count Err = %d", Err);
		if ( ENODEV == Err ) {
			ALOGE("Preview surface abandoned!");
			mPreviewStreamOps = NULL;
		}
		return false;

	}
	// Set window geometry
#if(DISPLAY_FMT == CAM_PIXFMT_YUV420SP)
	Err = mPreviewStreamOps->set_buffers_geometry(
			mPreviewStreamOps,
			r_pre_width,
			r_pre_height,
			/*toOMXPixFormat(format)*/HAL_PIXEL_FORMAT_YCrCb_420_SP); 
#elif(DISPLAY_FMT == CAM_PIXFMT_16BPP_RGB565)

	Err = mPreviewStreamOps->set_buffers_geometry(
			mPreviewStreamOps,
			r_pre_width,
			r_pre_height,
			/*toOMXPixFormat(format)*/HAL_PIXEL_FORMAT_RGB_565); 
#else	//CAM_PIXFMT_32BPP_BGR0888
	Err = mPreviewStreamOps->set_buffers_geometry(
			mPreviewStreamOps,
			r_pre_width,
			r_pre_height,
			/*toOMXPixFormat(format)*/HAL_PIXEL_FORMAT_RGBX_8888); 
#endif
	if(Err != 0){
		ALOGE("set_buffer_count Err = %d", Err);
		if ( ENODEV == Err ) {
			ALOGE("Preview surface abandoned!");
			mPreviewStreamOps = NULL;
		}
		return false;
	}

	mPreviewStreamOps->get_min_undequeued_buffer_count(mPreviewStreamOps, &undequeued);

	return true;
}



//TODO,we should return more detail value to our caller
status_t CameraHal::previewThread()
{
	//LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);

#ifdef CHECK_TTIME_DELAY
	gettimeofday(&tv2, NULL);
	ALOGE("time delay = %lld, line = %d", (long)(tv2.tv_sec - tv1.tv_sec) * 1000000LL + (long)(tv2.tv_usec - tv1.tv_usec), __LINE__);
	memcpy(&tv1, &tv2, sizeof(struct timeval));
#endif

	status_t err = NO_ERROR;
	//if mDataCbTimestamp called sei mDatasent true
	bool mDataSent = false;
	uint64_t timeStemp =0;
	frame_with_info frame1, frame2, frame3;
	memset(&frame1, 0, sizeof(frame_with_info));
	memset(&frame2, 0, sizeof(frame_with_info));
	memset(&frame3, 0, sizeof(frame_with_info));
	int source_width, source_height;
	if(!calcWidthHeight(mPreviewCfg.res, &source_width, &source_height))
	{	
		LOGMSG(DBGERR, "%s calcWidthHeigh()  failed line = %d", ERRHEAD, __LINE__);
		return BAD_VALUE;
	}
	if(!calcImage(&frame1.image, mPreviewCfg.fmt, source_width, source_height))
	{	
		LOGMSG(DBGERR, "%s calcWidthHeigh()  failed line = %d", ERRHEAD, __LINE__);
		return BAD_VALUE;
	}

	IM_RET ret;

	uint8_t* displayframe = (uint8_t*)mPreviewBufs[mCurrentPreviewFrame];

	ret = cam_get_preview_frame(mCamctx, &frame1.frame);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_get_preview_frame() failed, ret=%d", ERRHEAD, ret);
		stopPreview();
		mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_UNKNOWN, 0, mCallbackCookie); 
		return UNKNOWN_ERROR;
	}

	timeStemp = systemTime();

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	/*
	 * when sensor started, drop frames at beginning for these frams may be bad
	 * So same when taking pictures*/
	if(mDropFrame > 0) {
		if(mUseEncodeSruct ){
			memset(&mRecordingFrame[mDropFrame-1], 0, sizeof(cam_frame_t));
			memcpy(&mRecordingFrame[mDropFrame-1], &frame1.frame, sizeof(cam_frame_t));
			if(mRecordingMemory){
				mEncodeIn[mDropFrame-1].vir_addr = mRecordingFrame[mDropFrame-1].buffer.vir_addr;
				mEncodeIn[mDropFrame-1].phy_addr = mRecordingFrame[mDropFrame-1].buffer.phy_addr;
				mEncodeIn[mDropFrame-1].size = mRecordingFrame[mDropFrame-1].buffer.size;
			}
		}
		ret = cam_release_preview_frame(mCamctx, &frame1.frame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_release_preview_frame() failed, ret=%d line = %d", ERRHEAD, ret, __LINE__);
		}
		if(mDropFrame == 1)
			mBufferCopied = true;
		else
			mBufferCopied = false;
		mDropFrame--;
		usleep(10000);
		return NO_ERROR;
	}
	//recording datacallback
	if(mRecordEna){
		int i=0;
		if(mUseEncodeSruct ){
			for(i = 0; i < pre_buffer_property.minNumber + 2; i++)
			{
				if((mRecordingFrame[i].buffer.vir_addr== frame1.frame.buffer.vir_addr)&&
						(mRecordingFrame[i].buffer.phy_addr ==(int)frame1.frame.buffer.phy_addr))
				{
					//convert source data
					if(mNeedResizeForVideo)
					{
						memcpy(&frame3, &frame1, sizeof(frame1));
						frame3.frame.buffer.vir_addr = mVideoResize[i].vir_addr; 
						frame3.frame.buffer.phy_addr = mVideoResize[i].phy_addr;
						int format = r_pre_fmt;

						if(!calcImage(&frame3.image, format, r_vid_width, r_vid_height)){
							err = UNKNOWN_ERROR;
							goto EXIT;
						}

						int a_size, a_res;
						if(!calcRes(&a_res, r_vid_width, r_vid_height))
						{	
							LOGMSG(DBGERR, "%s calcRes()  failed line = %d", ERRHEAD, __LINE__);
							return BAD_VALUE;
						}
						if(!calcBuffersize(a_res, r_pre_fmt, &a_size))
						{
							LOGMSG(DBGERR, "%s calcBuffersize()  failed line = %d", ERRHEAD, __LINE__);
							return BAD_VALUE;
						}
						frame3.frame.dataSize = a_size;

						if(!ConvertFrame(&frame1, &frame3, 0, 0))
						{
							LOGMSG(DBGERR, "%s ConvertFrame failed! line = %d", ERRHEAD, __LINE__);
							err = UNKNOWN_ERROR;
							goto EXIT;
						}
					}
					uint8_t* recordingframe = (uint8_t*)mRecordingBufs[i];
					memcpy(recordingframe, (uint8_t *)&mEncodeIn[i], sizeof(InfotmMetaData::VideoEncInMediaBufferDataType));
					//here the time ns(1s/1000 000 000)
					mLock.unlock();
					mDataCbTimestamp(timeStemp, CAMERA_MSG_VIDEO_FRAME, mRecordingMemory, i, mCallbackCookie);            mDataSent = true;
					mLock.lock();
					break;
				}
			}
			if (i == pre_buffer_property.minNumber + 2)
			{
				LOGMSG(DBGERR, "%s cant match buffer address", ERRHEAD);
				err = UNKNOWN_ERROR;
				goto EXIT;
			}
		}
		else
		{
			for(i = 0; i < pre_buffer_property.minNumber + 2; i++)
			{
				if(!mBufferBusy[i]){
					//convert source data
					if(mNeedResizeForVideo)
					{
						memcpy(&frame3, &frame1, sizeof(frame1));
						frame3.frame.buffer.vir_addr = mVideoResize[i].vir_addr; 
						frame3.frame.buffer.phy_addr = mVideoResize[i].phy_addr;
						int format = r_pre_fmt;

						if(!calcImage(&frame3.image, format, r_vid_width, r_vid_height)){
							err = UNKNOWN_ERROR;
							goto EXIT;
						}

						int a_size, a_res;
						if(!calcRes(&a_res, r_vid_width, r_vid_height))
						{	
							LOGMSG(DBGERR, "%s calcRes()  failed line = %d", ERRHEAD, __LINE__);
							return BAD_VALUE;
						}
						if(!calcBuffersize(a_res, r_pre_fmt, &a_size))
						{
							LOGMSG(DBGERR, "%s calcBuffersize()  failed line = %d", ERRHEAD, __LINE__);
							return BAD_VALUE;
						}
						frame3.frame.dataSize = a_size;

						if(!ConvertFrame(&frame1, &frame3, 0, 0))
						{
							LOGMSG(DBGERR, "%s ConvertFrame failed! line = %d", ERRHEAD, __LINE__);
							err = UNKNOWN_ERROR;
							goto EXIT;
						}
					}
					if(mMsgEnabled & CAMERA_MSG_VIDEO_FRAME){
						uint8_t* recordingframe = (uint8_t*)mRecordingBufs[i];
						memcpy(recordingframe, mNeedResizeForVideo? (uint8_t *)frame3.frame.buffer.vir_addr:(uint8_t *)frame1.frame.buffer.vir_addr,
								mNeedResizeForVideo? frame3.frame.dataSize:frame1.frame.dataSize);
						{
							Mutex::Autolock lock(mCodecBufferLck);
							mBufferBusy[i] = true;
						}
						mLock.unlock();
						mDataCbTimestamp(timeStemp, CAMERA_MSG_VIDEO_FRAME, mRecordingMemory, i, mCallbackCookie); 
						mLock.lock();
					}
					break;
				}
			}
			if(i == pre_buffer_property.minNumber + 2){
				LOGMSG(DBGWARN, "%s record buf is busy", WARNHEAD);
				err = UNKNOWN_ERROR;
				goto EXIT;
			}
		}
	}

	if(displayframe != 0)
	{
		//convert source data
		if(mNeedResize || (r_pre_fmt !=(int) mPreviewCfg.fmt))
		{
			memcpy(&frame2, &frame1, sizeof(frame1));
			frame2.frame.buffer.vir_addr = mPreviewBuff.vir_addr; 
			frame2.frame.buffer.phy_addr = mPreviewBuff.phy_addr;
			int format = r_pre_fmt;

#if 0	//it also not rotate raw data for preview callback
			if(mRotation % 180)
			{
				if(!calcImage(&frame2.image, format, r_pre_height, r_pre_width)){
					err = UNKNOWN_ERROR;
					goto EXIT;
				}
			}
			else
			{
				if(!calcImage(&frame2.image, format, r_pre_width, r_pre_height)){
					err = UNKNOWN_ERROR;
					goto EXIT;
				}
			}
#else
			if(!calcImage(&frame2.image, format, r_pre_width, r_pre_height)){
				err = UNKNOWN_ERROR;
				goto EXIT;
			}
#endif

			int a_size, a_res;
			if(!calcRes(&a_res, r_pre_width, r_pre_height))
			{	
				LOGMSG(DBGERR, "%s calcRes()  failed line = %d", ERRHEAD, __LINE__);
				return BAD_VALUE;
			}
			if(!calcBuffersize(a_res, r_pre_fmt, &a_size))
			{
				LOGMSG(DBGERR, "%s calcBuffersize()  failed line = %d", ERRHEAD, __LINE__);
				return BAD_VALUE;
			}
			frame2.frame.dataSize = a_size;

			if(!ConvertFrame(&frame1, &frame2, mRotation, 1))
			{
				LOGMSG(DBGERR, "%s ConvertFrame failed! line = %d", ERRHEAD, __LINE__);
				err = UNKNOWN_ERROR;
				goto EXIT;
			}
		}
		//preview data callback
		if(mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME){
			if(mNeedResize){
				if(r_pre_fmt == CAM_PIXFMT_YUV420SP){
					//if(!Yuv420spRBswap((uint8_t *)frame2.frame.buffer.vir_addr, displayframe,  frame2.image.width, frame2.image.height))  
					if(!ConvertNV12toNV21((uint8_t *)frame2.frame.buffer.vir_addr, displayframe, frame2.image.width, frame2.image.height))
					{  
						err = UNKNOWN_ERROR;
						goto EXIT;
					}
					//	Convert420spto422sp((uint8_t *)frame2.buff.vir_addr, pointer,  frame2.image.width, frame2.image.height);
				}
				else{
					memcpy(displayframe, (uint8_t *)frame2.frame.buffer.vir_addr, frame2.frame.dataSize);
				}
			}
			else{
				if(r_pre_fmt == CAM_PIXFMT_YUV420SP){
					//if(!Yuv420spRBswap((uint8_t *)frame1.frame.buffer.vir_addr, displayframe,  frame1.image.width, frame1.image.height))
					if(!ConvertNV12toNV21((uint8_t *)frame1.frame.buffer.vir_addr, displayframe, frame1.image.width, frame1.image.height))
					{  
						err = UNKNOWN_ERROR;
						goto EXIT;
					}
				}
				else{
					memcpy(displayframe, (uint8_t *)frame1.frame.buffer.vir_addr, frame1.frame.dataSize);
				}
			}
			mLock.unlock();
			mDataCb(CAMERA_MSG_PREVIEW_FRAME, mPreviewMemory, mCurrentPreviewFrame, NULL, mCallbackCookie); 
			mLock.lock();
			mCurrentPreviewFrame = (mCurrentPreviewFrame + 1)%mPreviewBufferNumber;
		}

#ifdef CHECK_TTIME_DELAY
		gettimeofday(&tv3, NULL);
		ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
		//face detection
		if(mFaceDT&&(!mFaceDetecting))
		{
			if(mMsgEnabled & CAMERA_MSG_PREVIEW_METADATA){
				if(!(mFaceCounter % 10))
				{
					//convert preview frame to QVGA 420sp
					frame_with_info  faceframe;
					memcpy(&faceframe, &frame1, sizeof(frame_with_info));
					faceframe.frame.buffer.vir_addr = mFaceBuff.vir_addr; 
					faceframe.frame.buffer.phy_addr = mFaceBuff.phy_addr;

					if(!calcImage(&faceframe.image, CAM_PIXFMT_YUV420SP, 320, 240))
					{
						LOGMSG(DBGERR, "%s calcImage() failed! line = %d", ERRHEAD, __LINE__);
						return BAD_VALUE;
					}
					faceframe.frame.buffer.size = faceframe.image.width * faceframe.image.height * 3 / 2;

					faceframe.frame.dataSize = faceframe.frame.buffer.size;

					if(!ConvertFrame(&frame1, &faceframe, 0, 0))
					{
						LOGMSG(DBGERR, "%s ConvertFrame failed! line = %d", ERRHEAD, __LINE__);
						err = UNKNOWN_ERROR;
						goto EXIT;
					}

					mFaceSrc.format = IM_PIC_FMT_12BITS_YUV420SP;
					mFaceSrc.width = 320;
					mFaceSrc.height = 240;
					mFaceSrc.buffer[0].vir_addr = faceframe.frame.buffer.vir_addr;
					mFaceSrc.buffer[0].phy_addr = faceframe.frame.buffer.phy_addr;
					mFaceSrc.buffer[0].flag = faceframe.frame.buffer.flag;
					mFaceSrc.buffer[0].size = 320 * 240;
					mFaceSrc.buffer[1].vir_addr =(void*)((int *)faceframe.frame.buffer.vir_addr + 320*240);
					mFaceSrc.buffer[1].phy_addr = faceframe.frame.buffer.phy_addr + 320*240;
					mFaceSrc.buffer[1].flag = faceframe.frame.buffer.flag;
					mFaceSrc.buffer[1].size = 320 * 240 / 2;

					if (createThread(BeginFaceDTThread, this) == false){
						LOGMSG(DBGERR, "%s CameraHal::takePicture(), createThread() failed!", ERRHEAD);
						err = UNKNOWN_ERROR;
						goto EXIT;
					}
				}
			}
			mFaceCounter++;
		}

#ifdef CHECK_TTIME_DELAY
		gettimeofday(&tv3, NULL);
		ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
		//for display
		if(mPreviewStreamOps){
			int i = -1;
			status_t Err;
			Rect bounds;
			if(mAllocBuffer){
				mBufferCount = 4; //TODO, set the number
				if(!getBuffersFromWindow()){
					ALOGE("AllocatePreviewBuffer() failed!\n");
					err = UNKNOWN_ERROR;
					goto EXIT;
				}
				mAllocBuffer = false;
			}
#ifdef CHECK_TTIME_DELAY
			gettimeofday(&tv3, NULL);
			ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
			private_handle_t **hndl2hndl = NULL;
			buffer_handle_t* buf = NULL;
			private_handle_t *hnd = NULL;
			int stride =0;  // dummy variable to get stride
			void *rgbdata = NULL;
			int lock_try_count = 0;
			GraphicBufferMapper &mapper = GraphicBufferMapper::get();
			Err = mPreviewStreamOps->dequeue_buffer(mPreviewStreamOps,  \
					(buffer_handle_t **)&hndl2hndl, &stride);
			if (Err != 0) {
				ALOGE("dequeueBuffer failed:  (%d)",-Err);
				err = UNKNOWN_ERROR;
				if ( ENODEV == Err ) {
					ALOGE("Preview surface abandoned!");
					mPreviewStreamOps = NULL;
				}
				goto EXIT;
			}
			//TODO,we should check the displayer buffer(get from ANW)'s stride 
			IM_ASSERT(r_pre_width == stride);
			hnd = *hndl2hndl;
			Err = mPreviewStreamOps->lock_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
			if (Err != 0) {
				ALOGE("lockbuffer failed:  (%d)",  -Err);
				err = UNKNOWN_ERROR;
				if ( ENODEV == Err ) {
					ALOGE("Preview surface abandoned!");
					mPreviewStreamOps = NULL;
				}
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}

			bounds.left = 0;
			bounds.top = 0;
			bounds.right = r_pre_width;
			bounds.bottom = r_pre_height;


#ifdef CHECK_TTIME_DELAY
			gettimeofday(&tv3, NULL);
			ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
			while (mapper.lock((buffer_handle_t)hnd,CAMHAL_GRALLOC_USAGE, bounds, &rgbdata) < 0){
				if (++lock_try_count > 5){
					mLock.unlock();
					mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_UNKNOWN, 0, mCallbackCookie); 
					mLock.lock();
					err = UNKNOWN_ERROR;
					mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
					goto EXIT;
				}
				ALOGE("Gralloc Lock FrameReturn Error: Sleeping 15ms");
				usleep(15000);
			}
#ifdef CHECK_TTIME_DELAY
			gettimeofday(&tv3, NULL);
			ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif

			//************color convert begin

			frame_with_info frame4;
			memset(&frame4,0,sizeof(frame_with_info));
			memcpy(&frame4, &frame1, sizeof(frame_with_info));


			frame4.frame.buffer.vir_addr = rgbdata; 
			frame4.frame.buffer.phy_addr = hnd->phy_addr;

#if(DISPLAY_FMT == CAM_PIXFMT_YUV420SP)
			//display color format: yuv420sp
			if(!calcImage(&frame4.image, CAM_PIXFMT_YUV420SP, r_pre_width, r_pre_height)){
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}
			frame4.frame.buffer.size = (frame4.image.width * frame4.image.height * 3) >> 1;

			frame4.frame.dataSize = frame4.frame.buffer.size;
#elif(DISPLAY_FMT == CAM_PIXFMT_16BPP_RGB565)
			//display color format: rgb565
			if(!calcImage(&frame4.image, CAM_PIXFMT_16BPP_RGB565, r_pre_width, r_pre_height)){
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}
			frame4.frame.buffer.size = frame4.image.width * frame4.image.height << 1;

			frame4.frame.dataSize = frame4.frame.buffer.size;
#else	//(DISPLAY_FMT == CAM_PIXFMT_32BPP_BGR0888)
			//display color format: bgr0888
			if(!calcImage(&frame4.image, CAM_PIXFMT_32BPP_BGR0888, r_pre_width, r_pre_height)){
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}

			frame4.frame.buffer.size = frame4.image.width * frame4.image.height << 2;

			frame4.frame.dataSize = frame4.frame.buffer.size;
#endif

			if(!ConvertFrame(&frame1, &frame4, mRotation, 0))
			{
				LOGMSG(DBGERR, "%s ConvertFrame failed!, line = %d", ERRHEAD, __LINE__);
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}
#if(DISPLAY_FMT == CAM_PIXFMT_YUV420SP)
			if(!ConvertNV12toNV21((uint8_t *)frame4.frame.buffer.vir_addr, NULL, frame4.image.width, frame4.image.height))
			{
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}
#endif
			//**********color convert end

#ifdef CHECK_TTIME_DELAY
			gettimeofday(&tv3, NULL);
			ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
			mapper.unlock((buffer_handle_t)hnd);
			//set timeStemp 
			mPreviewStreamOps->set_timestamp(mPreviewStreamOps, timeStemp);
			Err = mPreviewStreamOps->enqueue_buffer(mPreviewStreamOps,(buffer_handle_t *)hndl2hndl);
			if (Err != 0) {
				ALOGE("Surface::queueBuffer returned error %d", Err);
				err = UNKNOWN_ERROR;
				mPreviewStreamOps->cancel_buffer(mPreviewStreamOps, (buffer_handle_t *)hndl2hndl);
				goto EXIT;
			}
#ifdef CHECK_TTIME_DELAY
			gettimeofday(&tv3, NULL);
			ALOGE("time delay = %lld, line = %d", (long)(tv3.tv_sec - tv2.tv_sec) * 1000000LL + (long)(tv3.tv_usec - tv2.tv_usec), __LINE__);
#endif
		}

	}

	err = NO_ERROR;
EXIT:
	if(mRecordEna){
		if(!mDataSent){
			ret = cam_release_preview_frame(mCamctx, &frame1.frame);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_release_preview_frame() failed, ret=%d, line = %d", ERRHEAD, ret, __LINE__);
			}
		}
	}
	else{
		ret = cam_release_preview_frame(mCamctx, &frame1.frame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_release_preview_frame() failed, ret=%d line = %d", ERRHEAD, ret, __LINE__);
		}
	}
	if(err != NO_ERROR){
		//mLock.unlock();
		//mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_UKNOWN, 0, mCallbackCookie);
		//mLock.lock();
	}

	//LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return err;		
}

status_t CameraHal::StartPreviewInternel()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	IM_RET ret;

	int i;
	//Mutex::Autolock lock(mThreadLock);

	gettimeofday(&tv1, NULL);
	//check size, make sure whether we need resize by pp
	if(!checkSize(mPreviewCfg.resBits, r_pre_width, r_pre_height))
	{
		LOGMSG(DBGINFO, "%s preview resBits dont contain requare width and height", INFOHEAD);
		mNeedResize = true;
	}
	else
	{
		mNeedResize = false;
		//check whether requare size eq current cconfig
		if((!checkSize(mPreviewCfg.res, r_pre_width, r_pre_height)) || ((uint32_t)r_pre_fmt != mPreviewCfg.fmt))
		{
			LOGMSG(DBGINFO, "%s set preview width and height", INFOHEAD);
			int a_res;
			if (!calcRes(&a_res, r_pre_width, r_pre_height))
			{
				LOGMSG(DBGERR, "%s calcRes()  failed, line =%d", ERRHEAD, __LINE__);
				return UNKNOWN_ERROR;
			}
			ret = cam_set_preview_config(mCamctx,  a_res, r_pre_fmt, mPreviewCfg.fps);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_preview_config()  failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
			ret = cam_get_preview_configs(mCamctx, &mPreviewCfg);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_get_preview_config()  failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
			if(mNeedreoganize)
			{
				ret = redir_cam_get_picture_configs(mCamctx, &mPictureCfg);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s cam_get_picture_configs() failed, ret=%d", ERRHEAD, ret);
					return UNKNOWN_ERROR;
				}
			}
		}
		else{
			LOGMSG(DBGINFO, "%s no need to set preview config", INFOHEAD);
		}
	}

	switch (r_pre_fmt)
	{
		case CAM_PIXFMT_YUV420SP:
		case CAM_PIXFMT_YUV420P:
			mFrameSize = r_pre_width * r_pre_height * 3 >> 1;
			break;
		case CAM_PIXFMT_YUV422I:
			mFrameSize = r_pre_width * r_pre_height << 1;
			break;
		case CAM_PIXFMT_32BPP_RGB0888:
			mFrameSize = r_pre_width * r_pre_height << 2;
			break;
		default:
			LOGMSG(DBGERR, "%s format error!,r_pre_fmt=%d  line = %d", ERRHEAD,r_pre_fmt, __LINE__);
			return UNKNOWN_ERROR;
	}

	//allocbuffer
	if(!mPreviewMemory){
		mPreviewMemory = mGet_memory(-1, mFrameSize, mPreviewBufferNumber, NULL); 
		if(mPreviewMemory == NULL){
			ALOGE("failed to get memory for preview mem!\n");
			return UNKNOWN_ERROR;
		}
		// Make an IMemory for each frame so that we can reuse them in callbacks.
		for (int i = 0; i < mPreviewBufferNumber; i++) {
			mPreviewBufs[i] = (unsigned char*)mPreviewMemory->data + i * mFrameSize;
		}
	}

	// start camera pareview path.
	if(!AssignBuffer(CAM_PATH_PREVIEW)){
		LOGMSG(DBGERR, "%s AssignBuffer() failed", ERRHEAD);
		goto Fail;
	}

	ret = cam_enable_stream(mCamctx, CAM_PATH_PREVIEW);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_enable_path() failed, ret=%d", ERRHEAD, ret);
		goto Fail;
	}

	mDropFrame = pre_buffer_property.minNumber + 2;
	mAllocBuffer = true;

	mPreviewThread = new PreviewThread(this);

	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return NO_ERROR;

Fail:
	ReleaseBuffer(CAM_PATH_PREVIEW);
	cam_disable_stream(mCamctx, CAM_PATH_PREVIEW);
	return UNKNOWN_ERROR;
}

int CameraHal::setPreviewWindow(struct preview_stream_ops *window)
{
	LOGMSG(DBGINFO, "%s %s(window=0x%x)", INFOHEAD, __FUNCTION__, (int)window);
	Mutex::Autolock lock(mLock);
	if(window){
		mPreviewStreamOps = window;
	}
	else{
		returnBuffersToWindow();
		mPreviewStreamOps = NULL;
	}
	return NO_ERROR;
}

status_t CameraHal::startPreview()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	//make sure picThread exit before enter preThread
	LOCK_IN();
	gPicThrdLock.lock();
	while(gPicThrdRuning == true){
		gPicThrdLock.unlock();
		//LOGMSG(DBGERR, "%s wait pic thread exit!", ERRHEAD);
		usleep(20);
		gPicThrdLock.lock();
	}
	gPicThrdLock.unlock();
	LOCK_OUT();

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	if(mPreviewThread != 0) {
		LOGMSG(DBGERR, "%s CameraHal::startPreview(), has been started", ERRHEAD);
		return INVALID_OPERATION;
	}
	//return StartPreviewInternel();
	status_t ret = StartPreviewInternel();
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return ret;
}

void CameraHal::StopPreviewInternel()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	sp<PreviewThread>  previewThread;
	{
		Mutex::Autolock lock(mLock);
		previewThread = mPreviewThread;
	}

	if (previewThread != 0) {
		previewThread->requestExitAndWait();
		ret = cam_disable_stream(mCamctx, CAM_PATH_PREVIEW);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_disable_path() failed, ret=%d", ERRHEAD, ret);
		}
		ReleaseBuffer(CAM_PATH_PREVIEW);
		if(mPreviewMemory){
			mPreviewMemory->release(mPreviewMemory);
			memset(mPreviewBufs,0,mPreviewBufferNumber);
			mPreviewMemory = NULL;
		}
		//mPreviewHeap = NULL;
	}

	mPreviewThread.clear();
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
}

void CameraHal::stopPreview()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	mFaceDT = false;
	StopPreviewInternel();
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
}

bool CameraHal::previewEnabled()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	return mPreviewThread != 0;
}

status_t CameraHal::StartRecordInternel()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	IM_RET ret;

	int aFrameSize;
	bool needReleaseBuf =  false;
	switch (mVideoFrameFormat/*mPreviewCfg.fmt*/)
	{
		case CAM_PIXFMT_YUV420SP:
		case CAM_PIXFMT_YUV420P:
			aFrameSize = r_vid_width * r_vid_height * 3 >> 1;
			break;
		case CAM_PIXFMT_YUV422I:
		case CAM_PIXFMT_YUV422SP:
			aFrameSize = r_vid_width * r_vid_height << 1;
			break;
		case CAM_PIXFMT_32BPP_RGB0888:
			aFrameSize = r_vid_width * r_vid_height << 2;
			break;
		default:
			LOGMSG(DBGERR, "%s format error!, line = %d", ERRHEAD, __LINE__);
			return UNKNOWN_ERROR;
	}
	if(aFrameSize != mFrameSize)
	{
		mFrameSize = aFrameSize;
		needReleaseBuf = true;
	}

	if(!checkSize(mPreviewCfg.res, r_vid_width, r_vid_height))
	{
		LOGMSG(DBGWARN, "%s video  frame should resize from  preview frame(w(%d), h(%d)) ", WARNHEAD, r_vid_width, r_vid_height);
		mNeedResizeForVideo = true;
	}
	else
	{
		mNeedResizeForVideo = false;
	}

	//alloc buffer for video when we need resize
	int flag = ALC_FLAG_PHY_MUST | ALC_FLAG_ALIGN_32BYTES;
	if(mNeedResizeForVideo)
	{
		for (int i = 0; i < pre_buffer_property.minNumber + 2; i++) {
			if(needReleaseBuf){
				if(mVideoResize[i].vir_addr != NULL){
					ret = alc_free(mBuffAlloc, &mVideoResize[i]);
					if(ret != IM_RET_OK){
						LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
					}
					mVideoResize[i].vir_addr = NULL;
				}
			}
			if(mVideoResize[i].vir_addr == NULL){
				ret = alc_alloc(mBuffAlloc, mFrameSize, &mVideoResize[i], flag);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
				}
			}
		}
	}

	if(mUseEncodeSruct ){
		if(mRecordingBufferNumber < pre_buffer_property.minNumber + 2)
		{
			return UNKNOWN_ERROR;
		}
		if(mRecordingMemory != NULL){
			if(mRecordingBufs[1] - mRecordingBufs[0] != sizeof(InfotmMetaData::VideoEncInMediaBufferDataType)){
				mRecordingMemory->release(mRecordingMemory);
				mRecordingMemory = NULL;
			}
		}
		if(mRecordingMemory == NULL){
			mRecordingMemory = mGet_memory(-1, sizeof(InfotmMetaData::VideoEncInMediaBufferDataType), pre_buffer_property.minNumber + 2, NULL); 
			if(mRecordingMemory == NULL){
				ALOGE("mGet_memory failed for recording memmory!\n"); 
				return UNKNOWN_ERROR;
			}
		}
		for (int i = 0; i < pre_buffer_property.minNumber + 2; i++) {
			mRecordingBufs[i] = (unsigned char*)mRecordingMemory->data + i * sizeof(InfotmMetaData::VideoEncInMediaBufferDataType);
			cam_frame_t *pEncodeIn;
			memset(&(mEncodeIn[i]), 0, sizeof(InfotmMetaData::VideoEncInMediaBufferDataType));
			mEncodeIn[i].vir_addr = mNeedResizeForVideo? mVideoResize[i].vir_addr : mRecordingFrame[i].buffer.vir_addr;
			mEncodeIn[i].phy_addr = mNeedResizeForVideo? mVideoResize[i].phy_addr : mRecordingFrame[i].buffer.phy_addr;
			mEncodeIn[i].size = mNeedResizeForVideo? mVideoResize[i].size : mRecordingFrame[i].buffer.size;
		}

	}else
	{
		if(mRecordingMemory != NULL){
			if(needReleaseBuf || (mRecordingBufs[1] - mRecordingBufs[0] != mFrameSize)){
				mRecordingMemory->release(mRecordingMemory);
				mRecordingMemory = NULL;
			}
		}
		if(mRecordingMemory == NULL){
			mRecordingMemory = mGet_memory(-1, mFrameSize, pre_buffer_property.minNumber + 2, NULL); 
			if(mRecordingMemory == NULL){
				ALOGE("mGet_memory failed!\n");
				return UNKNOWN_ERROR;
			}
		}
		// Make an IMemory for each frame so that we can reuse them in callbacks.
		for (int i = 0; i < mRecordingBufferNumber; i++) {
			mRecordingBufs[i] = (unsigned char*)mRecordingMemory->data + i * mFrameSize;
		}
	}

	for (int i = 0; i < mRecordingBufferNumber; i++) {
		mBufferBusy[i] = false;
	}

	mRecordEna = true;
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return NO_ERROR;
}


status_t CameraHal::startRecording()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	for (int i = 0; i < pre_buffer_property.minNumber + 2; i++) {
		mBufferBusy[i] = true;
	}

	if(mPreviewThread == 0)
	{
		StartPreviewInternel();
	}

	while(true){
		if(mBufferCopied)
		{
			return StartRecordInternel();
		}
		else
		{
			mLock.unlock();
			usleep(10000);
			mLock.lock();
		}
	}
}


void CameraHal::stopRecording()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();
	mRecordEna = false;
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);

}

bool CameraHal::recordingEnabled()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	Mutex::Autolock lock(mLock);
	return mRecordEna;
}

void CameraHal::releaseRecordingFrame(const void *opaque)
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);

	int i;
	IM_RET ret;

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();
	if(mUseEncodeSruct ){
		for (i = 0; i < pre_buffer_property.minNumber + 2; i++) {
			if(mRecordingBufs[i] == (unsigned char*)opaque) 
			{
				ret = cam_release_preview_frame(mCamctx, &mRecordingFrame[i]);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s cam_release_preview_frame() failed, ret=%d, line = %d", ERRHEAD, ret, __LINE__);
				}
				break;
			}
		}
		if(i == pre_buffer_property.minNumber + 2)
			LOGMSG(DBGINFO, "%s a invalid value for releaseRecordingFrame", INFOHEAD);
	}
	else
	{
		for (i = 0; i < pre_buffer_property.minNumber + 2; i++) {
			if(mRecordingBufs[i] == (unsigned char*)opaque) 
			{
				mBufferBusy[i] = false;
				break;
			}
		}
		if(i == pre_buffer_property.minNumber + 2)
			LOGMSG(DBGINFO, "%s a invalid value for releaseRecordingFrame", INFOHEAD);
	}
	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
}

int CameraHal::BeginAutoFocusThread(void *cookie)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	CameraHal *c = (CameraHal *)cookie;
	return c->AutoFocusThread();
}

int CameraHal::AutoFocusThread()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	if (mMsgEnabled & CAMERA_MSG_FOCUS)
		mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
	return NO_ERROR;
}

status_t CameraHal::autoFocus()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	Mutex::Autolock lock(mLock);
	if (createThread(BeginAutoFocusThread, this) == false)
		return UNKNOWN_ERROR;	
	return NO_ERROR;
}

status_t CameraHal::cancelAutoFocus()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	return NO_ERROR;
}

bool CameraHal::EncodeJpeg(frame_with_info *pic, int32_t *jfifsize, IM_Buffer *buff, int32_t userBufferSize)
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);

	IM_RET ret;

	mJencCfg.inputFormat = pic->image.type;
	mJencCfg.thumb.data = mThumbBuff.vir_addr;

	ret = jenc_configure(mJencInst, &mJencCfg);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s jenc_configure() failed, ret=%d", ERRHEAD, ret);
		return false;
	}

	mJencParam.inputWidth = pic->image.width;
	mJencParam.inputHeight = pic->image.height >> 4 << 4;
	mJencParam.xOffset = pic->image.xoffset;
	mJencParam.yOffset = pic->image.yoffset;
	mJencParam.codingWidth = pic->image.width;
	mJencParam.codingHeight = pic->image.height >> 4 << 4;
	if(pic->image.type == IM_IMAGE_YUV422SP){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->image.stride * (pic->image.height + pic->image.yoffset);
		mJencParam.buffCb.vir_addr = (void *)((IM_INT32)pic->frame.buffer.vir_addr + mJencParam.buffY.size);
		mJencParam.buffCb.phy_addr = (pic->frame.buffer.phy_addr != 0)?(pic->frame.buffer.phy_addr + mJencParam.buffY.size):0;
		mJencParam.buffCb.size = pic->image.strideCb * (pic->image.height + pic->image.yoffset);
		mJencParam.buffCr.vir_addr = NULL;
		mJencParam.buffCr.phy_addr = 0;
		mJencParam.buffCr.size = 0;
	}else if(pic->image.type == IM_IMAGE_YUV420SP){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->image.stride * (pic->image.height + pic->image.yoffset);
		mJencParam.buffCb.vir_addr = (void *)((IM_INT32)pic->frame.buffer.vir_addr + mJencParam.buffY.size);
		mJencParam.buffCb.phy_addr = (pic->frame.buffer.phy_addr != 0)?(pic->frame.buffer.phy_addr + mJencParam.buffY.size):0;
		mJencParam.buffCb.size = pic->image.strideCb * (pic->image.height + pic->image.yoffset) >> 1;
		mJencParam.buffCr.vir_addr = NULL;
		mJencParam.buffCr.phy_addr = 0;
		mJencParam.buffCr.size = 0;
	}else if(pic->image.type == IM_IMAGE_YUV420P){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->image.stride * (pic->image.height + pic->image.yoffset);
		mJencParam.buffCb.vir_addr = (void *)((IM_INT32)pic->frame.buffer.vir_addr + mJencParam.buffY.size);
		mJencParam.buffCb.phy_addr = (pic->frame.buffer.phy_addr != 0)?(pic->frame.buffer.phy_addr + mJencParam.buffY.size):0;
		mJencParam.buffCb.size = pic->image.strideCb * (pic->image.height + pic->image.yoffset) >> 2;
		mJencParam.buffCr.vir_addr = (void *)((IM_INT32)pic->frame.buffer.vir_addr + mJencParam.buffY.size + mJencParam.buffCb.size);
		mJencParam.buffCr.phy_addr = (pic->frame.buffer.phy_addr != 0)?(pic->frame.buffer.phy_addr + mJencParam.buffY.size + mJencParam.buffCb.size):0;
		mJencParam.buffCr.size = pic->image.strideCb * (pic->image.height + pic->image.yoffset) >> 2;
	}else if(pic->image.type == IM_IMAGE_YUV422I){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->frame.buffer.size;
		mJencParam.buffCb.vir_addr = NULL;
		mJencParam.buffCb.phy_addr = 0;
		mJencParam.buffCb.size = 0;
		mJencParam.buffCr.vir_addr = NULL;
		mJencParam.buffCr.phy_addr = 0;
		mJencParam.buffCr.size = 0;
	}else if(pic->image.type == IM_IMAGE_RGB565){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->frame.buffer.size;
		mJencParam.buffCb.vir_addr = NULL;
		mJencParam.buffCb.phy_addr = 0;
		mJencParam.buffCb.size = 0;
		mJencParam.buffCr.vir_addr = NULL;
		mJencParam.buffCr.phy_addr = 0;
		mJencParam.buffCr.size = 0;
	}else if(pic->image.type == IM_IMAGE_RGB0888
			|| pic->image.type == IM_IMAGE_BGR0888){
		mJencParam.buffY.vir_addr = pic->frame.buffer.vir_addr;
		mJencParam.buffY.phy_addr = pic->frame.buffer.phy_addr;
		mJencParam.buffY.size = pic->frame.buffer.size;
		mJencParam.buffCb.vir_addr = NULL;
		mJencParam.buffCb.phy_addr = 0;
		mJencParam.buffCb.size = 0;
		mJencParam.buffCr.vir_addr = NULL;
		mJencParam.buffCr.phy_addr = 0;
		mJencParam.buffCr.size = 0;
	}else{
		// shouldn't be here.
		LOGMSG(DBGERR, "%s don't support this format %d", ERRHEAD, pic->image.type);
		return false;
	}

	mJencParam.buffOut.vir_addr = buff->vir_addr;
	mJencParam.buffOut.phy_addr = buff->phy_addr;
	mJencParam.buffOut.size = userBufferSize;
	ret = jenc_encode(mJencInst, &mJencParam);


	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s jenc_encode() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	*jfifsize = mJencParam.jfifSize;

	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);
	return true;
}

status_t CameraHal::pictureThread()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	status_t err = NO_ERROR;
	bool mConvert = false;

	int32_t jfifsize = 0;
	int  i = 0;
	int size = 0; 
	int light = 0;
	int jpegHeapSize =0;
	camera_memory_t* rawPicture = NULL;
	camera_memory_t* compressedPicture = NULL;

	frame_with_info pic1, pic2, pic3;
	memset(&pic1, 0, sizeof(frame_with_info));
	memset(&pic2, 0, sizeof(frame_with_info));
	memset(&pic3, 0, sizeof(frame_with_info));

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	if(!calcImage(&pic1.image, r_pic_fmt, r_pic_width, r_pic_height))
	{	
		LOGMSG(DBGERR, "%s calcImage() failed, line=%d", ERRHEAD, __LINE__);
		return UNKNOWN_ERROR;
	}

	//check whether requare size eq current cconfig
	if((!checkSize(mPictureCfg.res, r_pic_width, r_pic_height))  || ((uint32_t)r_pic_fmt != mPictureCfg.fmt))
	{
		LOGMSG(DBGINFO, "%s set preview width and height", INFOHEAD);
		int a_res;
		if (!calcRes(&a_res, r_pic_width, r_pic_height))
		{	
			LOGMSG(DBGERR, "%s calcRes() failed, line=%d", ERRHEAD, __LINE__);
			return UNKNOWN_ERROR;
		}
		ret = redir_cam_set_picture_config(mCamctx, a_res, r_pic_fmt, mPictureCfg.fps);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_set_picture_configs()  failed, ret=%d", ERRHEAD, ret);
			return UNKNOWN_ERROR;
		}
		ret = redir_cam_get_picture_configs(mCamctx, &mPictureCfg);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s redir_cam_get_picture_config()  failed, ret=%d", ERRHEAD, ret);
			return UNKNOWN_ERROR;
		}
		if(mNeedreoganize)
		{
			ret = cam_get_preview_configs(mCamctx, &mPreviewCfg);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_get_picture_configs() failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}
	else{
		LOGMSG(DBGINFO, "%s no need to set picture config", INFOHEAD);
	}

	if(!AssignBuffer(CAM_PATH_PICTURE)){
		LOGMSG(DBGERR, "%s AssignBuffer() failed", ERRHEAD);
		rawPicture = mGet_memory(-1, 4, 1, NULL); 
		if(rawPicture == NULL){
			err = UNKNOWN_ERROR;
			goto Fail;
		}

		if (mMsgEnabled & CAMERA_MSG_SHUTTER){
			mLock.unlock();
			mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
			mLock.lock();
		}

		if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE){
			mLock.unlock();
			mDataCb(CAMERA_MSG_RAW_IMAGE, rawPicture, 0,NULL, mCallbackCookie);
			mLock.lock();
		}
		compressedPicture = mGet_memory(-1, 4 , 1, NULL); 
		if(compressedPicture == NULL){
			err = UNKNOWN_ERROR;
			goto Fail;
		}
		mLock.unlock();
		mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, compressedPicture, 0, NULL, mCallbackCookie);
		mLock.lock();
		err = UNKNOWN_ERROR;
		goto Fail;
	}

	switch(r_pic_fmt)
	{
		case CAM_PIXFMT_YUV422SP:
		case CAM_PIXFMT_YUV422I:
			size = r_pic_width * r_pic_height * 2;
			break;
		case CAM_PIXFMT_YUV420SP:
		case CAM_PIXFMT_YUV420P:
			size = r_pic_width * r_pic_height * 3 / 2;
			break;
		case CAM_PIXFMT_16BPP_RGB565:
			size = r_pic_width * r_pic_height * 2;
			break;
		case CAM_PIXFMT_32BPP_RGB0888:
			size = r_pic_width * r_pic_height * 4;
			break;
		default:
			err = UNKNOWN_ERROR;
			goto Fail;
	}

	rawPicture = mGet_memory(-1, size, 1, NULL); 
	if(rawPicture == NULL){
		err = UNKNOWN_ERROR;
		goto Fail;
	}

	jpegHeapSize = r_pic_width * r_pic_height * 2;// should be enough.

	/*mRawHeap = new MemoryHeapBase(size);
	  if(mRawHeap == NULL){
	  LOGMSG(DBGERR, "%s new MemoryHeapBase(size=%d) failed", ERRHEAD, size);
	  err = -1;
	  goto Fail;
	  }
	  mRawBuffer = new MemoryBase(mRawHeap, 0, size);
	  if(mRawBuffer == NULL){
	  LOGMSG(DBGERR, "%s new MemoryBase() failed", ERRHEAD);
	  err = -1;
	  goto Fail;
	  }*/

	//
	cam_takepic_config_t	prepare_cfg;
	memset(&prepare_cfg, 0, sizeof(cam_takepic_config_t));
	prepare_cfg.needBusAddr = true;
	prepare_cfg.align = IM_BUFFER_ALIGN_32BYTES;
	ret = redir_cam_prepare_take_picture(mCamctx, &prepare_cfg);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_prepare_take_picture() failed, ret=%d", ERRHEAD, ret);
		err = UNKNOWN_ERROR;
		goto Fail;
	}

	mDropFrame = (r_pic_width > 640) ? 4:6;

	//turn on light when need
	if ((mCaps&CAM_CAP_FLASH_MODE_SUPPORT) && mNeedTurnOnLight)
	{
		light = 1;
		ret = cam_set_property(mCamctx, CAM_KEY_RW_LIGHT_TURN_ON, (void *)&(light), sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_LIGHT_TURN_ON) failed, ret=%d", ERRHEAD, ret);
			return UNKNOWN_ERROR;
		}
	}

	while( mDropFrame > 0){
		ret = redir_cam_take_picture(mCamctx, &pic1.frame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_take_picture() failed, ret=%d", ERRHEAD, ret);
			err = UNKNOWN_ERROR;
			goto Fail;
		}
		mDropFrame--;
		if(mDropFrame > 0){
			if(pic1.frame.buffer.vir_addr != IM_NULL){
				ret = redir_cam_release_picture(mCamctx, &pic1.frame);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s 1cam_release_picture() failed, ret=%d", ERRHEAD, ret);
				}
			}
			usleep(30000);
		}

	}

	//turn off light
	if ((mCaps&CAM_CAP_FLASH_MODE_SUPPORT) && mNeedTurnOnLight)
	{
		light = 0;
		ret = cam_set_property(mCamctx, CAM_KEY_RW_LIGHT_TURN_ON, (void *)&(light), sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_LIGHT_TURN_ON) failed, ret=%d", ERRHEAD, ret);
			return UNKNOWN_ERROR;
		}
	}

	memcpy(&pic2, &pic1, sizeof(frame_with_info));
	pic2.frame.buffer.vir_addr = mJpegBuff.vir_addr; 
	pic2.frame.buffer.phy_addr = mJpegBuff.phy_addr;
	pic2.frame.buffer.size = pic1.frame.buffer.size;
	pic2.frame.dataSize = pic2.frame.buffer.size;

#if 0//not need rotate, we only need set rotate value to exif info
	if(mRotation % 180){
		if(!calcImage(&pic2.image, mPictureCfg.fmt, r_pic_height, r_pic_width)){
			err = UNKNOWN_ERROR;
			goto Fail;         
		}
	}
	else{
		if(!calcImage(&pic2.image, mPictureCfg.fmt, r_pic_width, r_pic_height)){
			err = UNKNOWN_ERROR;
			goto Fail;
		}
	}

	if(mRotation != 0){
		if((r_pic_width  > 2048) && (mPictureCfg.fmt == CAM_PIXFMT_YUV420SP))
		{
			if(!RotateYUV420sp((char*)pic1.frame.buffer.vir_addr, (char*)pic2.frame.buffer.vir_addr, mRotation, r_pic_width, r_pic_height))
			{
				LOGMSG(DBGERR, "%s  warning::RotateYUV420sp() failed", ERRHEAD);
				mConvert = false;
			}
			else{
				mConvert = true;
			}
		}
		else{
			//May return error
			if(!ConvertFrame(&pic1, &pic2, mRotation, 2))
			{
				LOGMSG(DBGERR, "%s warning:: ConvertFrame() failed", ERRHEAD);
				mConvert = false;
			}
			else{
				mConvert = true;
			}
		}
	}
#endif

	//thumbnail maybe not need, if you want, you must insert it to exif of main jpeg file, but CTS need
	//convert for thumb
	if(r_thumb_width > 0)
	{
		int32_t thumb_in_size = r_thumb_width * r_thumb_height;
		memcpy(&pic3, &pic1, sizeof(frame_with_info));
		pic3.frame.buffer.vir_addr = mThumbBuff.vir_addr; 
		pic3.frame.buffer.phy_addr = mThumbBuff.phy_addr;
		switch (mPictureCfg.fmt)
		{
			case CAM_PIXFMT_YUV420P:
			case CAM_PIXFMT_YUV420SP:
				thumb_in_size = thumb_in_size * 3 /2; 
				break;
			case CAM_PIXFMT_YUV422I:
			case CAM_PIXFMT_YUV422SP:
			case CAM_PIXFMT_16BPP_RGB565:
				thumb_in_size  *= 2; 
				break;
			case CAM_PIXFMT_32BPP_RGB0888:
				thumb_in_size  *= 4; 
				break;
			default:
				err = UNKNOWN_ERROR;
				goto Fail;         

		}
		pic3.frame.buffer.size = thumb_in_size;
		pic3.frame.dataSize = thumb_in_size;
		if(mRotation % 180){
			if(!calcImage(&pic3.image, mPictureCfg.fmt, r_thumb_height, r_thumb_width)){
				err = UNKNOWN_ERROR;
				goto Fail;         
			}
		}
		else{
			if(!calcImage(&pic3.image, mPictureCfg.fmt, r_thumb_width, r_thumb_height)){
				err = UNKNOWN_ERROR;
				goto Fail;         
			}
		}
		//here we need do rotate and resize, but blt can only do one of the request once, how to do it??
		if(!ConvertFrame(&pic1, &pic3, mRotation, 2))
		{
			LOGMSG(DBGERR, "%s warning:: ConvertFrame() failed", ERRHEAD);
		}
		mJencCfg.thumb.width = 0;
		mJencCfg.thumb.height = 0;
		if( EncodeJpeg(&pic3, &jfifsize, &mJpegOutBuff, mOutbufsize) != true){
			LOGMSG(DBGERR, "%s EncodeJpeg() failed", ERRHEAD);
		}
		memcpy(mThumbBuff.vir_addr, mJpegOutBuff.vir_addr, jfifsize);
		mJencCfg.thumb.dataLength= jfifsize;
		memset(mJpegOutBuff.vir_addr,0,jfifsize);
		jfifsize = 0;
	}

	if (mMsgEnabled & CAMERA_MSG_SHUTTER){
		mLock.unlock();
		mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
		mLock.lock();
	}

	if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE){
		// stop preview, and display shutter, and display captured picture(todo).
		memcpy((uint8_t *)rawPicture->data, pic1.frame.buffer.vir_addr, pic1.frame.dataSize);

		mLock.unlock();
		mDataCb(CAMERA_MSG_RAW_IMAGE, rawPicture, 0,NULL, mCallbackCookie);
		mLock.lock();
	}
	if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY){
		mLock.unlock();
		mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);
		mLock.lock();
	}

	//
	if(mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE){
		mJencCfg.thumb.width = 0;
		mJencCfg.thumb.height = 0;

		if( EncodeJpeg(mConvert? &pic2: &pic1, &jfifsize, &mJpegOutBuff, mOutbufsize) != true){
			LOGMSG(DBGERR, "%s EncodeJpeg() failed", ERRHEAD);
			err = UNKNOWN_ERROR;
			goto Fail;
		}
		mLock.unlock();
		ret = AddExifDoneCb(jfifsize);
		mLock.lock();
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s AddExifDoneCb() fail!, ret=%d", ERRHEAD, ret);
			err = UNKNOWN_ERROR;
			goto Fail;
		}

	}

	if(mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME){
		//mLock.unlock();
		//mDataCb(CAMERA_MSG_POSTVIEW_FRAME, mRawBuffer, mCallbackCookie);
		//mLock.lock();
	}


	if(pic1.frame.buffer.vir_addr != IM_NULL){
		ret = redir_cam_release_picture(mCamctx, &pic1.frame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_release_picture() failed, ret=%d", ERRHEAD, ret);
			err = UNKNOWN_ERROR;
			goto Fail;
		}
	}

	ret = redir_cam_cancel_take_picture(mCamctx);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_cancel_take_picture() failed, ret=%d", ERRHEAD, ret);
		err = UNKNOWN_ERROR;
		goto Fail;
	}
	if(rawPicture)
		rawPicture->release(rawPicture);

	if(compressedPicture)
		compressedPicture->release(compressedPicture);

	ReleaseBuffer(CAM_PATH_PICTURE);

	LOGMSG(DBGINFO, "%s %s()--", INFOHEAD, __FUNCTION__);

	mLock.unlock();

	LOCK_IN();
	gPicThrdLock.lock();
	gPicThrdRuning = false;
	gPicThrdLock.unlock();
	LOCK_OUT();
	return err;

Fail:
	mLock.unlock();
	mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_UNKNOWN, 0, mCallbackCookie);
	mLock.lock();
	if(pic1.frame.buffer.vir_addr != IM_NULL){
		ret = redir_cam_release_picture(mCamctx, &pic1.frame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_release_picture() failed, ret=%d", ERRHEAD, ret);
		}
	}

	if(!ReleaseBuffer(CAM_PATH_PICTURE)){
		LOGMSG(DBGERR, "%s ReleaseBuffer() failed, ret=%d", ERRHEAD, ret);
	}

	if(err != NO_ERROR){
		//mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_UKNOWN, 0, mCallbackCookie);

	}

	if(rawPicture)
		rawPicture->release(rawPicture);

	if(compressedPicture)
		compressedPicture->release(compressedPicture);

	if ((mCaps&CAM_CAP_FLASH_MODE_SUPPORT) && mNeedTurnOnLight){
		light = 0;
		ret = cam_set_property(mCamctx, CAM_KEY_RW_LIGHT_TURN_ON, (void *)&(light), sizeof(int));
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_LIGHT_TURN_ON) failed, ret=%d", ERRHEAD, ret);
			return UNKNOWN_ERROR;
		}
	}

	mLock.unlock();

THREAD_EXIT:	
	LOCK_IN();
	gPicThrdLock.lock();
	gPicThrdRuning = false;
	gPicThrdLock.unlock();
	LOCK_OUT();
	return UNKNOWN_ERROR;
}

int CameraHal::BeginPictureThread(void *cookie)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	CameraHal *c = (CameraHal *)cookie;
	return c->pictureThread();
}

status_t CameraHal::takePicture()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	StopPreviewInternel();
	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	mFaceDT = false;

	if (createThread(BeginPictureThread, this) == false){
		LOGMSG(DBGERR, "%s CameraHal::takePicture(), createThread() failed!", ERRHEAD);
		return UNKNOWN_ERROR;
	}

	LOCK_IN();
	gPicThrdLock.lock();
	gPicThrdRuning = true;
	gPicThrdLock.unlock();
	LOCK_OUT();

	return NO_ERROR;
}

status_t CameraHal::cancelPicture()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	LOCK_IN();
	gPicThrdLock.lock();
	while(gPicThrdRuning == true){
		gPicThrdLock.unlock();
		usleep(20);
		gPicThrdLock.lock();
	}
	gPicThrdLock.unlock();
	LOCK_OUT();

	return NO_ERROR;
}

status_t CameraHal::faceDTThread()
{
	LOGMSG(DBGINFO, "%s %s()++", INFOHEAD, __FUNCTION__);
	mFaceDetecting = true;
	IM_RET ret;
	ret = img_facedet_exec(mImage, &mFaceSrc, &mFaceDes);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s img_facedet_exec() failed, ret=%d", ERRHEAD, ret);
	}

	for(int i = 0; i < mFaceDes.num; i++)
	{

		mFaces[i].rect[1] = mFaceDes.pos[i].row_i * 2000 / 240  - 1000;
		mFaces[i].rect[0] = mFaceDes.pos[i].col_j * 2000 /320 - 1000;
		mFaces[i].rect[3] = (mFaceDes.pos[i].row_i + mFaceDes.pos[i].height) * 2000 / 240 -1000;
		mFaces[i].rect[2] = (mFaceDes.pos[i].col_j + mFaceDes.pos[i].width) * 2000 / 320 - 1000;
		//all below need be improved
		mFaces[i].id = 0;
		mFaces[i].score = 50;
		mFaces[i].left_eye[0] = (3 * mFaces[i].rect[0] +  mFaces[i].rect[2])/4;
		mFaces[i].left_eye[1] = (mFaces[i].rect[1] +  mFaces[i].rect[3])/2;
		mFaces[i].right_eye[0] = (mFaces[i].rect[0] + 3* mFaces[i].rect[2])/4;
		mFaces[i].right_eye[1] = (mFaces[i].rect[1] +  mFaces[i].rect[3])/2;
		mFaces[i].mouth[0] = (mFaces[i].rect[0] +  mFaces[i].rect[2])/2;
		mFaces[i].mouth[1] = (mFaces[i].rect[1] + 3* mFaces[i].rect[3])/4;
	}
	mMetadata.number_of_faces = mFaceDes.num;
	mMetadata.faces = mFaces;

	camera_memory_t *tmpBuffer = mGet_memory(-1, 1, 1, NULL);
	if(tmpBuffer == NULL)
	{
		LOGMSG(DBGERR, "%s mGet_memory() failed, line=%d", ERRHEAD, __LINE__);
		return UNKNOWN_ERROR;
	}
	mLock.unlock();
	mDataCb(CAMERA_MSG_PREVIEW_METADATA, tmpBuffer, 0,  &mMetadata, mCallbackCookie); 
	mLock.lock();
	if(tmpBuffer != NULL)
	{
		tmpBuffer->release(tmpBuffer);
	}

	mFaceDetecting = false;
	return 0;
}

int CameraHal::BeginFaceDTThread(void *cookie)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	CameraHal *c = (CameraHal *)cookie;
	return c->faceDTThread();
}
int CameraHal::AddExifDoneCb(int jfifsize)
{
	IM_RET ret = IM_RET_OK;
	int32_t picSize = 0;
	camera_memory_t* compressedPicture = NULL;
	LOGMSG(DBGINFO, "%s CameraHal::AddExifDoneCb()++", INFOHEAD);

	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();

	ExifElementsTable *exifTable = new ExifElementsTable();
	if(!exifTable){
		LOGMSG(DBGERR, "%s new exifTable fail!", ERRHEAD);
		ret = IM_RET_FAILED;
		goto Exit;
	}

	ret = mExifEn->insertExifJpeg(exifTable,&picSize);	
	LOGMSG(DBGINFO, "%s insertExifJpeg picSize = %d", INFOHEAD, picSize);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s insertExifJpeg failed, ret=%d", ERRHEAD, ret);
		goto Exit;
	}

	if(picSize >0)//creat EXIF success
	{
		compressedPicture = mGet_memory(-1, picSize , 1, NULL); 
		if(compressedPicture && compressedPicture->data)
		{    
			ret = mExifEn->createExifJpeg(exifTable,(int*)compressedPicture->data,picSize);	
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s insertExifJpeg failed, ret=%d", ERRHEAD, ret);
				goto Exit;
			}
		}
		else
		{    
			LOGMSG(DBGERR, "%s mGet_memory fail!", ERRHEAD);
			ret = IM_RET_FAILED;
			goto Exit;

		}
	}
	else if(0 == picSize)
	{    
		compressedPicture = mGet_memory(-1, jfifsize, 1, NULL); 
		if(compressedPicture && compressedPicture->data)
		{    

			memcpy((uint8_t *)compressedPicture->data, mJpegOutBuff.vir_addr, jfifsize);
		}
		else
		{    
			LOGMSG(DBGERR, "%s mGet_memory fail!", ERRHEAD);
			ret = IM_RET_FAILED;
			goto Exit;

		}
	}
	else
	{
		LOGMSG(DBGERR, "%s create EXIF generate error!", ERRHEAD);
		ret = IM_RET_FAILED;
		goto Exit;
	} 
	mLock.unlock();
	mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, compressedPicture, 0, NULL, mCallbackCookie);
	mLock.lock();

Exit:    
	if(exifTable)
		delete exifTable;

	if(compressedPicture)
		compressedPicture->release(compressedPicture);

	LOGMSG(DBGINFO, "%s CameraHal::AddExifDoneCb()--", INFOHEAD);
	return ret;
}
int CameraHal::dump(int fd) const
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	return NO_ERROR;
}

status_t CameraHal::setParameters(const char* params)
{
	LOGMSG(DBGINFO, "%s %s(params=%s)", INFOHEAD, __FUNCTION__, params);
	CameraParameters p;
	String8 str_params(params);
	p.unflatten(str_params);
	return setParameters(p);
}

status_t CameraHal::setParameters(const CameraParameters& params)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	String8 strparams(params.flatten());
	LOGMSG(DBGTIP, "%s setParameters=%s", INFOHEAD, strparams.string());

	CameraParameters lp = params;

	IM_RET ret;

	int    i;

	Mutex::Autolock lock(mLock);

	//set features
	if(mCaps&CAM_CAP_WB_MODE_SUPPORT){
		int a_wb_val  = lookup(wb_map, params.get(CameraParameters::KEY_WHITE_BALANCE), -1);
		if(a_wb_val  < 0)
		{
			LOGMSG(DBGERR, "%s  white ballance parameters invalid", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		int a_wb_val2  = lookup(wb_map, mParameters.get(CameraParameters::KEY_WHITE_BALANCE), -1);
		if(a_wb_val2  < 0)
		{
			LOGMSG(DBGERR, "%s  get a invalid white ballance value", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if(a_wb_val !=  a_wb_val2){
			ret = cam_set_property(mCamctx, CAM_KEY_RW_WB_MODE, (void *)&(a_wb_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_WB_MODE) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}
	if(mCaps&CAM_CAP_SPECIAL_EFFECT_SUPPORT){
		int a_se_val = lookup(se_map, params.get(CameraParameters::KEY_EFFECT), -1);
		if(a_se_val < 0)
		{
			LOGMSG(DBGERR, "%s special effect  parameters invalid", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		int a_se_val2 = lookup(se_map, mParameters.get(CameraParameters::KEY_EFFECT), -1);
		if(a_se_val2  < 0)
		{
			LOGMSG(DBGERR, "%s  get a special effect  invalid value", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if(a_se_val != a_se_val2){
			ret = cam_set_property(mCamctx, CAM_KEY_RW_SPECIAL_EFFECT, (void *)&(a_se_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_SPECIAL_EFFECT_MODE) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}
	if(mCaps&CAM_CAP_SCENE_MODE_SUPPORT)
	{
		int a_scn_val = lookup(scn_map, params.get(CameraParameters::KEY_SCENE_MODE), -1);
		if(a_scn_val < 0)
		{
			LOGMSG(DBGERR, "%s  scene mode parameters invalid", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		int a_scn_val2 = lookup(scn_map, mParameters.get(CameraParameters::KEY_SCENE_MODE), -1);
		if(a_scn_val2  < 0)
		{
			LOGMSG(DBGERR, "%s  get a invalid sence mode  value", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if(a_scn_val != a_scn_val2){
			ret = cam_set_property(mCamctx, CAM_KEY_RW_SCENE_MODE, (void *)&(a_scn_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_SCENE_MODE) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}
	if(mCaps&CAM_CAP_ANTIBANDING)
	{
		int a_ab_val = lookup(ab_map, params.get(CameraParameters::KEY_ANTIBANDING), -1);
		if(a_ab_val < 0)
		{
			LOGMSG(DBGERR, "%s  antibanding parameters invalid", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		int a_ab_val2 = lookup(ab_map, mParameters.get(CameraParameters::KEY_ANTIBANDING), -1);
		if(a_ab_val2 < 0)
		{
			LOGMSG(DBGERR, "%s  get a invalid antibanding  value", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if (a_ab_val != a_ab_val2){
			ret = cam_set_property(mCamctx, CAM_KEY_RW_ANTIBANDING_MODE, (void *)&(a_ab_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_ANTIBANDING_MODE) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}
	if(mCaps&CAM_CAP_EXPOSURE)
	{
		const char *support = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
		if((support == NULL)||(!strcmp(support, "true")))
		{	
			int a_exp_val = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
			int a_exp_val2 = mParameters.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
			if (a_exp_val != a_exp_val2){
				ret = cam_set_property(mCamctx, CAM_KEY_RW_EXPOSURE_COMPENSATION, (void *)&(a_exp_val), sizeof(int));
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_EXPOSURE_COMPENSATION) failed, ret=%d", ERRHEAD, ret);
					return UNKNOWN_ERROR;
				}
			}
		}
	}
	if (mCaps&CAM_CAP_FLASH_MODE_SUPPORT)
	{
		int a_fm_val = lookup(fm_map, params.get(CameraParameters::KEY_FLASH_MODE), -1);
		if(a_fm_val == -1)
		{
			LOGMSG(DBGERR, "%s flash mode   parameters invalid", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		int a_fm_val2 = lookup(fm_map, mParameters.get(CameraParameters::KEY_FLASH_MODE), CAM_FLASH_MODE_OFF);
		if(a_fm_val2  == -1)
		{
			LOGMSG(DBGERR, "%s  get a invalid flash  mode  value", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if (a_fm_val != a_fm_val2){
			ret = cam_set_property(mCamctx, CAM_KEY_RW_FLASH_MODE, (void *)&(a_fm_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_FLASH_MODE) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
		if(a_fm_val == CAM_FLASH_MODE_ON)
		{
			mNeedTurnOnLight = true;
		}
		else
			mNeedTurnOnLight = false;
	}

	if(mCaps&CAM_CAP_ZOOM){
		int a_zoom_val = params.getInt(CameraParameters::KEY_ZOOM);
		int a_zoom_val2 = mParameters.getInt(CameraParameters::KEY_ZOOM);
		if( (a_zoom_val > mMax_zoom) || ( a_zoom_val < 0))
		{
			LOGMSG(DBGERR, "%s set a invalid zoom parameters", ERRHEAD);
			return UNKNOWN_ERROR;
		}
		if (a_zoom_val  != a_zoom_val2) {
			LOGMSG(DBGINFO, "%s a_zoom_val = %d", INFOHEAD, a_zoom_val);
			ret = cam_set_property(mCamctx, CAM_KEY_RW_ZOOM, (void *)&(a_zoom_val), sizeof(int));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_set_property(CAM_KEY_RW_ZOOM) failed, ret=%d", ERRHEAD, ret);
				return UNKNOWN_ERROR;
			}
		}
	}

	//focus mode
	if (strcmp(params.get(CameraParameters::KEY_FOCUS_MODE), CameraParameters::FOCUS_MODE_AUTO))
	{
		LOGMSG(DBGERR, "%s focus mode parameters invalid", ERRHEAD);
		return BAD_VALUE;
	}

	//GPS
	const char *latitude = params.get(CameraParameters::KEY_GPS_LATITUDE);
	if(latitude){
		LOGMSG(DBGINFO, "%s  latitude= %s", INFOHEAD,latitude);
		mParameters.set(CameraParameters::KEY_GPS_LATITUDE,latitude);
	}    
	else
		mParameters.remove(CameraParameters::KEY_GPS_LATITUDE);

	const char *longitude = params.get(CameraParameters::KEY_GPS_LONGITUDE);
	if(longitude){
		LOGMSG(DBGINFO, "%s  longitude = %s", INFOHEAD,longitude );
		mParameters.set(CameraParameters::KEY_GPS_LONGITUDE,longitude);
	} 
	else
		mParameters.remove(CameraParameters::KEY_GPS_LONGITUDE);

	const char *altitude = params.get(CameraParameters::KEY_GPS_ALTITUDE);
	if(altitude){
		LOGMSG(DBGINFO, "%s  altitude  = %s", INFOHEAD,altitude);
		mParameters.set(CameraParameters::KEY_GPS_ALTITUDE,altitude);
	}
	else
		mParameters.remove(CameraParameters::KEY_GPS_ALTITUDE);

	const char *timestamp = params.get(CameraParameters::KEY_GPS_TIMESTAMP);
	if(timestamp){
		LOGMSG(DBGINFO, "%s  timestamp  = %s", INFOHEAD, timestamp);
		mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP,timestamp );
	}
	else
		mParameters.remove(CameraParameters::KEY_GPS_TIMESTAMP);

	const char *method = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
	if(method){
		LOGMSG(DBGINFO, "%s   method  = %s", INFOHEAD, method);
		mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD,method );
	}
	else
		mParameters.remove(CameraParameters::KEY_GPS_PROCESSING_METHOD);

	//set preview(codec) parameters
	int  r_fps;
	r_fps = params.getPreviewFrameRate();
	LOGMSG(DBGINFO, "%s r_fps = %d", INFOHEAD, r_fps);

	//TODO:
	const char *preformat = params.getPreviewFormat();
	int pre_fmt;
	LOGMSG(DBGINFO, "%s preformat = %s", INFOHEAD, preformat);
	if(preformat == NULL)
	{
		pre_fmt = mPreviewCfg.fmt;
	}
	else if(!strcmp(preformat, CameraParameters::PIXEL_FORMAT_YUV422SP))
	{
		pre_fmt = CAM_PIXFMT_YUV422SP;
	}
	else if(!strcmp(preformat, CameraParameters::PIXEL_FORMAT_YUV420SP))
	{
		pre_fmt = CAM_PIXFMT_YUV420SP;
	}
	else if(!strcmp(preformat, CameraParameters::PIXEL_FORMAT_YUV420P))
	{
		pre_fmt = CAM_PIXFMT_YUV420P;
	}
	else if(!strcmp(preformat, CameraParameters::PIXEL_FORMAT_YUV422I))
	{
		pre_fmt = CAM_PIXFMT_YUV422I;
	}
	else{
		pre_fmt = mPreviewCfg.fmt;
	}
	if(pre_fmt != r_pre_fmt)
	{
		r_pre_fmt = pre_fmt;
		if (!(r_pre_fmt & mPreviewCfg.fmtBits))
		{
			LOGMSG(DBGERR, "%s set a invalid preview fmt, line(%d) ", ERRHEAD, __LINE__);
			return BAD_VALUE;
		}
	}

	int pre_width, pre_height;
	params.getPreviewSize(&pre_width, &pre_height);
	LOGMSG(DBGINFO, "%s pre_width = %d, pre_height = %d", INFOHEAD, pre_width, pre_height);
	if((pre_width <= 0) || (pre_height <=0))	
	{
		LOGMSG(DBGERR, "%s Invalid preview width and height, line(%d) ", ERRHEAD, __LINE__);
		return BAD_VALUE;
	}
	if((pre_width != r_pre_width) || (pre_height != r_pre_height))
	{
		r_pre_width = pre_width;
		r_pre_height = pre_height;
	}

	//set fps
	int min_fps = 0; 
	int max_fps = 0;
	params.getPreviewFpsRange(&min_fps, &max_fps);

	if((min_fps <= 0) || (max_fps <= 0) || (min_fps > max_fps))
	{
		LOGMSG(DBGWARN, "%s Invalid max and min fps, line(%d) ", WARNHEAD, __LINE__);
		return BAD_VALUE;
	}

	//set Video size
	int vid_width, vid_height;
	params.getVideoSize(&vid_width, &vid_height);
	if((vid_width <= 0) || (vid_height <=0))	
	{
		LOGMSG(DBGERR, "%s Invalid video width and height, line(%d) ", ERRHEAD, __LINE__);
	}	
	if((vid_width != r_vid_width) || (vid_height != r_vid_height))
	{
		r_vid_width = vid_width;
		r_vid_height = vid_height;
	}
	//
	const char *encodestruct = params.get(ENCODESTRUCT);
	if(encodestruct){ 
		if (!strcmp(params.get(ENCODESTRUCT), "true"/*CameraParameters::TRUE)*/))
		{
			mUseEncodeSruct = true;
		}
		else{
			mUseEncodeSruct = false;
		}
	}

	//set picture parameters
	const char *picformat = params.getPictureFormat();
	int pic_fmt = 0;
	if(picformat != NULL){
		if(!strcmp(picformat, CameraParameters::PIXEL_FORMAT_YUV422SP)){
			pic_fmt = CAM_PIXFMT_YUV422SP;
		}else if(!strcmp(picformat, CameraParameters::PIXEL_FORMAT_YUV420SP)){
			pic_fmt = CAM_PIXFMT_YUV420SP;
		}else if(!strcmp(picformat, CameraParameters::PIXEL_FORMAT_YUV420P)){
			pic_fmt = CAM_PIXFMT_YUV420P;
		}else if(!strcmp(picformat,  CameraParameters::PIXEL_FORMAT_YUV422I)){
			pic_fmt = CAM_PIXFMT_YUV422I;
		}else if(!strcmp(picformat,  CameraParameters::PIXEL_FORMAT_RGBA8888)){
			pic_fmt = CAM_PIXFMT_32BPP_RGB0888;
		}else if(!strcmp(picformat, CameraParameters::PIXEL_FORMAT_RGB565)){
			pic_fmt = CAM_PIXFMT_16BPP_RGB565;
		}else if(!strcmp(picformat, CameraParameters::PIXEL_FORMAT_JPEG)){
			pic_fmt = mPreviewCfg.fmt;
		}
	}
	else{
		pic_fmt = mPreviewCfg.fmt;	// don't change.
		//r_pic_fmt = mPictureCfg.fmt;	// don't change.
	}
	if(pic_fmt != r_pic_fmt)
	{
		r_pic_fmt = pic_fmt;
		if (!(r_pic_fmt & mPictureCfg.fmtBits))
		{
			LOGMSG(DBGERR, "%s set a invalid preview fmt, line(%d) ", ERRHEAD, __LINE__);
			return BAD_VALUE;
		}
	}

	int pic_width, pic_height;
	params.getPictureSize(&pic_width, &pic_height);
	LOGMSG(DBGINFO, "%s pic_width = %d, pic_height = %d", INFOHEAD, pic_width, pic_height);

	if((pic_width  < 0) || (pic_height < 0))
	{
		LOGMSG(DBGERR, "%s Invalid picture width and height, line(%d) ", ERRHEAD, __LINE__);
		return UNKNOWN_ERROR;
	}
	if((pic_width != r_pic_width) || (pic_height != r_pic_height))
	{
		r_pic_width = pic_width;
		r_pic_height = pic_height;
	}

	//jpeg qLevel
	int jpegqLevel = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
	if ((jpegqLevel > 0)  && (jpegqLevel <= 100)){
		if(mJencCfg.qLevel != jpegqLevel)
			mJencCfg.qLevel = jpegqLevel; 
	}
	else{
		LOGMSG(DBGERR, "%s jpeg quality  parameters invalid, jpegqLevel(%d) ", ERRHEAD, jpegqLevel);
		return UNKNOWN_ERROR;
	}

	int jpegThumbLevel =  params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
	if ((jpegThumbLevel <= 0)  || (jpegThumbLevel > 100)){
		LOGMSG(DBGERR, "%s jpeg thumbnail  quality  parameters invalid, jpegThumbLevel(%d) ", ERRHEAD, jpegThumbLevel );
		return UNKNOWN_ERROR;
	}

	int tn_width, tn_height;
	tn_width = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
	tn_height = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
	if(( (tn_width == 128 ) && (tn_height == 96)) || ( (tn_width == 0 ) && (tn_height == 0)))
	{
		if( (r_thumb_width != tn_width)  || (r_thumb_height != tn_height))
		{
			r_thumb_width = tn_width;
			r_thumb_height = tn_height;
		}
	}
	else{
		LOGMSG(DBGERR, "%s jpeg thumbnail  size  parameters invalid tn_width(%d) tn_heifht(%d)", ERRHEAD, tn_width, tn_height);
		return UNKNOWN_ERROR;
	}

	int rotation = params.getInt(CameraParameters::KEY_ROTATION);
	//ALOGE("@@@@@from app:  rotation = %d\n", rotation);
	if((rotation != -1) && (mRotation != rotation)) mRotation = rotation;
	if(rotation == -1) mRotation = 0;

	mParameters = lp;
	return NO_ERROR;
}

char* CameraHal::getParameters()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	Mutex::Autolock lock(mLock);
	String8 strparams (mParameters.flatten());
	char* params_string;
	params_string = (char*) malloc(sizeof(char) * (strparams.length()+1));
	strcpy(params_string, strparams.string());
	LOGMSG(DBGINFO, "params_string=%s", params_string);
	return params_string;
}

void CameraHal::putParameters(char *parms)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	Mutex::Autolock lock(mLock);
	if(parms != IM_NULL) free(parms);
}

status_t CameraHal::storeMetaDataInBuffers(bool enable)
{
	LOGMSG(DBGINFO, "%s %s(enable=%d)", INFOHEAD, __FUNCTION__, enable);
	return INVALID_OPERATION;
}

status_t CameraHal::sendCommand(int32_t command, int32_t arg1, int32_t arg2)
{
	LOGMSG(DBGINFO, "%s %s(cmd=%d, arg1=%d, arg2=%d)", INFOHEAD, __FUNCTION__, command, arg1, arg2);
	switch(command)
	{
		case CAMERA_CMD_START_FACE_DETECTION:
			//check it
			// mFaceDT = true;
			return NO_ERROR;
		case CAMERA_CMD_STOP_FACE_DETECTION:
			mFaceDT = false;
			return NO_ERROR;
		case CAMERA_CMD_START_SMOOTH_ZOOM:
			return NO_ERROR;
		case CAMERA_CMD_STOP_SMOOTH_ZOOM:
			return NO_ERROR;
		case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
			return NO_ERROR;
		default:
			return BAD_VALUE;
	}
}

void CameraHal::release()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	LOCK_IN();
	Mutex::Autolock lock(mLock);
	LOCK_OUT();
	ResourceDeinit();
}

bool CameraHal::ResourceInit()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	IM_TCHAR version[IM_VERSION_STRING_LEN_MAX] = {0};
	cam_version(version);
	LOGMSG(DBGTIP, "%s InfotmMediaCamera version=%s", TIPHEAD, version);
	jenc_version(version);
	LOGMSG(DBGTIP, "%s InfotmMediaJpegenc version=%s", TIPHEAD, version);
	//	cvt_version(version);
	//	LOGMSG(DBGTIP, "%s InfotmMediaConvert version=%s", TIPHEAD, version);
	alc_version(version);
	LOGMSG(DBGTIP, "%s InfotmMediaBuffalloc version=%s", TIPHEAD, version);

	//
	IM_RET ret = jenc_init(&mJencInst);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s jenc_open() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	mSupportJpeg = true;
	memset(&mJencCfg, 0, sizeof(mJencCfg));	
	memset(&mJencParam, 0, sizeof(mJencParam));

	mJencCfg.qLevel = 100;
	mJencCfg.rotation = JENC_ROTATION_0;
	mJencCfg.unitsType = JENC_UNITS_NO;
	mJencCfg.markerType = JENC_SINGLE_MARKER;
	mJencCfg.xDensity = 1;	
	mJencCfg.yDensity = 1;	
	mJencCfg.comLength = 0;	
	mJencCfg.comment = NULL;
	mJencCfg.thumb.format  = JENC_THUMB_JPEG;

	mJencParam.xOffset = 0;
	mJencParam.yOffset = 0;
	mJencParam.codingMode = JENC_ENCMODE_WHOLEFRAME;
	mJencParam.sliceLines = 0;

	//
#ifdef USE_BLT
	ret = blt_open(&mBlt);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s blt_open() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	memset(&mBlt_tsf, 0, sizeof(blt_transfer_t));
#else
#endif

	/*
	   mShmAddr = (mm_shared_t *)malloc(sizeof(mm_shared_t));
	   mShareMemhandle = SoShmGet(sizeof(mm_shared_t), SHARE_MEMORY_KEY);
	   if(mShmAddr==NULL || mShareMemhandle ==NULL){
	   LOGMSG(DBGERR,"%s can not allocate shm or shm handle!\n", ERRHEAD);
	   return false;
	   }
	   if (SoShmRead(mShareMemhandle, 0, mShmAddr, sizeof(mm_shared_t)) != MMRET_OK) {
	   LOGMSG(DBGERR,"%s SoShmRead() failed !\n", ERRHEAD);
	   return false;
	   }*/

	char alc_user[] = "CAMHAL";
	ret = alc_open(&mBuffAlloc, alc_user);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR,"%s alc_open() failed\n", ERRHEAD);
		return false;
	}

	//face detect
	ret = img_init(&mImage);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s img_init() failed, ret=%d", ERRHEAD, ret);
	}
	memset(&mMetadata, 0, sizeof(camera_frame_metadata_t));
	memset(mFaces, 0, IMG_MAX_FACE_DETECT_NUM * sizeof(camera_face_t));
	memset(&mFaceSrc, 0, sizeof(img_face_detect_src_t));
	memset(&mFaceDes, 0, sizeof(img_face_detect_des_t));

	//set source
	int cls = CAM_MODULE_GET_UID_LIBCLASS(cameraID);
	int box = CAM_MODULE_GET_UID_BOX(cameraID);
	int mdl = CAM_MODULE_GET_UID_MODULE(cameraID);

	facing = mCamModuleTree.libcls[cls].boxes[box].modules[mdl].facing;

	ret = cam_open(&mCamctx, cameraID);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_open() failed, ret=%d", ERRHEAD, ret);
		return false;
	}

	memset(&mPreviewCfg, 0, sizeof(cam_preview_config_t));
	memset(&mPictureCfg, 0, sizeof(cam_preview_config_t));

	//get caps and re_oganize stream path when need
	ret = cam_get_property(mCamctx, CAM_KEY_R_CAPS, &mCaps, sizeof(int32_t));
	LOGMSG(DBGINFO, "%s mCaps = %d", INFOHEAD, mCaps);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_get_caps() failed, ret=%d", ERRHEAD, ret);
		return false;
	}

	memset(&mReoganize , 0, sizeof(cam_reorg_path_t));

	switch(mCaps & (CAM_CAP_CODEC_SUPPORT|CAM_CAP_PREVIEW_SUPPORT|CAM_CAP_PICTURE_SUPPORT))
	{

		case CAM_CAP_PREVIEW_SUPPORT:
			mReoganize.preRedirTo = CAM_PATH_PREVIEW;
			mReoganize.coRedirTo = CAM_PATH_PREVIEW;
			mReoganize.picRedirTo = CAM_PATH_PREVIEW;
			mNeedreoganize = true;
			break;

		case CAM_CAP_PREVIEW_SUPPORT|CAM_CAP_CODEC_SUPPORT:
			mReoganize.preRedirTo = CAM_PATH_PREVIEW;
			mReoganize.coRedirTo = CAM_PATH_CODEC;
			mReoganize.picRedirTo = CAM_PATH_PREVIEW;
			mNeedreoganize = true;
			break;

		case CAM_CAP_PREVIEW_SUPPORT|CAM_CAP_PICTURE_SUPPORT:
			mReoganize.preRedirTo = CAM_PATH_PREVIEW;
			mReoganize.coRedirTo = CAM_PATH_PREVIEW;
			mReoganize.picRedirTo = CAM_PATH_PICTURE;
			mNeedreoganize = false;
			break;


		case CAM_CAP_CODEC_SUPPORT|CAM_CAP_PICTURE_SUPPORT|CAM_CAP_PREVIEW_SUPPORT:
			mReoganize.preRedirTo = CAM_PATH_PREVIEW;
			mReoganize.coRedirTo = CAM_PATH_CODEC;
			mReoganize.picRedirTo = CAM_PATH_PICTURE;
			mNeedreoganize = false;
			break;

		default :
			return false;
	}

	if(InitDefaultParameters() != true){
		LOGMSG(DBGERR, "%s InitDefaultParameters() failed", ERRHEAD);
		return false;	
	}

	ret = cam_start(mCamctx);
	if(ret != IM_RET_OK){
		LOGMSG(DBGERR, "%s cam_start() failed, ret=%d", ERRHEAD, ret);
		return false;
	}
	mCheckInit = true;

	//for jpeg encode
	mExifEn = new ExifEncode(this);
	return true;
}

bool CameraHal::ResourceDeinit()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	if(mCamctx != NULL){
		ret = cam_stop(mCamctx);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_stop() failed, ret=%d", ERRHEAD, ret);
		}
		ret = cam_close(mCamctx);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_close() failed, ret=%d", ERRHEAD, ret);
		}
		mCamctx = NULL;
	}

	if(mJencInst != NULL){
		ret = jenc_deinit(mJencInst);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s jenc_close() failed, ret=%d", ERRHEAD, ret);
		}
		mJencInst = NULL;
	}

#ifdef USE_BLT
	if(mBlt != NULL){
		ret = blt_close(mBlt);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s blt_close() failed, ret=%d", ERRHEAD, ret);
		}
		mBlt = NULL;
	}
#else
#endif

	/*
	   if(mShmAddr != NULL){
	   free(mShmAddr);
	   mShmAddr = NULL;
	   }
	   if(mShareMemhandle != NULL){
	   SoShmFree(mShareMemhandle);
	   mShareMemhandle = NULL;
	   }*/

	if(mBuffAlloc != IM_NULL){
		if(mJpegBuff.vir_addr != NULL){
			ret = alc_free(mBuffAlloc, &mJpegBuff);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
			}
		}
		if(mJpegOutBuff.vir_addr != NULL){
			ret = alc_free(mBuffAlloc, &mJpegOutBuff);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
			}
		}
		if(mPreviewBuff.vir_addr != NULL){
			ret = alc_free(mBuffAlloc, &mPreviewBuff);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
			}
		}
		if(mFaceBuff.vir_addr != NULL){
			ret = alc_free(mBuffAlloc, &mFaceBuff);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
			}
		}
		if(mThumbBuff.vir_addr != NULL){
			ret = alc_free(mBuffAlloc, &mThumbBuff);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
			}
		}
		for (int i = 0; i < pre_buffer_property.minNumber + 2; i++) {
			if(mVideoResize[i].vir_addr != NULL){
				ret = alc_free(mBuffAlloc, &mVideoResize[i]);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
				}
			}
		}
		if(mBigMemory)
		{
			ret = alc_deinit_bigmem(mBuffAlloc);
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s alc_deinit_bigmem() failed, ret=%d", ERRHEAD, ret);
			}
		}
		alc_close(mBuffAlloc);
		mBuffAlloc = NULL;
	}

	if(mRecordingMemory != NULL){
		mRecordingMemory->release(mRecordingMemory);
		mRecordingMemory = NULL;
	}

	if(mImage != IM_NULL)
	{
		ret = img_deinit(mImage);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s img_deinit() failed, ret=%d", ERRHEAD, ret);
		}
	}

	coPathCounter = 0;
	prePathCounter = 0;

	if(mExifEn)
		delete mExifEn;

	return true;
}

IM_RET CameraHal::redir_cam_get_picture_configs(IN CAMCTX cam, OUT cam_picture_config_t *cfgs)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	if(mNeedreoganize)
		return cam_get_preview_configs(cam, (cam_preview_config_t*)cfgs);
	else
		return cam_get_picture_configs(cam, cfgs);

}

IM_RET CameraHal::redir_cam_set_picture_config(IN CAMCTX cam, IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	if(mNeedreoganize){
		IM_RET ret;
		ret = cam_set_preview_config(cam, res, fmt, fps);
		if(ret !=IM_RET_OK)
		{
			LOGMSG(DBGERR, "%s cam_set_preview_config() failed, ret=%d", ERRHEAD, ret);
			return ret;
		}
		ret = cam_get_preview_configs(cam, &mPreviewCfg);
		if(ret !=IM_RET_OK)
		{
			LOGMSG(DBGERR, "%s cam_get_preview_configs() failed, ret=%d", ERRHEAD, ret);
		}
		return ret;
	}
	else
		return cam_set_picture_config(cam, res, fmt, fps);
}

IM_RET CameraHal::redir_cam_prepare_take_picture(IN CAMCTX cam, IN cam_takepic_config_t *cfg)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	if(mNeedreoganize){
		ret = cam_enable_stream(cam, CAM_PATH_PREVIEW);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_enable_path() failed, ret=%d", ERRHEAD, ret);
		}
		return ret;
	}
	else
		return cam_prepare_take_picture(cam,cfg);
}

IM_RET CameraHal::redir_cam_cancel_take_picture(IN CAMCTX cam)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	if(mNeedreoganize){
		ret = cam_disable_stream(cam, CAM_PATH_PREVIEW);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_disable_path() failed, ret=%d", ERRHEAD, ret);
		}
		return ret;
	}
	else
		return cam_cancel_take_picture(cam);
}

IM_RET CameraHal::redir_cam_take_picture(IN CAMCTX cam, OUT cam_frame_t *picFrame)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;
	if(mNeedreoganize){
		ret =  cam_get_preview_frame(cam, picFrame);
		if(ret != IM_RET_OK){
			LOGMSG(DBGERR, "%s cam_get_preview_frame() failed, ret=%d", ERRHEAD, ret);
			stopPreview();
		}
		return ret;
	}
	else
		return cam_take_picture(cam, picFrame);
}

IM_RET CameraHal::redir_cam_release_picture(IN CAMCTX cam, IN cam_frame_t *picFrame)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	if(mNeedreoganize)
		return cam_release_preview_frame(cam, picFrame);
	else
		return cam_release_picture(cam, picFrame);
}

int32_t CameraHal::GetRealPath(int32_t path)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	switch(path)
	{
		case CAM_PATH_PREVIEW:
			return mReoganize.preRedirTo;
		case CAM_PATH_CODEC:
			return mReoganize.coRedirTo;
		case CAM_PATH_PICTURE:
			return mReoganize.picRedirTo;
		default:
			return 0;

	}
}

int32_t CameraHal::PathCounter(int32_t path, bool add_counter)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	switch(path)
	{
		case CAM_PATH_PREVIEW:
			add_counter? ++prePathCounter : --prePathCounter;
			if(prePathCounter<0)prePathCounter = 0;
			return prePathCounter;
		case CAM_PATH_CODEC:
			add_counter? ++coPathCounter : --coPathCounter;
			if(coPathCounter<0)coPathCounter = 0;
			return coPathCounter;
		case CAM_PATH_PICTURE:
			return 0;
		default:
			return -1;
	}
}

bool CameraHal::AssignBuffer(int32_t path)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	IM_RET ret;
	IM_Buffer buff;
	int flag = ALC_FLAG_PHY_MUST | ALC_FLAG_ALIGN_32BYTES;
	int bufsize;

	switch(GetRealPath(path))
	{
		case CAM_PATH_PREVIEW:
			memset(&pre_buffer_property, 0, sizeof(cam_buffer_property_t));

			ret = cam_get_property(mCamctx, CAM_KEY_R_PREVIEW_BUFFER_REQUIREMENT, (void *)&pre_buffer_property, sizeof(cam_buffer_property_t));
			if(ret != IM_RET_OK){
				LOGMSG(DBGERR, "%s cam_get_property(CAM_KEY_R_PREVIEW_BUFFER_MIN) failed, ret=%d", ERRHEAD, ret);
				return false;
			}
			if(path == CAM_PATH_PICTURE)
				mBufferNeed = pre_buffer_property.minNumber;
			else
				mBufferNeed = pre_buffer_property.minNumber + 2;

			if(mBufferNeed > mRecordingBufferNumber) return false;

			for(int i=0; i < mBufferNeed; i++){
				if(path == CAM_PATH_PICTURE  )
				{
					if(!calcBuffersize(mPictureCfg.res, mPictureCfg.fmt, &bufsize)) return false;
				}
				else{
					if(!calcBuffersize(mPreviewCfg.res, mPreviewCfg.fmt, &bufsize)) return false;
				}

				if(mBigMemory)
					flag |= ALC_FLAG_BIGMEM;
				ret = alc_alloc(mBuffAlloc, bufsize ,&mBuffer[i], flag);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s alc_alloc() failed, ret=%d", ERRHEAD, ret);
				}

				ret = cam_assign_buffer(mCamctx, CAM_PATH_PREVIEW, IM_FALSE, &mBuffer[i]);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s cam_alloc_buffer() failed, ret=%d", ERRHEAD,  ret);
					return false;
				}
			}

			break;

		case CAM_PATH_CODEC:
			/*			memset(&co_buffer_property, 0, sizeof(cam_buffer_property_t));

						ret = cam_get_property(mCamctx, CAM_KEY_R_CODEC_BUFFER_REQUIREMENT, (void *)&co_buffer_property, sizeof(cam_buffer_property_t));
						if(ret != IM_RET_OK){
						return false;
						}

						for(int  i=0; i < co_buffer_property.minNumber; i++){
						ret = cam_assign_buffer(mCamctx, CAM_PATH_CODEC, IM_TRUE, &buff);
						if(ret != IM_RET_OK){
						return false;
						}
						}*/

			break;

		case CAM_PATH_PICTURE:
			break;

		default :
			return false;
	}

	return true;
}

bool CameraHal::ReleaseBuffer(int32_t path)
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);
	IM_RET ret;

	if(mBuffAlloc != IM_NULL){
		for(int i = 0; i < mBufferNeed; i++)
		{
			if(mBuffer[i].vir_addr != NULL)
			{
				ret = alc_free(mBuffAlloc, &mBuffer[i]);
				if(ret != IM_RET_OK){
					LOGMSG(DBGERR, "%s alc_free() failed, ret=%d", ERRHEAD, ret);
					return false;
				}
			}
			mBuffer[i].vir_addr = NULL;
		}
	}

	return true;
	/*
	   if(PathCounter(GetRealPath(path), false) < 0){
	   return false;
	   }

	   return true;*/
}
status_t CameraHal::returnBuffersToWindow()
{
	return NO_ERROR;
}

int CameraHalManager::HAL_getNumberOfCameras()
{
	LOGMSG(DBGINFO, "%s %s()", INFOHEAD, __FUNCTION__);

	IM_RET ret;
	int i, j, k;
	IM_BOOL hasBackCam = IM_FALSE;

	memset(&mCamModuleTree, 0, sizeof(cam_module_tree_t));

	//get cam_module_tree
	ret = cam_get_modules(&mCamModuleTree);
	if(ret != IM_RET_OK)
	{
		LOGMSG(DBGERR, "%s cam_get_modules() failed ret = %d", ERRHEAD, ret);
		return 0;
	}
	numberofcamera = 0;
	for(i = 0; i < CAM_LIBCLASS_ENUM_MAX; i++)
	{
		if(mCamModuleTree.libcls[i].boxNum == 0)
		{
			continue;
		}
		else{
			for(j = 0; j < mCamModuleTree.libcls[i].boxNum; j++)
			{
				for(k = 0; k < mCamModuleTree.libcls[i].boxes[j].moduleNum; k++)
				{
					ALOGE("cameraHal: i =  %d, j = %d, k = %d", i, j, k);
					if((hasBackCam == IM_FALSE)
						&& (CAM_MODULE_FACING_BACK == mCamModuleTree.libcls[i].boxes[j].modules[k].facing))
					{
						//let cameraId(0) to be back camera if has back camera
						cameraUID[numberofcamera] = cameraUID[0];
						cameraUID[0] = CAM_MODULE_MAKE_UID(i, j, k);
						hasBackCam == IM_TRUE;
					}
					else
					{
						cameraUID[numberofcamera] = CAM_MODULE_MAKE_UID(i, j, k);
					}
					numberofcamera++;
					if(numberofcamera >= 10)
					{
						LOGMSG(DBGERR, "%s numberofcamera(=%d) is too large", ERRHEAD, numberofcamera);
					}
				}
			}
		}
	}

	ALOGE("numberofcamera = %d\n", numberofcamera);

	return numberofcamera; 
}

void CameraHalManager::HAL_getCameraInfo(int cameraId, struct camera_info* cameraInfo)
{
	LOGMSG(DBGINFO, "%s %s(cameraId=%d)", INFOHEAD, __FUNCTION__, cameraId);

	int cls, box, mdl, facing, orientation;

	if(numberofcamera == 0) return;

	if((cameraId >= numberofcamera) || (cameraId < 0))
	{
		LOGMSG(DBGERR, "%s input parameters error", ERRHEAD);
		return;
	}
	cls = CAM_MODULE_GET_UID_LIBCLASS(cameraUID[cameraId]);
	box = CAM_MODULE_GET_UID_BOX(cameraUID[cameraId]);
	mdl = CAM_MODULE_GET_UID_MODULE(cameraUID[cameraId]);
	facing = mCamModuleTree.libcls[cls].boxes[box].modules[mdl].facing;
	orientation = mCamModuleTree.libcls[cls].boxes[box].modules[mdl].orientation;

	camera_info sCameraInfo;
	memset(&sCameraInfo, 0, sizeof(camera_info));

	if(facing == CAM_MODULE_FACING_BACK)
	{
		sCameraInfo.facing  = CAMERA_FACING_BACK;
	}
	else if(facing == CAM_MODULE_FACING_FRONT)
	{
		sCameraInfo.facing = CAMERA_FACING_FRONT;
	}

	if(orientation == 0)
	{
		sCameraInfo.orientation = 0;
	}
	else if(orientation == 90)
	{
		sCameraInfo.orientation = 90;
	}
	else if(orientation == 180)
	{
		sCameraInfo.orientation = 180;
	}
	else if(orientation == 270)
	{
		sCameraInfo.orientation = 270;
	}

	memcpy(cameraInfo, &sCameraInfo, sizeof(camera_info));
}

}// namespace


