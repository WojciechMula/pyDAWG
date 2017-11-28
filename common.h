/*
	This is part of pydawg Python module.
	
	common definitions and includes

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : public domain
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

#ifdef __GNUC__
#	define	LIKELY(x)	__builtin_expect(x, 1)
#	define	UNLIKELY(x)	__builtin_expect(x, 0)
#	define	ALWAYS_INLINE	__attribute__((always_inline))
#	define	PURE			__attribute__((pure))
#	define	UNUSED			__attribute__((__unused__))
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

#if __SIZEOF_POINTER__ != 4 && __SIZEOF_POINTER__ != 8
#	error "unsupported pointer size"
#endif

#if __SIZEOF_POINTER__ == 4
#	define	MACHINE32BIT
#else
#	define	MACHINE64BIT
#endif


//#define DEBUG_MEM
#ifdef DEBUG_MEM
void* memalloc(size_t size) {
    void* addr = PyMem_Malloc(size);
    printf("alloc %p %u\n", addr, size);
    return addr;
}

void memfree(void* addr) {
    ASSERT(addr != NULL); // It's OK to call PyMem_Free with NULL, however such a call indicates mistakes.
    printf("free %p\n", addr);
    PyMem_Free(addr);
}
#else
#   define memalloc PyMem_Malloc
#   define memfree  PyMem_Free
#endif

#ifndef __cplusplus
typedef char	bool;
#	define true 1
#	define false 0
#endif

#endif
