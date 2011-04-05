#include "DAWGIterator_class.h"


static PyTypeObject dawg_iterator_type;


typedef struct DAWGIteratorStackItem {
	LISTITEM_data

	struct	DAWGNode*	node;
	uint8_t	letter;
	size_t	depth;
} DAWGIteratorStackItem;

#define StackItem DAWGIteratorStackItem


static PyObject*
DAWGIterator_new(DAWGclass* dawg) {
	ASSERT(dawg);

	DAWGIterator* iter;

	iter = (DAWGIterator*)PyObject_New(DAWGIterator, &dawg_iterator_type);
	if (iter == NULL)
		return NULL;

	iter->dawg		= dawg;
	iter->version	= dawg->version;
	list_init(&iter->stack);

	StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
	if (not new_item) {
		PyObject_Del((PyObject*)iter);
		PyErr_SetNone(PyExc_MemoryError);
		return NULL;
	}

	iter->buffer = memalloc(dawg->dawg.longest_word);
	if (iter->buffer == NULL) {
		PyObject_Del((PyObject*)iter);
		return NULL;
	}
	else {
		new_item->node   = dawg->dawg.q0;
		new_item->letter = 0;
		new_item->depth	 = 0;
		list_push_front(&iter->stack, (ListItem*)new_item);
	}

	Py_INCREF((PyObject*)iter->dawg);
	return (PyObject*)iter;
}


#define iter ((DAWGIterator*)self)

static void
DAWGIterator_del(PyObject* self) {
	if (iter->buffer)
		memfree(iter->buffer);
	
	list_delete(&iter->stack);
	Py_DECREF(iter->dawg);

	PyObject_Del(self);
}


static PyObject*
DAWGIterator_iter(PyObject* self) {
	Py_INCREF(self);
	return self;
}


static PyObject*
DAWGIterator_next(PyObject* self) {
	if (UNLIKELY(iter->version != iter->dawg->version)) {
		PyErr_SetString(PyExc_ValueError, "underlaying graph has changed, iterator is not valid anymore");
		return NULL;
	}

	while (true) {
		StackItem* item = (StackItem*)list_pop_first(&iter->stack);
		if (item == NULL or item->node == NULL)
			return NULL; /* Stop iteration */

		iter->state = item->node;
		const int n = iter->state->n;
		int i;
		for (i=0; i < n; i++) {
			StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
			if (not new_item) {
				PyErr_NoMemory();
				return NULL;
			}

			new_item->node  = iter->state->next[i].child;
			new_item->letter= iter->state->next[i].letter;
			new_item->depth = item->depth + 1;
			list_push_front(&iter->stack, (ListItem*)new_item);
		}

		iter->buffer[item->depth] = item->letter;

		if (iter->state->eow)
			return PyBytes_FromStringAndSize(iter->buffer + 1, item->depth);
	}
}

#undef StackItem
#undef iter

static PyTypeObject dawg_iterator_type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"DAWGIterator",								/* tp_name */
	sizeof(DAWGIterator),						/* tp_size */
	0,											/* tp_itemsize? */
	(destructor)DAWGIterator_del,				/* tp_dealloc */
	0,                                      	/* tp_print */
	0,                                         	/* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,											/* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	PyObject_GenericGetAttr,                    /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                         /* tp_flags */
	0,                                          /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	DAWGIterator_iter,							/* tp_iter */
	DAWGIterator_next,							/* tp_iternext */
	0,											/* tp_methods */
	0,						                	/* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,                                          /* tp_init */
	0,                                          /* tp_alloc */
	0,                                          /* tp_new */
};

#undef StackItem
