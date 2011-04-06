#include "hashtable.h"

#define L HASHNAME

HASH_STATIC int
L(hashtable_init)(L(HashTable)* hashtable, const size_t size) {
	hashtable->size	 = size;
	hashtable->count_threshold = size * 0.7;
	hashtable->count = 0;
	hashtable->table = HASH_ALLOC(size * sizeof(L(HashListItem*)));
	if (hashtable->table == NULL)
		return -1;
	else {
		size_t i;
		for (i=0; i < size; i++)
			hashtable->table[i] = NULL;

		return 0;
	}
}


HASH_STATIC int
L(hashtable_resize)(L(HashTable)* hashtable, const size_t newsize) {
	L(HashTable) new;
	if (L(hashtable_init)(&new, newsize) == 0) {
		L(HashListItem)* item;
		size_t i;
		for (i=0; i < hashtable->size; i++) {
			item = hashtable->table[i];
			while (item) {
#ifdef HASH_DATA_TYPE
				L(hashtable_add)(&new, item->hash, item->key, item->data);
#else
				L(hashtable_add)(&new, item->hash, item->key);
#endif
				item = item->next;
			}
		}

		HASH_FREE(hashtable->table);
		hashtable->table	= new.table;
		hashtable->count	= new.count;
		hashtable->size		= new.size;
		hashtable->count_threshold = new.count_threshold;
		return 0;
	}
	else
		return -1;
}


HASH_STATIC int
L(hashtable_clear)(L(HashTable)* hashtable) {
	size_t i;
	for (i=0; i < hashtable->size; i++) {
		L(HashListItem)* item = hashtable->table[i];
		L(HashListItem)* tmp;
		while (item) {
			tmp = item;
			item = item->next;
			HASH_FREE(tmp);
		}

		hashtable->table[i] = NULL;
	}

	hashtable->count = 0;
	return 0;
}


HASH_STATIC int
L(hashtable_destroy)(L(HashTable)* hashtable) {
	if (hashtable) {
		L(hashtable_clear)(hashtable);
		HASH_FREE(hashtable->table);
		hashtable->size = 0;
		return 0;
	}
	else
		return -1;
}


HASH_STATIC L(HashListItem)*
L(hashtable_get_list)(L(HashTable)* hashtable, const HASH_TYPE hash) {
	return hashtable->table[hash % hashtable->size];
}


HASH_STATIC int
#ifdef HASH_DATA_TYPE
L(hashtable_add)(L(HashTable)* hashtable, const HASH_TYPE hash, const HASH_KEY_TYPE key, const HASH_DATA_TYPE data)
#else
L(hashtable_add)(L(HashTable)* hashtable, const HASH_TYPE hash, const HASH_KEY_TYPE key)
#endif
{
	const size_t index = hash % hashtable->size;
	L(HashListItem)* item = (L(HashListItem)*)HASH_ALLOC(sizeof(L(HashListItem)));
	if (item) {
		item->next	= hashtable->table[index];
		item->hash	= hash;

#ifdef HASH_ASSIGN_KEY
		HASH_ASSIGN_KEY(item->key, key)
#else
		item->key	= (HASH_KEY_TYPE)key;
#endif

#ifdef HASH_DATA_TYPE
		item->data	= data;
#endif

		hashtable->table[index] = item;
		hashtable->count += 1;
		return 0;
	}
	else
		return -1;
}


HASH_STATIC L(HashListItem)*
L(hashtable_get)(L(HashTable)* hashtable, const HASH_TYPE hash, const HASH_KEY_TYPE key) {
	const size_t index = hash % hashtable->size;
	L(HashListItem)* item = hashtable->table[index];

	while (item) {
		if (HASH_EQ_FUN(item->key, key))
			return item;
		item = item->next;
	}

	return item;
}


HASH_STATIC L(HashListItem*)
L(hashtable_del)(L(HashTable)* hashtable, const HASH_TYPE hash, const HASH_KEY_TYPE key) {
	const size_t index = hash % hashtable->size;
	L(HashListItem)* item = hashtable->table[index];
	L(HashListItem)* prev;

	if (item) {
		if (HASH_EQ_FUN(item->key, key)) {
			// remove head
			hashtable->table[index] = item->next;
			item->next = NULL;

			hashtable->count -= 1;
			return item;
		}
		else {
			prev = item;
			item = item->next;
		}

		while (item) {
			if (HASH_EQ_FUN(item->key, key)) {
				prev->next = item->next; // unlink

				item->next = NULL;

				hashtable->count -= 1;
				return item;
			}
			prev = item;
			item = item->next;
		}
	}

	return NULL;
}


#undef L
