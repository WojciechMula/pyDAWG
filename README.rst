========================================================================
                               pyDAWG
========================================================================

.. image:: https://travis-ci.org/WojciechMula/pyDAWG.svg?branch=master
    :target: https://travis-ci.org/WojciechMula/pyDAWG

.. contents::

Introduction
============

``PyDAWG`` is a python module implements DAWG__ graph structure,
which allow to store set of strings and check existence of a string in
linear time (in terms of a string length).

DAWG is constructed by **incremental algorithm** described in *Incremental
algorithm for sorted data*, **Jan Daciuk**, **Stoyan Mihov**, **Bruce Watson**,
and **Richard Watson**, Computational Linguistics, 26(1), March 2000.
Prof. Jan Daciuk offers also some useful documentation, presentations and
even sample code on `his site`__.

The algorithm asserts that input words are **sorted** in
`lexicographic order`__; default Python ``sort()``
orders strings correctly.

Also **minimal perfect hashing** (MPH) is supported, i.e. there is a function
that maps words to unique number; this function is bidirectional, its possible
to find number for given word or get word from number.

__ http://en.wikipedia.org/wiki/DAWG
__ http://www.eti.pg.gda.pl/katedry/kiw/pracownicy/Jan.Daciuk/personal/
__ http://en.wikipedia.org/wiki/lexicographic%20order

------------------------------------------------------------------------

There are two versions of module:

* **C extension**, compatible only with Python3;
* pure python module, compatible with Python 2 and 3.

Python module implements subset of C extension API.


License
=======

Library is licensed under very liberal three-clauses BSD license.
Some portions has been released into public domain.

Full text of license is available in LICENSE file.


Author
======

Wojciech Muła, wojciech_mula@poczta.onet.pl


Installation
============

Compile time settings (can be change in setup.py):

* ``DAWG_UNICODE`` --- if defined, DAWG accepts and returns
  unicode strings, else bytes are supported

* ``DAWG_PERFECT_HASHING`` --- when defined, minimal perfect
  hashing is enabled (methods word2index and index2word are
  available)


Just run::

		$ python setup.py install

If compilation succed, module is ready to use.


API
===


Module
------

Module ``pydawg`` provides class ``DAWG`` and following members:

* ``EMPTY``, ``ACTIVE``, ``CLOSED`` --- symbolic constants for
  ``state`` member of ``DAWG`` object
* ``perfect_hashing`` -- see `Minimal perfect hashing`_
* ``unicode`` -- see `Unicode and bytes`_


Unicode and bytes
~~~~~~~~~~~~~~~~~

Type of strings accepted and returned by ``DAWG`` methods can be
either **unicode** or **bytes**, depending on compile time
settings (preprocessor definition ``DAWG_UNICODE``). Value of
module member ``unicode`` informs about chosen type.




``DAWG`` class
--------------

``DAWG`` class is picklable__, and also provide independent
way of marshaling with methods ``binload()`` and ``bindump()``.

__ http://docs.python.org/py3k/library/pickle.html


Property
~~~~~~~~

``state`` [read-only integer]
	Following values are possible:

	* ``pydawg.EMPTY`` --- no words in a set;
	* ``pydawg.ACTIVE`` --- there is at least one word in a set,
	  and adding new words is possible (see ``add_word`` & ``add_word_unchecked``);
	* ``pydawg.CLOSED`` --- there is at least one word in a set,
	  but adding new words is not allowed (see ``close``/``freeze``).


Basic mathods
~~~~~~~~~~~~~

``add_word(word) => bool``
	Add word, returns True if word didn't exists in a set.
	Procedure checks if ``word`` is greater then previously 
	added word (in lexicography order).

``add_word_unchecked(word) => bool``
	Does the same thing as ``add_word`` but do not check ``word``
	order. Method should be used if one is sure, that input data
	satisfy	algorithm requirements, i.e. words order is valid.

``exists(word) => bool`` or ``word in ...``
	Check if word is in set.

``match(word) => bool``
	Check if word or any of its prefix is in a set.

``longest_prefix(word) => int``
	Returns length of the longest prefix of word that exists in a set.

``len()`` protocol
	Returns number of distinct words.

``words() => list``
	Returns list of all words.

``find_all([word, [wildchar, [how]]]) => iterator``
	Returns iterator that match words depending on ``word`` argument.

	``find_all()``
		does the same job as ``iter()``

	``find_all(prefix)``
		Yields words that share a prefix

	``find_all(pattern, wildchar, [how])``
		Yields words that match a ``pattern`` with given ``wildchar`` (wildchar
		matches any char). Parameter ``how`` controls which words are matched:
		
		``MATCH_EXACT_LENGTH``
			words with the same length as a pattern

		``MATCH_AT_LEAST_PREFIX``
			words of length not less then pattern

		``MATCH_AT_MOST_PREFIX``
			words of length no greater then pattern


``clear()``
	Erase all words from set.

``close()`` or ``freeze()``
	Don't allow to add any new words, ``state`` value become
	``pydawg.CLOSED``. Also free memory occupied by	a hash table
	used to perform incremental algorithm (see also	``get_hash_stats()``).

	Can be reverted only by ``clear()``.


Iterator
~~~~~~~~

Class supports ``iter`` protocol, i.e. ``iter(DAWGobject)`` returns
iterator, a lazy version of ``words()`` method.


Minimal perfect hashing
~~~~~~~~~~~~~~~~~~~~~~~

Minimal `perfect hashing`__ (MPH) allows to find unique number
representing any word from DAWG, and also find word with given number.
Numbers are in always in range 1 ... ``len(DAWG)``.

Finally, this feature makes possible to perform fast lookups as
in a regular dictionary.

Algorithm used for MPH is described in *Applications of Finite Automata
Representing Large Vocabularies*, **Claudio Lucchesi** and **Tomasz
Kowaltowski**, Software Practice and Experience, 23(1), pp. 15--30, Jan.
1993.

MPH feature is enabled during compilation time if preprocessor
definition ``DAWG_PERFECT_HASHING`` exists. Module member
``perfect_hashing`` reflects this setting.

__ http://en.wikipedia.org/wiki/perfect%20hashing

.. warning::
	Words numbering is done for the whole DAWG. If new words
	are added with ``add_word`` or ``add_word_unchecked``,
	then current numbering is lost and when method ``word2index``
	or ``index2word`` is called, then DAWG is renumbered.
	
	Because of that frequent mixing these two groups of method
	will degrade performance.


``word2index(word) => index``
	Returns index of word, or None if word is not present in a DAWG.

``index2word(index) => word``
	Returns words associated with index, or None if index isn't valid.


Example
#######

::

	D = pydawg.DAWG()

	# fill DAWG with keys
	for key in sorted(dict):
		D.add_word_unchecked(key)

	# prepare values array
	V = [None] * len(D)

	for key, value in dict.items():
		index = D.word2index(key)
		assert index is not None

		V[index - 1] = value
		
	
	# lookups are possible now
	for word in user_input:
		index = D.word2index(word)
		if index is not None:
			print(word, "=>", V[index - 1])


Other
~~~~~

``dump() => (set of nodes, set of edges)``
	Returns sets describing DAWG, elements are tuples.
	
	Node tuple:

	* unique id of node (number)
	* end of word marker

	Edge tuple:

	* source node id
	* edge label --- letter
	* destination node id

	Distribution contains program ``dump2dot.py`` that shows how to
	convert output of this function to `graphviz`__ DOT language.

	__ http://graphviz.org

``bindump() => bytes``
	Returns binary DAWG data.

``binload(bytes)``
	Restore DAWG from binary data. Example::

		import pydawg

		A = pydawg.DAWG()
		with open('dump', 'wb') as f:
			f.write(A.bindump())

		B = pydawg.DAWG()
		with open('dump', 'rb') as f:
			B.binload(f.read())

``get_stats() => dict``
	Returns dictionary containing some statistics about
	underlaying data structure:

	* ``words_count``	--- number of distinct words (same as ``len(dawg)``)
	* ``longest_word``	--- length of the longest word
	* ``nodes_count``	--- number of nodes
	* ``edges_count``	--- number of edges
	* ``sizeof_node``	--- size of single node (in bytes)
	* ``sizeof_edge``	--- size of single node (in bytes)
	* ``graph_size``	--- size of whole graph (in bytes); it's about
	  ``nodes_count * sizeof_node + edges_count * sizeof_edge``

``get_hash_stats() => dict``
	Returns some statistics about hash table used by DAWG.

	* ``table_size``   --- number of table's elements
	* ``element_size`` --- size of single table item
	* ``items_count``  --- number of items saved in a table
	* ``item_size``    --- size of single item

	Approx memory occupied by hash table is
	``table_size * element_size + items_count * item_size``.
