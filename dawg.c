#include "dawg.h"
#include <string.h>

#include <malloc.h>


static int
DAWG_init(DAWG* dawg) {
	dawg->q0	= NULL;
	dawg->count	= 0;
	dawg->state	= EMPTY;

	dawg->n		= 0;
	dawg->reg	= NULL;

	dawg->prev_word.chars	= NULL;
	dawg->prev_word.length	= 0;

	return 0;
}


static DAWGNode*
DAWG_replace_or_register(DAWG* dawg, DAWGNode* state, String string, const size_t index);

static bool PURE
dawgnode_equivalence(DAWGNode* p, DAWGNode* q);


static int
string_cmp(const String s1, const String s2) {
	return strcmp(s1.chars, s2.chars);
}


int
DAWG_add_word(DAWG* dawg, String word) {
	if (string_cmp(dawg->prev_word, word) < 0)
		return -1;

	return DAWG_add_word_unchecked(dawg, word);
}


int
DAWG_add_word_unchecked(DAWG* dawg, String word) {
	int i = 0;

	if (dawg->q0 == NULL) {
		dawg->q0 = dawgnode_new(0);
		if (UNLIKELY(dawg->q0 == NULL))
			return -1;
	}
	
	DAWGNode* state = dawg->q0;
	ASSERT(state);

	// 1. skip existing prefix
	while (i < word.length and dawgnode_has_child(state, word.chars[i])) {
		state = dawgnode_get_child(state, word.chars[i]);
		ASSERT(state);
		i += 1;
	}

	// 2. minimize
	if (i < dawg->prev_word.length)
		DAWG_replace_or_register(dawg, state, dawg->prev_word, i);

	// 3. add sufix
	while (i < word.length) {
		DAWGNode* new = dawgnode_new(word.chars[i]);
		ASSERT(new);
		dawgnode_set_child(state, word.chars[i], new);

		state = new;
		i += 1;
	}

	if (state->eow == false) {
		state->eow = true;
		dawg->count += 1;
	}

	// save previous word
	if (dawg->prev_word.chars)
		free(dawg->prev_word.chars);


	dawg->prev_word.length	= word.length;
	dawg->prev_word.chars	= (char*)memalloc(word.length);
	ASSERT(dawg->prev_word.chars);
	memcpy(dawg->prev_word.chars, word.chars, word.length);

	word.length	= 0;
	word.chars	= NULL;
	
	return 1;
}


static int
DAWG_close(DAWG* dawg) {
	ASSERT(dawg);

	if (dawg->q0) {
		DAWG_replace_or_register(dawg, dawg->q0, dawg->prev_word, 0);
		return 1;
	}
	else
		return 0;
}


#include "slist.c"

typedef struct StackItem {
	LISTITEM_data

	DAWGNode*	parent;	///< parent node
	DAWGNode*	child;	///< child node
	uint8_t		label;	///< edge label
} StackItem;


static DAWGNode*
DAWG_replace_or_register(DAWG* dawg, DAWGNode* state, String string, const size_t index) {
	List stack;
	list_init(&stack);

	// save in reverse order suffix of word
	size_t i;
	for (i=index; i < string.length; i++) {
		StackItem* item = (StackItem*)list_item_new(sizeof(StackItem));
		item->parent = state;
		item->label  = string.chars[i];
		item->child  = state = dawgnode_get_child(item->parent, item->label);
		list_push_front(&stack, (ListItem*)item);
	}

	// replace or register
	while (1) {
		StackItem* item = (StackItem*)list_pop_first(&stack);
		if (item == NULL)
			break;

		// 1) try replace
		size_t i;
		bool replaced = false;
		for (i=0; i < dawg->n; i++) {
			DAWGNode* r = dawg->reg[i];
			ASSERT(r);

			if (dawgnode_equivalence(item->child, r)) {
				ASSERT(dawgnode_get_child(item->parent, item->label) == item->child);
				dawgnode_set_child(item->parent, item->label, r);
				//dawgnode_free(item->child);

				replaced = true;
				break;
			}
		}

		// 2) register new unique state
		if (not replaced) {
			dawg->reg = (DAWGNode**)memrealloc(dawg->reg, (dawg->n + 1) * sizeof(DAWGNode*));
			dawg->reg[dawg->n] = item->child;
			dawg->n += 1;
		}

		list_item_delete((ListItem*)item);
	} // while

	list_delete(&stack);
	return 0;
}


static bool PURE
dawgnode_equivalence(DAWGNode* p, DAWGNode* q) {

	if (p->eow != q->eow)
		return false;

	if (p->n != q->n)
		return false;

	size_t n = p->n;
#if 1
	size_t i;
	for (i=0; i < n; i++) {
		if (p->next[i].letter != q->next[i].letter)
			return false;

		if (p->next[i].child != q->next[i].child)
			return false;
	}
#else
	// in debug version we follow exact definition of states equivalence
	DAWGNode* cp;
	DAWGNode* cq;
	size_t i, j;
	for (i=0; i < n; i++) {
		cp = p->next[i].child;
		cq = NULL;
		for (j=0; j < n; j++)
			if (q->next[j].letter == p->next[i].letter) {
				cq = q->next[j].child;
				break;
			}

		if (cq == NULL)
			return false;

		if (not dawgnode_equivalence(cp, cq))
			return false;
	}
#endif
	return true;
}

void
DAWG_clear_aux(DAWGNode* node) {
	size_t i;
	for (i=0; i < node->n; i++)
		DAWG_clear_aux(node->next[i].child);

	dawgnode_free(node);
}


static int
DAWG_clear(DAWG* dawg) {
	DAWG_clear_aux(dawg->q0);

	dawg->q0 = NULL;
	dawg->state	= EMPTY;
	dawg->n		= 0;
	memfree(dawg->reg);
	memfree(dawg->prev_word.chars);

	dawg->prev_word.chars	= NULL;
	dawg->prev_word.length	= 0;

	return 0;
}


int
DAWG_traverse_aux(DAWGNode* node, const size_t depth, DAWG_traverse_callback callback, void* extra) {
	if (callback(node, depth, extra) == 0)
		return 0;

	size_t i;
	for (i=0; i < node->n; i++) {
		if (DAWG_traverse_aux(node->next[i].child, depth + 1, callback, extra) == 0)
			return 0;
	}

	return 1;
}


static void
DAWG_traverse(DAWG* dawg, DAWG_traverse_callback callback, void* extra) {
	ASSERT(dawg);
	ASSERT(callback);

	DAWG_traverse_aux(dawg->q0, 0, callback, extra);
}


int
DAWG_get_stats_aux(DAWGNode* node, const size_t depth, void* extra) {
#define stats ((DAWGStatistics*)extra)
	stats->nodes_count	+= 1;
	stats->edges_count	+= node->n;
	stats->words_count	+= (int)(node->eow != 0);
	stats->graph_size	+= dawgnode_get_size(node);

	if (depth > stats->longest_word)
		stats->longest_word = depth;
#undef stats
	return 1;
}


static void 
DAWG_get_stats(DAWG* dawg, DAWGStatistics* stats) {
	ASSERT(dawg);
	ASSERT(stats);

	stats->nodes_count	= 0;
	stats->edges_count	= 0;
	stats->words_count	= 0;
	stats->longest_word	= 0;
	stats->sizeof_node	= sizeof(DAWGNode);
	stats->graph_size	= 0;

	DAWG_traverse(dawg, DAWG_get_stats_aux, stats);
}



static size_t PURE
DAWG_find(DAWG* dawg, const uint8_t* word, const size_t wordlen, DAWGNode** result) {
	ASSERT(dawg);

	DAWGNode* node = dawg->q0;
	size_t i=0;
	for (i=0; i < wordlen; i++) {
		node = dawgnode_get_child(node, word[i]);
		if (node == NULL)
			break;
	}

	if (result)
		*result = node;

	return i;
}


static bool PURE
DAWG_exists(DAWG* dawg, const uint8_t* word, const size_t wordlen) {
	DAWGNode* node;

	if (DAWG_find(dawg, word, wordlen, &node) > 0 and node)
		return node->eow;
	else
		return false;
}


static bool PURE
DAWG_longest_prefix(DAWG* dawg, const uint8_t* word, const size_t wordlen) {
	return DAWG_find(dawg, word, wordlen, NULL);
}


static bool PURE
DAWG_match(DAWG* dawg, const uint8_t* word, const size_t wordlen) {
	return DAWG_longest_prefix(dawg, word, wordlen) > 0;
}
