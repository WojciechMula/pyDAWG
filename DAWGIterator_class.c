/*
	This is part of pydawg Python module.

	Definition of python class DAWG iterator.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#include "DAWGIterator_class.h"
#include "dawgnode.h"


static PyTypeObject dawg_iterator_type;


typedef struct DAWGIteratorStackItem {
	LISTITEM_data

	struct	DAWGNode*	node;
	size_t	depth;
	DAWG_LETTER_TYPE letter;
} DAWGIteratorStackItem;

#define StackItem DAWGIteratorStackItem


static PyObject*
DAWGIterator_new(
	DAWGclass* dawg,
	DAWG_LETTER_TYPE* word,
	const size_t wordlen,

	const bool use_wildcard,
	const DAWG_LETTER_TYPE wildcard,

	const PatternMatchType matchtype
) {
	ASSERT(dawg);

	DAWGIterator* iter;

	iter = (DAWGIterator*)PyObject_New(DAWGIterator, &dawg_iterator_type);
	if (iter == NULL)
		return NULL;

	iter->dawg		= dawg;
	iter->version	= dawg->version;
	iter->pattern	= NULL;
	iter->pattern_length = 0;
	iter->use_wildcard = use_wildcard;
	iter->wildcard	= wildcard;
	iter->matchtype = matchtype;
	list_init(&iter->stack);

	ASSERT(
		matchtype == MATCH_EXACT_LENGTH or
		matchtype == MATCH_AT_LEAST_PREFIX or
		matchtype == MATCH_AT_MOST_PREFIX
	);

	StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
	if (not new_item) {
		PyObject_Del((PyObject*)iter);
		PyErr_NoMemory();
		return NULL;
	}

	iter->buffer = (DAWG_LETTER_TYPE*)memalloc((dawg->dawg.longest_word + 1) * DAWG_LETTER_SIZE);
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

	if (word and wordlen > 0) {
		iter->pattern = (DAWG_LETTER_TYPE*)memalloc(wordlen * DAWG_LETTER_SIZE);
		if (UNLIKELY(iter->pattern == NULL)) {
			PyObject_Del((PyObject*)iter);
			PyErr_NoMemory();
			return NULL;
		}
		else {
			iter->pattern_length = wordlen;
			memcpy(iter->pattern, word, wordlen * DAWG_LETTER_SIZE);
		}
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

		const size_t index = item->depth;
		if (iter->matchtype != MATCH_AT_LEAST_PREFIX and index > iter->pattern_length) {
			continue;
		}

		bool output;
		switch (iter->matchtype) {
			case MATCH_EXACT_LENGTH:
				output = (index == iter->pattern_length);
				break;

			case MATCH_AT_MOST_PREFIX:
				output = (index <= iter->pattern_length);
				break;
				
			case MATCH_AT_LEAST_PREFIX:
			default:
				output = (index >= iter->pattern_length);
				break;

		}

		iter->state = item->node;

		if ((index >= iter->pattern_length) or
		    (iter->use_wildcard and iter->pattern[index] == iter->wildcard)) {

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
				new_item->depth = index + 1;
				list_push_front(&iter->stack, (ListItem*)new_item);
			}
		}
		else {
			// process single letter
			const DAWG_LETTER_TYPE ch = iter->pattern[index];
			DAWGNode* node = dawgnode_get_child(iter->state, ch);

			if (node) {
				StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
				if (UNLIKELY(new_item == NULL)) {
					PyErr_NoMemory();
					return NULL;
				}

				new_item->node  = node;
				new_item->letter= ch;
				new_item->depth = index + 1;
				list_push_front(&iter->stack, (ListItem*)new_item);
			}
		}

		iter->buffer[item->depth] = item->letter;

		if (output and iter->state->eow)
#ifdef DAWG_UNICODE
			return PyUnicode_FromUnicode(iter->buffer + 1, item->depth);
#else
			return PyBytes_FromStringAndSize((char*)(iter->buffer + 1), item->depth);
#endif
	}
}

#undef StackItem
#undef iter

static PyTypeObject dawg_iterator_type = {
	PY_OBJECT_HEAD_INIT
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
