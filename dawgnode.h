#ifndef dawgnode_h_included__
#define dawgnode_h_included__

#include "common.h"

typedef struct DAWGNode {
	uint8_t		letter;		///< label of incoming edge
	DAWGNode*	parent;		///< source of incoming edge
	int			n;
	DAWGNode**	next;		///< outcoming edges (sorted by letter od connected nodes!)
	bool		eow;		///< End-Of-Word marker
} DAWGNode;


DAWGNode*
dawgnode_new();


void
dawgnode_free(DAWGNode* node);


bool
dawgnode_has_child(DAWGNode* node, const uint8_t byte);


DAWGNode*
dawgnode_get_child(DAWGNode* node, const uint8_t byte);


DAWGNode*
dawgnode_set_child(DAWGNode* node, const uint8_t byte, DAWGNode* child);

#endif
