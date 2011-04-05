========================================================================
                               pyDAWG
========================================================================


Introduction
------------

``PyDAWG`` python module implements :enwiki:`DAWG` graph structure, which
allow to store set of strings and check existence of string in linear time.

DAWG construction uses **incremental algorithm** by J.Duciuk and others.
see: . (Visit for paper and presentations ). The only disadvantage of the
algorithm is requirement to sort input words before adding them to DAWG.

There is a variant of incremental algorithm (by the same authors) that
do not have any assumption on input, but is not implemented in current
version of ``pyDAWG``.


Module
------

Module ``pydawg`` provides class ``DAWG``.


``DAWG`` class
--------------


Basic mathod
~~~~~~~~~~~~

``add_word(word) => bool``
	Add word, returns True if word didn't exists in a set.
	Procedure checks if ``word`` is greater then previously 
	added word (in lexicography order). This force 

``add_word_unchecked(word) => bool``
	Does the same thing as ``add_word`` but do not check ``word``.
	Method should be used if one is sure, that input data satisfy
	algorithm requirements, i.e. words order is correct.

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
	Erase all words from a set.

``close()``
	Don't allow to add any new words. Can be reverted only by ``clear()`` [???].


Iterator
~~~~~~~~

Class supports ``iter`` protocol, i.e. ``iter(DAWGobject)`` returns
iterator. Iterator is a lazy version of ``words()`` method.


Other
~~~~~

``get_stats() => dict``
	Returns dictionary containing some statistics about underlaying data structure:

	* ``nodes_count``	--- number of nodes
	* ``edges_count``	--- number of edges
	* ``words_count``	--- number of distinct words (same as ``len(dawg)``)
	* ``node_size``		--- size of single node (in bytes)
	* ``graph_size``	--- size of whole graph (in bytes); it's about
	  ``nodes_count * node_size + edges_count * pointer size``
	* ``longest_word``	--- length of the longest word


``dump() => (set of nodes, set of edges)``
	Returns sets describing DAWG, elements are tuples.
	
	Node tuple:

	* unique id of node (number)
	* end of word marker

	Edge tuple:

	* source node id
	* edge label --- letter
	* destination node id


Changes
-------

*none*
