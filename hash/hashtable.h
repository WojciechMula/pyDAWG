/*
	This is part of pydawg Python module.

	Hash table template header.

	See hashtable_test.c and hashtable_setup.h for example
	how to setup parameters.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : Public domain
	Date      : $Date$

	$Id$
*/

/* there is no include guard */
#define L HASHNAME

typedef struct L(HashListItem) {
	struct L(HashListItem)* next;
	HASH_TYPE		hash;	///< value of hash for key
	HASH_KEY_TYPE	key;	///< key
#ifdef HASH_DATA_TYPE
	HASH_DATA_TYPE	data;	///< associated data (optional)
#endif
} L(HashListItem);


typedef struct L(HashTable) {
	size_t size;			///< table size
	size_t count;			///< number of items
	size_t count_threshold;	///< count where resize of table is needed
	L(HashListItem)** table;
} L(HashTable);


/* init hash table, set initial size */
HASH_STATIC int
L(hashtable_init)(L(HashTable)* hashtable, const size_t size);

#ifndef HASH_RESIZE_UNUSED
/* change size of hash table */
HASH_STATIC int
L(hashtable_resize)(L(HashTable)* hashtable, const size_t newsize);
#endif

#ifndef HASH_DESTROY_UNUSED
/* destroy hash table */
HASH_STATIC int
L(hashtable_destroy)(L(HashTable)* hashtable);
#endif

#ifndef HASH_CLEAR_UNUSED
/* remove all elements from table */
HASH_STATIC int
L(hashtable_clear)(L(HashTable)* hashtable);
#endif

#ifndef HASH_GET_LIST_UNUSED
/* return head of element's list sharing same hash value */
HASH_STATIC L(HashListItem)*
L(hashtable_get_list)(L(HashTable)* hashtable, const HASH_TYPE hash);
#endif

/* add new item */
HASH_STATIC int
#ifdef HASH_DATA_TYPE
L(hashtable_add)(L(HashTable)* hashtable, const HASH_KEY_TYPE key, const HASH_DATA_TYPE data);
#else
L(hashtable_add)(L(HashTable)* hashtable, const HASH_KEY_TYPE key);
#endif

#ifndef HASH_GET_UNUSED
/* return list item contains searched element or NULL */
HASH_STATIC L(HashListItem)*
L(hashtable_get)(L(HashTable)* hashtable, const HASH_KEY_TYPE key);
#endif

#ifndef HASH_DEL_UNUSED
/* unbind list item containting given element;
   list item have to be free with HASH_FREE */
HASH_STATIC L(HashListItem)*
L(hashtable_del)(L(HashTable)* hashtable, const HASH_KEY_TYPE key);
#endif

#undef L
