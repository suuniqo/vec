
#include "../include/vec.h"


/* ========= INCLUDES ========== */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>


/* =========== DATA ============ */

/*
 * Dynamic array of variable length,
 * resizes and shrinks dynamically
 * which can store data of any size while 
 * it is constant
 */
typedef struct vec {
    size_t len;                                 /* current number of elements */
    size_t capacity;                            /* current allocated memory */
    size_t elem_size;                           /* size of the data type stored */
    void* elems;                                /* memory containing the elements */
} vec_t;

/*
 * Mapping between values of 'vec_err_t'
 * and their printable values
 */
static const char* const VEC_ERR_MSG[] = {
    "VEC_ERR_OK",
    "VEC_ERR_IOOB",
    "VEC_ERR_INVARG",
    "VEC_ERR_INVOP",
    "VEC_ERR_NOMEM"
};

#define MIN_CAPACITY 16                         /* Minimum capacity of the vector */
#define MAX_CAPACITY INT_MAX                    /* Maximum capacity of the vector */
#define MAX_DATA_SIZE INT_MAX                   /* Maximum size in bytes of elements */

#define SHRINK_POLICY 4                         /* Minimum ratio of capacity / len */
#define SHRINK_FACTOR 2                         /* Factor by which the capacity shrinks */

#define GROWTH_POLICY 1                         /* Maximum ratio of len / capacity */
#define GROWTH_FACTOR 2                         /* Factor by which the capacity grows */


/* ========== PRIVATE ========== */

/* === Reallocation === */

/*
 * Reallocates the vector with `capacity` memory
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 */
static vec_err_t
vec_reallocate(vec_t* vec, size_t capacity) {
    void* new_elems = realloc(vec->elems, capacity * vec->elem_size); 

    if (new_elems == NULL) {
        return VEC_ERR_NOMEM;
    }

    vec->elems = new_elems;
    vec->capacity = capacity;

    return VEC_ERR_OK;
}

/*
 * Destroys the instance of vec_t.
 */
static void
vec_free(vec_t** vec) {
    free((*vec)->elems);
    free(*vec);

    *vec = NULL;
}

/*
 * Reallocates the vector if `GROW_POLICY` is fulfilled, doubling it's capacity.
 *
 * [Returns] `NOMEM` if fails to allocate memory,
 * `INVOP` if growing would surpass maximum space or `OK` otherwise.
 */
static vec_err_t
vec_check_grow(vec_t* vec) {
    if (vec->len < vec->capacity * GROWTH_POLICY) {
        return VEC_ERR_OK;
    }
    if (vec->capacity > MAX_CAPACITY / GROWTH_FACTOR) {
        return VEC_ERR_INVOP;
    }

    size_t new_capacity = vec->capacity * GROWTH_FACTOR;
    return vec_reallocate(vec, MIN_CAPACITY > new_capacity ? MIN_CAPACITY : new_capacity);
}

/*
 * Reallocates the vector if `SHRINK_POLIVY` is fulfilled, halving it's capacity.
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 */
static vec_err_t
vec_check_shrink(vec_t* vec) {
    if (vec->len >= vec->capacity / SHRINK_POLICY || vec->capacity <= MIN_CAPACITY) {
        return VEC_ERR_OK;
    }

    size_t new_capacity = vec->capacity / SHRINK_FACTOR;
    return vec_reallocate(vec, new_capacity < MIN_CAPACITY ? MIN_CAPACITY : new_capacity);
}



/* === Writing === */

/*
 * Writes into `vec` at `idx` the contents of `src`.
 */
static void
vec_write_idx(vec_t* vec, size_t idx, const void* src) {
    memcpy((uint8_t*) vec->elems + idx * vec->elem_size, src, vec->elem_size);
}

/*
 * Writes into `dst` the contents of `vec` at `idx`.
 */
static void
vec_write_var(const vec_t* vec, size_t idx, void* dst) {
    memcpy(dst, (uint8_t*) vec->elems + idx * vec->elem_size, vec->elem_size);
}



/* ========== PUBLIC ========== */


/* === Memory Management === */

/*
 * Creates a new instance of vec_t, allocating memory for 
 * `capacity` (or default value if negative) elements of size `elem_size`
 *
 * [Returns] `INVOP` if element size is smaller than one byte
 * or is bigger than `MAX_DATA_SIZE`, `INVARG` if `vec` is NULL,
 * NOMEM` if fails to allocate memory `OK` otherwise.
 */
vec_err_t
vec_make(vec_t** vec, size_t elem_size, size_t capacity) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (elem_size < 1 || elem_size > MAX_DATA_SIZE) {
        return VEC_ERR_INVOP;
    }
    if (capacity < MIN_CAPACITY) {
        capacity = MIN_CAPACITY;
    }

    *vec = malloc(sizeof(vec_t));
    if (*vec == NULL) {
        return VEC_ERR_NOMEM;
    }

    **vec = (vec_t) {
        .len = 0,
        .capacity = capacity,
        .elem_size = elem_size,
        .elems = malloc(capacity * elem_size)
    };

    if ((*vec)->elems == NULL) {
        free(*vec);
        *vec = NULL;
        return VEC_ERR_NOMEM;
    }

    return VEC_ERR_OK;
}

/*
 * Resizes the vector with `capacity` memory
 *
 * [Returns] `NOMEM` if fails to allocate memory,
 * `INVARG` if the vector is NULL, `INVOP` if capacity isn't 
 * in the valid range or `OK` otherwise.
 */
vec_err_t
vec_resize(vec_t* vec, size_t capacity) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (capacity <= MIN_CAPACITY || capacity > MAX_CAPACITY) {
        return VEC_ERR_INVOP;
    }

    return vec_reallocate(vec, capacity);
}

/*
 * Reallocates the vector, reserving the exact amount of
 * memory needed to hold it's current contents
 *
 * [Returns] `INVARG` if the vector is NULL, `INVOP` if it's empty,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
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

    return vec_reallocate(vec, vec->len);
}

/*
 * Resets the vector, reallocating
 * and deleting it's contents if enough memory.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `NOMEM` if fails to allocate memory `OK` otherwise.
 */
vec_err_t
vec_clear(vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec->capacity <= MIN_CAPACITY) {
        vec->len = 0;
        return VEC_ERR_OK;
    }

    void* new_elems = malloc(MIN_CAPACITY * vec->elem_size); 

    if (new_elems == NULL) {
        return VEC_ERR_NOMEM;
    }

    free(vec->elems);

    vec->len = 0;
    vec->capacity = MIN_CAPACITY;
    vec->elems = new_elems;

    return VEC_ERR_OK;
}

/*
 * Fills the first `len` positions of the vector with `val`.
 * If it hasn't enough capacity it is reallocated to hold `len` elements.
 *
 * [Returns] `INVARG` if the `vec` or `val` are NULL,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_fill(vec_t* vec, const void* val, size_t len) {
    if (vec == NULL || val == NULL) {
        return VEC_ERR_INVARG;
    }
    if (vec->capacity < len) {
        vec_err_t resize_status = vec_resize(vec, len);

        if (resize_status != VEC_ERR_OK) {
            return resize_status;
        }

        vec->len = len;
    }

    for (size_t idx = 0; idx < len; ++idx) {
        memcpy((uint8_t*) vec->elems + idx * vec->elem_size, val, vec->elem_size);
    }

    return VEC_ERR_OK;
}

/*
 * If `dst` is NULL or hasn't enough capacity
 * makes a new vector containing a copy of
 * the elements of `src` in `dst`.
 * Else the elements of `src` are copied into `dst`.
 * Note that if `dst` is reallocated, it's
 * capacity will be the length of `src`.
 *
 * [Returns] `INVARG` if the `src` or `dst` are NULL
 *  or if their element size do not match,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_clone(const vec_t* src, vec_t** dst) {
    if (dst == NULL || src == NULL || (*dst != NULL && (*dst)->elem_size != src->elem_size)) {
        return VEC_ERR_INVARG;
    }

    if (*dst == NULL || (*dst)->capacity < src->len) {
        vec_t *clone = NULL;
        vec_err_t make_status = vec_make(&clone, src->elem_size, src->len);

        if (make_status != VEC_ERR_OK) {
            return make_status;
        }

        if (*dst != NULL) {
            vec_free(dst);
        }

        *dst = clone;
    }

    memcpy((*dst)->elems, src->elems, src->len * src->elem_size);
    (*dst)->len = src->len;

    return VEC_ERR_OK;
}



/*
 * Destroys the instance of vec_t.
 */
void
vec_destroy(vec_t** vec) {
    if (vec == NULL || *vec == NULL) {
        return;
    }

    vec_free(vec);
}



/* === Write Operations === */

/* == In-Place == */

/*
 * Sets the element at `idx` of `vec` with the value of `src`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_set(vec_t* vec, size_t idx, const void* src) {
    if (vec == NULL || src == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_write_idx(vec, idx, src);

    return VEC_ERR_OK;
}

/*
 * Substitutes with `val` the element of `vec` at `idx`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_replace(vec_t* vec, size_t idx, const void* val, void* old_val) {
    if (vec == NULL || val == NULL || old_val == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_write_var(vec, idx, old_val);
    vec_write_idx(vec, idx, val);

    return VEC_ERR_OK;
}

/*
 * Swaps the elements of `vec` at `idx1` at and `idx2`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx1 >= vec->len || idx2 >= vec->len) {
        return VEC_ERR_IOOB;
    }

    uint8_t* temp = malloc(vec->elem_size);

    vec_write_var(vec, idx1, temp);
    vec_write_idx(vec, idx1, (uint8_t*) vec->elems + idx2 * vec->elem_size);
    vec_write_idx(vec, idx2, temp);

    free(temp);

    return VEC_ERR_OK;
}


/* == Not In-Place == */

/*
 * Inserts `val` at the position `idx`,
 * shifting all elements after it to the right.
 * It allows inserting at the end of `vec`.
 *
 * If it has no space left it reallocates,
 * doubling it's size
 *
 * [Returns] `INVARG` if `vec` or `val` is NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 * `IOOB` if the index is out of bounds,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_insert(vec_t* vec, size_t idx, const void* val) {
    if (vec == NULL || val == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx > vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_err_t resize_status = vec_check_grow(vec);
    if (resize_status != VEC_ERR_OK) {
        return resize_status;
    }

    memmove((uint8_t*) vec->elems + (idx + 1) * vec->elem_size,
            (uint8_t*) vec->elems + idx * vec->elem_size,
            (vec->len - idx) * vec->elem_size);

    ++vec->len;
    vec_write_idx(vec, idx, val);

    return VEC_ERR_OK;
}

/*
 * Removes the value at the position `idx`,
 * shifting all elements after it to the left.
 * Optionally sets `removed` to the removed value.
 *
 * If `VEC_DISABLE_SHRINK` isn't defined and
 * the length of `vec` is less than 1/4 it's capacity
 * it will reallocate halving it's size.
 *
 * [Returns] `INVARG` if the vector is NULL
 *  or `removed` has a different size than the one expexted by `vec`,
 * `IOOB` if the index is out of bounds,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_remove(vec_t* vec, size_t idx, void* removed) {
    if (vec == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }
    if (removed != NULL) {
        vec_get(vec, idx, removed);
    }

    if (idx != vec->len - 1) {
        memmove((uint8_t*) vec->elems + idx * vec->elem_size,
                (uint8_t*) vec->elems + (idx + 1) * vec->elem_size,
                (vec->len - idx - 1) * vec->elem_size);
    }

    --vec->len;

#if !defined(VEC_DISABLE_SHRINK)

    return vec_check_shrink(vec);

#endif /* #if !defined () */

    return VEC_ERR_OK;
}

/*
 * Inserts `val` at the end of `vec`.
 *
 * If it has no space left it reallocates,
 * doubling it's size
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_push(vec_t* vec, const void* val) {
    return vec_insert(vec, vec->len, val);
}

/*
 * Removes the last element of `vec` and
 * optionally sets `popped` to the removed value.
 *
 * If `VEC_DISABLE_SHRINK` isn't defined and
 * the length of `vec` is less than 1/4 it's capacity
 * it will reallocate halving it's size.
 *
 * [Returns] `INVARG` if the vector is NULL 
 * `INVOP` if the vector is empty, `NOMEM` if fails
 * to allocate memory or `OK` otherwise.
 */
vec_err_t
vec_pop(vec_t* vec, void* popped) {
    if (vec_is_empty(vec)) {
        return VEC_ERR_INVOP;
    }

    return vec_remove(vec, vec->len - 1, popped);
}



/* === Read Operations === */

/*
 * Sets the parameter `dst` with the value of `vec` at `idx`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
vec_err_t
vec_get(const vec_t* vec, size_t idx, void* dst) {
    if (vec == NULL || dst == NULL) {
        return VEC_ERR_INVARG;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_write_var(vec, idx, dst);

    return VEC_ERR_OK;
}

/*
 * Sets `first` to the first element of `vec`.
 *
 * [Returns] `INVARG` if `vec` or `last` are NULL
 * `IOOB` if empty or `OK` otherwise
 */
vec_err_t
vec_first(const vec_t* vec, void* first) {
    return vec_get(vec, 0, first);
}

/*
 * Sets `last` to the last element of `vec`.
 *
 * [Returns] `INVARG` if `vec` or `last` are NULL
 * `IOOB` if empty or `OK` otherwise
 */
vec_err_t
vec_last(const vec_t* vec, void* last) {
    return vec_get(vec, vec->len - 1, last);
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
 * Sets `cap` to the capacity of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL, or `OK` otherwise.
 */
vec_err_t
vec_capacity(const vec_t* vec, size_t* cap) {
    if (vec == NULL || cap == NULL) {
        return VEC_ERR_INVARG;
    }

    *cap = vec->capacity;

    return VEC_ERR_OK;
}

/*
 * Sets `space` to the space left in `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
vec_err_t
vec_space(const vec_t* vec, size_t* space) {
    if (vec == NULL || space == NULL) {
        return VEC_ERR_INVARG;
    }

    *space = vec->capacity - vec->len;

    return VEC_ERR_OK;
}

/*
 * [Returns] 1 if `vec` is empty or NULL and 0 otherwise.
 */
bool
vec_is_empty(const vec_t* vec) {
    return vec == NULL || vec->len == 0;
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
        printf("[ ]\n");
        return VEC_ERR_OK;
    }

    uint8_t* byte_p = (uint8_t*) vec->elems;

    printf("[ ");
    for (size_t i = 0; i < vec->len; ++i) {
        printf("0x");
        for (int j = 0; j < vec->elem_size; ++j) {
            printf("%02X", byte_p[i * vec->elem_size + j]);
        }

        if (i != vec->len - 1) {
            printf(", ");
        }
    }
    printf(" ]\n");

    return VEC_ERR_OK;
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
        printf("[ ]\n");
        return VEC_ERR_OK;
    }

    uint8_t* byte_p = (uint8_t*) vec->elems;

    printf("len: %lu, cap: %lu\n", vec->len, vec->capacity);
    printf("[ ");
    for (size_t i = 0; i < vec->capacity; ++i) {
        printf("0x");
        for (int j = 0; j < vec->elem_size; ++j) {
            printf("%02X", i >= vec->len ? 0 : byte_p[i * vec->elem_size + j]);
        }

        if (i != vec->capacity - 1) {
            printf(", ");
        }
    }
    printf(" ]\n");

    return VEC_ERR_OK;
}



/* === Error Handling === */
  
/*
 * [Returns] the printable version of a
 * `vec_err_t` or NULL if `err`
 * doesn't belog to the enum.
 */
const char*
vec_get_err_msg(vec_err_t err) {
    if (err >= VEC_ERR_COUNT || err < 0) {
        return NULL;
    }

    return VEC_ERR_MSG[err];
}
