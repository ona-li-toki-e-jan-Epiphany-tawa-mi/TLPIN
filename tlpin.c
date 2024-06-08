#include <stdio.h>

#include "array.h"



typedef ARRAY_OF(char)  string_t;

#define FREAD_CHUNK_SIZE 1024
string_t read_entire_file(FILE* file) {
    string_t contents = {0};

    while (0 == feof(file) && 0 == ferror(file)) {
        array_resize(&contents, FREAD_CHUNK_SIZE + contents.capacity);
        char*  write_location = contents.elements + contents.count;
        size_t bytes_read     = fread(write_location, sizeof(char), FREAD_CHUNK_SIZE, file);
        contents.count += bytes_read;
    }

    return contents;
}

int main(void) {
    FILE* source = fopen("test.tlpin", "r");
    if (NULL == source) {
        perror("Error: Unable to open file 'test.tunpl'");
        return 1;
    };

    string_t contents = read_entire_file(source);
    (void)fclose(source);

    printf("%.*s", contents.count, contents.elements);

    array_free(&contents);

    return 0;
}
