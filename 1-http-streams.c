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
  line_buf[sizeof(line_buf) - 1] = 0;
  while (true) {
    size_t bytesread = fread(&read_buf, sizeof(char), READ_SIZE, file);
    size_t previous_newline_pos = 0;
    for (size_t i = 0; i < bytesread; ++i) {
      size_t distanceFromBeginningOfReadBufOrLastNewline =
          i - previous_newline_pos;
      if (read_buf[i] == '\n') {
        printf("read: %.*s\n",
               (int)(line_offset + distanceFromBeginningOfReadBufOrLastNewline),
               line_buf);
        previous_newline_pos = i + 1;
        line_offset = 0;
        line_buf[sizeof(line_buf) - 1] = 0;
      } else {
        line_buf[line_offset + distanceFromBeginningOfReadBufOrLastNewline] =
            read_buf[i];
      }
    }

    line_offset += bytesread - previous_newline_pos;
    if (bytesread < READ_SIZE) {
      break;
    }
  }
}
