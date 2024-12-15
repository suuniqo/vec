

#define VEC_DISABLE_SHRINK

#include "../include/vec.h"
#include <stdio.h>


int main(void) {

    vec_t* vec;
    vec_make(&vec, sizeof(char), 0);

    vec_err_t err = VEC_ERR_OK;
    char input = 1;
    char output = 0;
    char c = 0;

    while (c != 'Q') {
        c = getchar();
        if (c == '\n') {
            continue;
        }

        switch (c) {
            case '1':
                err = vec_push(vec, &input);
                break;
            case '2':
                err = vec_pop(vec, &output);
                break;
            case '3':
                err = vec_remove(vec, 2, &output);
                break;
            case '4':
                err = vec_insert(vec, 2, &input);
                break;
            case '5':
                err = vec_replace(vec, 4, &input, &output);
                break;
            case '6':
                err = vec_swap(vec, 0, 0);
                break;
            case '7':
                err = vec_shrink_to_fit(vec);
                break;
            case '8':
                err = vec_resize(vec, input);
                break;
            default:
                break;
        }

        _vec_debug(vec);
        //vec_display(vec);
        printf("output was: %d\n", output);
        printf("err was: %s\n\n", vec_get_err_msg(err));
        ++input;
    }

    vec_destroy(vec);
    return 0;
}
