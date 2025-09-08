#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_SIZE 8
#define MAX_LINE_LEN READ_SIZE * 20

static char shared_line[MAX_LINE_LEN];
static bool shared_line_ready = false;
static bool shared_done_reading = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
void *readNextLine(void *fileArg) {
  FILE *file = (FILE *)fileArg;
  char read_buf[READ_SIZE];
  char line_buf[MAX_LINE_LEN];
  size_t line_offset = 0;
  while (true) {
    size_t bytesread = read(fileno(file), read_buf, READ_SIZE);
    for (size_t i = 0; i < bytesread; ++i) {
      if (read_buf[i] == '\n') {
        line_buf[line_offset] = 0;
        // printf("T1 before\n");fflush(stdout);
        pthread_mutex_lock(&mutex);
        // printf("T1 after\n");fflush(stdout);
        // printf("DEBUG read: %s\n", line_buf);
        memcpy(shared_line, line_buf, (line_offset + 1) * sizeof(char));
        shared_line_ready = true;
        pthread_cond_signal(&cond);
        while (shared_line_ready) {
          // printf("T2 before\n");fflush(stdout);
          pthread_cond_wait(&cond, &mutex);
          // printf("T2 after\n");fflush(stdout);
        }
        pthread_mutex_unlock(&mutex);
        line_offset = 0;
      } else {
        line_buf[line_offset] = read_buf[i];
        line_offset += 1;
      }
    }

    if (bytesread < READ_SIZE) {
      break;
    }
  }
  if (line_offset > 0) {
    line_buf[line_offset] = 0;
    // printf("T3 before\n");fflush(stdout);
    pthread_mutex_lock(&mutex);
    // printf("T3 after\n");fflush(stdout);
    memcpy(shared_line, line_buf, (line_offset + 1) * sizeof(char));
    shared_line_ready = true;
    pthread_cond_signal(&cond);
    while (shared_line_ready) {
      // printf("T4 before\n");fflush(stdout);
      pthread_cond_wait(&cond, &mutex);
      // printf("T4 after\n");fflush(stdout);
    }
    pthread_mutex_unlock(&mutex);
  }
  // printf("T5 before\n");fflush(stdout);
  pthread_mutex_lock(&mutex);
  // printf("T5 after\n");fflush(stdout);
  shared_done_reading = true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main() {
  FILE *file = fopen("./messages.txt", "r");
  if (file == NULL) {
    perror("Failed to open the file");
    return EXIT_FAILURE;
  }
  pthread_t tid;
  pthread_create(&tid, NULL, readNextLine, file);

  while (true) {
    // printf("M1 before\n"); fflush(stdout);
    pthread_mutex_lock(&mutex);
    // printf("M1 after\n");fflush(stdout);
    while (!shared_line_ready && !shared_done_reading) {
      // printf("M2 before\n");fflush(stdout);
      pthread_cond_wait(&cond, &mutex);
      // printf("M2 after\n");fflush(stdout);
    }
    if (shared_line_ready) {
      printf("read: %s\n", shared_line);
      fflush(stdout);
      shared_line_ready = false;
      pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    if (shared_done_reading) {
      break;
    }
  }
  pthread_join(tid, NULL);
  fclose(file);
  return 0;
}
