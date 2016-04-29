/*
	This is part of pydawg Python module.

	Declaration of python class DAWG iterator.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
	License   : Public domain
*/

#ifndef pydawg_DAWGIterator_class_h_included__
#define pydawg_DAWGIterator_class_h_included__

#include "common.h"
#include "dawgnode.h"
#include "dawg.h"
#include "slist.h"

typedef enum {
	MATCH_EXACT_LENGTH,
	MATCH_AT_LEAST_PREFIX,
	MATCH_AT_MOST_PREFIX
} PatternMatchType;

typedef struct DAWGIterator {
	PyObject_HEAD

	DAWGclass*	dawg;		///< DAWG
	int			version;	///< DAWG version, used to invalidate iterator when DAWG has chanbed
	DAWGNode*	state;		///< current node
	List		stack;		///< stack
	DAWG_LETTER_TYPE* buffer;	///< string buffer

	DAWG_LETTER_TYPE* pattern;	///< pattern
	size_t pattern_length;
	DAWG_LETTER_TYPE wildcard;
	bool use_wildcard;

	PatternMatchType matchtype;
} DAWGIterator;


/* create new iterator object */
static PyObject*
DAWGIterator_new(
	DAWGclass* dawg,
	DAWG_LETTER_TYPE* word,
	const size_t wordlen,

	const bool use_wildcard,
	const DAWG_LETTER_TYPE wildcard,

	const PatternMatchType matchtype
);

#endif
