/*
	This is part of pydawg Python module.

	Implementation of DAWG procedures.
	Algorithm used to construct minimized DAWG comes from

	"Incremental algorithm for sorted data", Jan Daciuk,
	Stoyan Mihov, Bruce Watson, and Richard Watson,
	Computational Linguistics, 26(1), March 2000.


	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#include "dawg.h"
#include <string.h>

#include <malloc.h>


static int
DAWG_init(DAWG* dawg) {
	dawg->q0	= NULL;
	dawg->count	= 0;
	dawg->state	= EMPTY;
	dawg->longest_word = 0;
	dawg->visited_marker = 1;

	hashtable_init(&dawg->reg, 101);

	dawg->prev_word.chars	= NULL;
	dawg->prev_word.length	= 0;

	return 0;
}


static int
DAWG_free(DAWG* dawg) {
	int result = DAWG_clear(dawg);
	hashtable_destroy(&dawg->reg);
	return result;
}


static DAWGNode*
DAWG_replace_or_register(DAWG* dawg, DAWGNode* state, String string, const size_t index);



static int
string_cmp(const String s1, const String s2) {
	size_t i;
	size_t n = s1.length < s2.length ? s1.length : s2.length;
	for (i=0; i < n; i++) {
		if (s1.chars[i] < s2.chars[i])
			return -1;
		else
		if (s1.chars[i] > s2.chars[i])
			return +1;
	}

	if (i < s1.length)
		return 1;
	else if (i < s2.length)
		return -1;
	else
		return 0;
}


int
DAWG_add_word(DAWG* dawg, String word) {
	const int k = string_cmp(dawg->prev_word, word);
	if (k > 0)
		return DAWG_WORD_LESS;
	else
		return DAWG_add_word_unchecked(dawg, word);
}


static int
resize_hash(HashTable* hashtable) {
	if (hashtable->count > hashtable->count_threshold)
		return hashtable_resize(hashtable, hashtable->size * 2);
	else
		return 0;
}


int
DAWG_add_word_unchecked(DAWG* dawg, String word) {
	if (dawg->state == CLOSED) {
		return DAWG_FROZEN;
    }

	int ret = 1;
	size_t i = 0;

	if (dawg->q0 == NULL) {
		dawg->q0 = dawgnode_new();
		if (UNLIKELY(dawg->q0 == NULL))
			return DAWG_NO_MEM;
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

	// 3. add suffix
	while (i < word.length) {
		DAWGNode* new = dawgnode_new();
		if (new == NULL)
			return DAWG_NO_MEM;
		
		HashListItem* item = hashtable_del(&dawg->reg, state);

		dawgnode_set_child(state, word.chars[i], new);

		if (item) {
			HASH_FREE(item);
			resize_hash(&dawg->reg);
			hashtable_add(&dawg->reg, state);
		}

		state = new;
		i += 1;
	}

	if (state->eow == false) {
		state->eow = true;
		dawg->count += 1;
		ret = 1; // new word
	}
	else
		ret = 0; // existing word

	dawg->state = ACTIVE;

	// update longest_word
	if (word.length > dawg->longest_word)
		dawg->longest_word = word.length;

	// save previous word
	if (dawg->prev_word.chars)
		memfree(dawg->prev_word.chars);

	dawg->prev_word.length	= word.length;
	dawg->prev_word.chars	= (DAWG_LETTER_TYPE*)memalloc(word.length * DAWG_LETTER_SIZE);
	if (UNLIKELY(dawg->prev_word.chars == NULL))
		return DAWG_NO_MEM;
	else
		memcpy(dawg->prev_word.chars, word.chars, word.length * DAWG_LETTER_SIZE);

	word.length	= 0;
	word.chars	= NULL;

	return ret;
}


static int
DAWG_close(DAWG* dawg) {
	ASSERT(dawg);

	if (dawg->q0) {
		DAWG_replace_or_register(dawg, dawg->q0, dawg->prev_word, 0);
		hashtable_destroy(&dawg->reg);
		if (dawg->prev_word.chars) {
			memfree(dawg->prev_word.chars);

			dawg->prev_word.chars	= NULL;
			dawg->prev_word.length	= 0;
		}
		dawg->state = CLOSED;
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
	DAWG_LETTER_TYPE label;	///< edge label
} StackItem;


static DAWGNode*
DAWG_replace_or_register(DAWG* dawg, DAWGNode* state, String string, const size_t index) {
	List stack;
	list_init(&stack);

	// save in reverse order suffix of word
	size_t i;
	for (i=index; i < string.length; i++) {
		StackItem* item = (StackItem*)list_item_new(sizeof(StackItem));
		if (item) {
			item->parent = state;
			item->label  = string.chars[i];
			item->child  = state = dawgnode_get_child(item->parent, item->label);
			list_push_front(&stack, (ListItem*)item);
		}
		else
			goto error;
	}

	// replace or register
	while (1) {
		StackItem* item = (StackItem*)list_pop_first(&stack);
		if (item == NULL)
			break;

		// 1) try replace
		bool replaced = false;

		HashListItem* reg = hashtable_get_list(&dawg->reg, HASH_GET_HASH(item->child));
		while (reg) {
			DAWGNode* r = reg->key;
			ASSERT(r);

			if (dawgnode_equivalence(item->child, r)) {
				ASSERT(dawgnode_get_child(item->parent, item->label) == item->child);

				HashListItem* prev = hashtable_del(&dawg->reg, item->parent);

				dawgnode_set_child(item->parent, item->label, r);
				dawgnode_free(item->child);

				if (prev) {
					HASH_FREE(prev);
					resize_hash(&dawg->reg);
					hashtable_add(&dawg->reg, item->parent);
				}

				replaced = true;
				break;
			}
			else
				reg = reg->next;
		}

		// 2) register new unique state
		if (not replaced) {
			resize_hash(&dawg->reg);
			hashtable_add(&dawg->reg, item->child);
		}

		list_item_delete((ListItem*)item);
	} // while

error:
	list_delete(&stack);
	return 0;
}


static bool PURE
dawgnode_equivalence(DAWGNode* p, DAWGNode* q) {
	/*
		Both states p and q are equivalent (subtrees
		rooted at p and q form same languages):

		1. both are final/non-final
		2. has same number of children
		3. outgoing edges has same labels
		4. outgoing edges come to the same states
	
	*/

	if (p->eow != q->eow)
		return false;

	if (p->n != q->n)
		return false;

	size_t n = p->n;
	size_t i;
	for (i=0; i < n; i++) {
		// outcoming edges are always sorted by letter,
		// so side-by-side comparison is possible

		if (p->next[i].letter != q->next[i].letter)
			return false;

		if (p->next[i].child != q->next[i].child)
			return false;
	}

	return true;
}


/*
	used by hashtable for registry
	FNV hash (http://isthe.com/chongo/tech/comp/fnv/)
*/
static uint32_t PURE
dawgnode_hash(const DAWGNode* p) {
	/*
		hash is calulated from following components:
		- end-of-word marker
		- outgoing link count
		- link labels
		- address of link destinations

		compare with dawgnode_equivalence
	*/
	static const uint32_t FNV_offset	= 2166136261u;
	static const uint32_t FNV_prime		= 16777619u;
	
	uint32_t hash = FNV_offset;
#define FNV_step(x) hash = (hash * FNV_prime); hash = hash ^ (x);
	FNV_step(p->n);
	FNV_step(p->eow);

	size_t i;
	for (i=0; i < p->n; i++) {
#define byte0(x) ((x) & 0xff)
#define byte1(x) (((x) >> 8) & 0xff)
#define byte2(x) (((x) >> 16) & 0xff)
#define byte3(x) (((x) >> 24) & 0xff)

#define FNV_step32(x) \
        FNV_step(byte0(x)) \
        FNV_step(byte1(x)) \
        FNV_step(byte2(x)) \
        FNV_step(byte3(x))

#if DAWG_LETTER_SIZE == 1
		FNV_step(p->next[i].letter);
#elif DAWG_LETTER_SIZE == 2
		FNV_step(byte0(p->next[i].letter));
		FNV_step(byte1(p->next[i].letter));
#else
		FNV_step32(p->next[i].letter);
#endif

#if defined(MACHINE32BIT)
		const uint32_t ptr = (uint32_t)(p->next[i].child);
        FNV_step32(ptr);
#elif defined(MACHINE64BIT)
        const uint64_t ptr = (uint64_t)(p->next[i].child);
        FNV_step32(ptr & 0xfffffffful);
        FNV_step32(ptr >> 32);
#else
#   error "Unsupported data size"
#endif
	}

#undef byte0
#undef byte1
#undef byte2
#undef byte3
	
#undef FNV_step
	return hash;
}


int
DAWG_clear_aux(DAWGNode* node, UNUSED const size_t depth, void* extra) {
  DAWGNode ***ptr = (DAWGNode ***) extra;
  // Store the node in the current position in the list and advance to the next
  **ptr = node;
  (*ptr)++;
  return 1;
}


static int
DAWG_clear(DAWG* dawg) {

	// Delete all nodes
	if(dawg->q0) {
		DAWGStatistics stats;
		DAWGNode **aux;
		DAWGNode **aux_copy;
		size_t i;
		// Find how many nodes
		DAWG_get_stats(dawg, &stats);
		// Get the list of pointers to each node
		aux = memcalloc(stats.nodes_count, sizeof(DAWGNode *));
		if(aux==NULL)
			return DAWG_NO_MEM;
		aux_copy = aux;
		DAWG_traverse_DFS_once(dawg, DAWG_clear_aux, &aux_copy);
		// Go over the list and free all nodes
		for(i=0; i<stats.nodes_count; i++)
			memfree( aux[i] );
		memfree(aux);
	}

	// Clear the main structure
	dawg->q0	= NULL;
	dawg->count	= 0;
	dawg->state	= EMPTY;
	dawg->longest_word = 0;

	if (dawg->reg.size == 0)
		hashtable_init(&dawg->reg, 101);
	else
		hashtable_clear(&dawg->reg);

	if (dawg->prev_word.chars) {
		memfree(dawg->prev_word.chars);
		dawg->prev_word.chars = NULL;
	}

	dawg->prev_word.chars	= NULL;
	dawg->prev_word.length	= 0;

	return 0;
}


int
DAWG_traverse_DFS_aux(DAWGNode* node, const size_t depth, DAWG_traverse_callback callback, void* extra) {
	if (callback(node, depth, extra) == 0)
		return 0;

	size_t i;
	for (i=0; i < node->n; i++) {
		if (DAWG_traverse_DFS_aux(node->next[i].child, depth + 1, callback, extra) == 0)
			return 0;
	}

	return 1;
}


static int
DAWG_traverse_DFS(DAWG* dawg, DAWG_traverse_callback callback, void* extra) {
	ASSERT(dawg);
	ASSERT(callback);

	return DAWG_traverse_DFS_aux(dawg->q0, 0, callback, extra);
}


void
DAWG_traverse_clear_visited(DAWGNode* node) {
	node->visited = 0;

	int i;
	for (i=0; i < node->n; i++)
		DAWG_traverse_clear_visited(node->next[i].child);
}


int
DAWG_traverse_DFS_once_aux(DAWGNode* node, const size_t depth, const uint16_t visited, DAWG_traverse_callback callback, void* extra) {
	if (node->visited != visited) {
		node->visited = visited;
		int i;
		for (i=0; i < node->n; i++)
			if (DAWG_traverse_DFS_once_aux(node->next[i].child, depth + 1, visited, callback, extra) == 0)
				return 0;

		return callback(node, depth, extra);
	}
	else
		return 1;
}


static int
DAWG_traverse_DFS_once(DAWG* dawg, DAWG_traverse_callback callback, void* extra) {
	ASSERT(dawg);
	ASSERT(callback);

	if (dawg->q0) {
		if (dawg->visited_marker == 0) {
			// counter wrapped, visited fields have to be cleared
			//puts("cleared");
			DAWG_traverse_clear_visited(dawg->q0);
			dawg->visited_marker += 1;
		}

		const int ret = DAWG_traverse_DFS_once_aux(dawg->q0, 0, dawg->visited_marker, callback, extra);
		dawg->visited_marker += 1;
		return ret;
	}
	else
		return 1;
}


int
DAWG_get_stats_aux(DAWGNode* node, UNUSED const size_t depth, UNUSED void* extra) {
#define stats ((DAWGStatistics*)extra)
	stats->nodes_count	+= 1;
	stats->edges_count	+= node->n;
	stats->graph_size	+= dawgnode_get_size(node);
#undef stats
	return 1;
}


static void 
DAWG_get_stats(DAWG* dawg, DAWGStatistics* stats) {
	ASSERT(dawg);
	ASSERT(stats);

	stats->nodes_count	= 0;
	stats->edges_count	= 0;
	stats->words_count	= dawg->count;
	stats->longest_word	= dawg->longest_word;
	stats->sizeof_node	= sizeof(DAWGNode);
	stats->sizeof_edge	= sizeof(DAWGEdge);
	stats->graph_size	= 0;

	DAWG_traverse_DFS_once(dawg, DAWG_get_stats_aux, stats);
}


static void 
DAWG_get_hash_stats(DAWG* dawg, DAWGHashStatistics* stats) {
	ASSERT(dawg);
	ASSERT(stats);

	stats->table_size	= dawg->reg.size;
	stats->element_size	= sizeof(HashListItem*);
	stats->items_count	= dawg->reg.count;
	stats->item_size	= sizeof(HashListItem);
}



static size_t PURE
DAWG_find(DAWG* dawg, const DAWG_LETTER_TYPE* word, const size_t wordlen, DAWGNode** result) {
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
DAWG_exists(DAWG* dawg, const DAWG_LETTER_TYPE* word, const size_t wordlen) {
	DAWGNode* node;

	if (DAWG_find(dawg, word, wordlen, &node) > 0 and node)
		return node->eow;
	else
		return false;
}


static size_t PURE
DAWG_longest_prefix(DAWG* dawg, const DAWG_LETTER_TYPE* word, const size_t wordlen) {
	return DAWG_find(dawg, word, wordlen, NULL);
}


static bool PURE
DAWG_match(DAWG* dawg, const DAWG_LETTER_TYPE* word, const size_t wordlen) {
	return DAWG_longest_prefix(dawg, word, wordlen) > 0;
}


#include "dawg_pickle.c"
#include "dawg_mph.c"
