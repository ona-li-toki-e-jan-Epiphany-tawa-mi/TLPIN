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
 * Dead-simple dynamic arrays.
 *
 * Preprocessor parameters:
 * - ARRAY_INITIAL_CAPACITY - The initial capacity of dynamic arrays. Has
 *   default value.
 * - ARRAY_CAPACITY_MULTIPLIER - How much to scale the size of dynamic arrays by
 *   on resizing. Has default value.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifndef ARRAY_INITIAL_CAPACITY
#define ARRAY_INITIAL_CAPACITY 10
#endif // ARRAY_INITIAL_CAPACITY

#ifndef ARRAY_CAPACITY_MULTIPLIER
#define ARRAY_CAPACITY_MULTIPLIER 2
#endif // ARRAY_CAPACITY_MULTIPLIER



/*
 * Creates an array type that stores the given type. Intantiations of this type
 * should be zero-initialized.
 */
#define ARRAY_OF(type)                          \
    struct {                                    \
        type*  elements;                        \
        size_t count;                           \
        size_t capacity;                        \
    }

/*
 * Computes the size, in bytes, of the dynamic array's elements.
 *
 * array - ARRAY_OF(type)*.
 */
#define array_element_byte_size(array) (sizeof(*(array)->elements))

/*
 * Computes the size, in bytes, of the occupied portion of the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 */
#define array_occupied_byte_size(array) ((array)->count * array_element_byte_size((array)))

/*
 * Computes the size, in bytes, of the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 */
#define array_byte_size(array) ((array)->capacity * array_element_byte_size((array)))

/*
 * Fetches the element at the given index in the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 * index - size_t.
 */
#define array_at(array, index) (array)->elements[(index)]

/*
 * Sets the value of the element at the given index in the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 * index - size_t.
 * value - type.
 */
#define array_set(array, index, value) ((array)->elements[(index)] = (value))

/*
 * Appends an element to the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 * element - type.
 */
#define array_append(array, element)                                                  \
    {                                                                                 \
        if ((array)->count >= (array)->capacity) {                                    \
            (array)->capacity = 0 == (array)->capacity                                \
                              ? ARRAY_INITIAL_CAPACITY                                \
                              : ARRAY_CAPACITY_MULTIPLIER * (array)->capacity;        \
            (array)->elements = realloc((array)->elements, array_byte_size((array))); \
            if (NULL == (array)->elements) {                                          \
                (void)fputs(                                                          \
                     "Error: Unable to reallocate dynamic array; buy more RAM lol",   \
                     stderr                                                           \
                );                                                                    \
                exit(1);                                                              \
            };                                                                        \
        }                                                                             \
                                                                                      \
        (array)->elements[(array)->count++] = (element);                              \
    }

/*
 * Resizes the dynamic array to the given size. If the size specified is smaller
 * than the array's current size, the array will be truncated.
 * Has no effect if the current and specified size are the same.
 *
 * array - ARRAY_OF(type)*.
 * size - size_t.
 */
#define array_resize(array, size)                                                 \
    {                                                                             \
        if ((size) == (array)->capacity)                                          \
            goto larray_resize_end;                                               \
                                                                                  \
        if ((size) < (array)->count)                                              \
            (array)->count = (size);                                              \
        (array)->capacity = (size);                                               \
                                                                                  \
        (array)->elements = realloc((array)->elements, array_byte_size((array))); \
        if (NULL == (array)->elements) {                                          \
            (void)fputs(                                                          \
                 "Error: Unable to reallocate dynamic array; buy more RAM lol",   \
                 stderr                                                           \
            );                                                                    \
            exit(1);                                                              \
        }                                                                         \
                                                                                  \
     larray_resize_end:                                                           \
    }

/*
 * Applies a function to each of the elements of the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 * function - type(*)(type).
 */
#define array_map(array, function)                    \
    {                                                 \
        for (size_t i = 0; i < (array)->count; ++i) { \
            array_set(                                \
                (array),                              \
                i,                                    \
                (function)(array_at((array), i))      \
            );                                        \
        }                                             \
    }

/*
 * Frees the memory allocated by the dynamic array.
 *
 * array - ARRAY_OF(type)*.
 */
#define array_free(array)                       \
    {                                           \
        free((array)->elements);                \
        (array)->elements = NULL;               \
        (array)->count    = 0;                  \
        (array)->capacity = 0;                  \
    }
