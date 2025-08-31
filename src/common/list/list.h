#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  LIST_FAILURE        = 1,
  LIST_MALLOC_FAILURE = 2,

  LIST_SUCCESS        = (1 << 8),
  LIST_VALUE_INSERTED = (1 << 9),
  LIST_VALUE_REMOVED  = (1 << 10)

} list_status_t;

typedef struct list_t list_t;

typedef void* list_value_t;

typedef list_value_t (*list_value_copy_func_t)(const void * value);

typedef void (*list_value_free_func_t)(list_value_t value);

typedef int (*list_value_cmp_func_t)(const void * a, const void * b);

typedef struct list_iter_t list_iter_t;

list_t * list_create(
  size_t                 value_size,
  list_value_copy_func_t value_copy_func,
  list_value_free_func_t value_free_func
);

list_t * list_copy(const list_t * other);

void list_destroy(list_t * list);

list_value_t list_front(const list_t * list);
list_value_t list_back(const list_t * list);

list_status_t list_push_front(list_t * list, const void * value);
list_status_t list_push_back(list_t * list, const void * value);

void list_pop_front(list_t * list);
void list_pop_back(list_t * list);

int list_empty(const list_t * const list);

size_t list_size(const list_t * const list);

list_status_t list_insert_after_iter(list_t * list, list_iter_t * it, const void * value);
list_status_t list_insert_before_iter(list_t * list, list_iter_t * it, const void * value);

list_status_t list_remove(list_t * list, list_iter_t * it);

list_iter_t * list_begin(const list_t * list);
list_iter_t * list_end(const list_t * list);

list_value_t list_iter_value(const list_iter_t * it);

list_iter_t * list_iter_inc(list_iter_t * it);
list_iter_t * list_iter_dec(list_iter_t * it);

int list_iter_cmp(const list_iter_t * const a, const list_iter_t * const b);

void list_iter_destroy(list_iter_t * it);

#endif // __LIST_H__
