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



// typedefs for the memory allocation functions accepted by the arrays.
typedef void*(*array_realloc_t)(void*, size_t);
typedef void(*array_free_t)(void*t);



/**
 * Creates an array type that stores the given type. Intantiations of this type
 * should be zero-initialized to start.
 */
#define ARRAY_OF(type)                          \
    struct {                                    \
        type*  elements;                        \
        size_t count;                           \
        size_t capacity;                        \
    }

/**
 * Frees the memory allocated by the dynamic array and resets it.
 * @param array (ARRAY_OF(type)*).
 * @param free (array_free_t) - function to free the underlying memory.
 */
#define array_free(array, free)                 \
    {                                           \
        (free)((array)->elements);              \
        (array)->elements = NULL;               \
        (array)->count    = 0;                  \
        (array)->capacity = 0;                  \
    }

/**
 * Computes the size, in bytes, of the dynamic array's elements.
 * @param array (ARRAY_OF(type)*).
 */
#define array_element_byte_size(array) (sizeof(*(array)->elements))

/**
 * Computes the size, in bytes, of the occupied portion of the dynamic array.
 * @param array (ARRAY_OF(type)*).
 */
#define array_occupied_byte_size(array) ((array)->count * array_element_byte_size((array)))

/**
 * Computes the size, in bytes, of the dynamic array.
 * @param array (ARRAY_OF(type)*).
 */
#define array_byte_size(array) ((array)->capacity * array_element_byte_size((array)))

/**
 * Fetches the element at the given index in the dynamic array.
 * @param array (ARRAY_OF(type)*).
 * @param index (size_t).
 */
#define array_at(array, index) (array)->elements[(index)]

/**
 * Sets the value of the element at the given index in the dynamic array.
 * @param array (ARRAY_OF(type)*).
 * @param index (size_t).
 * @param value (type).
 */
#define array_set(array, index, value) ((array)->elements[(index)] = (value))

/**
 * Swaps the contents of the dynamic arrays.
 * @param array1 (ARRAY_OF(type)*).
 * @param array2 (ARRAY_OF(type)*).
 */
#define array_swap(array1, array2)                      \
    {                                                   \
        void* array1_elements = (array1)->elements;     \
        (array1)->elements    = (array2)->elements;     \
        (array2)->elements    = array1_elements;        \
                                                        \
        size_t array1_count = (array1)->count;          \
        (array1)->count     = (array2)->count;          \
        (array2)->count     = array1_count;             \
                                                        \
        size_t array1_capacity = (array1)->capacity;    \
        (array1)->capacity     = (array2)->capacity;    \
        (array2)->capacity     = array1_capacity;       \
    }

/**
 * Reallocates the memory of the dynamic array if it's capacity has changed.
 * @param array (ARRAY_OF(type)*).
 * @param realloc (array_realloc_t) - function to reallocate the underlying
 *        memory.
 */
#define array_reallocate(array, realloc)                                            \
    {                                                                               \
        (array)->elements = (realloc)((array)->elements, array_byte_size((array))); \
        if (NULL == (array)->elements) {                                            \
            (void)fputs(                                                            \
                 "Error: Unable to reallocate dynamic array; buy more RAM lol",     \
                 stderr                                                             \
            );                                                                      \
            exit(1);                                                                \
        }                                                                           \
    }

/**
 * Resizes the dynamic array to the given size. If the size specified is smaller
 * than the array's current size, the array will be truncated.
 * Has no effect if the current and specified size are the same.
 * @param array (ARRAY_OF(type)*).
 * @param size  (size_t).
 * @param realloc (array_realloc_t) - function to reallocate the underlying
 *        memory.
 */
#define array_resize(array, size, realloc)        \
    {                                             \
        if ((size) != (array)->capacity) {        \
            if ((size) < (array)->count)          \
                (array)->count = (size);          \
            (array)->capacity = (size);           \
            array_reallocate((array), (realloc)); \
        }                                         \
    }

/**
 * Increases the size of the dynamic array by the given amount.
 * @param array (ARRAY_OF(type)*).
 * @param size  (size_t).
 * @param realloc (array_realloc_t) - function to reallocate the underlying
 *        memory.
 */
#define array_expand(array, size, realloc)      \
    {                                           \
        (array)->capacity += (size);            \
        array_reallocate((array), (realloc));   \
    }

/**
 * Appends an element to the dynamic array.
 * @param array   (ARRAY_OF(type)*).
 * @param element (type).
 * @param realloc (array_realloc_t) - function to reallocate the underlying
 *        memory.
 */
#define array_append(array, element, realloc)                                  \
    {                                                                          \
        if ((array)->count >= (array)->capacity) {                             \
            (array)->capacity = 0 == (array)->capacity                         \
                              ? ARRAY_INITIAL_CAPACITY                         \
                              : ARRAY_CAPACITY_MULTIPLIER * (array)->capacity; \
                                                                               \
            array_reallocate((array), (realloc));                              \
        }                                                                      \
        array_set((array), (array)->count++, (element));                       \
    }

/**
 * Appends multiple elements to the dynamic array.
 * @param array         (ARRAY_OF(type)*).
 * @param buffer        (type*).
 * @param element_count (size_t).
 * @param realloc (array_realloc_t) - function to reallocate the underlying
 *        memory.
 */
#define array_append_many(array, buffer, element_count, realloc)            \
    {                                                                       \
        if ((element_count) + (array)->count >= (array)->capacity) {        \
            if (0 == (array)->capacity) {                                   \
                (array)->capacity = ARRAY_INITIAL_CAPACITY;                 \
            }                                                               \
            while ((element_count) + (array)->count >= (array)->capacity) { \
                (array)->capacity *= ARRAY_CAPACITY_MULTIPLIER;             \
            }                                                               \
            array_reallocate((array), (realloc));                           \
        }                                                                   \
                                                                            \
        (void)memcpy(                                                       \
             (array)->elements + (array)->count,                            \
             (buffer),                                                      \
             (element_count)*sizeof(*(buffer))                              \
         );                                                                 \
        (array)->count += (element_count);                                  \
    }
