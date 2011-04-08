// hashtable type
#include "hash/hashtable_undefall.h"

#define HASH_TYPE		uintptr_t
#define HASH_KEY_TYPE	DAWGNode*
#define HASH_DATA_TYPE	uint32_t
#define HASH_EQ_FUN(a, b)	((a) == (b))
#define HASH_GET_HASH(x)	(uintptr_t)(x)	// simple casting
#define HASH_STATIC		static
#define	HASH_ALLOC		memalloc
#define HASH_FREE		memfree
#define HASHNAME(name) addr_##name

#include "hash/hashtable.c"
// hashtable type


typedef struct SaveAux {
	bool error;
	addr_HashTable LUT;

	int 	id;
	size_t	size;
	size_t	top;
	void* 	array;
} SaveAux;


static int
save_fill_address_table(DAWGNode* node, const size_t depth, void* extra) {
#define self ((SaveAux*)extra)
#define hashtable (self->LUT)
	if (hashtable.count > hashtable.count_threshold) {
		if (addr_hashtable_resize(&hashtable, hashtable.size * 2) < 0) {
			self->error = true;
			return 0;
		}
	}

	addr_hashtable_add(&hashtable, HASH_GET_HASH(node), node, self->id++);
	return 1;
#undef hashtable
#undef self
}


/*
	Format of data:

	- magick		: 4 bytes = 0xda32
	- state			: 1 byte
	- nodes count	: 4 bytes
	- words count	: 4 bytes
	- longest word	: 4 bytes
	- id of root node	: 4 bytes

	Format of node:

	- id			: 4 bytes
	- eow			: 1 byte
	- n				: 4 bytes
	- array[n]
		- letter	: 1 byte
		- node id	: 4 bytes
*/

#define DUMP_HEADER_SIZE (1 + 5*4)
#define DUMP_NODE_SIZE (1 + 2*4)
#define DUMP_EDGE_SIZE (1 + 4)
#define DUMP_MAGICK	(0xda32)


static int
save_node(DAWGNode* node, const uint32_t node_id, void* array, addr_HashTable* addr) {
	int saved = 0;
	addr_HashListItem* item;
	DAWGNode* child;

#define save_4bytes(x) *(uint32_t*)(array + saved) = (x); saved += 4;
#define save_1byte(x) *(uint8_t*)(array + saved) = (x); saved += 1;

	// save node
	save_4bytes(node_id);
	save_1byte(node->eow);
	save_4bytes(node->n);

	// save links
	size_t i;
	for (i=0; i < node->n; i++) {

		child = node->next[i].child;
		item = addr_hashtable_get(addr, HASH_GET_HASH(child), child);
		ASSERT(item);
		
		save_1byte(node->next[i].letter);
		save_4bytes(item->data);
	}

	return saved;
#undef save_4bytes
#undef save_1byte
}


static int
DAWG_save(DAWG* dawg, DAWGStatistics* stats, void** array, size_t* size) {
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
	if (rec.array == NULL)
		return DAWG_NO_MEM;

	// make lookup table: node address => sequentail number
	DAWG_traverse_DFS_once(dawg, save_fill_address_table, &rec);
	if (rec.error) {
		memfree(rec.array);
		addr_hashtable_destroy(&rec.LUT);
		return DAWG_NO_MEM;
	}

	// save header
#define save_4bytes(x) *(uint32_t*)(rec.array + rec.top) = (x); rec.top += 4;
#define save_1byte(x) *(uint8_t*)(rec.array + rec.top) = (x); rec.top += 1;
	save_4bytes(DUMP_MAGICK);
	save_1byte(dawg->state);
	save_4bytes(rec.LUT.count);	// nodes count
	save_4bytes(dawg->count);
	save_4bytes(dawg->longest_word);
	if (dawg->state != EMPTY) {
		addr_HashListItem* item;
		item = addr_hashtable_get(&rec.LUT, HASH_GET_HASH(dawg->q0), dawg->q0);
		ASSERT(item);
		save_4bytes(item->data);
	}
	else {
		save_4bytes(0);
	}
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
load_node(void* array, DAWGNode** _node, DAWGNode** id2node) {
	int loaded = 0;
	DAWGNode* node = memalloc(sizeof(DAWGNode));

	// load id and save a in lookup table
	const uint32_t id = *(uint32_t*)(array + loaded);
	loaded += 4;

	id2node[id] = node;

#define get_1byte (loaded += 1, (*(uint8_t*)(array + loaded - 1)))
#define get_4bytes (loaded += 4, (*(uint32_t*)(array + loaded - 4)))

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
			node->next[i].letter	= get_1byte;
			node->next[i].child		= (DAWGNode*)(get_4bytes);
		}
	}
	else
		node->next = NULL;

	*_node = node;
	return loaded;

#undef get_1byte
#undef get_4bytes
}


int
DAWG_load(DAWG* dawg, void* array, size_t size) {
	int result;
	size_t top = 0;
	size_t i;

	uint32_t	magick;
	DAWGState	state;
	uint32_t	nodes_count;
	uint32_t	words_count;
	uint32_t	longest_word;
	uint32_t	root_id;

	if (size < DUMP_HEADER_SIZE)
		return DAWG_DUMP_TRUNCATED;

#define get_1byte (top += 1, (*(uint8_t*)(array + top - 1)))
#define get_4bytes (top += 4, (*(uint32_t*)(array + top - 4)))

	// parse header
	magick = get_4bytes;
	if (magick != DUMP_MAGICK)
		return DAWG_DUMP_INVALID_MAGICK;

	state	= get_1byte;
	if (state != EMPTY and state != ACTIVE and state != CLOSED)
		return DAWG_DUMP_INVALID_STATE;

	nodes_count		= get_4bytes;
	words_count		= get_4bytes;
	longest_word	= get_4bytes;
	root_id			= get_4bytes;

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
				const uint32_t id = (uint32_t)(node->next[j].child);
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

	DAWG_clear(dawg);
	dawg->q0 = (state == EMPTY) ? NULL : id2node[root_id];
	dawg->count	= words_count;
	dawg->longest_word = longest_word;
	dawg->state = state;

	if (state == ACTIVE) {
		// recreate registry lookup table
		for (i=0; i < nodes_count; i++) {
			DAWGNode* node = id2node[i];
			hashtable_add(
				&dawg->reg,
				HASH_GET_HASH(node),
				node
			);

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
