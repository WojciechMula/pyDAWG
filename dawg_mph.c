/*
	This is part of pydawg Python module.

	Implementation of functions related to minmal perfect hashing.
	This file is included directly in dawg.c.

	Algorithm is described in:
	
	"Applications of Finite Automata Representing Large Vocabularies",
	Claudio Lucchesi and Tomasz Kowaltowski, 
	Software Practice and Experience, 23(1), pp. 15--30, Jan. 1993.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#ifdef DAWG_PERFECT_HASHING
static int
DAWG_mph_numerate_nodes_aux(DAWGNode* node, UNUSED const size_t depth, UNUSED void* extra) {
	size_t i;
	node->number = (int)(node->eow != 0);
	for (i=0; i < node->n; i++)
		node->number += node->next[i].child->number;
		
	return 1;
}

static void
DAWG_mph_numerate_nodes(DAWG* dawg) {
	ASSERT(dawg);
	DAWG_traverse_DFS_once(dawg, DAWG_mph_numerate_nodes_aux, NULL);
}


static size_t
DAWG_mph_word2index(DAWG* dawg, const DAWG_LETTER_TYPE* word, const size_t wordlen) {
	size_t index = 0;

	size_t i, j;
	DAWGNode* state;
	DAWGNode* next;

	state = dawg->q0;
	for (i = 0; i < wordlen; i++) {
		const DAWG_LETTER_TYPE c = word[i];
		next = dawgnode_get_child(state, c);
		if (next) {
			for (j=0; j < state->n; j++)
				if (state->next[j].letter < c)
					index += state->next[j].child->number;

			state = next;
			if (state->eow)
				index += 1;
		}
		else
			return DAWG_NOT_EXISTS;
	}

	return state->eow ? index : DAWG_NOT_EXISTS;
}


static int
DAWG_mph_index2word(DAWG* dawg, size_t index, DAWG_LETTER_TYPE** word, size_t* wordlen) {
	ASSERT(dawg);
	if (index < 1 or index > dawg->count)
		return DAWG_NOT_EXISTS;

	*wordlen = 0;
	*word = (DAWG_LETTER_TYPE*)memalloc((dawg->longest_word + 1) * DAWG_LETTER_SIZE);
	if (*word == NULL)
		return DAWG_NO_MEM;

	size_t i;
	size_t count;
	DAWGNode* state;
	DAWGNode* tmp;

	state = dawg->q0;
	count = index;
	do {
		for (i=0; i < state->n; i++) {
			tmp = state->next[i].child;
			if (tmp->number < count)
				count -= tmp->number;
			else {
				(*word)[*wordlen] = state->next[i].letter;
				*wordlen += 1;

				state = tmp;
				if (state->eow)
					count -= 1;

				break;
			}
		}
	}
	while (count > 0);

	return DAWG_EXISTS;
}
#endif

