#ifndef pydawg_DAWGIterator_class_h_included__
#define pydawg_DAWGIterator_class_h_included__

#include "common.h"
#include "dawgnode.h"
#include "dawg.h"
#include "slist.h"

typedef struct DAWGIterator {
	PyObject_HEAD

	DAWGclass*	dawg;
	int			version;
	DAWGNode*	state;
	List		stack;
	char*		buffer;

} DAWGIterator;


/* create new iterator object */
static PyObject*
DAWGIterator_new(DAWGclass* dawg);

#endif
