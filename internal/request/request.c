#include "./map.h"

typedef struct {
    char* httpVersion;
    char* method;
    char* requestTarget;
} RequestLine;

typedef struct {
    RequestLine requestLine;
    StringMap headers;
    char* body;
} Request;
