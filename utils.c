/*
	This is part of pyahocorasick Python module.
	
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
pymod_get_string(PyObject* obj, char** word, ssize_t* wordlen, bool* unicode) {
	if (PyBytes_Check(obj)) {
		*word = (char*)PyBytes_AS_STRING(obj);
		*wordlen = PyBytes_GET_SIZE(obj);
		Py_INCREF(obj);
		*unicode = false;
		return obj;
	}
	else if (PyUnicode_Check(obj)) {
		*word = (char*)PyUnicode_AS_UNICODE(obj);
		*wordlen = PyUnicode_GET_SIZE(obj);
		Py_INCREF(obj);
		*unicode = true;
		return obj;
	}
	else {
		PyErr_SetString(PyExc_ValueError, "string or bytes object expected");
		return NULL;
	}
}

