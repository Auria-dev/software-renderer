#include "array.h"

#define ARRAY_RAW_DATA(array) ((int*)(array) - 2)
#define ARRAY_CAPACITY(array) (ARRAY_RAW_DATA(array)[0])
#define ARRAY_OCCUPIED(array) (ARRAY_RAW_DATA(array)[1])

void* array_hold(void* array, int count, int item_size) {
    if (array == NULL) {
        int raw_size = (sizeof(int) * 2) + (item_size * count);
        int* base = (int*)malloc(raw_size);
        base[0] = count;  // capacity
        base[1] = count;  // occupied
        return base + 2;
    } else if (ARRAY_OCCUPIED(array) + count <= ARRAY_CAPACITY(array)) {
        ARRAY_OCCUPIED(array) += count;
        return array;
    } else {
        int needed_size = ARRAY_OCCUPIED(array) + count;
        int double_curr = ARRAY_CAPACITY(array) * 2;
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        int raw_size = sizeof(int) * 2 + item_size * capacity;
        int* base = (int*)realloc(ARRAY_RAW_DATA(array), raw_size);
        base[0] = capacity;
        base[1] = occupied;
        return base + 2;
    }
}

void* array_remove(void* array, int index, int item_size) {
    if (array == NULL || index < 0 || index >= ARRAY_OCCUPIED(array)) {
        return array;
    }

    int elements_to_shift = ARRAY_OCCUPIED(array) - index - 1;

    if (elements_to_shift > 0) {
        char* destination = (char*)array + index * item_size;
        char* source = (char*)array + (index + 1) * item_size;
        memmove(destination, source, elements_to_shift * item_size);
    }

    ARRAY_OCCUPIED(array)--;
    return array;
}

int array_length(void* array) {
    return (array != NULL) ? ARRAY_OCCUPIED(array) : 0;
}

void array_free(void* array) {
    if (array != NULL) {
        free(ARRAY_RAW_DATA(array));
    }
}