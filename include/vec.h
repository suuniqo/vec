
#if !defined(VEC_H)
#define VEC_H


/* ========= INCLUDES ========== */

#include <stddef.h>
#include <stdint.h>


/* =========== DATA ============ */

/*
 * Dynamic array of variable length
 */
typedef struct vec vec_t;

/*
 * Enumeration of vector error types
 */
typedef enum {
    VEC_ERR_OK = 0,                             /* operation completed succesfully */
    VEC_ERR_IOOB,                               /* index was out of bounds */
    VEC_ERR_INVARG,                             /* pointer to vector was NULL */
    VEC_ERR_INVOP,                              /* can't perform operation */
    VEC_ERR_NOMEM,                              /* failed to allocate memory */
    VEC_ERR_COUNT,                              /* number of errors in the enum */
} vec_err_t;


/* ========== METHODS ========== */

/*
 * Creates a new instance of vec_t, allocating memory for 
 * `capacity` (or default value if negative) integers.
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_make(vec_t** vec, size_t capacity);

/*
 * Reallocates the vector, reserving the exact amount of
 * memory needed to hold it's current contents
 *
 * [Returns] `INVARG` if the vector is NULL, `INVOP` if it's empty,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
extern vec_err_t
vec_shrink_to_fit(vec_t* vec);

/*
 * Resets the vector, reallocating
 * and deleting it's contents.
 *
 * [Returns] `NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_clear(vec_t *vec);

/*
 * Destroys the instance of vec_t.
 */
extern void
vec_free(vec_t *vec);

/*
 * Substitutes with `val` the element of `vec` at `idx`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_replace(vec_t *vec, size_t idx, int32_t val, int32_t* old_val);

/*
 * Inserts `val` at the end of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
extern vec_err_t
vec_push(vec_t *vec, int32_t val);

/*
 * Removes the last element of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `INVOP` if the vector is empty, `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
extern vec_err_t
vec_pop(vec_t *vec, int32_t* popped);

/*
 * Inserts `val` at the position `idx`,
 * shifting all elements after it to the right.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds,
 * `INVOP` if it has reached maximum capacity,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
extern vec_err_t
vec_insert(vec_t *vec, size_t idx, int32_t val);

/*
 * Removes the value at the position `idx`,
 * shifting all elements after it to the left.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds,
 * `NOMEM` if fails to allocate menory or `OK` otherwise.
 */
extern vec_err_t
vec_remove(vec_t *vec, size_t idx, int32_t* removed);

/*
 * Swaps the elements of `vec` at `idx1` at and `idx2`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_swap(vec_t *vec, size_t idx1, size_t idx2);

/*
 * Sets the parameter `val` with the element at `idx` of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_get(const vec_t *vec, size_t idx, int32_t* val);

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
extern vec_err_t
vec_len(const vec_t *vec, size_t* len);

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
extern size_t
vec_capacity(const vec_t* vec, size_t* cap);

/*
 * [Returns] 1 if `vec` is empty or 0 otherwise.
 */
extern int32_t
vec_is_empty(const vec_t* vec);

/*
 * Prints the formatted contents of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
extern vec_err_t
vec_display(const vec_t* vec);

/*
 * [Returns] the printable version of a
 * `vec_err_t` or NULL if `err`
 * doesn't belog to the enum.
 */
extern const char*
vec_get_err_msg(vec_err_t err);

/*
 * Prints the formatted contents of the memory allocated by `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
extern vec_err_t
_vec_debug(const vec_t* vec);


#endif /* defined(VEC_H) */
