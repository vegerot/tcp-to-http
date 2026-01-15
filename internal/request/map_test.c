#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// could try `__builtin_expect` if this is too slow
#define MY_ASSERT(cond)                                                       \
	(!(cond) ? MY_ASSERT_FAIL(__func__, __FILE__, __LINE__, #cond) : (void)0);

#define MY_ASSERT_FAIL(func, file, line, cond_literal)                        \
                                                                               \
	__extension__({                                                            \
		fflush(stdout);                                                        \
		fprintf(stderr, "\n%s:%d: %s: Assertion `%s` failed.\n", file, line,   \
				func, cond_literal);                                           \
		abort();                                                               \
	})

#define MY_ASSERT_EQUALS(got, want)                                           \
	(((got) != (want))                                                         \
		 ? MY_ASSERT_NUMBER_EQUALS_FAIL(__func__, __FILE__, __LINE__, #got,   \
										 got, #want, want)                     \
		 : (void)0);
#define MY_ASSERT_VALUE_EQUALS(_got, want)                                    \
	do {                                                                       \
		/** Assign `_got` to a variable to avoid rerunning a function if       \
		 * `_got` has side effects*/                                           \
		/** TODO: do the same thing for `want`*/                               \
		/** BUG: this makes the error message not helpful because it doesn't   \
		 * show the expression, just `got`. TODO: make the error message       \
		 * better */                                                           \
		Value got = _got;                                                      \
		MY_ASSERT_EQUALS((got).type, (want).type);                            \
		switch ((want).type) {                                                 \
		case (VAL_BOOL):                                                       \
			MY_ASSERT_EQUALS((got).as.boolean, (want).as.boolean);            \
			break;                                                             \
		case (VAL_NUMBER):                                                     \
			MY_ASSERT_EQUALS((got).as.number, (want).as.number)               \
			break;                                                             \
		case (VAL_NIL):                                                        \
			break;                                                             \
		default:                                                               \
			MY_ASSERT(false && "wtf?!");                                      \
		}                                                                      \
	} while (0);

#define MY_ASSERT_NUMBER_EQUALS_FAIL(func, file, line, got_name, got_value,   \
									  want_name, want_value)                   \
                                                                               \
	__extension__({                                                            \
		fflush(stdout);                                                        \
		fprintf(stderr, "\n%s:%d: %s: Expected %s(=%d) to equal %s(%d)\n",     \
				file, line, func, got_name, (int)(got_value), want_name,       \
				(int)(want_value));                                            \
		abort();                                                               \
	})

#define MY_ASSERT_STRING_EQUALS(got, want)                                    \
	do {                                                                       \
		if (strcmp(got, want) == 0)                                            \
			break;                                                             \
		MY_ASSERT_STRING_EQUALS_FAIL(__func__, __FILE__, __LINE__, #got, got, \
									  #want, want);                            \
	} while (0);

#define MY_ASSERT_STRING_EQUALS_FAIL(func, file, line, got_name, got_value,   \
									  want_name, want_value)                   \
                                                                               \
	__extension__({                                                            \
		fflush(stdout);                                                        \
		fprintf(stderr, "\n%s:%d: %s: Expected %s(=%s) to equal %s(=%s)\n",    \
				file, line, func, got_name, got_value, want_name, want_value); \
		abort();                                                               \
	})


#ifndef DEBUG_TRACE_EXECUTION
#define MY_UNREACHABLE(reason)                                                \
	__extension__({                                                            \
		__builtin_unreachable();                                               \
		fflush(stdout);                                                        \
		fprintf(stderr, "\n%s:%d: %s: Unreachable code reached: %s\n",         \
				__FILE__, __LINE__, __func__, reason);                         \
		abort();                                                               \
	})
#else
#define MY_UNREACHABLE(reason)                                                \
	__extension__({                                                            \
		fflush(stdout);                                                        \
		fprintf(stderr, "\n%s:%d: %s: Unreachable code reached: %s\n",         \
				__FILE__, __LINE__, __func__, reason);                         \
		abort();                                                               \
	})
#endif

#define MAP_IMPLEMENTATION
#include "map.h"

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
