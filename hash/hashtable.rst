Simple hash table
=================

:Author:  Wojciech Muła
:License: public domain
:Date:    $Date$


Functions
~~~~~~~~~

``hashtable_init(hashtable, size) => status``
	Initialize hash table, set initial size.

``hashtable_resize(hashtable, size)``
	Change table of hash table.

``hashtable_destroy(hashtable)``
	Destroy hash table.

``hashtable_clear(hashtable)``
	Remove all elements from table.

``hashtable_get_list(hashtabe, hashvalue) => list item``
	Return head of element's list sharing same hash value.

``hashtable_add(hashtable, key, data)`` or ``hashtable_add(hashtable, key)``
	Add new key, data or just key (if **HASH_DATA_TYPE** is not defined).

``hashtable_get(hashtable, key) => list item``
	Return list item contains searched element or ``NULL``.

``hashtable_del(hashtable, key) => list item``
	Unbind list item containting given element.
	List item have to be free with **HASH_FREE**.


Usage
~~~~~

Library is parametrized with following preprocesor definitions:

**HASHNAME(name)**
	Returns final name of type or function. For example
	if defined as ``prefix_##name`` then names of all
	library types and functions start with ``prefix_``.
	This makes possible to have multiple versions of hash
	table with different names/types.

**HASH_TYPE**
	Type of hash value, for example ``uint32_t``.

**HASH_KEY_TYPE**
	Type of key, for example ``char*``, ``int``.

**HASH_DATA_TYPE** [optional]
	Type of data associated with key. If ommited that data field
	is not available nor used.

**HASH_EQ_FUN(a, b)**
	Key equality function, for example ``(strcmp((a), (b)) == 0)`` when
	keys are strings, or ``((a) == (b))`` for integer, chars etc.

**HASH_GET_HASH(x)**
	Returns hash for key ``x``.

**HASH_STATIC**
	Define as ``static`` if hash functions have to be declared as static.
	Define as *empty string* otherwise.

**HASH_ALLOC**
	Name of function that allocating memory, for example ``malloc``.

**HASH_FREE**
	Name of function that free memory, for example ``free``.

**HASH_x_UNUSED**
	Where x = **RESIZE**, **DESTROY**, **CLEAR**, **GET_LIST**,
	**GET**, **DEL**. If any defined, then adequate function is
	not available.


There is file ``hashtable_undefall.h`` that clear all definitions
related to library.


Example
~~~~~~~

See sample code ``hashtable_setup.h`` and ``hashtable_test.c``.


