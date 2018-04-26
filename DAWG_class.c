/*
	This is part of pydawg Python module.

	Definition of Python class DAWG.
	(wrapper for functions from dawg/dawgnode.{c,h})

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#include "DAWG_class.h"
#include "DAWGIterator_class.h"


static
PyTypeObject dawg_type;

static PyObject*
dawgmeth_binload(PyObject* self, PyObject* arg);


PyObject*
dawgobj_new(UNUSED PyTypeObject* type, UNUSED PyObject* args, UNUSED PyObject* kwargs) {
	DAWGclass	*dawg;

	dawg = (DAWGclass*)PyObject_New(DAWGclass, &dawg_type);
    if (UNLIKELY(dawg == NULL)) {
        return NULL;
    }

	DAWG_init(&dawg->dawg);
	dawg->version		= 0;
	dawg->stats_version	= -1;	// stats are not valid
#ifdef DAWG_PERFECT_HASHING
	dawg->mph_version	= -1;	// numbers are not valid
#endif

	if (PyTuple_Check(args) and PyTuple_Size(args) > 0) {
		if (PyTuple_Size(args) == 1) {
			PyObject* ret = dawgmeth_binload((PyObject*)dawg, PyTuple_GET_ITEM(args, 0));
			if (ret == NULL) {
				Py_DECREF(dawg);
				return NULL;
			}
		}
		else {
			PyErr_SetString(PyExc_ValueError, "constructor do not accept any arguments");
			Py_DECREF(dawg);
			return NULL;
		}
	}

	return (PyObject*)dawg;
}


static void
dawgobj_del(PyObject* self) {
#define dawg (((DAWGclass*)self)->dawg)
	DAWG_free(&dawg);
	PyObject_Del(self);
#undef dawg
}


static PyObject*
get_string(PyObject* value, String* string) {
	PyObject* obj;

	obj = pymod_get_string(
			value,
			&string->chars,
			&string->length
	);

	return obj;
}


#define dawgmeth_add_word_doc \
	"Add word, returns True if word didn't exists in a set." \
	"Procedure checks if ``word`` is greater then previously " \
	"added word (in lexicography order)."

static PyObject*
dawgmeth_add_word(PyObject* self, PyObject* value) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	String	word;
	PyObject* tmp;

	tmp = get_string(value, &word);
	if (tmp == NULL)
		return NULL;

	const int ret = DAWG_add_word(&dawg, word);
	Py_DECREF(tmp);

	switch (ret) {
		case 1:
			obj->version += 1;
			Py_RETURN_TRUE;

		case 0:
			Py_RETURN_FALSE;

		case DAWG_FROZEN:
			PyErr_SetString(
				PyExc_AttributeError,
				"DAWG has been freezed, no further chanages are allowed"
			);
			return NULL;

		case DAWG_WORD_LESS:
			PyErr_SetString(
				PyExc_ValueError,
				"word is less then previosuly added, can't update DAWG"
			);
			return NULL;

		case DAWG_NO_MEM:
			PyErr_NoMemory();
			return NULL;

		default:
			Py_RETURN_NONE;
	}
#undef dawg
}


#define dawgmeth_add_word_unchecked_doc \
	"Does the same thing as ``add_word`` but do not check ``word`` "\
	"order. Method should be used if one is sure, that input data " \
	"satisfy	algorithm requirements, i.e. words order is valid." \

static PyObject*
dawgmeth_add_word_unchecked(PyObject* self, PyObject* value) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	String	word;
	PyObject*	tmp;

	tmp = get_string(value, &word);
	if (tmp == NULL)
		return NULL;

	const int ret = DAWG_add_word_unchecked(&dawg, word);
	Py_DECREF(tmp);

	switch (ret) {
		case 1:
			obj->version += 1;
			Py_RETURN_TRUE;

		case 0:
			Py_RETURN_FALSE;

		case DAWG_FROZEN:
			PyErr_SetString(
				PyExc_AttributeError,
				"DAWG has been freezed, no further chanages are allowed"
			);
			return NULL;

		case DAWG_NO_MEM:
			PyErr_NoMemory();
			return NULL;

		default:
			Py_RETURN_NONE;
	}
#undef dawg
#undef obj
}


static int
dawgmeth_contains(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return -1;

	const int ret = DAWG_exists(&dawg, word.chars, word.length);
	Py_DECREF(obj);
	return ret;
#undef dawg
}


#define dawgmeth_exists_doc \
	"Check if word is in set."

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


#define dawgmeth_match_doc \
	"Check if word or any of its prefix is in a set."

static PyObject*
dawgmeth_match(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	const int ret = DAWG_match(&dawg, word.chars, word.length);
	Py_DECREF(obj);

	if (ret)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef dawg
}


static PyObject*
dawgmeth_iterator(PyObject* self) {
	return DAWGIterator_new(
			(DAWGclass*)self,
			NULL,
			0,
			false,
			0,
			MATCH_AT_LEAST_PREFIX
	);
}


#define dawgmeth_find_all_doc \
	""

static PyObject*
dawgmeth_find_all(PyObject* self, PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	PyObject* arg1 = NULL;
	PyObject* arg2 = NULL;
	PyObject* arg3 = NULL;
	DAWG_LETTER_TYPE* word;
	size_t wordlen;

	DAWG_LETTER_TYPE wildcard;
	bool use_wildcard = false;
	PatternMatchType matchtype = MATCH_AT_LEAST_PREFIX;

	// arg 1: prefix/prefix pattern
	if (args) 
		arg1 = PyTuple_GetItem(args, 0);
	else
		arg1 = NULL;
	
	if (arg1) {
		arg1 = pymod_get_string(arg1, &word, &wordlen);
		if (arg1 == NULL)
			goto error;
	}
	else {
		PyErr_Clear();
		word = NULL;
		wordlen = 0;
	}

	// arg 2: wildcard
	if (args)
		arg2 = PyTuple_GetItem(args, 1);
	else
		arg2 = NULL;

	if (arg2) {
		DAWG_LETTER_TYPE* tmp;
		size_t len;

		arg2 = pymod_get_string(arg2, &tmp, &len);
		if (arg2 == NULL)
			goto error;
		else {
			if (len == 1) {
				wildcard = tmp[0];
				use_wildcard = true;
			}
			else {
				PyErr_SetString(PyExc_ValueError, "wildcard have to be single character");
				goto error;
			}
		}
	}
	else {
		PyErr_Clear();
		wildcard = 0;
		use_wildcard = false;
	}

	// arg3: matchtype
	matchtype = MATCH_AT_LEAST_PREFIX;
	if (args) {
		arg3 = PyTuple_GetItem(args, 2);
		if (arg3) {
			Py_ssize_t val = PyNumber_AsSsize_t(arg3, PyExc_OverflowError);
			if (val == -1 and PyErr_Occurred())
				goto error;

			switch ((PatternMatchType)val) {
				case MATCH_AT_LEAST_PREFIX:
				case MATCH_AT_MOST_PREFIX:
				case MATCH_EXACT_LENGTH:
					matchtype = (PatternMatchType)val;
					break;

				default:
					PyErr_SetString(PyExc_ValueError,
						"third argument have to be one of MATCH_EXACT_LENGTH, "
						"MATCH_AT_LEAST_PREFIX, MATCH_AT_LEAST_PREFIX"
					);
					goto error;
			}
		}
		else {
			PyErr_Clear();
			if (use_wildcard)
				matchtype = MATCH_EXACT_LENGTH;
			else
				matchtype = MATCH_AT_LEAST_PREFIX;
		}
	}


	// 
	DAWGIterator* iter;
	iter = (DAWGIterator*)DAWGIterator_new(
				(DAWGclass*)self,
				word,
				wordlen,
				use_wildcard,
				wildcard,
				matchtype
			);

	Py_XDECREF(arg1);
	Py_XDECREF(arg2);

	if (iter)
		return (PyObject*)iter;
	else
		return NULL;

error:
	Py_XDECREF(arg1);
	Py_XDECREF(arg2);
	return NULL;
#undef dawg
}


#define dawgmeth_longest_prefix_doc \
	"Returns length of the longest prefix of word that exists in a set."

static PyObject*
dawgmeth_longest_prefix(PyObject* self, PyObject* value) {
#define dawg (((DAWGclass*)self)->dawg)
	String	word;
	PyObject*	obj;

	obj = get_string(value, &word);
	if (obj == NULL)
		return NULL;

	const size_t len = DAWG_longest_prefix(&dawg, word.chars, word.length);
	Py_DECREF(obj);

	return Py_BuildValue("i", len);
#undef dawg
}


static Py_ssize_t
dawgmeth_len(PyObject* self) {
#define dawg (((DAWGclass*)self)->dawg)
	return dawg.count;
#undef dawg
}


#define dawgmeth_clear_doc \
	"Erase all words from set."

static PyObject*
dawgmeth_clear(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	int result = DAWG_clear(&dawg);
	if(result==DAWG_NO_MEM) {
		PyErr_NoMemory();
		return NULL;
	}
	obj->version += 1;
	Py_RETURN_NONE;
#undef dawg
#undef obj
}


#define dawgmeth_close_doc \
	"Don't allow to add any new words. Also free some memory (a hash table) " \
	"used to perform incremental algorithm." \
	"Can be reverted only by ``clear()``." \


static PyObject*
dawgmeth_close(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	DAWG_close(&dawg);
	obj->version += 1;
	Py_RETURN_NONE;
#undef dawg
#undef obj
}


#define dawgmeth_get_stats_doc \
	"Returns dictionary containing some statistics about underlaying data structure:\n" \
	"* ``nodes_count``	--- number of nodes\n" \
	"* ``edges_count``	--- number of edges\n" \
	"* ``words_count``	--- number of distinct words (same as ``len(dawg)``)\n" \
	"* ``node_size``	--- size of single node (in bytes)\n" \
	"* ``graph_size``	--- size of whole graph (in bytes); it's about\n" \
	"  ``nodes_count * node_size + edges_count * pointer size``\n" \
	"* ``longest_word``	--- length of the longest word\n" \
	"* ``hash_tbl_size``	--- size of a helper hash table\n" \
	"* ``hash_tbl_count`` --- number of items in a helper hash table"

static void update_stats(DAWGclass *obj) {
	if (obj->stats_version != obj->version) {
		DAWG_get_stats(&obj->dawg, &obj->stats);
		obj->stats_version = obj->version;
	}
}


static PyObject*
dawgmeth_get_stats(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	update_stats(obj);

    PyObject* dict = Py_BuildValue(
        "{s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
#define emit(name) #name, obj->stats.name
        emit(nodes_count),
        emit(edges_count),
        emit(words_count),
        emit(longest_word),
        emit(sizeof_node),
        emit(sizeof_edge),
        emit(graph_size)
#undef emit
    );

	return dict;
#undef dawg
#undef obj
}


#define dawgmeth_get_hash_stats_doc \
	"Returns dict containing some info about hash table used by DAWG"

static PyObject*
dawgmeth_get_hash_stats(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	DAWGHashStatistics stats;
	DAWG_get_hash_stats(&dawg, &stats);

    PyObject* dict = Py_BuildValue(
        "{s:i,s:i,s:i,s:i}",
#define emit(name) #name, stats.name
        emit(table_size),
        emit(element_size),
        emit(items_count),
        emit(item_size)
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
dump_aux(DAWGNode* node, UNUSED const size_t depth, void* extra) {
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


#define dawgmeth_dump_doc \
	"Returns  sets describing DAWG, elements are tuples." \
	"Node tuple: unique id of node (number), end of word marker" \
	"Edge tuple: source node id, edge label --- letter, destination node id"

static PyObject*
dawgmeth_dump(PyObject* self, UNUSED PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	DumpAux dump;

	dump.nodes	= NULL;
	dump.edges	= NULL;
	dump.error	= 0;

	dump.nodes	= PySet_New(0);
	dump.edges	= PySet_New(0);
	if (dump.nodes == NULL or dump.edges == NULL)
		goto error;

	DAWG_traverse_DFS(&dawg, dump_aux, &dump);
	if (dump.error)
		goto error;
	else
		return Py_BuildValue("OO", dump.nodes, dump.edges);

error:
	Py_XDECREF(dump.nodes);
	Py_XDECREF(dump.edges);
	return NULL;
#undef dawg
}


typedef struct WordsAux {
	PyObject* list;
	DAWG_LETTER_TYPE* buffer;
	bool error;
} WordsAux;


static int
words_aux(DAWGNode* node, const size_t depth, WordsAux* words) {
	PyObject* item;
	DAWGNode* child;
	int i;

	if (node->eow) {
#ifdef DAWG_UNICODE
		item = PyUnicode_FromUnicode(words->buffer, depth);
#else
		item = PyBytes_FromStringAndSize((char*)words->buffer, depth);
#endif
		if (item == NULL) {
			words->error = true;
			return 0;
		}
		else if (PyList_Append(words->list, item) < 0) {
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


#define dawgmeth_words_doc \
	"Returns number of distinct words."

static PyObject*
dawgmeth_words(PyObject* self, UNUSED PyObject* args) {
#define dawg (((DAWGclass*)self)->dawg)
	WordsAux words;

	words.error		= false;
	words.buffer	= NULL;
	words.list		= NULL;

	words.buffer = (DAWG_LETTER_TYPE*)memalloc((dawg.longest_word + 1) * DAWG_LETTER_SIZE);
	if (words.buffer == NULL) {
		PyErr_NoMemory();
		goto error;
	}

	words.list = PyList_New(0);
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

	if (words.buffer != NULL)
		memfree(words.buffer);

	return NULL;
#undef dawg
}


#define dawgmeth_bindump_doc \
	"Returns binary image of DAWG"

static PyObject*
dawgmeth_bindump(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	uint8_t* array;
	size_t size;

	PyObject* res;

	update_stats(obj);

	switch (DAWG_save(&dawg, &obj->stats, &array, &size)) {
		case DAWG_OK:
			res = PyBytes_FromStringAndSize((char*)array, size);
			memfree(array);
			return res;

		case DAWG_NO_MEM:
			PyErr_NoMemory();
			return NULL;

		default:
			memfree(array);
			PyErr_SetString(PyExc_AssertionError, "internal error, function DAWG_save returned unexpected value");
			return NULL;
	}
#undef dawg
#undef obj
}


#define dawgmeth_binload_doc \
	"Load DAWG with data returned by bindump"

static PyObject*
dawgmeth_binload(PyObject* self, PyObject* arg) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	if (not PyBytes_Check(arg)) {
		PyErr_SetString(PyExc_TypeError, "bytes object expected");
		return NULL;
	}

	void* array;
	ssize_t size;

	if (PyBytes_AsStringAndSize(arg, (char**)&array, &size) < 0)
		return NULL;
	else
		Py_INCREF(arg);

	const int ret = DAWG_load(&dawg, array, size);
	Py_DECREF(arg);
	switch (ret) {
		case DAWG_OK:
			obj->version = -1;
			obj->stats_version = -2;
#ifdef DAWG_PERFECT_HASHING
			obj->mph_version = -2;
#endif

			Py_RETURN_NONE;

		case DAWG_NO_MEM:
			PyErr_NoMemory();
			return NULL;

		case DAWG_DUMP_TRUNCATED:
			PyErr_SetString(
				PyExc_ValueError,
				"input data truncated"
			);
			return NULL;

		case DAWG_DUMP_INVALID_MAGICK:
			PyErr_SetString(
				PyExc_ValueError,
				"input data invalid: header corrupted - bad magick"
			);
			return NULL;

		case DAWG_DUMP_INVALID_STATE:
			PyErr_SetString(
				PyExc_ValueError,
				"input data invalid: header corrupted - invalid DAWG state"
			);
			return NULL;

		case DAWG_DUMP_INVALID_ROOT_ID:
			PyErr_SetString(
				PyExc_ValueError,
				"input data invalid: header corrupted - invalid root node id"
			);
			return NULL;

		case DAWG_DUMP_CORRUPTED_1:
			PyErr_SetString(
				PyExc_ValueError,
				"input data invalid: missing nodes"
			);
			return NULL;

		case DAWG_DUMP_CORRUPTED_2:
			PyErr_SetString(
				PyExc_ValueError,
				"input data invalid: invalid index"
			);
			return NULL;

		default:
			PyErr_SetString(PyExc_AssertionError, "internal error, function DAWG_load returned unexpected value");
			return NULL;
	}
#undef obj
#undef dawg
}


#define dawgmeth___reduce___doc \
	"reduce protocol"

static PyObject*
dawgmeth___reduce__(PyObject* self, UNUSED PyObject* args) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	// return pair: type, bytes
	PyObject* bytes;

	bytes = dawgmeth_bindump(self, NULL);
	if (bytes)
		return Py_BuildValue("O(O)", Py_TYPE(self), bytes);
	else
		return NULL;
#undef obj
#undef dawg
}


#ifdef DAWG_PERFECT_HASHING

#define dawgmeth_word2index_doc \
	"word2index(word) => integer\n" \
	"Returns unique integer in range 1..len() identifies a word." \
	"If word is not present in DAWG, returns None"

static PyObject*
dawgmeth_word2index(PyObject* self, PyObject* arg) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	String word;
	PyObject* bytes;

	bytes = get_string(arg, &word);
	if (bytes == NULL)
		return NULL;

	if (obj->mph_version != obj->version) {
		DAWG_mph_numerate_nodes(&dawg);
		obj->mph_version = obj->version;
	}

	const size_t result = DAWG_mph_word2index(
							&dawg,
							word.chars,
							word.length
						);
	Py_DECREF(bytes);

	switch (result) {
		case DAWG_NOT_EXISTS:
			Py_RETURN_NONE;

		default:
			return Py_BuildValue("i", result);
	}
#undef obj
#undef dawg
}


#define dawgmeth_index2word_doc \
	"index2word(integer) => string\n" \
	"Returns word identified by given integer."

static PyObject*
dawgmeth_index2word(PyObject* self, PyObject* arg) {
#define obj ((DAWGclass*)self)
#define dawg (obj->dawg)
	Py_ssize_t index;

	index = PyNumber_AsSsize_t(arg, PyExc_OverflowError);
	if (index == -1 and PyErr_Occurred())
		return NULL;

	if (obj->mph_version != obj->version) {
		DAWG_mph_numerate_nodes(&dawg);
		obj->mph_version = obj->version;
	}
	
	DAWG_LETTER_TYPE* word;
	size_t wordlen;

	const int result = DAWG_mph_index2word(&dawg, index, &word, &wordlen);
	switch (result) {
		case DAWG_NOT_EXISTS:
			Py_RETURN_NONE;

		case DAWG_NO_MEM:
			PyErr_NoMemory();
			return NULL;

		case DAWG_EXISTS:
			{
			PyObject* result;
#ifdef DAWG_UNICODE
			result = PyUnicode_FromUnicode(word, wordlen);
#else
			result = PyBytes_FromStringAndSize((const char*)word, (ssize_t)wordlen);
#endif
			memfree(word);
			return result;
			}

		default:
			ASSERT(0);
	}

#undef obj
#undef dawg
}
#endif



static
PySequenceMethods dawg_as_sequence;

static
PyMemberDef dawg_members[] = {
	{
		"state",
		T_INT,
		offsetof(DAWGclass, dawg) + offsetof(DAWG, state),
		READONLY,
		"current state of DAWG"
	},

	{NULL}
};


#define method(name, kind) {#name, dawgmeth_##name, kind, dawgmeth_##name##_doc}
static
PyMethodDef dawg_methods[] = {
	method(add_word,			METH_O),
	method(add_word_unchecked,	METH_O),
	method(exists,				METH_O),
	method(match,				METH_O),
	method(longest_prefix,		METH_O),
	method(words,				METH_NOARGS),
	method(find_all,			METH_VARARGS),
	method(clear,				METH_NOARGS),
	method(close,				METH_NOARGS),
	{"freeze", dawgmeth_close, METH_NOARGS, dawgmeth_close_doc},	// alias

#ifdef DAWG_PERFECT_HASHING
	method(word2index,			METH_O),
	method(index2word,			METH_O),
#endif

	method(bindump,				METH_NOARGS),
	method(binload,				METH_O),
	method(__reduce__,			METH_NOARGS),

	method(get_stats,			METH_NOARGS),
	method(get_hash_stats,		METH_NOARGS),
	method(dump,				METH_NOARGS),

	{NULL, NULL, 0, NULL}
};
#undef method

static
PyTypeObject dawg_type = {
	PY_OBJECT_HEAD_INIT
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
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                         /* tp_flags */
	0,                                          /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	dawgmeth_iterator,							/* tp_iter */
	0,                                          /* tp_iternext */
	dawg_methods,								/* tp_methods */
	dawg_members,								/* tp_members */
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
