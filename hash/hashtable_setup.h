
#define		HASH_TYPE			uint32_t
#define		HASH_KEY_TYPE		char*
#define		HASH_DATA_TYPE		int
#define		HASH_EQ_FUN(a, b)	(strcmp((a), (b)) == 0)
#define		HASH_GET_HASH(x)	(fnv_hash(x))
#define		HASH_STATIC
#define		HASH_ALLOC	malloc
#define		HASH_FREE	free

#define 	HASHNAME(name)	test_##name

