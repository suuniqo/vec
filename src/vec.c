
#include "../include/vec.h"


/* ========= INCLUDES ========== */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


/* =========== DATA ============ */

/*
 * Dynamic array of variable length.
 */
typedef struct vec {
    int32_t* elems;                             /* memory containing the elements */
    size_t len;                                 /* current number of elements */
    size_t capacity;                            /* current allocated memory */
} vec_t;

/*
 * Mapping between values of 'vec_err_t'
 * and their printable values
 */
const char* const VEC_ERR_MSG[] = {
    "VEC_ERR_OK",
    "VEC_ERR_IOOB",
    "VEC_ERR_INVARG",
    "VEC_ERR_INVOP",
    "VEC_ERR_NOMEM"
};

#define MIN_CAPACITY 16                         /* Minimum capacity of the vector */

#define SHRINK_POLICY 4                         /* Minimum ratio of len / capacity */
#define SHRINK_FACTOR 2                         /* Factor by which the capacity shrinks */

#define GROWTH_POLICY 1                         /* Maximum ratio of len / capacity */
#define GROWTH_FACTOR 2                         /* Factor by which the capacity grows */


/* ========== PRIVATE ========== */

/*
 * Reallocates the vector with `capacity` memory
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 *
 * [Aborts] if the new capacity is smaller than the length
 * of the vector which indicates there's something wrong with
 * the resizing policies and they've been tampered.
 */
static vec_err_t
vec_resize(vec_t* vec, size_t capacity) {
    if (capacity < vec->len) {
        exit(EXIT_FAILURE);
    }

    int32_t* new_elems = realloc(vec->elems, capacity * sizeof(int32_t)); 

    if (new_elems == NULL) {
        return VEC_ERR_NOMEM;
    }

    vec->elems = new_elems;
    vec->capacity = capacity;

    return VEC_ERR_OK;
}

/*
 * Reallocates the vector, doubling it's capacity.
 */
static vec_err_t
vec_check_grow(vec_t* vec) {
    if (vec->len < vec->capacity * GROWTH_POLICY) {
        return VEC_ERR_OK;
    }
    if (vec->capacity > INT_MAX / GROWTH_FACTOR) {
        return VEC_ERR_INVOP;
    }

    return vec_resize(vec, vec->capacity * GROWTH_FACTOR);
}

/*
 * Reallocates the vector, halfing it's capacity.
 */
static vec_err_t
vec_check_shrink(vec_t* vec) {
    if (vec->len >= vec->capacity / SHRINK_POLICY || vec->capacity <= MIN_CAPACITY) {
        return VEC_ERR_OK;
    }

    return vec_resize(vec, vec->capacity / SHRINK_FACTOR);
}


/* ========== METHODS ========== */

/*
 * Creates a new instance of vec_t, allocating memory for 
 * `capacity` (or default value if negative) integers.
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 */
vec_err_t
vec_make(vec_t** vec, size_t capacity) {
    if (capacity < MIN_CAPACITY) {
        capacity = MIN_CAPACITY;
    }

    *vec = malloc(sizeof(vec_t));
    if (vec == NULL) {
        return VEC_ERR_NOMEM;
    }

    **vec = (vec_t) {
        .len = 0,
        .capacity = capacity,
        .elems = malloc(capacity * sizeof(int32_t))
    };

    if ((*vec)->elems == NULL) {
        free(vec);
        return VEC_ERR_NOMEM;
    }

    return VEC_ERR_OK;
}

/*
 * Reallocates the vector, reserving the exact amount of
 * memory needed to hold it's current contents
 *
 * [Returns] `INVARG` if the vector is NULL, `INVOP` if it's empty,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
vec_err_t
vec_shrink_to_fit(vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec->len == 0) {
        return VEC_ERR_INVOP;
    }
    if (vec->len == vec->capacity) {
        return VEC_ERR_OK;
    }

    return vec_resize(vec, vec->len);
}

/*
 * Resets the vector, reallocating
 * and deleting it's contents.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `NOMEM` if fails to allocate memory `OK` otherwise.
 */
vec_err_t
vec_clear(vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }

    int32_t* new_elems = malloc(MIN_CAPACITY * sizeof(int32_t)); 

    if (new_elems == NULL) {
        return VEC_ERR_NOMEM;
    }

    free(vec->elems);

    *vec = (vec_t) {
        .elems = new_elems,
        .capacity = MIN_CAPACITY,
        .len = 0
    };

    return VEC_ERR_OK;
}

/*
 * Destroys the instance of vec_t.
 */
void
vec_free(vec_t* vec) {
    free(vec->elems);
    free(vec);
}

/*
 * Substitutes with `val` the element of `vec` at `idx`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_replace(vec_t* vec, size_t idx, int32_t val, int32_t* old_val) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec->len <= idx) {
        return VEC_ERR_IOOB;
    }

    if (old_val != NULL) {
        *old_val = vec->elems[idx];
    }

    vec->elems[idx] = val;

    return VEC_ERR_OK;
}

/*
 * Inserts `val` at the end of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
vec_err_t
vec_push(vec_t* vec, int32_t val) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }

    vec_err_t resize_status = vec_check_grow(vec);

    if (resize_status != VEC_ERR_OK) {
        return resize_status;
    }

    vec->elems[vec->len++] = val;

    return VEC_ERR_OK;
}

/*
 * Removes the last element of `vec` and
 * optionally sets `popped` to the removed value.
 *
 * [Returns] `INVARG` if the vector is NULL 
 * `INVOP` if the vector is empty, `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
vec_err_t
vec_pop(vec_t* vec, int32_t* popped) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec_is_empty(vec)) {
        return VEC_ERR_INVOP;
    }

    if (popped != NULL) {
        *popped = vec->elems[vec->len - 1];
    }

    --vec->len;

    return vec_check_shrink(vec);
}

/*
 * Inserts `val` at the position `idx`,
 * shifting all elements after it to the right.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
vec_err_t
vec_insert(vec_t* vec, size_t idx, int32_t val) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx > vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_err_t resize_status = vec_check_grow(vec);
    if (resize_status != VEC_ERR_OK) {
        return resize_status;
    }

    for (ssize_t i = vec->len - 1; i >= idx; --i) {
        vec->elems[i + 1] = vec->elems[i];
    }

    vec->elems[idx] = val;
    ++vec->len;

    return VEC_ERR_OK;
}

/*
 * Removes the value at the position `idx`,
 * shifting all elements after it to the left.
 * Optionally sets `removed` to the removed value.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
vec_err_t
vec_remove(vec_t* vec, size_t idx, int32_t* removed) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    if (removed != NULL) {
        *removed = vec->elems[idx];
    }

    for (size_t i = idx; i < vec->len - 1; ++i) {
        vec->elems[i] = vec->elems[i + 1];
    }

    --vec->len;

    return vec_check_shrink(vec);
}

/*
 * Swaps the elements of `vec` at `idx1` at and `idx2`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2) {
    int old_val;

    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx2 >= vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_err_t op_status = vec_replace(vec, idx1, vec->elems[idx2], &old_val);
    if (op_status != VEC_ERR_OK) {
        return op_status;
    }

    vec->elems[idx2] = old_val;

    return VEC_ERR_OK;
}

/*
 * Sets the parameter `val` with the element at `idx` of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_get(const vec_t* vec, size_t idx, int32_t* val) {
    if (vec == NULL || val == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    *val = vec->elems[idx];

    return VEC_ERR_OK;
}

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL, or `OK` otherwise.
 */
vec_err_t
vec_len(const vec_t* vec, size_t* len) {
    if (vec == NULL || len == NULL) {
        return VEC_ERR_INVARG;
    }

    *len = vec->len;

    return VEC_ERR_OK;
}

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
size_t
vec_capacity(const vec_t* vec, size_t* cap) {
    if (vec == NULL || cap == NULL) {
        return VEC_ERR_INVARG;
    }

    *cap = vec->capacity - vec->len;

    return VEC_ERR_OK;
}

/*
 * [Returns] 1 if `vec` is empty or NULL and 0 otherwise.
 */
int32_t
vec_is_empty(const vec_t* vec) {
    if (vec == NULL) {
        return 1;
    }

    return vec->len == 0;
}

/*
 * Prints the formatted contents of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
vec_err_t
vec_display(const vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec_is_empty(vec)) {
        printf("[ ]");
        return VEC_ERR_OK;
    }

    printf("[ ");
    for (size_t i = 0; i < vec->len - 1; ++i) {
        printf("%d, ", vec->elems[i]);
    }
    printf("%d ]\n", vec->elems[vec->len - 1]);

    return VEC_ERR_OK;
}

/*
 * [Returns] the printable version of a
 * `vec_err_t` or NULL if `err`
 * doesn't belog to the enum.
 */
const char*
vec_get_err_msg(vec_err_t err) {
    if (err >= VEC_ERR_COUNT) {
        return NULL;
    }

    return VEC_ERR_MSG[err];
}

/*
 * Prints the formatted contents of the memory allocated by `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
vec_err_t
_vec_debug(const vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec->capacity == 0) {
        printf("[ ]");
        return VEC_ERR_OK;
    }

    printf("len: %lu, cap: %lu\n", vec->len, vec->capacity);
    printf("[ ");
    for (size_t i = 0; i < vec->capacity - 1; ++i) {
        printf("%d, ", i >= vec->len ? 0 : vec->elems[i]);
    }
    printf("%d ]\n", vec->len != vec->capacity ? 0 : vec->elems[vec->capacity - 1]);

    return VEC_ERR_OK;
}
