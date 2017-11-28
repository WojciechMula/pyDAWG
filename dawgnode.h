/*
	This is part of pydawg Python module.

	Declaration of nodes/edges of graph.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#ifndef dawgnode_h_included__
#define dawgnode_h_included__

#include "common.h"

struct DAWGNode;

typedef struct DAWGEdge {
	DAWG_LETTER_TYPE	letter;		///< link label
	struct DAWGNode*	child;		///< destination
} DAWGEdge;


typedef struct DAWGNode {
	uint16_t			n;			///< number of outcoming edges
	DAWGEdge*			next;		///< outcoming edges - always sorted by letter
	uint16_t			visited;	///< visited (field used while traversing a graph)
	bool				eow;		///< End-Of-Word marker
#ifdef DAWG_PERFECT_HASHING
	int					number;		///< number of words reachable from this state
#endif
} DAWGNode;


/* allocate and initialize node */
DAWGNode*
dawgnode_new(void);


/* free memory occupied by node and its internal structures  */
void
dawgnode_free(DAWGNode* node);


/* check if node has child connected by edge labelled by letter */
bool PURE
dawgnode_has_child(DAWGNode* node, const DAWG_LETTER_TYPE letter);


/* returns node connected by edge labelled by letter */
DAWGNode* PURE
dawgnode_get_child(DAWGNode* node, const DAWG_LETTER_TYPE letter);


/* adds or replace link */
DAWGNode*
dawgnode_set_child(DAWGNode* node, const DAWG_LETTER_TYPE letter, DAWGNode* child);


/* returns size of node and its internal structures */
size_t PURE
dawgnode_get_size(DAWGNode* node);

#endif
