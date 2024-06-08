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
 * Appends the element to the dynamic array pointed at by array.
 */
#define array_append(array, element)                                                \
    {                                                                               \
        if ((array)->count >= (array)->capacity) {                                  \
            (array)->capacity = 0 == (array)->capacity                              \
                              ? ARRAY_INITIAL_CAPACITY                              \
                              : ARRAY_CAPACITY_MULTIPLIER * (array)->capacity;      \
            (array)->elements = realloc(                                            \
                 (array)->elements,                                                 \
                 (array)->capacity * sizeof(*(array)->elements)                     \
            );                                                                      \
            if (NULL == (array)->elements) {                                        \
                (void)fputs(                                                        \
                     "Error: unable to reallocate dynamic array; buy more RAM lol", \
                     stderr                                                         \
                );                                                                  \
                exit(1);                                                            \
            };                                                                      \
        }                                                                           \
        (array)->elements[(array)->count++] = (element);                            \
    }

/*
 * Fetches the element at the given index in the dynamic array pointed at by
 * array.
 */
#define array_at(array, index) (array)->elements[index]

/*
 * Frees the memory allocated by the dynamic array pointed at by array and
 * resets it to an empty state.
 */
#define array_free(array)                       \
    {                                           \
        free((array)->elements);                \
        (array)->elements = NULL;               \
        (array)->count    = 0;                  \
        (array)->capacity = 0;                  \
    }
