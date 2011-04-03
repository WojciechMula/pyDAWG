#include "dawgnode.h"


DAWGNode*
dawgnode_new() {
	DAWGNode* new = (DAWGNode*)memalloc(sizeof(DAWGNode));
	if (new) {
		new->letter	= 0;
		new->parent	= NULL;
		new->n		= 0;
		new->next	= NULL;
		new->eow	= false;
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


bool PURE ALWYS_INLINE
dawgnode_has_child(DAWGNode* node, const uint8_t byte) {
	return (dawgnode_get_child(node, byte) != NULL);
}


DAWGNode* PURE
dawgnode_get_child(DAWGNode* node, const uint8_t byte) {
	if (node and node->next) {
		size_t i;
		for (i=0; i < node->n; i++)
			if (byte == node->next[i]->letter)
				return (node->next[i]);

		return NULL;
	}
	else
		return NULL;
}


DAWGNode*
dawgnode_set_child(DAWGNode* node, const uint8_t byte, DAWGNode* child) {
	ASSERT(node);
	if (node->next) {
		size_t i;
		for (i=0; i < node->n; i++)
			if (byte == node->next[i]->letter) {
				// replace
				node->next[i] = child;
				return child;
			}
	}

	// insert (keep alphabetic order)
	DAWGNode** newnext = memalloc((node->n + 1) * sizeof(DAWGNode));
	if (newnext == NULL)
		return;

	size_t i, j;
	i = j = 0;
	while (i < node->n and node->next[i]->letter < byte)
		newnext[j++] = node->next[i++];

	newnext[j++] = child;

	while (i < node->n)
		newnext[j++] = node->next[i++];

	return child;
}
