/*
	This is part of pydawg Python module.

	Definitions of nodes/edges of graph.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#include "dawgnode.h"


DAWGNode*
dawgnode_new() {
	DAWGNode* new = (DAWGNode*)memalloc(sizeof(DAWGNode));
	if (new) {
		new->n		= 0;
		new->next	= NULL;
		new->eow	= false;
		new->visited = 0;
#ifdef DAWG_PERFECT_HASHING
		new->number = 0;
#endif
	}

	return new;
}


void
dawgnode_free(DAWGNode* node) {
	if (node) {
		if (node->next)
			memfree(node->next);

		memfree(node);
	}
}


bool PURE
dawgnode_has_child(DAWGNode* node, const DAWG_LETTER_TYPE letter) {
	return (dawgnode_get_child(node, letter) != NULL);
}


static int PURE
dawgnode_get_child_idx(DAWGNode* node, const DAWG_LETTER_TYPE letter) {
	if (node and node->next) {
		// binary search
		int a = 0;
		int b = ((int)node->n) - 1;
		int c;

		while (a <= b) {
			c = (a + b)/2;
			if (node->next[c].letter == letter)
				return c;
			else if (node->next[c].letter > letter)
				b = c - 1;
			else
				a = c + 1;
		}
	}

	return -1;
}


DAWGNode* PURE
dawgnode_get_child(DAWGNode* node, const DAWG_LETTER_TYPE letter) {
	const int idx = dawgnode_get_child_idx(node, letter);
	if (idx >= 0)
		return node->next[idx].child;
	else
		return NULL;
}


DAWGNode*
dawgnode_set_child(DAWGNode* node, const DAWG_LETTER_TYPE letter, DAWGNode* child) {
	ASSERT(node);
	if (node->next) {
		int idx = dawgnode_get_child_idx(node, letter);
		if (idx >= 0) {
			// replace
			node->next[idx].child = child;
			return child;
		}
	}

	// insert (keep alphabetic order)
	DAWGEdge* newnext = memalloc((node->n + 1) * sizeof(DAWGEdge));
	if (newnext == NULL)
		return NULL;

	size_t i, j;
	i = j = 0;
    ASSERT(node->n == 0 || (node->n > 0 && node->next != NULL));
	while (i < node->n and node->next[i].letter < letter)
		newnext[j++] = node->next[i++];

	newnext[j].letter = letter;
	newnext[j].child = child;
	j += 1;

	while (i < node->n)
		newnext[j++] = node->next[i++];

	// assign the new next table
    if (node->next) {
	    memfree(node->next);
    }

	node->next = newnext;
	node->n += 1;

	ASSERT(dawgnode_get_child_idx(node, letter) >= 0);
	return child;
}


size_t PURE
dawgnode_get_size(DAWGNode* node) {
	if (node) {
		if (node->n > 0)
			return sizeof(DAWGNode) + node->n * sizeof(DAWGEdge);
		else
			return sizeof(DAWGNode);
	}
	else
		return 0;
}
