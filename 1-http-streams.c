#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define READ_SIZE 8
int main() {
  FILE *file = fopen("./messages.txt", "r");
  if (file == NULL) {
    perror("Failed to open the file");
    return EXIT_FAILURE;
  }
  char read_buf[READ_SIZE];
  char line_buf[READ_SIZE * 20];
  size_t line_offset = 0;
  while (true) {
    size_t bytesread = fread(&read_buf, sizeof(char), READ_SIZE, file);
    for (size_t i = 0; i < bytesread; ++i) {
      if (read_buf[i] == '\n') {
        line_buf[line_offset] = 0;
        printf("read: %s\n", line_buf);
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
}
