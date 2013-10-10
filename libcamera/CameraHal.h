/*  
 *   CameraHal.h
 *
 *   Author: neville   <ranaldo_fu@infotm.com.ic>
 *
 *
 */

#ifndef __CAMERAHAL_H__
#define __CAMERAHAL_H__
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <unistd.h>
#include <utils/threads.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <IM_jpegencapi.h>
#include <IM_imageapi.h>
#include <IM_buffallocapi.h>
#include <IM_bltapi.h>
#include <utils/threads.h>
#include <camera/CameraParameters.h>
#include <hardware/camera.h>
#include <InfotmMetaData.h>

#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <hal_public.h>
#include <gralloc_priv.h>



//#define _DEBUG_
#ifdef _DEBUG_
#define DBGINFO		1
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1
#define LOGMSG(on, str, args...)	if(on)  ALOGE(str, ##args);
#else
#define DBGINFO		0
#define DBGWARN		0
#define DBGERR		1
#define DBGTIP		0
#define LOGMSG(on, str, args...)	if(on)  ALOGE(str, ##args);
#endif

#define INFOHEAD	"CAMHAL_I::" 
#define WARNHEAD	"CAMHAL_W::" 
#define ERRHEAD		"CAMHAL_E::" 
#define TIPHEAD		"CAMHAL_T::" 


/*Only three format supported now for native display 
* 1. CAM_PIXFMT_16BPP_RGB565
* 2. CAM_PIXFMT_32BPP_BGR0888
* 3. CAM_PIXFMT_YUV420SP(NV12, YYYY_UVUV) 
*/
//==============================================================================
// 1. CAM_PIXFMT_YUV420SP(NV12), it has bug now, but I can not find reason.
// 2. For RGB888, here use BGR0888 but not ARGB888 because only RGB0888/BGR0888 
//	instead of ARGB8888/ABGR8888 supported converting from YUV420SP by G2D CSC.
//==============================================================================

//#define DISPLAY_FMT	CAM_PIXFMT_YUV420SP	
#define DISPLAY_FMT	CAM_PIXFMT_32BPP_BGR0888	 

#define CAMHAL_GRALLOC_USAGE GRALLOC_USAGE_HW_TEXTURE | \
                             GRALLOC_USAGE_HW_RENDER | \
                             GRALLOC_USAGE_SW_READ_RARELY | \
                             GRALLOC_USAGE_SW_WRITE_NEVER

namespace android {

static const int        	    MAXCAMERANUMBER = 10;
static cam_module_tree_t				mCamModuleTree;
static int						numberofcamera;
static int						cameraUID[MAXCAMERANUMBER];
static const int32_t			mPreviewBufferNumber = 6;	  // from cam stub
static const int32_t			mRecordingBufferNumber = 8;	  // from cam stub
//rane@2012/11/13:  use setParameters() send a key
//This key will defined both in CameraSource and CameraHal; if You want to change it, check CaneraHal
const char ENCODESTRUCT[]  = "encode-struct";



typedef struct{
	cam_frame_t frame;
	IM_IMAGE_FORMAT	image;
}frame_with_info;

class ExifEncode;

class CameraHal{
    friend class ExifEncode;
public:
	CameraHal(int cameraid);
	~CameraHal();

    bool CheckInit();
	//virtual sp<IMemoryHeap> getPreviewHeap() const;
	//virtual sp<IMemoryHeap> getRawHeap() const;
    void setCallbacks(camera_notify_callback notify_cb,
                        camera_data_callback data_cb,
                        camera_data_timestamp_callback data_cb_timestamp,
                        camera_request_memory get_memory,
                        void *user);
	void enableMsgType(int32_t msgType);
	void disableMsgType(int32_t msgType);
	bool msgTypeEnabled(int32_t msgType);
	int dump(int fd) const;
	int setPreviewWindow(struct preview_stream_ops *window);
	int startPreview();
	void stopPreview();
	bool previewEnabled();
	int startRecording();
	void stopRecording();
	bool recordingEnabled();
	void releaseRecordingFrame(const void *opaque);
	int autoFocus();
 	int cancelAutoFocus();
	int takePicture();
	int cancelPicture();
    int    setParameters(const char* params);
    int    setParameters(const CameraParameters& params);
    char*  getParameters();
    void putParameters(char *parms);
	status_t storeMetaDataInBuffers(bool enable);
	int sendCommand(int32_t command, int32_t arg1, int32_t arg2);
	void release();

	//static sp<CameraHardwareInterface> createInstance();

protected:
	class PreviewThread : public Thread {
		CameraHal * mHardware;
	public:
		PreviewThread(CameraHal *hw) :
#ifdef SINGLE_PROCESS
			Thread(true),
#else
			Thread(false),
#endif
			mHardware(hw) { }

		virtual void onFirstRef() {
			run("CameraPreviewThread", PRIORITY_URGENT_DISPLAY);
		}   
		virtual bool threadLoop() {
			if(NO_ERROR != mHardware->previewThread()){
				return false;
			}
			// loop until we need to quit
            usleep(10);
			return true;
		}   
	};  

	int StartPreviewInternel();
	int StartRecordInternel();
	static int BeginAutoFocusThread(void *cookie);
	static int BeginPictureThread(void *cookie);
	static int BeginFaceDTThread(void *cookie);
	int AutoFocusThread();
	int previewThread();
	int recordThread();
	int pictureThread();
    int faceDTThread();
	bool InitDefaultParameters();
	bool ResourceInit();
	bool ResourceDeinit();
	bool ConvertFrame(frame_with_info *inframe, frame_with_info *outframe, int32_t rotation, int32_t usage);
	bool ConvertNV12toNV21(uint8_t* in, uint8_t* out, int32_t width, int32_t height);
	bool Convert420spto422sp(uint8_t* in, uint8_t* out, int32_t width, int32_t height);
	bool Yuv420spRBswap(uint8_t* in, uint8_t* out, int32_t width, int32_t height);
	void StopPreviewInternel();
	bool EncodeJpeg(frame_with_info *pic, int32_t *jfifsize, IM_Buffer *buff, int32_t userBufferSize);
    
    //exif callback
    int AddExifDoneCb(int jfifsize);
	
    /*reognize path*/
	/*目前暂时不考虑使用codec path. 因此实际上使用是preview path 和 picture path。
	* 当前所有的camere module都必须支持preview path。 因此实际情况只有两种：
	 * 1 只使用preview path
	 * 2 使用preview pasth 和 picture path*/
	IM_RET redir_cam_get_picture_configs(IN CAMCTX cam, OUT cam_picture_config_t *cfgs);
	IM_RET redir_cam_set_picture_config(IN CAMCTX cam, IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps);
	IM_RET redir_cam_prepare_take_picture(IN CAMCTX cam, IN cam_takepic_config_t *cfg);
	IM_RET redir_cam_cancel_take_picture(IN CAMCTX cam);
	IM_RET redir_cam_take_picture(IN CAMCTX cam, OUT cam_frame_t *picFrame);
	IM_RET redir_cam_release_picture(IN CAMCTX cam, IN cam_frame_t *picFrame);
	
    int32_t GetRealPath(int32_t path);
	int32_t PathCounter(int32_t path, bool add_counter);
	bool AssignBuffer(int32_t path);
	bool ReleaseBuffer(int32_t path);
	
	int  returnBuffersToWindow();
    bool getBuffersFromWindow();


    bool    mCheckInit;
	int		cameraID;
	int		facing;
	//static wp<CameraHardwareInterface> singleton;
	CameraParameters		mParameters;
	CAMCTX					mCamctx;
	cam_preview_config_t	mPreviewCfg;
	cam_picture_config_t	mPictureCfg;
	sp<PreviewThread>       mPreviewThread;
	//Mutex			mLock;
	mutable Mutex       mLock;

	//callback
	camera_notify_callback    mNotifyCb;
    camera_data_callback      mDataCb;
	camera_data_timestamp_callback  mDataCbTimestamp;
	camera_request_memory        mGet_memory;
    void               *mCallbackCookie;

	//preview buffer
	camera_memory_t* mPreviewMemory;
	unsigned char* mPreviewBufs[mPreviewBufferNumber];
    bool                mAllocBuffer;
    int                 mCurrentPreviewFrame;

	//preview buffer
	camera_memory_t* mRecordingMemory;
	unsigned char* mRecordingBufs[mRecordingBufferNumber];
	bool           mBufferBusy[mRecordingBufferNumber];

	//display
    preview_stream_ops_t*  mPreviewStreamOps;
    unsigned int mDisplayState;
    bool mDisplayEnabled;
    int mBufferCount;
    uint32_t* mOffsetsMap;
    int mFD;

	int32_t			mMsgEnabled;
	int32_t					mFrameSize;
	sp<MemoryHeapBase>  	mRawHeap;	
	sp<MemoryBase>      	mRawBuffer;
	sp<MemoryBase>      	mJpegBuffer;
	Mutex			mCodecBufferLck;
	JENCCTX			mJencInst;
	JENC_CONFIG		mJencCfg;
	JENC_ENCODE		mJencParam;
	bool 			mSupportJpeg;
	int32_t			mRotation;
//	CVTCTX			mConvert;
//	CVT_IMAGE		mCvtimg;
	ALCCTX 			mBuffAlloc;
	IM_Buffer		mJpegBuff;
    IM_Buffer       mJpegOutBuff;
	IM_Buffer		mPreviewBuff;
	IM_Buffer		mCodecBuff;
    IM_Buffer       mFaceBuff;
    IM_Buffer       mThumbBuff;
	IM_Buffer		mVideoResize[mRecordingBufferNumber];
	uint32_t		mCaps;
	bool            mNeedreoganize;
    bool            mBigMemory;
    IM_Buffer       mBuffer[mRecordingBufferNumber];
    int32_t         mBufferNeed;

	//for encoder
    ExifEncode      *mExifEn;
    
    InfotmMetaData::VideoEncInMediaBufferDataType mEncodeIn[mRecordingBufferNumber];
	cam_frame_t		mRecordingFrame[mRecordingBufferNumber];

	cam_buffer_property_t	pre_buffer_property;
	cam_buffer_property_t	co_buffer_property;
	cam_buffer_property_t	pic_buffer_property;
	int32_t			mDropFrame;

	frame_with_info		mInframe;
	frame_with_info		mOutframe;

	int32_t				mPicBufSize;
	int32_t				mPreBufSize;

	//a flag for camerelib can only provide preview path
	//As a resuilt we'll not start recordThread.
	bool                mRecordEna;
	uint32_t			mVideoFrameFormat;
	bool				mUseEncodeSruct;
	bool				mBufferCopied;

	//when requare size unsupported,  resize source 
	bool				mNeedResize;
	bool				mNeedResizeForVideo;
	int32_t				r_pre_width;
	int32_t				r_pre_height;
	int32_t     	    r_pre_fmt;
	int32_t				r_pic_width;
	int32_t				r_pic_height;
	int32_t			    r_pic_fmt;
    int32_t             r_thumb_width;
    int32_t             r_thumb_height;
	int32_t				r_vid_width;
	int32_t				r_vid_height;
 	int32_t             mOutbufsize;
    int32_t             mMax_zoom;

	int					prePathCounter;
	int					coPathCounter ;
	int					picPathCounter ;
	cam_reorg_path_t	mReoganize;

	Mutex				mCvtLock;
	Mutex				mThreadLock;

    BLTINST            mBlt;
    blt_transfer_t     mBlt_tsf;

    bool               mNeedTurnOnLight;

    //face detecte
    bool                     mFaceDT;
    camera_frame_metadata_t  mMetadata;
    camera_face_t            mFaces[IMG_MAX_FACE_DETECT_NUM];
    IMGCTX                   mImage;
    img_face_detect_src_t    mFaceSrc;
    img_face_detect_des_t    mFaceDes;
    int                      mFaceCounter;
    bool                     mFaceDetecting;
};

class CameraHalManager{
	public:
		CameraHalManager(){};
		~CameraHalManager(){};
		int HAL_getNumberOfCameras();
		void HAL_getCameraInfo(int cameraId, struct camera_info* cameraInfo);
};


}; //namespace

#endif
