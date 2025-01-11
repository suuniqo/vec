

#include <limits.h>
#include <stdlib.h>
#include "../include/vec.h"
#include <stdio.h>

#define VEC_DISABLE_SHRINK

typedef struct person {
    int age;
    int height;
    char sex;
} person_t;

void print_person(person_t* person) {
    if (person == NULL)
        return;

    printf("height: %d\nage: %d\nsex: %c\n", person->height, person->age, person->sex);
}

/*int main(void) {*/
/**/
/*    vec_t* vec;*/
/*    vec_make(&vec, sizeof(person_t), 0);*/
/**/
/*    vec_err_t err = VEC_ERR_OK;*/
/*    person_t input = {*/
/*        .age = 0,*/
/*        .height = 180,*/
/*        .sex = 'H'*/
/*    };*/
/*    person_t* &out = malloc(sizeof(person_t));*/
/**/
/*    char c = 0;*/
/**/
/*    while (c != 'Q') {*/
/*        c = getchar();*/
/*        if (c == '\n') {*/
/*            continue;*/
/*        }*/
/**/
/*        switch (c) {*/
/*            case '1':*/
/*                err = vec_push(vec, &input);*/
/*                break;*/
/*            case '2':*/
/*                err = vec_pop(vec, &out);*/
/*                break;*/
/*            case '3':*/
/*                err = vec_remove(vec, 2, &out);*/
/*                break;*/
/*            case '4':*/
/*                err = vec_insert(vec, 2, &input);*/
/*                break;*/
/*            case '5':*/
/*                err = vec_replace(vec, 4, &input, &out);*/
/*                break;*/
/*            case '6':*/
/*                err = vec_swap(vec, 0, 0);*/
/*                break;*/
/*            case '7':*/
/*                err = vec_shrink_to_fit(vec);*/
/*                break;*/
/*            case '8':*/
/*                err = vec_resize(vec, 20);*/
/*                break;*/
/*            case '9':*/
/*                err = vec_fill(vec, &input, 0);*/
/*                break;*/
/*            default:*/
/*                break;*/
/*        }*/
/**/
/*        _vec_debug(vec);*/
/*        //vec_display(vec);*/
/*        print_person(&out);*/
/*        printf("err was: %s\n\n", vec_get_err_msg(err));*/
/**/
/*        input.age += 10;*/
/*    }*/
/**/
/*    vec_t* new;*/
/*    vec_make(&new, sizeof(person_t), 0);*/
/*    vec_push(new, &input);*/
/*    vec_push(new, &input);*/
/*    vec_push(new, &input);*/
/*    _vec_debug(new);*/
/**/
/*    err = vec_clone(vec, &new);*/
/*    _vec_debug(vec);*/
/**/
/*    if (err != VEC_ERR_OK) {*/
/*        printf("couldn't clone\n");*/
/*    } else {*/
/*        printf("clone succesfull\n");*/
/*        _vec_debug(new);*/
/*        vec_pop(new, &out);*/
/**/
/*        print_person(&out);*/
/*        vec_destroy(&new);*/
/*    }*/
/**/
/*    vec_destroy(&vec);*/
/*    return 0;*/
/*}*/

int main(void) {

    vec_t* vec;
    vec_make(&vec, sizeof(char), 0);

    char c = 0;
    vec_err_t err = VEC_ERR_OK;

    char out = 0;;
    char input = 33;

    int tests = 1000;
    int len = 100000;

    for (int i = 0; i < tests; i++) {
        vec_fill(vec, &input, len);
    }

    return 0;

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
                err = vec_pop(vec, &out);
                break;
            case '3':
                err = vec_remove(vec, 2, &out);
                break;
            case '4':
                err = vec_insert(vec, 2, &input);
                break;
            case '5':
                err = vec_replace(vec, 4, &input, &out);
                break;
            case '6':
                err = vec_swap(vec, 0, 0);
                break;
            case '7':
                err = vec_shrink_to_fit(vec);
                break;
            case '8':
                err = vec_resize(vec, 20);
                break;
            case '9':
                err = vec_fill(vec, &input, 2);
                break;
            default:
                break;
        }

        _vec_debug(vec);
        //vec_display(vec);
        printf("out was: %c\n", out);
        size_t len = 0;
        vec_len(vec, &len);
        printf("len is: %lu\n", len);
        vec_capacity(vec, &len);
        printf("cap is: %lu\n", len);
        vec_space(vec, &len);
        printf("space is: %lu\n", len);

        printf("err was: %s\n\n", vec_get_err_msg(err));

        input += 1;
    }

    vec_t* new;
    vec_make(&new, sizeof(person_t), 0);
    vec_push(new, &input);
    vec_push(new, &input);
    vec_push(new, &input);
    _vec_debug(new);

    err = vec_clone(vec, &new);
    _vec_debug(vec);

    if (err != VEC_ERR_OK) {
        printf("couldn't clone\n");
    } else {
        printf("clone succesfull\n");
        _vec_debug(new);
        vec_pop(new, &out);

        printf("out was: %c\n", out);
        vec_destroy(&new);
    }

    vec_destroy(&vec);
    return 0;
}
