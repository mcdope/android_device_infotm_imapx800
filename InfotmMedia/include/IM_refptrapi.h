/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Revision History: 
** ----------------- 
** 1.0.1	leo@2012/05/05: first commit.
**
*****************************************************************************/

#ifndef __IM_REFPTRAPI_H__
#define __IM_REFPTRAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef REFPTR_EXPORTS
	#define REFPTR_API		__declspec(dllexport)	/* For dll lib */
#else
	#define REFPTR_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define REFPTR_API
#endif	
//#############################################################################
//
//
//
typedef void *	REFPTR;

//
//
//
typedef void (*func_refptr_destroy_t)(void *dataptr, IM_UINT32 key);

//
// APIs.
//
REFPTR_API IM_UINT32 refptr_version(OUT IM_TCHAR *ver_string);

REFPTR_API IM_RET refptr_create(OUT REFPTR *rp, IN void *dataptr, IN IM_UINT32 key, IN func_refptr_destroy_t destroy); // auto add ref to this refptr.
REFPTR_API IM_RET refptr_release(IN REFPTR rp);	// del ref to this refptr, when refcnt is 0, call the destroy function.
REFPTR_API IM_RET refptr_addref(IN REFPTR rp);
REFPTR_API IM_RET refptr_lock_access(IN REFPTR rp, IN IM_INT32 timeout, OUT void **dataptr, OUT IM_UINT32 *key);
REFPTR_API IM_RET refptr_unlock_access(IN REFPTR rp);


//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_REFPTRAPI_H__

