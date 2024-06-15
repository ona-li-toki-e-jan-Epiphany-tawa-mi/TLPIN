/*
 * MIT License
 *
 * Copyright (c) 2024 ona-li-toki-e-jan-Epiphany-tawa-mi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Dead-simple sized strings.
 *
 * Sized strings are a specialization of the dymanic arrays from array.h.
 *
 * The functions not also present for dynamic arrays occur first.
 *
 * Preprocessor parameters:
 * - SIZED_STRING_IMPLEMENTATION - Define in *one* source file to instantiate
 *   the implementation.
 * - ARRAY_INITIAL_CAPACITY - The initial capacity of the dynamic arrays
 *   underlying the strings. Has default value
 * - ARRAY_CAPACITY_MULTIPLIER - How much to scale the size of dynamic arrays
 *   underlying the strings by on resizing. Has default value.
 */

#ifndef _SIZED_STRING_H
#define _SIZED_STRING_H

#include "array.h"



typedef ARRAY_OF(char) sstring_t;

/**
 * Returns the C-string version of the sized string.
 * @param malloc - the function to allocate the c-string with.
 */
char* sstring_to_cstring(const sstring_t* string, void*(*malloc)(size_t));

/**
 * The format specifier expected by format functions like printf. Must be
 * prepended with a '%'.
 */
#define SSTRING_PRINT ".*s"

/**
 * The arguments expected by format functions like printf.
 * @param sstring (sstring_t*).
 */
#define SSTRING_FORMAT(sstring) (int)(sstring)->count, (sstring)->elements

typedef enum {
    SSTRING_CONVERT_SUCCESS,    // Successful conversion.
    SSTRING_CONVERT_PARSE_FAIL, // Not a valid number.
    SSTRING_CONVERT_UNDERFLOW,  // Conversion caused an underflow.
    SSTRING_CONVERT_OVERFLOW    // Conversion caused an overflow.
} SstringConvertResult;

/**
 * Converts the string into a long int with the given base/radix.
 * result is written to to show whether the conversion was successful. You can
 * leave it NULL to ignore the result.
 * Otherwise, it behaves the same as strtol.
 */
long int sstring_to_long(const sstring_t* string, int base, SstringConvertResult* result);

/**
 * Converts the string into a double.
 * result is written to to show whether the conversion was successful. You can
 * leave it NULL to ignore the result.
 * Otherwise, it behaves the same as strtod.
 */
double sstring_str_to_double(const sstring_t* string, SstringConvertResult* result);

/**
 * Reads in a file stream until EOF and returns a string with it's contents.
 * @param chunk_size - the maximum number of bytes to read at a time.
 * @param realloc - function to reallocate the underlying memory.
 */
sstring_t sstring_read_file(FILE* file, size_t chunk_size, array_realloc_t realloc);



/**
 * Frees the memory allocated by the string and resets it.
 * @param free - function to free the underlying memory.
 */
void sstring_free(sstring_t* string, array_free_t free);

/**
 * Computes the size, in bytes, of the string's elements.
 */
size_t sstring_element_byte_size(const sstring_t* string);

/**
 * Computes the size, in bytes, of the occupied portion of the string.
 */
size_t sstring_occupied_byte_size(const sstring_t* string);

/**
 * Computes the size, in bytes, of the string.
 */
size_t sstring_byte_size(const sstring_t* string);

/**
 * Fetches the character at the given index in the string.
 */
char sstring_at(const sstring_t* string, size_t index);

/**
 * Sets the character at the given index in the string.
 */
void sstring_set(sstring_t* string, size_t index, char value);

/**
 * Swaps the contents of the strings.
 */
void sstring_swap(sstring_t* string1, sstring_t* string2);

/**
 * Reallocates the memory of the string if it's capacity has changed.
 * @param realloc - function to reallocate the underlying memory.
 */
void sstring_reallocate(sstring_t* string, array_realloc_t realloc);

/**
 * Resizes the string to the given size. If the size specified is smaller than
 * the array's current size, the string will be truncated.
 * Has no effect if the current and specified size are the same.
 * @param realloc - function to reallocate the underlying memory.
 */
void sstring_resize(sstring_t* string, size_t size, array_realloc_t realloc);

/**
 * Increases the size of the string by the given amount.
 * @param realloc - function to reallocate the underlying memory.
 */
void sstring_expand(sstring_t* string, size_t size, array_realloc_t realloc);

/**
 * Appends a character to the string.
 * @param realloc - function to reallocate the underlying memory.
 */
void sstring_append(sstring_t* string, char element, array_realloc_t realloc);

/**
 * Appends multiple characters to the string.
 * @param realloc - function to reallocate the underlying memory.
 */
void sstring_append_many( sstring_t *restrict string
                        , char *restrict buffer
                        , size_t count
                        , array_realloc_t realloc);



#ifdef SIZED_STRING_IMPLEMENTATION

#include <errno.h>
#include <limits.h>
#include <math.h>

char* sstring_to_cstring(const sstring_t* string, void*(*malloc)(size_t)) {
    char* cstring = malloc(string->count + 1);
    if (NULL == cstring) {
            (void)fputs(
                 "Error: Unable to allocate memory for C-string conversion; buy more RAM lol",
                 stderr
            );
            exit(1);
    }

    (void)memcpy(cstring, string->elements, string->count);
    cstring[string->count] = '\0';

    return cstring;
}

long int sstring_to_long(const sstring_t* string, int base, SstringConvertResult* result) {
    char cstring[string->count + 1];
    (void)memcpy(cstring, string->elements, string->count);
    cstring[string->count] = '\0';

    errno = 0;

    char*    end_pointer = cstring;
    long int value       = strtol(cstring, &end_pointer, base);

    if (NULL != result) {
        if (string->count != (size_t)(end_pointer - cstring)) {
            *result = SSTRING_CONVERT_PARSE_FAIL;
        } else if (ERANGE == errno && LONG_MAX == value) {
            *result = SSTRING_CONVERT_OVERFLOW;
        } else if (ERANGE == errno) {
            *result = SSTRING_CONVERT_UNDERFLOW;
        } else {
            *result = SSTRING_CONVERT_SUCCESS;
        }
    }

    return value;
}

double sstring_to_double(const sstring_t* string, SstringConvertResult* result) {
    char cstring[string->count + 1];
    (void)memcpy(cstring, string->elements, string->count);
    cstring[string->count] = '\0';

    errno = 0;

    char*  end_pointer = cstring;
    double value       = strtod(cstring, &end_pointer);

    if (NULL != result) {
        if (string->count != (size_t)(end_pointer - cstring)) {
            *result = SSTRING_CONVERT_PARSE_FAIL;
        } else if (ERANGE == errno && fabs(HUGE_VAL) == fabs(value)) {
            *result = SSTRING_CONVERT_OVERFLOW;
        } else if (ERANGE == errno) {
            *result = SSTRING_CONVERT_UNDERFLOW;
        } else {
            *result = SSTRING_CONVERT_SUCCESS;
        }
    }

    return value;
}

sstring_t sstring_read_file(FILE* file, size_t chunk_size, array_realloc_t realloc) {
    sstring_t contents = {0};

    while (0 == feof(file) && 0 == ferror(file)) {
        sstring_expand(&contents, chunk_size, realloc);
        char*  write_location = contents.elements + contents.count;
        size_t bytes_read     = fread(write_location, sizeof(char), chunk_size, file);
        contents.count += bytes_read;
    }

    return contents;
}

void sstring_free(sstring_t* string, array_free_t free) {
    array_free(string, free);
}

size_t sstring_element_byte_size(const sstring_t* string) {
    return array_element_byte_size(string);
}

size_t sstring_occupied_byte_size(const sstring_t* string) {
    return array_occupied_byte_size(string);
}

size_t sstring_byte_size(const sstring_t* string) {
    return array_byte_size(string);
}

char sstring_at(const sstring_t* string, size_t index) {
    return array_at(string, index);
}

void sstring_set(sstring_t* string, size_t index, char value) {
    array_set(string, index, value);
}

void sstring_swap(sstring_t* string1, sstring_t* string2) {
    array_swap(string1, string2);
}

void sstring_reallocate(sstring_t* string, array_realloc_t realloc) {
    array_reallocate(string, realloc);
}

void sstring_resize(sstring_t* string, size_t size, array_realloc_t realloc) {
    array_resize(string, size, realloc);
}

void sstring_expand(sstring_t* string, size_t size, array_realloc_t realloc) {
    array_expand(string, size, realloc);
}

void sstring_append(sstring_t* string, char element, array_realloc_t realloc) {
    array_append(string, element, realloc);
}

void sstring_append_many( sstring_t *restrict string
                        , char *restrict buffer
                        , size_t count
                        , array_realloc_t realloc) {
    array_append_many(string, buffer, count, realloc);
}

#endif // SIZED_STRING_IMPLEMENTATION

#endif // _SIZED_STRING_H
