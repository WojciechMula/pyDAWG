/*
	This is part of pydawg Python module.

	Procedures to serialize/deserialize DAWG.
	This file is included directly in dawg.c.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

// hashtable type
#include "hash/hashtable_undefall.h"

#define HASH_TYPE		uint32_t
#define HASH_KEY_TYPE	DAWGNode*
#define HASH_DATA_TYPE	uintptr_t
#define HASH_EQ_FUN(a, b)	((a) == (b))
#define HASH_GET_HASH(x)	(HASH_TYPE)((uintptr_t)(x) & 0xffffffff)	// simple casting
#define HASH_STATIC		static
#define	HASH_ALLOC		memalloc
#define HASH_FREE		memfree
#define HASHNAME(name) addr_##name

#define HASH_GET_LIST_UNUSED
#define HASH_DEL_UNUSED

#include "hash/hashtable.c"
// hashtable type


typedef struct SaveAux {
	bool error;
	addr_HashTable LUT;

	int 	id;
	size_t	size;
	size_t	top;
	uint8_t* 	array;
} SaveAux;


static int
save_fill_address_table(DAWGNode* node, UNUSED const size_t depth, UNUSED void* extra) {
#define self ((SaveAux*)extra)
#define hashtable (self->LUT)
	if (hashtable.count > hashtable.count_threshold) {
		if (addr_hashtable_resize(&hashtable, hashtable.size * 2) < 0) {
			self->error = true;
			return 0;
		}
	}

	addr_hashtable_add(&hashtable, node, self->id++);
	return 1;
#undef hashtable
#undef self
}


/*
	Format of data:

	- magick		: 4 bytes
	- state			: 1 byte
	- nodes count	: 8 bytes
	- words count	: 8 bytes
	- longest word	: 8 bytes
	- id of root node	: 4 or 8 bytes

	Format of node:

	- id			: 4 or 8 bytes
	- eow			: 1 byte
	- n				: 4 bytes
	- array[n]
		- letter	: 1, 2 or 4 byte(s)
		- node id	: 4 or 8 bytes
*/

#ifdef MACHINE32BIT
#	define DUMP_ID_SIZE 4
#else
#	define DUMP_ID_SIZE 8
#endif

#define DUMP_HEADER_SIZE (1 + 4 + 3*8 + DUMP_ID_SIZE)
#define DUMP_NODE_SIZE (DUMP_ID_SIZE + 1 + 4)
#define DUMP_EDGE_SIZE (DAWG_LETTER_SIZE + DUMP_ID_SIZE)

#ifdef MACHINE32BIT
#	define DUMP_MAGICK_LO 0x32
#else
#	define DUMP_MAGICK_LO 0x64
#endif

#ifdef DAWG_UNICODE
#	define 	DUMP_MAGICK_HI	0xda00
#else
#	define 	DUMP_MAGICK_HI	0xdb00
#endif

#define DUMP_MAGICK (DUMP_MAGICK_LO | DUMP_MAGICK_HI)


static int
save_node(DAWGNode* node, const nodeid_t node_id, uint8_t* array, addr_HashTable* addr) {
	int saved = 0;
	addr_HashListItem* item;
	DAWGNode* child;

#define save_1byte(x) *(uint8_t*)(array + saved) = (x); saved += 1;
#define save_2bytes(x) *(uint16_t*)(array + saved) = (x); saved += 2;
#define save_4bytes(x) *(uint32_t*)(array + saved) = (x); saved += 4;
#ifdef MACHINE64BIT
#define save_8bytes(x) *(uint64_t*)(array + saved) = (x); saved += 8;
#endif

	// save node
#ifdef MACHINE32BIT
	save_4bytes(node_id);
#else
	save_8bytes(node_id);
#endif
	save_1byte(node->eow);
	save_4bytes(node->n);

	// save links
	size_t i;
	for (i=0; i < node->n; i++) {

		child = node->next[i].child;
		item = addr_hashtable_get(addr, child);
		ASSERT(item);

#if DAWG_LETTER_SIZE == 1
		save_1byte(node->next[i].letter);
#elif DAWG_LETTER_SIZE == 2
		save_2bytes(node->next[i].letter);
#else
		save_4bytes(node->next[i].letter);
#endif

#ifdef MACHINE32BIT
		save_4bytes(item->data);
#else
		save_8bytes(item->data);
#endif
	}

	return saved;
#ifdef MACHINE64BIT
#undef save_8bytes
#endif
#undef save_4bytes
#undef save_2bytes
#undef save_1byte
}


static int
DAWG_save(DAWG* dawg, DAWGStatistics* stats, uint8_t** array, size_t* size) {
	ASSERT(dawg);
	ASSERT(stats);

	SaveAux rec;
	
	rec.error	= false;
	rec.id		= 0;
	addr_hashtable_init(&rec.LUT, 1021);
	
	rec.size	= stats->nodes_count * DUMP_NODE_SIZE +
				  stats->edges_count * DUMP_EDGE_SIZE;
	rec.size	= rec.size + DUMP_HEADER_SIZE;
	rec.top		= 0;
	rec.array	= memalloc(rec.size);
	if (rec.array == NULL) {
		addr_hashtable_destroy(&rec.LUT);
		return DAWG_NO_MEM;
	}

	// make lookup table: node address => sequential number
	DAWG_traverse_DFS_once(dawg, save_fill_address_table, &rec);
	if (rec.error) {
		memfree(rec.array);
		addr_hashtable_destroy(&rec.LUT);
		return DAWG_NO_MEM;
	}

	// save header
#define save_8bytes(x) *(uint64_t*)(rec.array + rec.top) = (x); rec.top += 8;
#define save_4bytes(x) *(uint32_t*)(rec.array + rec.top) = (x); rec.top += 4;
#define save_1byte(x) *(uint8_t*)(rec.array + rec.top) = (x); rec.top += 1;
	save_4bytes(DUMP_MAGICK);
	save_1byte(dawg->state);
	save_8bytes(rec.LUT.count);	// nodes count
	save_8bytes(dawg->count);
	save_8bytes(dawg->longest_word);
	if (dawg->state != EMPTY) {
		addr_HashListItem* item;
		item = addr_hashtable_get(&rec.LUT, dawg->q0);
		ASSERT(item);
#ifdef MACHINE32BIT
		save_4bytes(item->data);
#else
		save_8bytes(item->data);
#endif
	}
	else {
#ifdef MACHINE32BIT
		save_4bytes(0);
#else
		save_8bytes(0);
#endif
	}
#undef save_8bytes
#undef save_4bytes
#undef save_1byte

	// save nodes
	size_t i;
	for (i=0; i < rec.LUT.size; i++) {
		addr_HashListItem* item = rec.LUT.table[i];
		while (item) {
			const int saved =
				save_node(item->key, item->data, rec.array + rec.top, &rec.LUT);

			ASSERT(saved > 0);
			rec.top += saved;

			ASSERT(rec.top <= rec.size);
			item = item->next;
		}
	}

	ASSERT(rec.top == rec.size);

	addr_hashtable_destroy(&rec.LUT);

	*size	= rec.size;
	*array	= rec.array;

	return 0;
}


static int
load_node(uint8_t* array, DAWGNode** _node, DAWGNode** id2node) {
	int loaded = 0;
	DAWGNode* node = memalloc(sizeof(DAWGNode));
	if (node == NULL) {
		return DAWG_NO_MEM;
	}

	// load id and save it in the lookup table
#ifdef MACHINE32BIT
	const uint32_t id = *(uint32_t*)(array + loaded);
	loaded += 4;
#else
	const uint64_t id = *(uint64_t*)(array + loaded);
	loaded += 8;
#endif

	id2node[id] = node;

#define get_1byte (loaded += 1, (*(uint8_t*)(array + loaded - 1)))
#define get_2bytes (loaded += 2, (*(uint16_t*)(array + loaded - 2)))
#define get_4bytes (loaded += 4, (*(uint32_t*)(array + loaded - 4)))
#define get_8bytes (loaded += 8, (*(uint64_t*)(array + loaded - 8)))

	// load node data
	node->eow		= get_1byte;
	node->n			= get_4bytes;
	node->visited	= 0;
	
	if (node->n) {
		node->next	= memalloc(node->n * sizeof(DAWGEdge));
		if (node->next == NULL) {
			memfree(node);
			return DAWG_NO_MEM;
		}

		size_t i;
		for (i=0; i < node->n; i++) {
#if DAWG_LETTER_SIZE == 1
			node->next[i].letter = get_1byte;
#elif DAWG_LETTER_SIZE == 2
			node->next[i].letter = get_2bytes;
#else
			node->next[i].letter = get_4bytes;
#endif

#ifdef MACHINE32BIT
			node->next[i].child = (DAWGNode*)(get_4bytes);
#else
			node->next[i].child = (DAWGNode*)(get_8bytes);
#endif
		}
	}
	else
		node->next = NULL;

	*_node = node;
	return loaded;

#undef get_1byte
#undef get_2bytes
#undef get_4bytes
#undef get_8bytes
}


int
DAWG_load(DAWG* dawg, uint8_t* array, size_t size) {
	int result;
	size_t top = 0;
	size_t i;

	uint32_t	magick;
	DAWGState	state;
	uint64_t	nodes_count;
	uint64_t	words_count;
	uint64_t	longest_word;
	nodeid_t	root_id;

	if (size < DUMP_HEADER_SIZE)
		return DAWG_DUMP_TRUNCATED;

#define get_1byte (top += 1, (*(uint8_t*)(array + top - 1)))
#define get_4bytes (top += 4, (*(uint32_t*)(array + top - 4)))
#define get_8bytes (top += 8, (*(uint64_t*)(array + top - 8)))

	// parse header
	magick = get_4bytes;
	if (magick != DUMP_MAGICK)
		return DAWG_DUMP_INVALID_MAGICK;

	state	= get_1byte;
	if (state != EMPTY and state != ACTIVE and state != CLOSED)
		return DAWG_DUMP_INVALID_STATE;

	nodes_count		= get_8bytes;
	words_count		= get_8bytes;
	longest_word	= get_8bytes;
#ifdef MACHINE32BIT
	root_id			= get_4bytes;
#else
	root_id			= get_8bytes;
#endif

	// lookup table: id => node address
	DAWGNode**	id2node = NULL;

	if (state != EMPTY) {
		if (root_id >= nodes_count)
			return DAWG_DUMP_INVALID_ROOT_ID;

		// 0. allocate lookup table
		id2node = memalloc(nodes_count * sizeof(DAWGNode*));
		if (id2node == NULL)
			return DAWG_NO_MEM;
		else
			memset(id2node, 0, nodes_count * sizeof(DAWGNode*));

		// 1. load nodes and fill id2node LUT
		for (i=0; i < nodes_count; i++) {
			DAWGNode* node;
			const int tmp = load_node(array + top, &node, id2node);

			if (LIKELY(tmp > 0)) {
				top += tmp;

				if (UNLIKELY(top > size)) {
					result = DAWG_DUMP_TRUNCATED;
					goto error;
				}
			}
			else {
				result = DAWG_NO_MEM;
				goto error;
			}
		}

		// 2. check if root node is ok
		if (id2node[root_id] == NULL) {
			result = DAWG_DUMP_CORRUPTED_1;
			goto error;
		}

		// 3. update node addresses in an edges array
		for (i=0; i < nodes_count; i++) {
			// get i-th node
			DAWGNode* node;
			DAWGNode* child;
			node = id2node[i];
			if (UNLIKELY(node == NULL)) {
				result = DAWG_DUMP_CORRUPTED_1;
				goto error;
			}

			// update edges
			size_t j;
			for (j=0; j < node->n; j++) {
#ifdef MACHINE32BIT
				const uint32_t id = (uint32_t)(node->next[j].child);
#else
				const uint64_t id = (uint64_t)(node->next[j].child);
#endif
				if (UNLIKELY(id >= nodes_count)) {
					result = DAWG_DUMP_CORRUPTED_2;
					goto error;
				}
				else {
					child = id2node[id];
					if (UNLIKELY(child == NULL)) {
						result = DAWG_DUMP_CORRUPTED_1;
						goto error;
					}
				}

				node->next[j].child = child;
			}
		} // for
	} // node

	result = DAWG_clear(dawg);
	if(result==DAWG_NO_MEM)
		goto error;
	dawg->q0 = (state == EMPTY) ? NULL : id2node[root_id];
	dawg->count	= words_count;
	dawg->longest_word = longest_word;
	dawg->state = state;

	if (state == ACTIVE) {
		// recreate registry lookup table
		for (i=0; i < nodes_count; i++) {
			DAWGNode* node = id2node[i];
			hashtable_add(&dawg->reg, node);
			resize_hash(&dawg->reg);
		}
	}
	
	if (id2node)
		memfree(id2node);

	return DAWG_OK;

error:
	for (i=0; i < nodes_count; i++)
		if (id2node[i] != NULL)
			dawgnode_free(id2node[i]);
	
	if (id2node)
		memfree(id2node);

	return result;
}
