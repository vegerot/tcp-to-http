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
    char buf[READ_SIZE];
    while (true) {
        size_t bytesread = fread(&buf, sizeof(char), READ_SIZE, file);
        printf("%.*s", (int)bytesread, buf);
        if (bytesread < READ_SIZE) {
            break;
        }
    }
}
