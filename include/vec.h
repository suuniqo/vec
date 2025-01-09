
#if !defined(VEC_H)
#define VEC_H


/* ========= INCLUDES ========== */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


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
    VEC_ERR_OK = 0,                             /* operation completed succesfully */
    VEC_ERR_IOOB,                               /* index was out of bounds */
    VEC_ERR_INVARG,                             /* pointer to vector was NULL */
    VEC_ERR_INVOP,                              /* can't perform operation */
    VEC_ERR_NOMEM,                              /* failed to allocate memory */
    VEC_ERR_COUNT,                              /* number of errors in the enum */
} vec_err_t;


/* ========== METHODS ========== */

/* === Memory Management === */

/*
 * Creates a new instance of vec_t, allocating memory for 
 * `capacity` (or default value if negative) elements of size `elem_size`
 *
 * [Returns] `INVARG` if element size is smaller than one byte or `vec` is NULL,
 * NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_make(vec_t** vec, size_t elem_size, size_t capacity);

/*
 * Resizes the vector with `capacity` memory
 *
 * [Returns] `NOMEM` if fails to allocate memory,
 * `INVARG` if the vector is NULL, `INVOP` if capacity isn't 
 * in the valid range or `OK` otherwise.
 */
extern vec_err_t
vec_resize(vec_t* vec, size_t capacity);

/*
 * Reallocates the vector, reserving the exact amount of
 * memory needed to hold it's current contents
 *
 * [Returns] `INVARG` if the vector is NULL, `INVOP` if it's empty,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_shrink_to_fit(vec_t* vec);

/*
 * Resets the vector, reallocating
 * and deleting it's contents if enough memory.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_clear(vec_t* vec);

/*
 * If `dest` is NULL makes a new vector containing
 * a copy of the elements of `src` in `dest`
 * Else `dest` is reallocated if necessary and the
 * elements of `src` are copied into it.
 *
 * [Returns] `INVARG` if the `src` or `dest` are NULL,
 * `NOMEM` if fails to allocate memory `OK` otherwise.
 */
extern vec_err_t
vec_clone(vec_t* src, vec_t** dest);

/*
 * Destroys the instance of vec_t.
 */
extern void
vec_destroy(vec_t** vec);



/* === Write Operations === */

/* == In-Place == */

/*
 * Sets the element at `idx` of `vec` with the value of `src`
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_set(vec_t* vec, size_t idx, const void* src);

/*
 * Substitutes with `val` the element of `vec` at `idx`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_replace(vec_t* vec, size_t idx, const void* val, void* old_val);

/*
 * Swaps the elements of `vec` at `idx1` at and `idx2`.
 *
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_swap(vec_t* vec, size_t idx1, size_t idx2);


/* == Not In-Place == */

/*
 * Inserts `val` at the position `idx`,
 * shifting all elements after it to the right.
 * It allows inserting at the end of `vec`.
 *
 * If it has no space left it reallocates,
 * doubling it's size
 *
 * [Returns] `INVARG` if the vector is NULL,
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
 * [Returns] `INVARG` if the vector is NULL,
 * `IOOB` if the index is out of bounds,
 * `NOMEM` if fails to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_remove(vec_t *vec, size_t idx, void* removed);

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
 * [Returns] `INVARG` if the vector is NULL 
 * `INVOP` if the vector is empty, `NOMEM` if fails
 * to allocate memory or `OK` otherwise.
 */
extern vec_err_t
vec_pop(vec_t* vec, void* popped);



/* === Read Operations === */

/*
 * Sets the parameter `dest` with the value of `vec` at `idx`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 * `IOOB` if any index is out of bounds or `OK` otherwise.
 */
extern vec_err_t
vec_get(const vec_t* vec, size_t idx, void* dest);

/*
 * Sets `first` to the first element of `vec`.
 *
 * [Returns] `IOOB` if empty or `OK` otherwise
 */
extern vec_err_t
vec_first(const vec_t* vec, void* first);

/*
 * Sets `last` to the last element of `vec`.
 *
 * [Returns] `IOOB` if empty or `OK` otherwise
 */
extern vec_err_t
vec_last(const vec_t* vec, void* last);

/*
 * Sets `len` to the length of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
extern vec_err_t
vec_len(const vec_t* vec, size_t* len);

/*
 * Sets `cap` to the capacity of `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL, or `OK` otherwise.
 */
extern vec_err_t
vec_capacity(const vec_t* vec, size_t* cap);

/*
 * Sets `space` to the space left in `vec`.
 *
 * [Returns] `INVARG` if any of the parameters are NULL,
 */
extern vec_err_t
vec_space(const vec_t* vec, size_t* space);

/*
 * [Returns] 1 if `vec` is empty or 0 otherwise.
 */
extern bool
vec_is_empty(const vec_t* vec);

/*
 * Prints the formatted contents of `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
extern vec_err_t
vec_display(const vec_t* vec);

/*
 * Prints the formatted contents of the memory allocated by `vec`.
 *
 * [Returns] `INVARG` if the vector is NULL and `OK` otherwise.
 */
extern vec_err_t
_vec_debug(const vec_t* vec);



/* === Error Handling === */

/*
 * [Returns] the printable version of a
 * `vec_err_t` or NULL if `err`
 * doesn't belog to the enum.
 */
extern const char*
vec_get_err_msg(vec_err_t err);


#endif /* defined(VEC_H) */
