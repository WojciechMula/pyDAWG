/*
	This is part of pydawg Python module.

	Declaration of nodes/edges of graph.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#ifndef dawgnode_h_included__
#define dawgnode_h_included__

#include "common.h"

struct DAWGNode;

typedef struct DAWGEdge {
	uint8_t				letter;		///< link label
	struct DAWGNode*	child;		///< destination
} DAWGEdge;


typedef struct DAWGNode {
	int					n;			///< number of outcoming edges
	DAWGEdge*			next;		///< outcoming edges - always sorted by letter
	uint16_t			visited;	///< visited (field used while traversing a graph)
	bool				eow;		///< End-Of-Word marker
	
} DAWGNode;


/* allocate and initialize node */
DAWGNode*
dawgnode_new(const uint8_t letter);


/* free memory occupied by node and its internal structures  */
void
dawgnode_free(DAWGNode* node);


/* check if node has child connected by edge labelled by byte */
bool PURE
dawgnode_has_child(DAWGNode* node, const uint8_t byte);


/* returns node connected by edge labelled by byte */
DAWGNode* PURE
dawgnode_get_child(DAWGNode* node, const uint8_t byte);


/* adds on replace link */
DAWGNode*
dawgnode_set_child(DAWGNode* node, const uint8_t byte, DAWGNode* child);


/* returns size of node and its internal structures */
size_t PURE
dawgnode_get_size(DAWGNode* node);

#endif
