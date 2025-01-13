
#if !defined(VEC_H)
#define VEC_H


/* ========= INCLUDES ========== */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* =========== DATA ============ */

/*
 * Dynamic array of variable length,
 * resizes and shrinks dynamically
 * which can store data of any size while 
 * it is constant
 */
typedef struct vec vec_t;

/*
 * Enumeration of vector error types
 */
typedef enum {
    VEC_ERR_OK = 0,                             /* Operation completed succesfully */
    VEC_ERR_IOOB,                               /* Index was out of bounds */
    VEC_ERR_NULLPTR,                            /* A pointer parameter was NULL */
    VEC_ERR_INVPTR,                             /* Vector magic number mismatch */
    VEC_ERR_INVOP,                              /* Can't perform operation */
    VEC_ERR_NOMEM,                              /* Failed to allocate memory */
    VEC_ERR_COUNT,                              /* Number of errors in the enum */
} vec_err_t;


/* ========== METHODS ========== */

/* === Memory Management === */

/*
 * Creates a new instance of vec_t, allocating memory for 
 * `capacity` (or default value if negative) elements of size `elem_size`
 *
 * [Returns] `INVOP` if element size is smaller than one byte
 * or is bigger than `MAX_DATA_SIZE`, `NULLPTR` if `vec` is NULL,
 * NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_make(vec_t** vec, size_t elem_size, size_t capacity);

/*
 * Resizes the vector with `capacity` memory
 *
 * [Returns] `NOMEM` if fails to allocate memory,
 * `NULLPTR` if the vector is NULL, 
 * `INVPTR` if the vector pointer validation fails,
 * `INVOP` if capacity isn't in the valid range or
 * `OK` otherwise.
 */
extern vec_err_t
vec_resize(vec_t* vec, size_t capacity);

/*
 * Reallocates the vector, reserving the exact amount of
 * memory needed to hold it's current contents
 *
 * [Returns] `NULLPTR` if the vector is NULL, `INVOP` if it's empty,
 * `INVPTR` if the vector pointer validation fails,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_shrink_to_fit(vec_t* vec);

/*
 * Resets the vector, reallocating
 * and deleting it's contents if enough memory.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_clear(vec_t* vec);

/*
 * If `dst` is NULL or hasn't enough capacity
 * makes a new vector containing a copy of
 * the elements of `src` in `dst`.
 * Else the elements of `src` are copied into `dst`.
 * Note that if `dst` is reallocated, it's
 * capacity will be the length of `src`.
 *
 * [Returns] `NULLPTR` if the `src` or `dst` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `INVOP` if their element size do not match,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_clone(const vec_t* src, vec_t** dst);

/*
 * Destroys the instance of vec_t.
 *
 * [Returns] `NULLPTR` if the pointer or what it points to is NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_destroy(vec_t** vec);


/* === Write Operations === */

/*
 * Sets the element at `idx` of `vec` with the value of `src`
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_set(vec_t* vec, size_t idx, const void* src);

/*
 * Substitutes with `val` the element of `vec` at `idx`.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if the index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_replace(vec_t* vec, size_t idx, const void* val, void* old_val);

/*
 * Swaps the elements of `vec` at `idx1` at and `idx2`.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `NOMEM` if there isn't enough space for a temp variable,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2);

/*
 * Inserts `val` at the position `idx`,
 * shifting all elements after it to the right.
 * It allows inserting at the end of `vec`.
 *
 * If it has no space left it reallocates,
 * doubling it's size
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if the index is out of bounds,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_insert(vec_t* vec, size_t idx, const void* val);

/*
 * Removes the value at the position `idx`,
 * shifting all elements after it to the left.
 * Optionally sets `removed` to the removed value.
 *
 * If `VEC_DISABLE_SHRINK` isn't defined and
 * the length of `vec` is less than 1/4 it's capacity
 * it will reallocate halving it's size.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if the index is out of bounds,
 * `NOMEM` if fails to reallocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_remove(vec_t *vec, size_t idx, void* removed);

/*
 * Inserts `val` at the end of `vec`.
 *
 * If it has no space left it reallocates,
 * doubling it's size
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_push(vec_t* vec, const void* val);

/*
 * Removes the last element of `vec` and
 * optionally sets `popped` to the removed value.
 *
 * If `VEC_DISABLE_SHRINK` isn't defined and
 * the length of `vec` is less than 1/4 it's capacity
 * it will reallocate halving it's size.
 *
 * [Returns] `NULLPTR` if the vector is NULL 
 * `INVPTR` if the vector pointer validation fails,
 * `INVOP` if the vector is empty, `NOMEM` if fails
 * to reallocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_pop(vec_t* vec, void* popped);

/*
 * Fills the first `len` positions of the vector with `val`.
 * If it hasn't enough capacity it is reallocated to hold `len` elements.
 *
 * [Returns] `NULLPTR` if the `vec` or `val` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_fill(vec_t* vec, const void* val, size_t len);

/*
 * Keeps the first `len` elements of `vec` and discards the rest.
 * If `len` isn't smaller than the current length of `vec` it does nothing.
 *
 * If `VEC_DISABLE_SHRINK` isn't defined and
 * the length of `vec` is less than 1/4 it's capacity
 * it will reallocate halving it's size.
 *
 * [Returns] `NULLPTR` if the `vec` or `val` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_truncate(vec_t* vec, size_t len);

/*
 * Concatenates the contents of `other` into `self`.
 * If self hasn't enough capacity it is reallocated.
 *
 * [Returns] `NULLPTR` if `self` or `other` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `INVOP` if extending `self` would surpass the maximum capacity
 *  `self` equals `other`, or they do not have the same element size,
 * `NOMEM` if fails to reallocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_extend(vec_t* self, const vec_t* other);

/* === Read Operations === */

/*
 * Sets the parameter `dst` with the value of `vec` at `idx`.
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_get(const vec_t* vec, size_t idx, void* dst);

/*
 * Sets `first` to the first element of `vec`.
 *
 * [Returns] `NULLPTR` if `vec` or `last` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if empty or `OK` otherwise
 */
extern vec_err_t
vec_first(const vec_t* vec, void* first);

/*
 * Sets `last` to the last element of `vec`.
 *
 * [Returns] `NULLPTR` if `vec` or `last` are NULL,
 * `INVPTR` if the vector pointer validation fails,
 * `IOOB` if empty or `OK` otherwise
 */
extern vec_err_t
vec_last(const vec_t* vec, void* last);

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_len(const vec_t* vec, size_t* len);

/*
 * Sets `cap` to the capacity of `vec`.
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_capacity(const vec_t* vec, size_t* cap);

/*
 * Sets `space` to the space left in `vec`.
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_space(const vec_t* vec, size_t* space);

/*
 * Sets `is_empty` to 1 if the length of the vector is 0, or to 0 otherwise.
 *
 * [Returns] `NULLPTR` if any of the parameters are NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_is_empty(const vec_t* vec, bool* is_empty);

/*
 * Prints the formatted contents of `vec`.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_display(const vec_t* vec);

/*
 * Prints the formatted contents of the memory allocated by `vec`.
 *
 * [Returns] `NULLPTR` if the vector is NULL,
 * `INVPTR` if the vector pointer validation fails, or `OK` otherwise.
 */
extern vec_err_t
vec_debug(const vec_t* vec);


/* === Error Handling === */

/*
 * [Returns] the printable version of a
 * `vec_err_t` or NULL if `err`
 * doesn't belong to the enum.
 */
extern const char*
vec_get_err_msg(vec_err_t err);


#endif /* defined(VEC_H) */
