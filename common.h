/*
	This is part of pydawg Python module.
	
	common definitions and includes

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : public domain
	Date      : $Date: $

	$Id$
*/

#ifndef ahocorasick_common_h_included__
#define ahocorasick_common_h_included__

#include <Python.h>
#include <structmember.h>	// PyMemberDef

#include <iso646.h>

#define memalloc	PyMem_Malloc
#define memfree		PyMem_Free
#define memrealloc	PyMem_Realloc

#ifdef __GNUC__
#	define	LIKELY(x)	__builtin_expect(x, 1)
#	define	UNLIKELY(x)	__builtin_expect(x, 0)
#	define	ALWAYS_INLINE	__attribute__((always_inline))
#	define	PURE			__attribute__((pure))
#	define	UNUSED			__attribute__((unused))
#else
#	define	LIKELY(x)	x
#	define	UNLIKELY(x)	x
#	define	ALWAYS_INLINE
#	define	PURE
#	define	UNUSED
#endif

#define DEBUG
#ifdef DEBUG
#	include <assert.h>
#	define	ASSERT(expr)	do {if (!(expr)) {printf("%s:%s:%d - '%s' failed!\n", __FILE__, __FUNCTION__, __LINE__, #expr); abort();} }while(0)
#else
#	define	ASSERT(expr)
#endif

#ifndef __cplusplus
typedef char	bool;
#	define true 1
#	define false 0
#endif

#endif
