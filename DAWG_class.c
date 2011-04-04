#include "DAWG_class.h"


static
PyTypeObject	dawg_type;


PyObject*
dawgobj_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
	DAWGclass	*dawg;

	dawg = (DAWGclass*)PyObject_New(DAWGclass, &dawg_type);
    if (UNLIKELY(dawg == NULL))
        return NULL;

	DAWG_init(&dawg->dawg);
	dawg->version		= 0;
	dawg->stats_version	= -1;	// stats are not valid

	return (PyObject*)dawg;
}


static void
dawgobj_del(PyObject* self) {
#define dawg (((DAWGclass*)self)->dawg)
	//DAWG_clear(&dawg); XXX
	PyObject_Del(self);
#undef automaton
}


static PyObject*
get_string(PyObject* value, String* string) {
	bool unicode;
	PyObject* obj;

	obj = pymod_get_string(
			value,
			&string->chars,
			&string->length,
			&unicode
	);

	return obj;
}


#define dawgmeth_add_word_doc \
	"Add new word."

static PyObject*
dawgmeth_add_word(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	const int ret = DAWG_add_word(&dawg, word);
	Py_DECREF(obj);

	switch (ret) {
		case 1:
			Py_RETURN_TRUE;

		case 0:
			Py_RETURN_FALSE;

		default:
			Py_RETURN_NONE;
	}
#undef dawg
}


static PyObject*
dawgmeth_add_word_unchecked(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	const int ret = DAWG_add_word_unchecked(&dawg, word);
	Py_DECREF(obj);

	if (ret)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef dawg
}


static int
dawgmeth_contains(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return -1;

	const int ret = DAWG_exists(&dawg, (uint8_t*)word.chars, word.length);
	Py_DECREF(obj);
	return ret;
#undef dawg
}


static PyObject*
dawgmeth_exists(PyObject* self, PyObject* value) {
	switch (dawgmeth_contains(self, value)) {
		case 1:
			Py_RETURN_TRUE;

		case 0:
			Py_RETURN_FALSE;

		default:
			return NULL;
	}
}


static PyObject*
dawgmeth_match(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	const int ret = DAWG_match(&dawg, (uint8_t*)word.chars, word.length);
	Py_DECREF(obj);

	if (ret)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef dawg
}


static PyObject*
dawgmeth_longest_prefix(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	size_t len = DAWG_longest_prefix(&dawg, (uint8_t*)word.chars, word.length);
	Py_DECREF(obj);

	return Py_BuildValue("i", len);
#undef dawg
}


static int
dawgmeth_len(PyObject* self) {
#define dawg (((DAWGclass*)self)->dawg)
	return dawg.count;
#undef dawg
}


static PyObject*
dawgmeth_clear(PyObject* self, PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	DAWG_clear(&dawg);
	Py_RETURN_NONE;
#undef dawg
}


static PyObject*
dawgmeth_close(PyObject* self, PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	DAWG_close(&dawg);
	Py_RETURN_NONE;
#undef dawg
}


static PyObject*
dawgmeth_get_stats(PyObject* self, PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	if (obj->stats_version != obj->version) {
		DAWG_get_stats(&dawg, &obj->stats);
		obj->stats_version = obj->version;
	}

    PyObject* dict = Py_BuildValue(
        "{s:i,s:i,s:i,s:i,s:i,s:i}",
#define emit(name) #name, obj->stats.name
        emit(nodes_count),
        emit(edges_count),
        emit(words_count),
        emit(longest_word),
        emit(sizeof_node),
        emit(graph_size)
#undef emit
    );

	return dict;
#undef dawg
#undef obj
}


typedef struct DumpAux {
	PyObject*	nodes;
	PyObject*	edges;
	char		error;
} DumpAux;


static int
dump_aux(DAWGNode* node, const size_t depth, void* extra) {
#define Dump ((DumpAux*)extra)
	PyObject* tuple;
	DAWGNode* child;
	int i;

#define append_tuple(list) \
	if (tuple == NULL) { \
		Dump->error = 1; \
		return 0; \
	} \
	else if (PySet_Add(list, tuple) < 0) { \
		Dump->error = 1; \
		return 0; \
	}

	// 1.
	tuple = Py_BuildValue("ii", node, (int)(node->eow));
	append_tuple(Dump->nodes)

	// 2.
	for (i=0; i < node->n; i++) {
		child = node->next[i].child;
		tuple = Py_BuildValue("ici", node, node->next[i].letter, child);
		append_tuple(Dump->edges)
	}

#undef append_tuple
#undef dump
	return 1;
}


static PyObject*
dawgmeth_dump(PyObject* self, PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	DumpAux dump;

	dump.nodes	= NULL;
	dump.edges	= NULL;
	dump.error	= 0;

	dump.nodes	= PySet_New(0);
	dump.edges	= PySet_New(0);
	if (dump.nodes == NULL or dump.edges == NULL)
		goto error;

	DAWG_traverse(&dawg, dump_aux, &dump);
	if (dump.error)
		goto error;
	else {
		return Py_BuildValue("OO", dump.nodes, dump.edges);
	}

error:
	Py_XDECREF(dump.nodes);
	Py_XDECREF(dump.edges);
	return NULL;
#undef dawg
}


typedef struct WordsAux {
	PyObject*	list;
	char		buffer[1024];
	bool		error;
} WordsAux;


static int
words_aux(DAWGNode* node, const size_t depth, WordsAux* words) {
	PyObject* bytes;
	DAWGNode* child;
	int i;

	if (node->eow) {
		bytes = PyBytes_FromStringAndSize(words->buffer, depth);
		if (bytes == NULL) {
			words->error = true;
			return 0;
		}
		else if (PyList_Append(words->list, bytes) < 0) {
			words->error = true;
			return 0;
		}
	}
	
	for (i=0; i < node->n; i++) {
		words->buffer[depth] = node->next[i].letter;
		child = node->next[i].child;
		if (words_aux(child, depth + 1, words) == 0)
			return 0;
	}

#undef words
	return 1;
}


static PyObject*
dawgmeth_words(PyObject* self, PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	WordsAux words;

	words.error	= false;
	words.list	= PyList_New(0);

	if (words.list == NULL)
		goto error;

	if (dawg.q0 != NULL)
		words_aux(dawg.q0, 0, &words);

	if (words.error)
		goto error;
	else {
		Py_INCREF(words.list);
		return words.list;
	}

error:
	Py_XDECREF(words.list);
	return NULL;
#undef dawg
}



static
PySequenceMethods dawg_as_sequence;

#define method(name, kind) {#name, dawgmeth_##name, kind, NULL}
#define methoddoc(name, kind) {#name, dawgmeth_##name, kind, dawgmeth_##name##_doc}
static
PyMethodDef dawg_methods[] = {
	method(add_word,			METH_O),
	method(add_word_unchecked,	METH_O),
	method(exists,				METH_O),
	method(match,				METH_O),
	method(longest_prefix,		METH_O),
	method(words,				METH_NOARGS),
	method(clear,				METH_NOARGS),
	method(close,				METH_NOARGS),

	method(get_stats,		METH_NOARGS),
	method(dump,			METH_NOARGS),

	{NULL, NULL, 0, NULL}
};
#undef method

static
PyTypeObject dawg_type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"pydawg.DAWG",								/* tp_name */
	sizeof(DAWGclass),							/* tp_size */
	0,											/* tp_itemsize? */
	(destructor)dawgobj_del,		       	  	/* tp_dealloc */
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
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	dawg_methods,								/* tp_methods */
	0,							                /* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,											/* tp_init */
	0,                                          /* tp_alloc */
	dawgobj_new,								/* tp_new */
};
