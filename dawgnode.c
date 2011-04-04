#include "dawgnode.h"


DAWGNode*
dawgnode_new(const uint8_t letter) {
	DAWGNode* new = (DAWGNode*)memalloc(sizeof(DAWGNode));
	if (new) {
		new->n		= 0;
		new->next	= NULL;
		new->eow	= false;
		new->visited	= 0;
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
dawgnode_has_child(DAWGNode* node, const uint8_t byte) {
	return (dawgnode_get_child(node, byte) != NULL);
}


static int PURE
dawgnode_get_child_idx(DAWGNode* node, const uint8_t byte) {
	if (node and node->next) {
		// binary search
		int a = 0;
		int b = ((int)node->n) - 1;
		int c;

		while (a <= b) {
			c = (a + b)/2;
			if (node->next[c].letter == byte)
				return c;
			else if (node->next[c].letter > byte)
				b = c - 1;
			else
				a = c + 1;
		}
	}

	return -1;
}


DAWGNode* PURE
dawgnode_get_child(DAWGNode* node, const uint8_t byte) {
	const int idx = dawgnode_get_child_idx(node, byte);
	if (idx >= 0)
		return node->next[idx].child;
	else
		return NULL;
}


DAWGNode*
dawgnode_set_child(DAWGNode* node, const uint8_t byte, DAWGNode* child) {
	ASSERT(node);
	if (node->next) {
		int idx = dawgnode_get_child_idx(node, byte);
		if (idx >= 0) {
			// replace
			node->next[idx].child = child;
			return child;
		}
	}

	// insert (keep alphabetic order)
	DAWGEdge* newnext = memalloc((node->n + 1) * sizeof(DAWGEdge));
	ASSERT(newnext);
	if (newnext == NULL)
		return NULL;

	size_t i, j;
	i = j = 0;
	while (i < node->n and node->next[i].letter < byte)
		newnext[j++] = node->next[i++];

	newnext[j].letter = byte;
	newnext[j].child = child;
	j += 1;

	while (i < node->n)
		newnext[j++] = node->next[i++];


	// assign new next table
	memfree(node->next);
	node->next = newnext;
	node->n += 1;

	ASSERT(dawgnode_get_child_idx(node, byte) >= 0);
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
