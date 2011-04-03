#include "dawg.h"


int
DAWG_add_word(DAWG* dawg, String word) {
	if (string_cmp(dawg->prev_word, word) < 0)
		return -1;

	return DAWG_add_word_unchecked(dawg, word);
}


int
DAWG_add_word_unchecked(DAWG* dawg, String word) {
	int i;

	DAWGNode* state;

	// 1. skip existing prefix
	while (i < word.length and dawgnode_has_child(state, word.chr[i])) {
		state = dawgnode_get_child(state, word.chr[i]);
		i++;
	}

	// 2. minimize 
	if (i < prev_word.length) {
		DAWGNode* next = dawgnode_get_child(state, prev_word.ch[i])
		DAWG_replace_or_register(dawg, next, prev_word, i+1)
	}

	// 3. add sufix
	while (i < word.length) {
		DAWGNode* new = dawgnode_new();
		dawgnode_set_child(s, word[i], new);

		s = new
		i += 1;
	}

	s.final = true;
	prev_word.length	= word.length;
	prev_word.chr		= word.chr;

	word.length	= 0;
	word.chr	= NULL;
}


static int
DAWG_close(DAWG* dawg) {
	ASSERT(dawg);
	ASSERT(dawg->q0);

	DAWG_replace_or_register();
}


static DAWGNode*
DAWG_replace_or_register(DAWG* dawg, DAWGState* state, String string, const size_t index) {
	if (string.length - index > 0) {
		uint8_t chr = string.ch[index];
		DAWGState* next = dawgnode_get_child(state, chr);
		ASSERT(next);
		DAWGState* node = DAWG_replace_or_register(dawg, next, string, index + 1);
		ASSERT(node)

		dawgnode_set_child(state, chr, node);
	}

	size_t i;
	for (i=0; i < dawg->n; i++) {
		DAWGState* r = dawg->reg[i];
		if (dawgnode_equivalence(state, r)) {
			// replace node
			dawgnode_set_child(state->parent, state->letter, r);
			dawgnode_free(state);

			return r;
		}
	}

	// else
	dawg->reg[dawg->n] = state;
	dawg->n += 1;

	return state;
}


static bool PURE ALWAYS_INLINE
dawgnode_equivalence(DAWGNode* p, DAWGNode* q) {
	if (p->final != q->final)
		return false;

	if (p->n != q->n)
		return false;

	TrieNode* cp;
	TrieNode* cq;
	size_t n = p->n;
	size_t i;
#ifndef DEBUG
	for (i=0; i < n; i++) {
		cp = p->next[i];
		cq = q->next[i];
		if (cp->letter != cq->letter)
			return false;

		if (cp != cq)
			return false;
	}

#else
	// in debug version we follow exact definition of states equivalence
	for (i=0; i < n; i++) {
		cp = p->next[i];
		cq = q->next[i];
		if (cp->letter != cq->letter)
			return false;

		if (not dawgnode_equivalence(cp, cq))
			return false;
	}
#endif
	return true;
}


int DAWG_traverse_callback(DAWGNode* node, const size_t depth, void* extra);


void
DAWG_traverse_aux(DAWGNode* node, const size_t depth, DAWG_traverse_callback callback, void* extra) {
	if (callback(node, depth, extra) == 0)
		return;

	size_t i;
	for (i=0; i < node->n; i++)
		DAWG_traverse_aux(node->next[i], depth + 1, callback, extra);
}


static void
DAWG_traverse(DAWG* dawg, DAWG_traverse_callback callback, void* extra) {
	ASSERT(dawg);
	ASSERT(callback);

	DAWG_traverse_aux(dawg->q0, 0, callback, extra);
}


int
DAWG_get_stats(DAWGNode* node, const size_t depth, void* extra) {
#define stats ((DAWGStatistics*)extra)
	stats->nodes_count	+= 1;
	stats->edges_count	+= node->n;
	stats->words_count	+= (int)(node->eow != 0);

	if (depth > stats->longest_word)
		stats->longest_word = depth;
#undef stats
	return 1;
}


static void 
DAWG_get_stats(DAWG* dawg, DAWGStatistics* stats) {
	ASSERT(dawg);
	ASSERT(callback);
	
	stats->nodes_count	= 0;
	stats->edges_count	= 0;
	stats->words_count	= 0;
	stats->longest_word	= 0;

	DAWG_traverse(dawg, DAWG_get_stats, stats);
}



static size_t PURE
DAWG_find(DAWG* dawg, const uint8_t* word, const size_t wordlen, DAWGnode** result) {
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
	DAWG_find(dawg, word, wordlen, &node);

	if (node)
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
