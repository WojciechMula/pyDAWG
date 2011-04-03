#ifndef dawg_h_included__
#define dawg_h_included__

#include "common.h"
#include "dawgnode.h"

typedef struct String {
	size_t	length;
	char*	chr;
} String;


typedef enum {
	ACTIVE,
	CLOSED
} DAWGState;


typedef struct DAWGStatistics {
	size_t	nodes_count;
	size_t	edges_count;
	size_t	words_count;
	size_t	longest_word;
} DAWGStatistics;


typedef struct DAWG {
	DAWGNode	q0;
	int			count;
	DAWGState	state;

	size_t		n;
	DAWGNode**	reg;
	String		prev_word;
} DAWG;

static int
DAWG_add_word(DAWG* dawg, String word);


static int
DAWG_add_word_unchecked(DAWG* dawg, String word);


static int
DAWG_close(DAWG* dawg);


typedef int (DAWG_traverse_callback*)(DAWGNode* node, const size_t depth, void* extra);


static void
DAWG_traverse(DAWG* dawg, DAWG_traverse_callback callback, void* extra);


int
DAWG_get_stats(DAWGNode* node, const size_t depth, void* extra);


static size_t PURE
DAWG_find(DAWG* dawg, const uint8_t* word, const size_t wordlen, DAWGnode** result);


static bool PURE
DAWG_exists(DAWG* dawg, const uint8_t* word, const size_t wordlen);


static bool PURE
DAWG_longest_prefix(DAWG* dawg, const uint8_t* word, const size_t wordlen);


#endif
