/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
 *		leo@2011/02/16: first commit.
 *		leo@2011/02/16: Add psr_ce_register_default_fio2() interface and PSR_FIO2 struct to support mhv file parser.
 *		leo@2011/03/23: Add notation in api.
** v1.1.0	leo@2012/05/02: add unified version control.
**
*****************************************************************************/ 

#ifndef __IM_PARSERAPI_H__
#define __IM_PARSERAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef PARSER_EXPORTS
	#define PARSER_API		__declspec(dllexport)	/* For dll lib */
#else
	#define PARSER_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define PARSER_API
#endif	
//#############################################################################

typedef void * PSRCTX;

typedef struct{
	void *vir_addr;
	IM_UINT32 phy_addr;
	IM_UINT32 size;
}PSR_PACKET_BUFFER;

// packet info flag.
#define PSR_PACKET_INFO		(0x0001)
#define PSR_PACKET_EOS		(0x0010)
#define PSR_PACKET_ERROR	(0x0020)

// packet data flag.
#define PSR_PACKET_KEY		(0x0001<<16)
#define PSR_VIDEO_FRAME_NONE	(0x0002<<16)
#define PSR_VIDEO_FRAME_I	(0x0004<<16)
#define PSR_VIDEO_FRAME_P	(0x0008<<16)
#define PSR_VIDEO_FRAME_B	(0x0010<<16)

// packet read flag.
#define PSR_READ_ADD_STREAM_HEAD	(0x01)
#define PSR_READ_ADD_FRAME_HEAD 	(0x02)

//packet head flag
#define PKFLAG_ADD_FRAMEHEAD	1
#define PKFLAG_DEL_FRAMEHEAD	2
#define PKFLAG_ADD_STREAMHEAD	4
#define PKFLAG_DEL_STREAMHEAD	8

typedef struct{
	IM_INT32 strm_id;
	PSR_PACKET_BUFFER buff;	// if in accel mode, buff will be rewrite but copy to.
	IM_UINT32 read_flag;  //PSR_READ_ADD_XXX_HEAD

	IM_UINT32 data_size;
	IM_UINT32 flag;
	IM_INT64 timestamp;
	IM_INT64 duration;
}PSR_PACKET;

typedef struct{
	IM_INT32 strm_id;
	// seek target position
	IM_INT64 target_ts;		// IN
	IM_INT64 actual_ts;		// OUT
}PSR_SEEK;

typedef struct{
	IM_BOOL enable;
	IM_UINT32 flag;	//PSR_READ_ADD_XXX_HEAD

	IM_INT32 pktnbr;
	PSR_PACKET **pkts;
}PSR_ACCEL_READ;

// FIO
#define FLAG_RO		1
#define FLAG_WO		2
#define FLAG_RW		3

#define WHENCE_SET	1
#define WHENCE_CUR	2
#define WHENCE_END	3

#define PSR_FIO2

#ifdef PSR_FIO2
typedef struct{
	IM_BOOL		mhv;
}PSR_FIO_PRIVATE;
typedef void *(*FIO_OPEN2)(void *filename, IM_INT32 flag, OUT void **priv);	// negative--failed, else file context.
typedef IM_INT32 (*FIO_READ2)(void *buff, IM_INT32 size, IM_INT32 count, void *stream, IN void *priv);	// -1--failed, else read bytes.
typedef IM_INT32 (*FIO_WRITE2)(void *buff, IM_INT32 size, IM_INT32 count, void *stream, IN void *priv);	// -1--failed, else written bytes.
typedef IM_INT64 (*FIO_SEEK2)(void *stream, IM_INT64 pos, IM_INT32 whence, IN void *priv);	// -1--failed, else offset from file head.
typedef IM_INT32 (*FIO_CLOSE2)(void *stream, IN void *priv);	// -1--failed, 0--success
typedef IM_INT64 (*FIO_GETSIZE2)(void *stream, IN void *priv);	// -1--failed, else file size
typedef struct{
	FIO_OPEN2		fio_open2;
	FIO_READ2		fio_read2;
	FIO_WRITE2		fio_write2;
	FIO_SEEK2		fio_seek2;
	FIO_CLOSE2		fio_close2;
	FIO_GETSIZE2		fio_getsize2;
}PSR_FIO;
#else
typedef void *(*FIO_OPEN)(void *filename, IM_INT32 flag);	// negative--failed, else file context.
typedef IM_INT32 (*FIO_READ)(void *buff, IM_INT32 size, IM_INT32 count, void *stream);	// -1--failed, else read bytes.
typedef IM_INT32 (*FIO_WRITE)(void *buff, IM_INT32 size, IM_INT32 count, void *stream);	// -1--failed, else written bytes.
typedef IM_INT64 (*FIO_SEEK)(void *stream, IM_INT64 pos, IM_INT32 whence);	// -1--failed, else offset from file head.
typedef IM_INT32 (*FIO_CLOSE)(void *stream);	// -1--failed, 0--success
typedef IM_INT64 (*FIO_GETSIZE)(void *stream);	// -1--failed, else file size
typedef struct{
	FIO_OPEN		fio_open;
	FIO_READ		fio_read;
	FIO_WRITE		fio_write;
	FIO_SEEK		fio_seek;
	FIO_CLOSE		fio_close;
	FIO_GETSIZE		fio_getsize;
}PSR_FIO;
#endif

//
// Extended external FIO, it designed for full-feature.
//
#define PSR_FIO_FLAG_RO		(1<<0)
#define PSR_FIO_FLAG_WO		(1<<1)
#define PSR_FIO_FLAG_RW		(1<<2)

#define PSR_FIO_WHENCE_SET	1
#define PSR_FIO_WHENCE_CUR	2
#define PSR_FIO_WHENCE_END	3

typedef struct{
	void	*stream;
	void	*priv;
}PSR_FIO_HANDLE, *PSR_FIO_HANDLER;

typedef PSR_FIO_HANDLER (*FIO_OPEN_EXT)(IN const IM_INT8 *filename, IN IM_INT32 flag);// IM_NULL--failed, else return the handler;
typedef IM_INT32 (*FIO_READ_EXT)(IN PSR_FIO_HANDLER handler, OUT void *buff, IN IM_INT32 size, IN IM_INT32 count);// -1--failed, else read bytes.
typedef IM_INT32 (*FIO_WRITE_EXT)(IN PSR_FIO_HANDLER handler, IN void *buff, IN IM_INT32 size, IN IM_INT32 count);// -1--failed, else written bytes.
typedef IM_INT64 (*FIO_SEEK_EXT)(IN PSR_FIO_HANDLER handler, IN IM_INT64 pos, IN IM_INT32 whence);// -1--failed, else offset from file head.
typedef IM_INT64 (*FIO_TELLPOS_EXT)(IN PSR_FIO_HANDLER handler);// -1--failed, else offset from file head.
typedef IM_INT64 (*FIO_GETSIZE_EXT)(IN PSR_FIO_HANDLER handler);// -1--failed, else offset from file head.
typedef IM_INT32 (*FIO_CLOSE_EXT)(IN PSR_FIO_HANDLER handler);// -1--failed, 0--success
typedef struct{
	FIO_OPEN_EXT		fio_open_ext;
	FIO_READ_EXT		fio_read_ext;
	FIO_WRITE_EXT		fio_write_ext;
	FIO_SEEK_EXT		fio_seek_ext;
	FIO_TELLPOS_EXT		fio_tellpos_ext;
	FIO_GETSIZE_EXT		fio_getsize_ext;
	FIO_CLOSE_EXT		fio_close_ext;
}PSR_FIO_EXT;

typedef struct{
	PSR_FIO_HANDLER handler;
	PSR_FIO_EXT	*fio;
}PSR_FIO_STRUCT;

/*============================Interface API==================================*/
/*
FUNC: get parser version.
PARAM: ver_string, buffer to save parser version string.
RETURN: (ver_major << 16) | (ver_minor).
*/
PARSER_API IM_UINT32 psr_version(OUT IM_TCHAR *ver_string);

/*
FUNC: open parser, and get a parser instance.
PARAM: media_name, input media name.
	parser, to save parser instance.
	media_info, to save the media infomation.
	fio, register external file io.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_open(IN IM_TCHAR *media_name, OUT PSRCTX *parser, OUT IM_MEDIA_INFO **media_info, IN PSR_FIO *fio=IM_NULL);
PARSER_API IM_RET psr_open_ext(IN IM_TCHAR *media_name, OUT PSRCTX *parser, OUT IM_MEDIA_INFO **media_info, IN PSR_FIO_EXT *fio=IM_NULL);

/*
FUNC: close parser.
PARAM: parser, parser instance.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_close(IN PSRCTX parser);

/*
FUNC: read a packet, it's a block wait.
PARAM: parser, parser instance.
	packet, read configuation.
	timeout, timeout of read operation.
RETURN: IM_RET_OK--succeed, 
	IM_RET_FALSE--this stream has been stopped.
	IM_RET_EOS--EOS.
	else--failed.
*/
PARSER_API IM_RET psr_read_packet(IN PSRCTX parser, INOUT PSR_PACKET *packet, IN IM_INT32 timeout=-1);

/*
FUNC: start parser thread.
PARAM: parser, parser instance.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_start(IN PSRCTX parser);

/*
FUNC: stop parser thread.
PARAM: parser, parser instance.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_stop(IN PSRCTX parser);

/*
FUNC: seek to a position, in ms unit.
PARAM: parser, parser instance.
	sek, sek configure.
	timeout, timeout of seek operation.
RETURN: IM_RET_OK succeed, IM_RET_EOS--EOS, else failed.
*/
PARSER_API IM_RET psr_seek(IN PSRCTX parser, INOUT PSR_SEEK *sek, IN IM_INT32 timeout=-1);

/*
FUNC: enable stream.
PARAM: parser, parser instance.
	strmid, stream id.
	enaPrefetch, enbale stream cache mode or not.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_enable_stream(IN PSRCTX parser, IN IM_INT32 strmid, IN IM_BOOL enaPrefetch=IM_TRUE);

/*
FUNC: disable stream.
PARAM: parser, parser instance.
	strmid, stream id.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_disable_stream(IN PSRCTX parser, IN IM_INT32 strmid);

/*
FUNC: enbale accelerate read mode.
PARAM: parser, parser instance.
	strmid, stream id.
	accel, accel mode configure.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_set_accel_read(IN PSRCTX parser, IN IM_INT32 strmid, IN PSR_ACCEL_READ *accel);

/*
FUNC: release a packet to parser, this packet can be used in accelerate mode.
PARAM: parser, parser instance.
	strmid, stream id.
	packet, released to parser to do accel read.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_release_accel_packet(IN PSRCTX parser, IN IM_INT32 strmid, PSR_PACKET *packet);

#ifdef _WINCE_
/*
FUNC: get a default fio implementation.
PARAM: fio, save this default fio pointer.
RETURN: IM_RET_OK succeed, else failed.
*/
PARSER_API IM_RET psr_ce_register_default_fio(OUT PSR_FIO *fio);
#endif
//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_PARSERAPI_H__

