/**
 * DESC:
 *
 *
 *
 *
 *
 *
 * LOG:
 *		leo@2011/02/16: first commit.
 */

#ifndef __IM_PARSERLIB_H__
#define __IM_PARSERLIB_H__


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef PARSERLIB_EXPORTS
	#define PARSERLIB_API		__declspec(dllexport)	/* For dll lib */
#else
	#define PARSERLIB_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define PARSERLIB_API
#endif
//#############################################################################

class IM_Parserlib{
public:	
	virtual ~IM_Parserlib(){}

	virtual IM_UINT32 lib_version(OUT IM_TCHAR *ver_string) = 0;
	virtual IM_RET lib_open(IM_TCHAR *media_name, IM_MEDIA_INFO **media_info, PSR_FIO *fio) = 0;
	virtual IM_RET lib_open_ext(IM_TCHAR *media_name, IM_MEDIA_INFO **media_info, PSR_FIO_EXT *fio) = 0;
	virtual IM_RET lib_close() = 0;
	virtual IM_RET lib_read(PSR_PACKET *pkt) = 0;
	virtual IM_RET lib_seek(PSR_SEEK *sek) = 0;
	virtual IM_RET lib_packData(PSR_PACKET *dst, PSR_PACKET *src, IM_UINT32 mode) = 0;
	virtual IM_RET lib_freePacketData(void *data) = 0;
};

PARSERLIB_API IM_RET parserlib_create(OUT IM_Parserlib **psrlib);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_PARSERLIB_H__
