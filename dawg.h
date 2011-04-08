#ifndef dawg_h_included__
#define dawg_h_included__

#include "common.h"
#include "dawgnode.h"

#define	DAWG_OK 		(0)
#define DAWG_NO_MEM		(-1)
#define DAWG_WORD_LESS	(-2)
#define DAWG_FROZEN		(-3)

#define DAWG_DUMP_TRUNCATED			(-100)
#define DAWG_DUMP_INVALID_MAGICK	(-101)
#define DAWG_DUMP_INVALID_STATE		(-102)
#define DAWG_DUMP_INVALID_ROOT_ID	(-103)
#define DAWG_DUMP_CORRUPTED_1		(-104)
#define DAWG_DUMP_CORRUPTED_2		(-105)


static bool PURE
dawgnode_equivalence(DAWGNode* p, DAWGNode* q);

static uint32_t PURE
dawgnode_hash(DAWGNode* p);

// setup hash table
#include "hash/hashtable_undefall.h"

#define	HASH_TYPE			uint32_t
#define	HASH_KEY_TYPE		DAWGNode*
//#define HASH_DATA_TYPE		-- no data
#define	HASH_EQ_FUN(a, b)	((a) == (b))
#define	HASH_GET_HASH(x)	(dawgnode_hash(x))
#define	HASH_STATIC			static
#define	HASH_ALLOC	malloc
#define	HASH_FREE	free

#define HASHNAME(name)	name
#include "hash/hashtable.c"

// end setup hash table


typedef struct String {
	ssize_t	length;
	char*	chars;
} String;


typedef enum {
	EMPTY,
	ACTIVE,
	CLOSED
} DAWGState;


typedef struct DAWGStatistics {
	size_t	nodes_count;
	size_t	edges_count;
	size_t	words_count;
	size_t	longest_word;

	size_t	sizeof_node;
	size_t	graph_size;
} DAWGStatistics;


typedef struct DAWGHashStatistics {
	size_t	table_size;
	size_t	element_size;
	size_t	items_count;
	size_t	item_size;
} DAWGHashStatistics;


typedef struct DAWG {
	DAWGNode*	q0;				///< start state
	int			count;			///< number of distinct words
	int			longest_word;	///< length of the longest word (useful when iterating through words, cheap to keep up to date)
	DAWGState	state;			///< DAWG state

	HashTable	reg;			///< registry -- valid states
	String		prev_word;		///< previosuly added word
} DAWG;


/* init DAWG structure */
static int
DAWG_init(DAWG* dawg);


/* free all structures */
static int
DAWG_free(DAWG* dawg);


/* add new word, check if new word is larger then previosuly added */
static int
DAWG_add_word(DAWG* dawg, String word);


/* add new word, but do not check word order */
static int
DAWG_add_word_unchecked(DAWG* dawg, String word);


/* clear whole DAWG */
static int
DAWG_clear(DAWG* dawg);


/* minimize remaining states and do not allow to add new words */
static int
DAWG_close(DAWG* dawg);


typedef int (*DAWG_traverse_callback)(DAWGNode* node, const size_t depth, void* extra);


/* traverse in DFS order, nodes might be visited many times;
   callback is called before visiting children
 */
static int
DAWG_traverse_DFS(DAWG* dawg, DAWG_traverse_callback callback, void* extra);


/* traverse in DFS order, nodes are visited exactly once
   callback is called after visiting children
 */
static int
DAWG_traverse_DFS_once(DAWG* dawg, DAWG_traverse_callback callback, void* extra);


/* calculate some statistics */
static void
DAWG_get_stats(DAWG* dawg, DAWGStatistics* stats);


/* get some statistics about hash table */
static void
DAWG_get_hash_stats(DAWG* dawg, DAWGHashStatistics* stats);


/* find word - returns longest prefix and last visited node */
static size_t PURE
DAWG_find(DAWG* dawg, const uint8_t* word, const size_t wordlen, DAWGNode** result);


/* checks if word exists in DAWG */
static bool PURE
DAWG_exists(DAWG* dawg, const uint8_t* word, const size_t wordlen);


/* returns longest prefix of word that exists in a DAWG */
static bool PURE
DAWG_longest_prefix(DAWG* dawg, const uint8_t* word, const size_t wordlen);

/**	Save DAWG in byte array.

	@param[in]	dawg		DAWG object
	@param[in]	stats		current statistics about DAWG

	@param[out]	array		address of array containg DAWG,
							array have to be freed manually
	@param[out]	size		size of array

	@returns
		DAWG_OK
		DAWG_NO_MEM
*/
static int
DAWG_save(DAWG* dawg, DAWGStatistics* stats, void** array, size_t* size);


/** Loads DAWG from data returned by DAWG_save.

*/
int
DAWG_load(DAWG* dawg, void* array, size_t size);


#endif
