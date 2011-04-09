uint32_t fnv_hash(const char*);

#define		HASH_TYPE			uint32_t				// hash value
#define		HASH_KEY_TYPE		char*					// key type
#define		HASH_DATA_TYPE		int						// data type [optional]
#define		HASH_EQ_FUN(a, b)	(strcmp((a), (b)) == 0)	// key's equality
#define		HASH_GET_HASH(x)	(fnv_hash(x))			// hash function [optional]
#define		HASH_STATIC					// empty or "static"
#define		HASH_ALLOC	malloc			// allocator
#define		HASH_FREE	free			//   functions

#define 	HASHNAME(name)	test_##name	// macro have to form unique name!

