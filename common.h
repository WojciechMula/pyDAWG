/*
	This is part of pydawg Python module.
	
	common definitions and includes

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : public domain
*/

#ifndef dawg_common_h_included__
#define dawg_common_h_included__

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

#ifdef DEBUG
#	include <assert.h>
#	define	ASSERT(expr)	do {if (!(expr)) {printf("%s:%s:%d - '%s' failed!\n", __FILE__, __FUNCTION__, __LINE__, #expr); abort();} }while(0)
#else
#	define	ASSERT(expr)
#endif

#if defined(__SIZEOF_POINTER__)
#   define PYDAWG_POINTER_SIZE __SIZEOF_POINTER__
#elif defined(_WIN64)
#   define PYDAWG_POINTER_SIZE 8
#elif defined(_WIN32)
#   define PYDAWG_POINTER_SIZE 4
#else
#	error "Can't obtain the pointer size"
#endif

#if PYDAWG_POINTER_SIZE == 4
#	define	MACHINE32BIT
typedef uint32_t nodeid_t;
#elif PYDAWG_POINTER_SIZE == 8
#	define	MACHINE64BIT
typedef uint64_t nodeid_t;
#else
#   error "Unsupported pointer size"
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

void *memcalloc(size_t nmemb, size_t size) {
#   if PY_VERSION_HEX >=  0x03050000
    void* addr = PyMem_Calloc(nmemb, size);
#   else
    void *addr = memalloc(nmemb*size);
    memset(addr, 0, nmemb*size);
#   endif
    printf("calloc %p %u\n", addr, nmemb*size);
    return addr;
}

#else
#   define memalloc PyMem_Malloc
#   define memfree  PyMem_Free
#   if PY_VERSION_HEX >=  0x03050000
#     define memcalloc	PyMem_Calloc
#   else
      static inline void *memcalloc(size_t nmemb, size_t size) {
	void *addr = memalloc(nmemb*size);
	memset(addr, 0, nmemb*size);
	return addr;
      }
#   endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#   define PY_OBJECT_HEAD_INIT PyVarObject_HEAD_INIT(NULL, 0)
#else
#   define PY_OBJECT_HEAD_INIT PyVarObject_HEAD_INIT(&PyType_Type, 0)
#endif

#ifndef __cplusplus
typedef char	bool;
#	define true 1
#	define false 0
#endif

#endif
