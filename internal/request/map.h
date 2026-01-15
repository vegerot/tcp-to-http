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

#define MAP_IMPLEMENTATION
#ifdef MAP_IMPLEMENTATION
#include <stdlib.h>

#define MAP_GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

static uint32_t mapHashCString(char const* key, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619u;
    }
    return hash;
}

static void mapAdjustCapacity(StringMap* self, size_t newCapacity) {
    MapEntry* newEntries = (MapEntry*)malloc(sizeof(MapEntry) * newCapacity);
    for (size_t i = 0; i < newCapacity; ++i) {
        newEntries[i] = (MapEntry){.type = ENTRY_EMPTY, .key = NULL, .value = NULL};
    }
    StringMap newSelf = {
        .count = 0,
        .capacity = newCapacity,
        .entries = newEntries,
    };
    if (self->entries != NULL) {
        mapAddAll(self, &newSelf);
        free(self->entries);
    }
    *self = newSelf;
}

static MapEntry* mapFindEntry(StringMap const* self, char const* key, uint32_t hash) {
    if (self->capacity == 0) {
        assert(false && "find in uninitialized map");
    }
    uint32_t index = hash % self->capacity;
    MapEntry* lastTombstone = NULL;
    while (true) {
        MapEntry* curr = &self->entries[index];
        if (curr->type == ENTRY_VALUE) {
            if (strcmp(curr->key, key) == 0) return curr;
        } else if (curr->type == ENTRY_TOMBSTONE) {
            if (lastTombstone == NULL) lastTombstone = curr;
        } else if (curr->type == ENTRY_EMPTY) {
            return lastTombstone == NULL ? curr : lastTombstone;
        }
        index = (index + 1) % self->capacity;
    }
}

void mapInit(StringMap* self, size_t initialCapacity) {
    self->count = 0;
    self->capacity = 0;
    self->entries = NULL;
    if (initialCapacity > 0) {
        mapAdjustCapacity(self, initialCapacity);
    }
}

void mapFree(StringMap* self) {
    if (self->entries == NULL) {
        assert(false && "freeing uninitialized map");
    }
    free(self->entries);
    self->entries = NULL;
    self->capacity = 0;
    self->count = 0;
}

void mapAddAll(StringMap* from, StringMap* to) {
    if (from->count == 0) return;
    for (size_t i = 0; i < from->capacity; ++i) {
        MapEntry* entry = &from->entries[i];
        if (entry->type == ENTRY_VALUE) {
            mapSet(to, entry->key, entry->value);
        }
    }
}

bool mapSet(StringMap* self, char const* key, char const* value) {
    size_t keyLen = strlen(key);
    uint32_t hash = mapHashCString(key, keyLen);
    if (self->count + 1 > (size_t)((int)self->capacity * MAP_MAX_LOAD_RATIO)) {
        size_t newCapacity = MAP_GROW_CAPACITY(self->capacity);
        mapAdjustCapacity(self, newCapacity);
    }
    MapEntry* entry = mapFindEntry(self, key, hash);
    if (entry->type != ENTRY_VALUE) {
        self->count += 1;
    }
    entry->type = ENTRY_VALUE;
    entry->key = strdup(key);
    entry->value = strdup(value);
    return true;
}

char* mapGet(StringMap const* self, char const* key) {
    if (self->count == 0) return NULL;
    uint32_t hash = mapHashCString(key, strlen(key));
    MapEntry* entry = mapFindEntry(self, key, hash);
    if (entry->type != ENTRY_VALUE) return NULL;
    return entry->value;
}

bool mapDelete(StringMap* self, char const* key) {
    if (self->count == 0) return false;
    uint32_t hash = mapHashCString(key, strlen(key));
    MapEntry* entry = mapFindEntry(self, key, hash);
    if (entry->type != ENTRY_VALUE) return false;
    *entry = (MapEntry){.type = ENTRY_TOMBSTONE, .key = NULL, .value = NULL};
    self->count -= 1;
    return true;
}

char* mapFindString(StringMap* self, char const* chars, size_t length, uint32_t hash) {
    if (self->count == 0) return NULL;
    uint32_t index = hash % self->capacity;
    while (true) {
        MapEntry* entry = &self->entries[index];
        if (entry->type == ENTRY_EMPTY) {
            return NULL;
        } else if (entry->type == ENTRY_VALUE) {
            if (strlen(entry->key) == length && memcmp(entry->key, chars, length) == 0) {
                return entry->key;
            }
        }
        index = (index + 1) % self->capacity;
    }
}
#endif
