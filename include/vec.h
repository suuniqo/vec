
#if !defined(VEC_H)
#define VEC_H


/** 
 * @file            vec.h
 * @author          Andrés Súnico
 * @brief           Header of a dynamically resizing vector data structure
 */


/* ========= INCLUDES ========== */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


/* =========== DATA ============ */

/**
 * @brief           Dynamic array of variable length
 *
 * @note            Resizes and shrinks dynamically, can store data of any type
 *                      as long as it has a constant size
 */
typedef struct vec vec_t;

/**
 * @brief           Enumeration of vector error types
 */
typedef enum {
    VEC_ERR_OK = 0,                             /**< Operation completed succesfully */
    VEC_ERR_IOOB,                               /**< Index was out of bounds */
    VEC_ERR_NULLPTR,                            /**< A pointer parameter was NULL */
    VEC_ERR_INVPTR,                             /**< Vector magic number mismatch */
    VEC_ERR_INVOP,                              /**< Can't perform operation */
    VEC_ERR_NOMEM,                              /**< Failed to allocate memory */
    VEC_ERR_COUNT,                              /**< Number of errors in the enum */
} vec_err_t;

#define SHRINK_POLICY 4                         /**< Minimum ratio of capacity / len */
#define SHRINK_FACTOR 2                         /**< Factor by which the capacity shrinks */

#define GROWTH_POLICY 1                         /**< Maximum ratio of len / capacity */
#define GROWTH_FACTOR 2                         /**< Factor by which the capacity grows */

#define MIN_CAPACITY 16                         /**< Minimum capacity of the vector */
#define MAX_CAPACITY INT_MAX                    /**< Maximum capacity of the vector */

#define MIN_DATA_SIZE 1                         /**< Maximum size in bytes of elements */
#define MAX_DATA_SIZE INT_MAX                   /**< Maximum size in bytes of elements */


/* =========== MACRO =========== */

/* === Memory Management === */

/**
 * @brief           Straightforward version of `vec_make`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_MAKE_AS(type, capacity) ({ \
    vec_t* __tmp = NULL; \
    (void)vec_make(&__tmp, sizeof((type)), (capacity)); \
    __tmp; \
})


/* === Write Operations === */

/**
 * @brief           Straightforward version of `vec_set`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_SET_AS(type, vec, idx, val) ({ \
    (type) __tmp; \
    (void)vec_set((vec), (idx), &(val), &__tmp); \
    __tmp; \
})

/**
 * @brief           Straightforward version of `vec_remove`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_REMOVE_AS(type, vec, idx) ({ \
    (type) __tmp; \
    (void)vec_remove((vec), (idx), &__tmp); \
    __tmp; \
})

/**
 * @brief           Straightforward version of `vec_pop`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_POP_AS(type, vec) ({ \
    (type) __tmp; \
    (void)vec_pop((vec), &__tmp); \
    __tmp; \
})


/* === Read Operations === */

/**
 * @brief           Straightforward version of `vec_get`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_GET_AS(type, vec, idx) ({ \
    (type) __tmp; \
    (void)vec_get((vec), (idx), &__tmp) == VEC_ERR_OK ? __tmp : (type){0}; \
    __tmp; \
})

/**
 * @brief           Straightforward version of `vec_first`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_FIRST_AS(type, vec) ({ \
    (type) __tmp; \
    (void)vec_first((vec), &__tmp); \
    __tmp; \
})

/**
 * @brief           Straightforward version of `vec_last`, which ignores errors
 * @note            Use with caution, as it assumes that no errors occurred
 */
#define VEC_LAST_AS(type, vec) ({ \
    (type) __tmp; \
    (void)vec_last((vec), &__tmp); \
    __tmp; \
})


/* ========== METHODS ========== */

/* === Memory Management === */

/**
 * @brief           Creates a new instance of `vec_t`, allocating memory for `capacity`
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
 * - `INVOP`        If `elem_size` < `MIN_DATA_SIZE`, `elem_size` > `MAX_DATA_SIZE`, or if `vec` doesn't point to `NULL`
 * - `NULLPTR`      If `vec` is `NULL`
 * - `NOMEM`        If fails to allocate memory 
 * - `OK`           On success
 */
extern vec_err_t
vec_make(vec_t** vec, size_t elem_size, size_t capacity);

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
extern vec_err_t
vec_resize(vec_t* vec, size_t capacity);

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
extern vec_err_t
vec_shrink_to_fit(vec_t* vec);

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
extern vec_err_t
vec_clear(vec_t* vec);

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
extern vec_err_t
vec_clone(const vec_t* src, vec_t** dst);

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
extern vec_err_t
vec_destroy(vec_t** vec);


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
extern vec_err_t
vec_set(vec_t* vec, size_t idx, const void* src, void* old);

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
extern vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2);

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
extern vec_err_t
vec_insert(vec_t* vec, size_t idx, const void* src);

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
extern vec_err_t
vec_remove(vec_t *vec, size_t idx, void* removed);

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
extern vec_err_t
vec_push(vec_t* vec, const void* src);

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
extern vec_err_t
vec_pop(vec_t* vec, void* popped);

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
extern vec_err_t
vec_fill(vec_t* vec, const void* src, size_t len);

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
extern vec_err_t
vec_truncate(vec_t* vec, size_t len);

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
extern vec_err_t
vec_extend(vec_t* dst, const vec_t* src);

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
extern vec_err_t
vec_get(const vec_t* vec, size_t idx, void* dst);

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
extern vec_err_t
vec_first(const vec_t* vec, void* first);

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
extern vec_err_t
vec_last(const vec_t* vec, void* last);

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
extern vec_err_t
vec_len(const vec_t* vec, size_t* len);

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
extern vec_err_t
vec_capacity(const vec_t* vec, size_t* cap);

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
extern vec_err_t
vec_space(const vec_t* vec, size_t* space);

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
extern vec_err_t
vec_is_empty(const vec_t* vec, bool* is_empty);

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
extern vec_err_t
vec_display(const vec_t* vec, FILE* fd);

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
extern vec_err_t
vec_debug(const vec_t* vec, FILE* fd);


/* === Error Handling === */

/**
 * @return          The printable version of
 *                      a `vec_err_t` or NULL if `err`
 *                      doesn't belong to the enum
 */
extern const char*
vec_get_err_msg(vec_err_t err);


#endif /* defined(VEC_H) */
