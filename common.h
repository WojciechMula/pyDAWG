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

// setup supported character set
#ifdef DAWG_UNICODE
#	ifdef Py_UNICODE_WIDE
		// Python use UCS-4
#		define DAWG_LETTER_TYPE	Py_UNICODE
#		define DAWG_LETTER_SIZE 4
#	else
		// Python use UCS-2
#		define DAWG_LETTER_TYPE	Py_UNICODE
#		define DAWG_LETTER_SIZE 2
#	endif
#else
	// only bytes are supported
#	define DAWG_LETTER_TYPE	uint8_t
#	define DAWG_LETTER_SIZE 1
#endif



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
