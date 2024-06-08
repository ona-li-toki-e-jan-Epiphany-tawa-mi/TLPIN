#include "array.h"

typedef ARRAY_OF(int) int_array_t;

int main(void) {
    int_array_t array = {0};

    for (size_t i = 0; i < 1000; ++i) {
        array_append(&array, (int)i);
    }

    for (size_t i = 0; i < array.count; ++i) {
        (void)printf("%d\n", array_at(&array, i));
    }

    array_free(&array);
    return 0;
}
