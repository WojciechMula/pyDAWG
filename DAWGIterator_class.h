/*
	This is part of pydawg Python module.

	Declaration of python class DAWG iterator.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
	License   : Public domain
	Date      : $Date$

	$Id$
*/

#ifndef pydawg_DAWGIterator_class_h_included__
#define pydawg_DAWGIterator_class_h_included__

#include "common.h"
#include "dawgnode.h"
#include "dawg.h"
#include "slist.h"

typedef struct DAWGIterator {
	PyObject_HEAD

	DAWGclass*	dawg;		///< DAWG
	int			version;	///< DAWG version, used to invalidate iterator when DAWG has chanbed
	DAWGNode*	state;		///< current node
	List		stack;		///< stack
	DAWG_LETTER_TYPE* buffer;	///< string buffer

} DAWGIterator;


/* create new iterator object */
static PyObject*
DAWGIterator_new(DAWGclass* dawg);

#endif
