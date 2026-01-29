#include <stdbool.h>
#include <unistd.h>

#define MAP_IMPLEMENTATION
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

// TESTS

#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>

#include "./assertions.h"

// Returns the read-end of a socket carrying the provided bytes,
// or -1 on error. Caller must close the returned fd.
int createFdFromString(const char *s) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        return -1;
    }

    const char *p = s;
    size_t len = strlen(s);
    while (len > 0) {
        ssize_t n = write(sv[0], p, len);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(sv[0]);
            close(sv[1]);
            return -1;
        }
        p += n;
        len -= (size_t)n;
    }

    shutdown(sv[0], SHUT_WR);  // half-close write side to signal EOF
    close(sv[0]);              // fully close write side
    return sv[1];              // read side; caller closes after parsing
}

Request* requestFromFd(intptr_t fd) {
    Request* request = malloc(sizeof(Request));
    request->requestLine.httpVersion = "NULL";
    request->requestLine.method = "NULL";
    request->requestLine.requestTarget = NULL;
    mapInit(&request->headers, 0);
    request->body = NULL;
    return request;
}

static void testRequestParseLine() {
    Request* request = requestFromFd(createFdFromString("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n"));
    MY_ASSERT_STRING_EQUALS(request->requestLine.method, "GET");
    MY_ASSERT_STRING_EQUALS(request->requestLine.requestTarget, "/");
    MY_ASSERT_STRING_EQUALS(request->requestLine.httpVersion, "1.1");
    free(request);
}

static void testRequestParsePath() {
    Request* request = requestFromFd(createFdFromString("GET /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n"));
    MY_ASSERT_STRING_EQUALS(request->requestLine.method, "GET");
    MY_ASSERT_STRING_EQUALS(request->requestLine.requestTarget, "/coffee");
    MY_ASSERT_STRING_EQUALS(request->requestLine.httpVersion, "1.1");
    free(request);
}

static void testRequestError() {
    Request* request = requestFromFd(createFdFromString("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n"));
    MY_ASSERT(request==NULL);
}

int main() {
    testRequestParseLine();
    testRequestParsePath();
    return 0;
}


