#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_IMPLEMENTATION
#include "./map.h"

#include "./assertions.h"

static void basic(void) {
    StringMap map;
    mapInit(&map, 0);
    MY_ASSERT_EQUALS(map.count, 0);

    char const* k1 = "key1";
	char const* v1 = "value1";
    mapSet(&map, k1, v1);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k1), v1);

	char const* k2 = "key2";
	char const* v2 = "value2";
    mapSet(&map, k2, v2);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k2), v2);

	char const* k3 = "key3";
	char const* v3 = "value3";
    mapSet(&map, k3, v3);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k3), v3);

	mapSet(&map, k2, "value2b");
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k2), "value2b");

	MY_ASSERT_EQUALS(map.count, 3);

	mapFree(&map);
}

static void overwrite(void) {
    StringMap map ;
    mapInit(&map, 0);
    char const* k = "key";
    char const* v1 = "value1";
    mapSet(&map, k, v1);

    MY_ASSERT_STRING_EQUALS(mapGet(&map, k), v1);
    MY_ASSERT_EQUALS(map.count, 1);

    char const* v2 = "value2";
    mapSet(&map, k, v2);

    MY_ASSERT_STRING_EQUALS(mapGet(&map, k), v2);
    MY_ASSERT_EQUALS(map.count, 1);
    
    mapFree(&map);
}

static void delete_(void) {
    StringMap map;
    mapInit(&map, 8);
    char const* k1 = "key1";
    char const* v1 = "value1";
    mapSet(&map, k1, v1);
    char const* k2 = "key2";
    char const* v2 = "value2";
    mapSet(&map, k2, v2);
    MY_ASSERT_EQUALS(map.count, 2);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k1), v1);
    MY_ASSERT_EQUALS(mapDelete(&map, k1), true);
    MY_ASSERT(mapGet(&map, k1) == NULL);
    MY_ASSERT_EQUALS(mapDelete(&map, k1), false);
    MY_ASSERT_EQUALS(map.count, 1);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k2), v2);
    MY_ASSERT_EQUALS(mapDelete(&map, k2), true);
    MY_ASSERT(mapGet(&map, k2) == NULL);
    MY_ASSERT_EQUALS(mapDelete(&map, k2), false);
    MY_ASSERT_EQUALS(map.count, 0);
    mapFree(&map);
}

static void deletenoinfiniteloop(void) {
    StringMap map;
    mapInit(&map, 2);
    char const* k1 = "key1";
    char const* v1 = "value1";
    mapSet(&map, k1, v1);
    char const* k2 = "key2";
    char const* v2 = "value2";
    mapSet(&map, k2, v2);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k1), v1);
    MY_ASSERT_EQUALS(mapDelete(&map, k1), true);
    MY_ASSERT(mapGet(&map, k1) == NULL);
    MY_ASSERT_EQUALS(mapDelete(&map, k1), false);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k2), v2);
    MY_ASSERT_EQUALS(mapDelete(&map, k2), true);
    MY_ASSERT(mapGet(&map, k1) == NULL);
    MY_ASSERT_EQUALS(mapDelete(&map, k2), false);
    char* k3 = strdup("key3");
    mapGet(&map, k3);
    free(k3);
    mapFree(&map);
}

static void grow(void) {
    StringMap map;  
    mapInit(&map, 1);
    MY_ASSERT_EQUALS(map.capacity, 1);
    char const* k = "key";
    char const* v = "value";
    mapSet(&map, k, v);
    MY_ASSERT_STRING_EQUALS(mapGet(&map, k), v);
    MY_ASSERT_EQUALS(map.capacity, 8);
    mapFree(&map);
}

static void collision(void) {
    StringMap map;
    mapInit(&map, 0);
    for (char c = 'A'; c < (char)'z'; ++c) {
        char kbuf[2] = {c, '\0'};
        char* k = strdup(kbuf);
        char* v = strdup(kbuf);
        mapSet(&map, k, v);
    }
    for (char c = 'A'; c < (char)'z'; ++c) {
        char kbuf[2] = {c, '\0'};
        char* k = kbuf;
        char const* got = mapGet(&map, k);
        MY_ASSERT(got != NULL);
        MY_ASSERT_EQUALS(strlen(got), 1);
        MY_ASSERT_EQUALS(got[0], c);
    }
    mapFree(&map);
}

int main(void) {
    basic();
    overwrite();
    delete_();
    deletenoinfiniteloop();
    grow();
    collision();
    return 0;
}
