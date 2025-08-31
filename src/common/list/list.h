#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Status codes returned by list operations
 * 
 * These codes indicate the success or failure of list operations
 * and provide additional information about what happened.
 */
typedef enum
{
  LIST_FAILURE        = 1,    /**< General failure */
  LIST_MALLOC_FAILURE = 2,    /**< Memory allocation failed */

  LIST_SUCCESS        = (1 << 8), /**< Operation completed successfully */
  LIST_VALUE_INSERTED = (1 << 9), /**< New value was inserted */
  LIST_VALUE_REMOVED  = (1 << 10), /**< Value was removed */
  LIST_VALUE_UPDATED  = (1 << 11)  /**< Value was updated */

} list_status_t;

typedef struct list_t list_t;

typedef void* list_value_t;

/**
 * @brief Function pointer type for copying values
 * @param value Pointer to the value to copy
 * @return Pointer to the copied value, or NULL on failure
 */
typedef list_value_t (*list_value_copy_func_t)(const void * value);

/**
 * @brief Function pointer type for freeing values
 * @param value Pointer to the value to free
 */
typedef void (*list_value_free_func_t)(list_value_t value);

/**
 * @brief Function pointer type for comparing values
 * @param a Pointer to first value
 * @param b Pointer to second value
 * @return Shoudl return a negative number if a < b, 0 if a == b, a positive numberif a > b
 */
typedef int (*list_value_cmp_func_t)(const void * a, const void * b);

typedef struct list_iter_t list_iter_t;

/**
 * @brief Creates a new empty list
 * 
 * @param value_size Size of each value in bytes
 * @param value_copy_func Function to copy values (can be NULL for POD types)
 * @param value_free_func Function to free values (can be NULL, defaults to free())
 * @return Pointer to the created list, or NULL on failure
 * 
 * @note The list is initialized as empty. If value_copy_func is NULL,
 *       a default copy function using malloc and memcpy will be used.
 */
list_t * list_create(
  size_t                 value_size,
  list_value_copy_func_t value_copy_func,
  list_value_free_func_t value_free_func
);

/**
 * @brief Creates a deep copy of an existing list
 * 
 * @param other Pointer to the list to copy
 * @return Pointer to the copied list, or NULL on failure
 * 
 * @note All values in the new list are copied using the original list's copy function
 */
list_t * list_copy(const list_t * other);

/**
 * @brief Destroys a list and frees all associated memory
 * 
 * @param list Pointer to the list to destroy
 * 
 * @note This function frees all nodes and their values, then frees the list structure
 */
void list_destroy(list_t * list);

/**
 * @brief Returns the value at the front of the list
 * 
 * @param list Pointer to the list
 * @return Pointer to the front value, or NULL if list is empty
 */
list_value_t list_front(const list_t * list);

/**
 * @brief Returns the value at the back of the list
 * 
 * @param list Pointer to the list
 * @return Pointer to the back value, or NULL if list is empty
 */
list_value_t list_back(const list_t * list);

/**
 * @brief Adds a value to the front of the list
 * 
 * @param list Pointer to the list
 * @param value Pointer to the value to add
 * @return LIST_SUCCESS | LIST_VALUE_INSERTED on success, LIST_FAILURE on failure
 * 
 * @note The value is copied using the list's copy function
 */
list_status_t list_push_front(list_t * list, const void * value);

/**
 * @brief Adds a value to the back of the list
 * 
 * @param list Pointer to the list
 * @param value Pointer to the value to add
 * @return LIST_SUCCESS | LIST_VALUE_INSERTED on success, LIST_FAILURE on failure
 * 
 * @note The value is copied using the list's copy function
 */
list_status_t list_push_back(list_t * list, const void * value);

/**
 * @brief Removes the value at the front of the list
 * 
 * @param list Pointer to the list
 * 
 * @note The removed value is freed using the list's free function
 */
void list_pop_front(list_t * list);

/**
 * @brief Removes the value at the back of the list
 * 
 * @param list Pointer to the list
 * 
 * @note The removed value is freed using the list's free function
 */
void list_pop_back(list_t * list);

/**
 * @brief Checks if the list is empty
 * 
 * @param list Pointer to the list
 * @return 1 if empty, 0 if not empty
 */
int list_empty(const list_t * const list);

/**
 * @brief Returns the number of elements in the list
 * 
 * @param list Pointer to the list
 * @return Number of elements
 */
size_t list_size(const list_t * const list);

/**
 * @brief Updates the value at the position pointed by iterator
 * 
 * @param list Pointer to the list
 * @param it Iterator pointing to the element to update
 * @param value New value to set
 * @return LIST_SUCCESS | LIST_VALUE_UPDATED on success, LIST_FAILURE on failure
 * 
 * @note The old value is freed using the list's free function
 */
list_status_t list_update_value(list_t * list, list_iter_t * it, const void * value);

/**
 * @brief Inserts a value after the position pointed by iterator
 * 
 * @param list Pointer to the list
 * @param it Iterator pointing to the position
 * @param value Value to insert
 * @return LIST_SUCCESS | LIST_VALUE_INSERTED on success, LIST_FAILURE on failure
 * 
 * @note The new element becomes the successor of the element pointed by iterator
 */
list_status_t list_insert_after_iter(list_t * list, list_iter_t * it, const void * value);

/**
 * @brief Inserts a value before the position pointed by iterator
 * 
 * @param list Pointer to the list
 * @param it Iterator pointing to the position
 * @param value Value to insert
 * @return LIST_SUCCESS | LIST_VALUE_INSERTED on success, LIST_FAILURE on failure
 * 
 * @note The new element becomes the predecessor of the element pointed by iterator
 */
list_status_t list_insert_before_iter(list_t * list, list_iter_t * it, const void * value);

/**
 * @brief Removes the element pointed by iterator
 * 
 * @param list Pointer to the list
 * @param it Iterator pointing to the element to remove
 * @return LIST_SUCCESS | LIST_VALUE_REMOVED on success, LIST_FAILURE on failure
 * 
 * @note The iterator becomes invalid after removal and should not be used
 */
list_status_t list_remove(list_t * list, list_iter_t * it);

/**
 * @brief Returns an iterator pointing to the first element
 * 
 * @param list Pointer to the list
 * @return Iterator pointing to the first element, or NULL if list is empty
 * 
 * @note The returned iterator must be freed with list_iter_destroy
 */
list_iter_t * list_begin(const list_t * list);

/**
 * @brief Returns an iterator pointing past the last element
 * 
 * @param list Pointer to the list
 * @return Iterator pointing past the last element, or NULL on error
 * 
 * @note This iterator cannot be dereferenced, it's used for iteration bounds
 */
list_iter_t * list_end(const list_t * list);

/**
 * @brief Returns the value pointed by iterator
 * 
 * @param it Iterator
 * @return Pointer to the value, or NULL if iterator is invalid
 */
list_value_t list_iter_value(const list_iter_t * it);

/**
 * @brief Advances iterator to the next element
 * 
 * @param it Iterator to advance
 * @return The same iterator (for chaining), or NULL if iterator is invalid
 * 
 * @note If iterator is at the end, it remains unchanged
 */
list_iter_t * list_iter_inc(list_iter_t * it);

/**
 * @brief Moves iterator to the previous element
 * 
 * @param it Iterator to move
 * @return The same iterator (for chaining), or NULL if iterator is invalid
 * 
 * @note If iterator is at the beginning, it remains unchanged
 */
list_iter_t * list_iter_dec(list_iter_t * it);

/**
 * @brief Compares two iterators
 * 
 * @param a First iterator
 * @param b Second iterator
 * @return Negative if a < b, 0 if a == b, positive if a > b
 * 
 * @note NULL iterators are considered less than valid iterators
 */
int list_iter_cmp(const list_iter_t * const a, const list_iter_t * const b);

/**
 * @brief Destroys an iterator and frees its memory
 * 
 * @param it Iterator to destroy
 * 
 * @note This function frees the iterator structure but not the list
 */
void list_iter_destroy(list_iter_t * it);

#endif // __LIST_H__
