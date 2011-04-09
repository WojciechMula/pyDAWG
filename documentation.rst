========================================================================
                               pyDAWG
========================================================================

:Author:	Wojciech Mu³a, wojciech_mula@poczta.onet.pl
:Date:		$Date$


Introduction
------------

``PyDAWG`` is a python module implements :enwiki:`DAWG` graph structure,
which allow to store set of strings and check existence of a string in
linear time (in terms of a string length).

DAWG is constructed by **incremental algorithm** described in *Incremental
algorithm for sorted data*, **Jan Daciuk**, **Stoyan Mihov**, **Bruce Watson**,
and **Richard Watson**, Computational Linguistics, 26(1), March 2000.
Prof. Jan Daciuk offers also some useful documentation, presentations and
even sample code on `his site`__.

This algorithm asserts that input words are **sorted** in
:enwiki:`lexicographic order`; Python's ``sort()`` sorts strings
in this way.

__ http://www.eti.pg.gda.pl/katedry/kiw/pracownicy/Jan.Daciuk/personal/


Module
------

Module ``pydawg`` provides class ``DAWG``, symbolic constants
for class's ``state`` member (``EMPTY``, ``ACTIVE``, ``CLOSED``)
and member ``perfect_hashing`` (see `Minimal perfect hashing`_).


``DAWG`` class
--------------

``DAWG`` class is picklable__, and also provide independent
way of marshaling with methods ``binload()`` and ``bindump()``.

__ http://docs.python.org/py3k/library/pickle.html


Property
~~~~~~~~

``state`` [readonly integer]
	Following property values are possible:

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

Minimal :enwiki:`perfect hashing` (MPH) allows to find unique number
representing any word from DAWG, and also find word with given number.
Numbers are in always in range 1 ... ``len(DAWG)``.

Finally, this feature makes possible to perform fast lookups as
in a regular dictionary.

Algorithm used for MPH is described in *Applications of Finite Automata
Representing Large Vocabularies*, **Claudio Lucchiesi** and **Tomasz
Kowaltowski**, Software Practice and Experience, 23(1), pp. 15--30, Jan.
1993.

MPH feature is enabled during compilation time if preprocessor
definition ``DAWG_PERFECT_HASHING`` exists. Module member
``perfect_hashing`` reflects this setting.


.. warning::
	Words numbering is done for the whole DAWG. If new words
	are added with ``add_word`` and ``add_word_unchecked``,
	then current numbering is lost and next call of ``word2index``
	or ``index2word`` will renumber DAWG again. Frequent
	mixing these two groups of method will degrade performance.


``word2index(word) => index``
	Returns index of word, or None if word is not present in a DAWG.

``index2word(index) => word``
	Returns words associated with index, or None if index isn't valid.


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
	  ``nodes_count * node_size + edges_count * edge_size``

``get_hash_stats() => dict``
	Returns some statistics about hash table used by DAWG.

	* ``table_size``   --- number of table's elements
	* ``element_size`` --- size of single table item
	* ``items_count``  --- number of items saved in a table
	* ``item_size``    --- size of single item

	Approx memory occupied by hash table is
	``table_size * element_size + items_count * item_size``.


License
-------

Library is licensed under very liberal three-clauses BSD license.
Some portions has been released into public domain.

Full text of license is available in LICENSE file.
