/*
	This is part of pydawg Python module.

	Example of hash table template usage.

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : Public domain
	Date      : $Date$

	$Id$
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hashtable_setup.h"
#include "hashtable.c"

test_HashTable hash;


uint32_t __attribute__((pure))
fnv_hash(const char* string) {
	static const uint32_t FNV_offset	= 2166136261u;
	static const uint32_t FNV_prime		= 16777619u;

	uint8_t* byte = (uint8_t*)string;
	uint32_t hash = FNV_offset;
	while (*byte) {
		hash = hash * FNV_prime;
		hash = hash ^ *byte;
		byte++;
	}

	return hash;
}


typedef struct Data {
	char*	key;
	int		value;
} Data;

Data	input[] = {
	{"python",	100},
	{"c++",		101},
	{"cat",		102},
	{"hash",	103},
	{"house",	104},
	{"tree",	106},
	{"dog",		107},
	{"sky",		108},
	{"noise",	109},
	{"test",	110},
	{"length",	111},
	{"failure",	112},
	{NULL, 0}
};

void find_all() {
	size_t i = 0;
	while (input[i].key) {
		test_HashListItem* item = 
			test_hashtable_get(&hash, input[i].key);
		
		if (item)
			printf("'%s' => '%d'\n", item->key, item->data);
		else
			printf("! '%s' not found\n", input[i].key);

		i += 1;
	}
}


int main() {

	test_hashtable_init(&hash, 1023);

	size_t i;

	i = 0;
	while (input[i].key) {
		test_hashtable_add(&hash, input[i].key, input[i].value);
		i += 1;
	}

	find_all();
	test_hashtable_del(&hash, "noise");
	test_hashtable_del(&hash, "cat");
	find_all();

	test_hashtable_destroy(&hash);

	return 0;
}
