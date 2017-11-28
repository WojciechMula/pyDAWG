/*
	This is part of pydawg Python module.
	
	Helpers functions.
	This file is included directly.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : public domain
	Date      : $Date$

	$Id$
*/


/* returns bytes or unicode internal buffer */
static PyObject*
pymod_get_string(PyObject* obj, DAWG_LETTER_TYPE** word, size_t* wordlen) {
#ifdef DAWG_UNICODE
	if (PyUnicode_Check(obj)) {
		*word = (DAWG_LETTER_TYPE*)PyUnicode_AS_UNICODE(obj);
		*wordlen = (size_t)PyUnicode_GET_SIZE(obj);
		Py_INCREF(obj);
		return obj;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "string expected");
		return NULL;
	}
#else
	if (PyBytes_Check(obj)) {
		*word = (uint8_t*)PyBytes_AS_STRING(obj);
		*wordlen = (size_t)PyBytes_GET_SIZE(obj);
		Py_INCREF(obj);
		return obj;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "bytes expected");
		return NULL;
	}
#endif
}

