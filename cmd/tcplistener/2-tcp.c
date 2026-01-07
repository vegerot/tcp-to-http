#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void print_socket_4tuple(int sockfd) {
  struct sockaddr_in local;

  socklen_t local_size = sizeof(local);
  assert(getsockname(sockfd, (struct sockaddr *)&local, &local_size) == 0);
  assert(local.sin_family == AF_INET);
  char local_ip[INET6_ADDRSTRLEN];
  inet_ntop(local.sin_family, &local.sin_addr, local_ip, sizeof(local_ip));
  int local_port = ntohs(local.sin_port);

  struct sockaddr_in remote;
  socklen_t remote_size = sizeof(remote);
  assert(getpeername(sockfd, (struct sockaddr *)&remote, &remote_size) == 0);
  assert(remote.sin_family == AF_INET);
  char remote_ip[INET6_ADDRSTRLEN];
  inet_ntop(remote.sin_family, &remote.sin_addr, remote_ip, sizeof(remote_ip));
  int remote_port = ntohs(remote.sin_port);

  printf("local: %s:%d\n", local_ip, local_port);
  printf("remote: %s:%d\n", remote_ip, remote_port);
}

#define READ_SIZE 8
#define MAX_LINE_LEN READ_SIZE * 20

static char shared_line[MAX_LINE_LEN];
static bool shared_line_ready = false;
static bool shared_done_reading = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *readNextLine(void *fd) {
  char read_buf[READ_SIZE];
  char line_buf[MAX_LINE_LEN];
  size_t line_offset = 0;
  while (true) {
    ssize_t bytesread = read((int)(intptr_t)fd, read_buf, READ_SIZE);
    if (bytesread < 0) {
      perror("read error: ");
      printf("bytesread: %zu\n", bytesread);
      break;
    }
    for (ssize_t i = 0; i < bytesread; ++i) {
      if (read_buf[i] == '\n') {
        line_buf[line_offset] = '\0';
        // printf("T1 before\n");
        fflush(stdout);
        pthread_mutex_lock(&mutex);
        // printf("T1 after\n");
        // fflush(stdout);
        // printf("DEBUG read: %s\n", line_buf);
        memcpy(shared_line, line_buf, (line_offset + 1) * sizeof(char));
        shared_line_ready = true;
        pthread_cond_signal(&cond);
        while (shared_line_ready) {
          // printf("T2 before\n");
          // fflush(stdout);
          pthread_cond_wait(&cond, &mutex);
          // printf("T2 after\n");
          // fflush(stdout);
        }
        pthread_mutex_unlock(&mutex);
        line_offset = 0;
      } else {
        line_buf[line_offset] = read_buf[i];
        line_offset += 1;
      }
    }

    if (bytesread == 0) {
      break;
    }
  }
  // printf("T5 before\n");
  // fflush(stdout);
  pthread_mutex_lock(&mutex);
  // printf("T5 after\n");
  // fflush(stdout);
  shared_done_reading = true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char *argv[]) {

  bool shouldReplyWithEcho = false;
  if (argc >= 2 && strcmp(argv[1], "echo") == 0) {
    shouldReplyWithEcho = true;
  }

  // create the TCP socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket creation failed");
    return EXIT_FAILURE;
  }
  printf("Socket created: sockfd=%d\n", sockfd);
  int one = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&one,
                 sizeof(one)) != 0) {
    perror("set socket opt failed! ");
    return -1;
  }

  // bind the socket
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(42069);

  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    perror("socket bind failed");
    return EXIT_FAILURE;
  }
  char ip_out[69];
  printf("Socket binded: ip=%s:%hu\n",
         inet_ntop(servaddr.sin_family, &servaddr.sin_addr, ip_out,
                   INET_ADDRSTRLEN),
         ntohs(servaddr.sin_port));

  // listen for connections
  if (listen(sockfd, 5) != 0) {
    perror("Listen failed");
    return 1;
  }
  printf("Server listening...\n");

  // accept connection from client
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  while (true) {
    pthread_mutex_lock(&mutex);
    shared_line_ready = false;
    shared_done_reading = false;
    pthread_mutex_unlock(&mutex);

    int connfd = accept(sockfd, (struct sockaddr *)&client, &len);
    if (connfd < 0) {
      perror("Server accept failed");
      exit(EXIT_FAILURE);
    }
    printf("Accepted connection from client. connfd=%d\n", connfd);
    print_socket_4tuple(connfd);
    pthread_t tid;
    pthread_create(&tid, NULL, readNextLine, (void *)(intptr_t)connfd);
    while (!shared_done_reading) {
      // printf("M1 before\n");
      // fflush(stdout);
      pthread_mutex_lock(&mutex);
      // printf("M1 after\n");
      // fflush(stdout);
      while (!shared_line_ready && !shared_done_reading) {
        // printf("M2 before\n");
        // fflush(stdout);
        pthread_cond_wait(&cond, &mutex);
        // printf("M2 after\n");
        // fflush(stdout);
      }
      if (shared_line_ready) {
        printf("read: %s\n", shared_line);
        // fflush(stdout);
        if (shouldReplyWithEcho) {
          write(connfd, "echo: ", 6);
          write(connfd, shared_line,
                strlen(shared_line)); // perf: return line len
          write(connfd, "\r\n", 2);
        }
        shared_line_ready = false;
        pthread_cond_signal(&cond);
      }
      pthread_mutex_unlock(&mutex);
    }
    pthread_join(tid, NULL);
    printf("Connection closed. connfd=%d ", connfd);
    print_socket_4tuple(connfd);
    close(connfd);
  }

  close(sockfd);
}
