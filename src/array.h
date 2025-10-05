#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_push(array, value)                                              \
    do {                                                                      \
        (array) = array_hold((array), 1, sizeof(*(array)));                   \
        (array)[array_length(array) - 1] = (value);                           \
    } while (0);

#define array_remove_at(array, index)                                    \
    do {                                                                 \
        if ((array) != NULL && (index) >= 0 &&                           \
            (index) < array_length(array)) {                             \
            (void)array_remove((array), (index), sizeof(*(array)));      \
        }                                                                \
    } while (0)

void* array_hold(void* array, int count, int item_size);
int array_length(void* array);
void array_free(void* array);
void* array_remove(void* array, int index, int item_size);

#endif