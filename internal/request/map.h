#pragma once
#include <stdint.h>
#include <string.h>
#include "assert.h"

typedef enum { ENTRY_ERR, ENTRY_EMPTY, ENTRY_VALUE, ENTRY_TOMBSTONE } EntryType;
typedef struct {
	EntryType type;
	char* key;
	char* value;
} MapEntry;

typedef struct {
	/** number of non-ENTRY_EMPTY entries */
	size_t count;
	size_t capacity;
	MapEntry* entries;
} StringMap;

void mapInit(StringMap* self, size_t initialCapacity);
bool mapSet(StringMap* self, char const* key, char const* value);
void mapAddAll(StringMap* from, StringMap* to);
char* mapFindString(StringMap* self, char const* chars, size_t length,
						   uint32_t hash);
void mapFree(StringMap* self);

#define MAP_MAX_LOAD_RATIO 0.69
static_assert((int)MAP_MAX_LOAD_RATIO == 0,
			  "MAP_MAX_LOAD_RATIO must be less than 1");
char* mapGet(StringMap const* self, char const* key);

bool mapDelete(StringMap* self, char const* key);

#if defined(MAP_IMPLEMENTATION)

#endif
