/*
	This is part of pydawg Python module.

	Hash table template body.

	See hashtable_test.c and hashtable_setup.h for example
	how to setup parameters.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : Public domain
	Date      : $Date$

	$Id$
*/

#include "hashtable.h"

#define L HASHNAME

HASH_STATIC int
L(hashtable_init)(L(HashTable)* hashtable, const size_t size) {
	if (size == 0)
		return -2;

	hashtable->size	 = size;
	hashtable->count_threshold = (size_t)(size * 0.7);
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


#ifndef HASH_RESIZE_UNUSED
HASH_STATIC int
L(hashtable_resize)(L(HashTable)* hashtable, const size_t newsize) {
	L(HashTable) new;
	if (L(hashtable_init)(&new, newsize) == 0) {
		L(HashListItem)* item;
		L(HashListItem)* tmp;
		size_t i;
		for (i=0; i < hashtable->size; i++) {
			item = hashtable->table[i];
			while (item) {
				tmp = item->next;
				const size_t idx = (item->hash % newsize);
				item->next = new.table[idx];
				new.table[idx] = item;

				item = tmp;
			}
		}

		HASH_FREE(hashtable->table);
		hashtable->table	= new.table;
		hashtable->size		= new.size;
		hashtable->count_threshold = new.count_threshold;
		return 0;
	}
	else
		return -1;
}
#endif


#ifndef HASH_CLEAR_UNUSED
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
#endif


#ifndef HASH_DESTROY_UNUSED
HASH_STATIC int
L(hashtable_destroy)(L(HashTable)* hashtable) {
	if (hashtable) {
		L(hashtable_clear)(hashtable);
		HASH_FREE(hashtable->table);
		hashtable->size = 0;
		hashtable->table = NULL;
		return 0;
	}
	else
		return -1;
}
#endif


#ifndef HASH_GET_LIST_UNUSED
HASH_STATIC L(HashListItem)*
L(hashtable_get_list)(L(HashTable)* hashtable, const HASH_TYPE hash) {
	return hashtable->table[hash % hashtable->size];
}
#endif

HASH_STATIC int
#ifdef HASH_DATA_TYPE
L(hashtable_add)(L(HashTable)* hashtable, const HASH_KEY_TYPE key, const HASH_DATA_TYPE data)
#else
L(hashtable_add)(L(HashTable)* hashtable, const HASH_KEY_TYPE key)
#endif
{
	const HASH_TYPE hash = HASH_GET_HASH(key);
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


#ifndef HASH_GET_UNUSED
HASH_STATIC L(HashListItem)*
L(hashtable_get)(L(HashTable)* hashtable, const HASH_KEY_TYPE key) {
	const HASH_TYPE hash = HASH_GET_HASH(key);
	const size_t index = hash % hashtable->size;
	L(HashListItem)* item = hashtable->table[index];

	while (item) {
		if (HASH_EQ_FUN(item->key, key))
			return item;
		item = item->next;
	}

	return item;
}
#endif


#ifndef HASH_DEL_UNUSED
HASH_STATIC L(HashListItem*)
L(hashtable_del)(L(HashTable)* hashtable, const HASH_KEY_TYPE key) {
	const HASH_TYPE hash = HASH_GET_HASH(key);
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
#endif

#undef L
