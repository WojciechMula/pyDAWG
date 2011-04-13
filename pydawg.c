/*
	This is part of pydawg Python module.

	Python module body.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#include "common.h"
#include "dawgnode.h"
#include "dawg.h"
#include "DAWG_class.h"
#include "DAWGIterator_class.h"

// c libary inlined
#include "dawgnode.c"
#include "dawg.c"

// python class
#include "utils.c"
#include "DAWG_class.c"
#include "DAWGIterator_class.c"

// module
static
PyModuleDef pydawg_module = {
	PyModuleDef_HEAD_INIT,
	"pydawg",
	"pydawg module",
	-1,
	NULL
};


PyMODINIT_FUNC
PyInit_pydawg(void) {
	PyObject* module;

	dawg_as_sequence.sq_length   = dawgmeth_len;
	dawg_as_sequence.sq_contains = dawgmeth_contains;

	dawg_type.tp_as_sequence = &dawg_as_sequence;
	
	module = PyModule_Create(&pydawg_module);
	if (module == NULL)
		return NULL;

	if (PyType_Ready(&dawg_type) < 0) {
		Py_DECREF(module);
		return NULL;
	}
	else
		PyModule_AddObject(module, "DAWG", (PyObject*)&dawg_type);

#define constant(name) PyModule_AddIntConstant(module, #name, name)
	constant(EMPTY);
	constant(ACTIVE);
	constant(CLOSED);

	constant(MATCH_EXACT_LENGTH);
	constant(MATCH_AT_MOST_PREFIX);
	constant(MATCH_AT_LEAST_PREFIX);
#undef constant

#ifdef DAWG_PERFECT_HASHING
	PyModule_AddIntConstant(module, "perfect_hasing", 1);
#else
	PyModule_AddIntConstant(module, "perfect_hasing", 0);
#endif

#ifdef DAWG_UNICODE
	PyModule_AddIntConstant(module, "unicode", 1);
#else
	PyModule_AddIntConstant(module, "unicode", 0);
#endif

	return module;
}
