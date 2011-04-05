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

	return module;
}
