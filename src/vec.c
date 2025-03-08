
#include "../include/vec.h"


/** 
 * @file            vec.c
 * @author          Andrés Súnico
 * @brief           Implementation of a dynamically resizing vector data structure
 */


/* ========= INCLUDES ========== */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* =========== DATA ============ */

/**
 * @brief           Dynamic array of variable length
 *
 * @note            Resizes and shrinks dynamically, can store data of any type
 *                      as long as it has a constant size
 */
struct vec {
    uint32_t magic;                             /**< Magic number for pointer validation */
    size_t len;                                 /**< Current number of elements */
    size_t capacity;                            /**< Current allocated memory */
    size_t elem_size;                           /**< Size of the data type stored */
    void* elems;                                /**< Memory containing the elements */
};

/**
 * @brief           Mapping between values of `vec_err_t`
 *                      and their printable values.
 */
static const char* const VEC_ERR_MSG[] = {
    "VEC_ERR_OK",
    "VEC_ERR_IOOB",
    "VEC_ERR_NULLPTR",
    "VEC_ERR_INVPTR",
    "VEC_ERR_INVOP",
    "VEC_ERR_INVOP",
    "VEC_ERR_NOMEM"
};

#define VEC_MAGIC 0xF3EDB4BE                    /**< Magic value for vector pointer validation */


/* ========== PRIVATE ========== */

/* === Validation === */

/**
 * @brief           Verifies that `vec` is a correct instance
 *
 * @param[in]       vec: pointer to `vec_t` instance
 *
 * @return
 * - `NULLPTR`      If `vec` or its elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success 
 */
static vec_err_t
vec_validate_ptr(const vec_t* vec) {
    if (vec == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (vec->magic != VEC_MAGIC) {
        return VEC_ERR_INVPTR;
    }
    if (vec->elems == NULL) {
        return VEC_ERR_NULLPTR;
    }

    return VEC_ERR_OK;
}


/* === Reallocation === */

/**
 * @brief           Reallocates `vec` with `capacity` memory
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       capacity: memory to reallocate
 *
 * @return
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success 
 */
static vec_err_t
vec_reallocate(vec_t* vec, size_t capacity) {
    void* new_elems = realloc(vec->elems, capacity * vec->elem_size); 

    if (new_elems == NULL) {
        return VEC_ERR_NOMEM;
    }

    vec->elems = new_elems;
    vec->capacity = capacity;
    vec->len = vec->len > capacity ? capacity : vec->len;

    return VEC_ERR_OK;
}

/**
 * @brief           Destroys the instance of `vec_t`
 *
 * @param[in]       vec: double pointer to `vec_t` instance
 */
static void
vec_free(vec_t** vec) {
    free((*vec)->elems);
    free(*vec);

    *vec = NULL;
}

/**
 * @brief           Grows the capacity `vec` if the policy is fulfilled
 *
 * @param[in]       vec: pointer to `vec_t` instance
 *
 * @note            The new capacity will be `vec_capacity` * `GROWTH_FACTOR`
 * @note            It will grow if `vec_len` is greater or equal to `vec_capacity` * `GROWTH_POLICY`
 *
 * @return
 * - `NOMEM`        If fails to allocate memory
 * - `INVOP`        If growing would surpass maximum space
 * - `OK`           On success
 */
static vec_err_t
vec_check_grow(vec_t* vec) {
    if (vec->len < vec->capacity * GROWTH_POLICY) {
        return VEC_ERR_OK;
    }
    if (vec->capacity > MAX_CAPACITY / GROWTH_FACTOR || vec->capacity * GROWTH_FACTOR > MAX_CAPACITY) {
        return VEC_ERR_INVOP;
    }

    size_t new_capacity = vec->capacity * GROWTH_FACTOR;
    return vec_reallocate(vec, MIN_CAPACITY > new_capacity ? MIN_CAPACITY : new_capacity);
}

/**
 * @brief           Shrinks the vector if the policy is fulfilled
 *
 * @param[in]       vec: pointer to `vec_t` instance
 *
 * @note            The new capacity will be `vec_capacity` / `SHRINK_FACTOR`
 * @note            It will shrink if `vec_len` is less than `vec_capacity` / `SHRINK_POLICY`
 *
 * @return
 * - `NOMEM`        If fails to allocate memory
 * - `OK`           On success
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

/**
 * @brief           Writes into `vec` at `idx` the contents of `src`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx: position of `vec` where data will be written into
 * @param[in]       src: variable where data will be read from
 */
static void
vec_write_idx(vec_t* vec, size_t idx, const void* src) {
    memcpy((uint8_t*) vec->elems + (idx * vec->elem_size), src, vec->elem_size);
}

/**
 * @brief           Writes into `dst` the contents of `vec` at `idx`.
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx: position of `vec` where data will be read from
 * @param[out]      dst: variable where data will be written into
 */
static void
vec_write_var(const vec_t* vec, size_t idx, void* dst) {
    memcpy(dst, (uint8_t*) vec->elems + (idx * vec->elem_size), vec->elem_size);
}


/* ========== PUBLIC ========== */

/* === Memory Management === */

/**
 * @brief           Creates a new instance of vec_t, allocating memory for `capacity`
 *                      (or default value if negative) elements of size `elem_size`
 *
 * @param[out]      vec: double pointer to unintialized `vec_t` instance (so it must point to `NULL`)
 * @param[in]       elem_size: the size, in bytes, of the data type to be stored
 * @param[in]       capacity: the initial capacity of the vector
 *
 * @note            If `capacity` is smaller than  `MIN_CAPACITY`, then it will be clamped
 * @note            It's required that `vec` points to `NULL`
 *
 * @return
 * - `INVOP`        If `elem_size` is < `MIN_DATA_SIZE`, `elem_size` > `MAX_DATA_SIZE`, or if `vec` doesn't point to `NULL`
 * - `NULLPTR`      If `vec` is `NULL`
 * - `NOMEM`        If fails to allocate memory 
 * - `OK`           On success
 */
vec_err_t
vec_make(vec_t** vec, size_t elem_size, size_t capacity) {
    if (vec == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (elem_size < MIN_DATA_SIZE || elem_size > MAX_DATA_SIZE) {
        return VEC_ERR_INVOP;
    }
    if (*vec != NULL) {
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
        .magic = VEC_MAGIC,
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

/**
 * @brief           Resizes the vector with `capacity` memory
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       capacity: new capacity of `vec` 
 *
 * @return
 * - `NULLPTR`      If `vec` or its elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `NOMEM`        If memory reallocation fails
 * - `INVOP`        If capacity isn't in the valid range or
 * - `OK`           On success
 */
vec_err_t
vec_resize(vec_t* vec, size_t capacity) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (capacity <= MIN_CAPACITY || capacity > MAX_CAPACITY) {
        return VEC_ERR_INVOP;
    }

    return vec_reallocate(vec, capacity);
}

/**
 * @brief           Reallocates the vector, reserving the exact amount of
 *                      memory needed to hold it's current contents
 *
 * @param[in]       vec: pointer to `vec_t` instance
 *
 * @return
 * - `NULLPTR`      If `vec` or its elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If `vec` is empty
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_shrink_to_fit(vec_t* vec) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (vec->len == 0) {
        return VEC_ERR_INVOP;
    }
    if (vec->len == vec->capacity) {
        return VEC_ERR_OK;
    }

    return vec_reallocate(vec, vec->len);
}

/**
 * @brief           Resets the vector, reallocating
 *                      and deleting it's contents if enough memory
 *
 * @param[in]       vec: pointer to `vec_t` instance
 *
 * @return
 * - `NULLPTR`      If `vec` or its elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_clear(vec_t* vec) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
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

/**
 * @brief           If `dst` is NULL or hasn't enough capacity
 *                      makes a new vector containing a copy of
 *                      the elements of `src` in `dst`,
 *                      else the elements of `src` are copied into `dst`
 *
 * @param[in]       src: pointer to `vec_t` instance, where data will be read from
 * @param[out]      dst: double pointer to `vec_t` instance, where data will be written into
 *
 * @note            If `dst` is reallocated, it's
 *                      capacity will be the length of `src`
 *
 * @return
 * - `NULLPTR`      If `src` or `dst` are NULL, or if their elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If their element size do not match or `dst` points to `src`
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_clone(const vec_t* src, vec_t** dst) {
    vec_err_t vec_status;

    vec_status = vec_validate_ptr(src);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (dst == NULL) {
        return VEC_ERR_NULLPTR;
    }

    if (*dst != NULL) {
        vec_status = vec_validate_ptr(*dst);

        if (vec_status != VEC_ERR_OK) {
            return vec_status;
        }
        if ((*dst)->elem_size != src->elem_size || *dst == src) {
            return VEC_ERR_INVOP;
        }
    }

    if (*dst == NULL || (*dst)->capacity < src->len) {
        vec_t *clone = NULL;

        vec_status = vec_make(&clone, src->elem_size, src->len);

        if (vec_status != VEC_ERR_OK) {
            return vec_status;
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

/**
 * @brief           Destroys the instance of `vec_t`
 *
 * @param[in]       vec: double pointer to `vec_t` instance
 *
 * @return
 * - `NULLPTR`      If `vec`, what it points to, or it's elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_destroy(vec_t** vec) {
    if (vec == NULL) {
        return VEC_ERR_NULLPTR;
    }

    vec_err_t vec_status = vec_validate_ptr(*vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }

    vec_free(vec);

    return VEC_ERR_OK;
}


/* === Write Operations === */

/**
 * @brief           Sets the element at `idx` of `vec` with the value of `src`,
 *                      if old isn't `NULL`, the old value will be written into it
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx: position of `vec` where data will be written into
 * @param[in]       src: variable where data will be read from
 * @param[out]      old: variable where the old data will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `src` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `idx` greater or equal to the vector's length
 * - `OK`           On success
 */
vec_err_t
vec_set(vec_t* vec, size_t idx, const void* src, void* old) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (src == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    if (old != NULL) {
        vec_write_var(vec, idx, old);
    }
    vec_write_idx(vec, idx, src);

    return VEC_ERR_OK;
}

/**
 * @brief           Swaps the elements of `vec` at `idx1` at and `idx2`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx1: first position of `vec` where data will swapped
 * @param[in]       idx2: second position of `vec` where data will be swapped
 *
 * @return
 * - `NULLPTR`      If `vec` or it's elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If `idx1` equals `idx2`
 * - `IOOB`         If `idx1` or `idx2` are greater or equal to the vector's length
 * - `NOMEM`        If fails to allocate memory for an auxiliary variable
 * - `OK`           On success
 */
vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (idx1 == idx2) {
        return VEC_ERR_INVOP;
    }
    if (idx1 >= vec->len || idx2 >= vec->len) {
        return VEC_ERR_IOOB;
    }

    uint8_t* temp = malloc(vec->elem_size);
    if (temp == NULL) {
        return VEC_ERR_NOMEM;
    }

    vec_write_var(vec, idx1, temp);
    vec_write_idx(vec, idx1, (uint8_t*) vec->elems + (idx2 * vec->elem_size));
    vec_write_idx(vec, idx2, temp);

    free(temp);

    return VEC_ERR_OK;
}

/**
 * @brief           Inserts `src` at the position `idx`,
 *                      shifting all elements after it to the right.
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx: position of `vec` where data will be written into
 * @param[in]       src: variable where data will be read from
 *
 * @note            It allows inserting at the end of `vec`.
 * @note            If `vec_len` is greater or equal to `vec_capacity` * `GROWTH_POLICY` it reallocates,
 *                      growing it's capacity to `vec_capacity` * `GROWTH_FACTOR`
 *
 * @return 
 * - `NULLPTR`      If `vec`, it's elements or `src` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `idx` greater than the vector's length
 * - `INVOP`        If it has reached maximum capacity
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_insert(vec_t* vec, size_t idx, const void* src) {
    vec_err_t vec_status;

    vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }

    if (src == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (idx > vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_status = vec_check_grow(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }

    memmove((uint8_t*) vec->elems + ((idx + 1) * vec->elem_size),
            (uint8_t*) vec->elems + (idx * vec->elem_size),
            (vec->len - idx) * vec->elem_size);

    ++vec->len;
    vec_write_idx(vec, idx, src);

    return VEC_ERR_OK;
}

/**
 * @brief           Removes the value at the position `idx`,
 *                      shifting all elements after it to the left and
 *                      optionally writes `removed` with the removed value
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       idx: position of `vec` that will be removed
 * @param[out]      removed: If not `NULL`, it will be written with the removed data
 *
 * @note            If `VEC_DISABLE_SHRINK` isn't defined and
 *                      `vec_len` is less than `vec_capacity` / `SHRINK_POLICY`,
 *                      it's capacity will shrink to `vec_capacity` / `SHRINK_FACTOR`
 *
 * @return 
 * - `NULLPTR`      If `vec` or it's elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `idx` greater or equal to the vector's length
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_remove(vec_t* vec, size_t idx, void* removed) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }
    if (removed != NULL) {
        vec_write_var(vec, idx, removed);
    }

    if (idx != vec->len - 1) {
        memmove((uint8_t*) vec->elems + (idx * vec->elem_size),
                (uint8_t*) vec->elems + ((idx + 1) * vec->elem_size),
                (vec->len - idx - 1) * vec->elem_size);
    }

    --vec->len;

#if !defined(VEC_DISABLE_SHRINK)

    return vec_check_shrink(vec);

#endif /* #if !defined(VEC_DISABLE_SHRINK) */

    return VEC_ERR_OK;
}

/**
 * @brief           Inserts `src` at the end of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       src: variable where data will be read from
 *
 * @note            If `vec_len` is greater or equal to `vec_capacity` * `GROWTH_POLICY` it reallocates,
 *                      growing it's capacity to `vec_capacity` * `GROWTH_FACTOR`
 *
 * @return 
 * - `NULLPTR`      If `vec`, it's elements or `src` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If it has reached maximum capacity
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_push(vec_t* vec, const void* src) {
    return vec_insert(vec, vec->len, src);
}

/**
 * @brief           Removes the last element of `vec` and
 *                      optionally sets `popped` to the removed value
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      popped: If not `NULL`, it will be written with the removed data
 *
 * @note            If `VEC_DISABLE_SHRINK` isn't defined and
 *                      `vec_len` is less than `vec_capacity` / `SHRINK_POLICY`,
 *                      it's capacity will shrink to `vec_capacity` / `SHRINK_FACTOR`
 *
 * @return 
 * - `NULLPTR`      If `vec` or it's elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If `vec` is empty
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_pop(vec_t* vec, void* popped) {
    if (vec == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (vec->len == 0) {
        return VEC_ERR_INVOP;
    }

    return vec_remove(vec, vec->len - 1, popped);
}

/**
 * @brief           Fills the first `len` positions of the vector with `src`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       src: variable where data will be read from
 * @param[in]       len: number of elements with which `vec` will be filled
 *
 * @note            If it hasn't enough capacity it is reallocated to hold `len` elements
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `src` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If `len` is `0` or greater than `MAX_CAPACITY`
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_fill(vec_t* vec, const void* src, size_t len) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (src == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (len == 0) {
        return VEC_ERR_OK;
    }
    if (vec->capacity < len) {
        vec_status = vec_resize(vec, len);

        if (vec_status != VEC_ERR_OK) {
            return vec_status;
        }
    }
    if (vec->len < len) {
        vec->len = len;
    }

    memcpy((uint8_t*) vec->elems,
            src,
            vec->elem_size);

    size_t offset;
    for (offset = 1; offset << 1 < len; offset <<= 1) {
        memcpy((uint8_t*) vec->elems + (offset * vec->elem_size),
                vec->elems,
                offset * vec->elem_size);
    }

    memcpy((uint8_t*) vec->elems + (offset * vec->elem_size),
            vec->elems,
           (len - offset) * vec->elem_size);

    return VEC_ERR_OK;
}

/**
 * @brief           Keeps the first `len` elements of `vec` and discards the rest
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       len: number of elements which will be preserved in `vec`
 *
 * @note            If `len` isn't smaller than the current length of `vec` it does nothing
 * @note            If `VEC_DISABLE_SHRINK` isn't defined and
 *                      `vec_len` is less than `vec_capacity` / `SHRINK_POLICY`,
 *                      it's capacity will shrink to `vec_capacity` / `SHRINK_FACTOR`
 *
 * @return
 * - `NULLPTR`      If `vec` or it's elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_truncate(vec_t* vec, size_t len) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (len >= vec->len) {
        return VEC_ERR_OK;
    }

    vec->len = len;

#if !defined(VEC_DISABLE_SHRINK)

    return vec_check_shrink(vec);

#endif /* #if !defined(VEC_DISABLE_SHRINK) */

    return VEC_ERR_OK;
}

/**
 * @brief           Concatenates the contents of `other` into `self`
 *
 * @param[in]       dst: pointer to `vec_t` instance, where data will be written into
 * @param[in]       src: pointer to `vec_t` instance, where data will be read from
 *
 * @note            If self hasn't enough capacity it is reallocated
 *
 * @return 
 * - `NULLPTR`      If `self`, `other` or their elements are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `INVOP`        If extending `self` would surpass `MAX_CAPACITY`,
 *                      if `self` equals `other`, or if they do not have the same element size
 * - `NOMEM`        If memory reallocation fails
 * - `OK`           On success
 */
vec_err_t
vec_extend(vec_t* dst, const vec_t* src) {
    vec_err_t vec_status;

    if (dst == src) {
        return VEC_ERR_INVOP;
    }

    vec_status = vec_validate_ptr(dst);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }

    vec_status = vec_validate_ptr(src);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }

    if (dst->elem_size != src->elem_size) {
        return VEC_ERR_INVOP;
    }

    if (dst->capacity - src->len < dst->len)  {
        vec_status = vec_resize(dst, dst->len + src->len);

        if (vec_status != VEC_ERR_OK) {
            return vec_status;
        }
    }

    memcpy((uint8_t*)dst->elems + (dst->len * dst->elem_size),
            src->elems,
            src->len * src->elem_size);

    dst->len += src->len;

    return VEC_ERR_OK;
}


/* === Read Operations === */

/**
 * @brief           Sets the parameter `dst` with the value of `vec` at `idx`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      dst: variable where data will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `dst` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `idx` greater or equal to the vector's length
 * - `OK`           On success
 */
vec_err_t
vec_get(const vec_t* vec, size_t idx, void* dst) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (dst == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (idx >= vec->len) {
        return VEC_ERR_IOOB;
    }

    vec_write_var(vec, idx, dst);

    return VEC_ERR_OK;
}

/**
 * @brief           Sets `first` to the first element of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      first: variable where data will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `first` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `vec` is empty
 * - `OK`           On success
 */
vec_err_t
vec_first(const vec_t* vec, void* first) {
    return vec_get(vec, 0, first);
}

/**
 * @brief           Sets `last` to the last element of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      last: variable where data will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `last` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `IOOB`         If `vec` is empty
 * - `OK`           On success
 */
vec_err_t
vec_last(const vec_t* vec, void* last) {
    return vec_get(vec, vec->len - 1, last);
}

/**
 * @brief           Sets `len` to the length of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      len: variable where the length will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `len` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_len(const vec_t* vec, size_t* len) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (len == NULL) {
        return VEC_ERR_NULLPTR;
    }

    *len = vec->len;

    return VEC_ERR_OK;
}

/**
 * @brief           Sets `cap` to the capacity of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      cap: variable where the capacity will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `cap` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_capacity(const vec_t* vec, size_t* cap) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (cap == NULL) {
        return VEC_ERR_NULLPTR;
    }

    *cap = vec->capacity;

    return VEC_ERR_OK;
}

/**
 * @brief           Sets `space` to the space left in `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      space: variable where the space will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `space` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_space(const vec_t* vec, size_t* space) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (space == NULL) {
        return VEC_ERR_NULLPTR;
    }

    *space = vec->capacity - vec->len;

    return VEC_ERR_OK;
}

/**
 * @brief           Sets `is_empty` to `1` if the length of the vector is `0`, or to `0` otherwise
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[out]      is_empty: variable where the result will be written into
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `is_empty` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_is_empty(const vec_t* vec, bool* is_empty) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (is_empty == NULL) {
        return VEC_ERR_NULLPTR;
    }

    *is_empty = vec->len == 0;

    return VEC_ERR_OK;
}

/**
 * @brief           Prints the formatted contents of `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       fd: descriptor of the file where the output will be printed
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `fd` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_display(const vec_t* vec, FILE* fd) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (fd == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (vec->len == 0) {
        fprintf(fd, "[ ]\n");
        return VEC_ERR_OK;
    }

    uint8_t* byte_p = (uint8_t*) vec->elems;

    fprintf(fd, "[ ");
    for (size_t i = 0; i < vec->len; ++i) {
        fprintf(fd, "0x");
        for (size_t j = 0; j < vec->elem_size; ++j) {
            fprintf(fd, "%02X", byte_p[(i * vec->elem_size) + j]);
        }

        if (i != vec->len - 1) {
            fprintf(fd, ", ");
        }
    }
    fprintf(fd, " ]\n");

    return VEC_ERR_OK;
}

/**
 * @brief           Prints the formatted contents of the memory allocated by `vec`
 *
 * @param[in]       vec: pointer to `vec_t` instance
 * @param[in]       fd: descriptor of the file where the output will be printed
 *
 * @return
 * - `NULLPTR`      If `vec`, it's elements or `fd` are `NULL`
 * - `INVPTR`       If the vector pointer validation fails
 * - `OK`           On success
 */
vec_err_t
vec_debug(const vec_t* vec, FILE* fd) {
    vec_err_t vec_status = vec_validate_ptr(vec);

    if (vec_status != VEC_ERR_OK) {
        return vec_status;
    }
    if (fd == NULL) {
        return VEC_ERR_NULLPTR;
    }
    if (vec->capacity == 0) {
        printf("[ ]\n");
        return VEC_ERR_OK;
    }

    uint8_t* byte_p = (uint8_t*) vec->elems;

    fprintf(fd, "len: %lu, cap: %lu\n", vec->len, vec->capacity);
    fprintf(fd, "[ ");
    for (size_t i = 0; i < vec->capacity; ++i) {
        fprintf(fd, "0x");
        for (size_t j = 0; j < vec->elem_size; ++j) {
            fprintf(fd, "%02X", i >= vec->len ? 0 : byte_p[(i * vec->elem_size) + j]);
        }

        if (i != vec->capacity - 1) {
            fprintf(fd, ", ");
        }
    }
    fprintf(fd, " ]\n");

    return VEC_ERR_OK;
}


/* === Error Handling === */
  
/**
 * @return          The printable version of
 *                      a `vec_err_t` or NULL if `err`
 *                      doesn't belong to the enum
 */
const char*
vec_get_err_msg(vec_err_t err) {
    if (err >= VEC_ERR_COUNT || err < 0) {
        return NULL;
    }

    return VEC_ERR_MSG[err];
}
