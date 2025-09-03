
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "list.h"

#define MALLOC(__TYPE__) (__TYPE__*)malloc(sizeof(__TYPE__))
#define CALLOC(__SIZE__, __TYPE__) (__TYPE__*)calloc(__SIZE__, sizeof(__TYPE__))

typedef struct list_node_t list_node_t;

struct list_node_t
{
  list_value_t  m_value;
  list_node_t * m_next;
  list_node_t * m_previous;
};

struct list_t
{
  size_t                 m_value_size;
  list_value_copy_func_t m_value_copy_func;
  list_value_free_func_t m_value_free_func;

  size_t        m_length;
  list_node_t * m_first_node;
  list_node_t * m_last_node;
};

struct list_iter_t
{
  list_node_t * m_node;
};

static void * copy_func_default(const void * const value, size_t size)
{
  void * copy = malloc(size);

  if (copy == NULL)
    return NULL;

  memcpy(copy, value, size);

  return copy;
}

static list_value_t value_copy(const list_t * const list, const void * const value)
{
  return list->m_value_copy_func ?
         list->m_value_copy_func(value) :
         copy_func_default(value, list->m_value_size);
}

static list_node_t * node_create(const list_t * const list, const void * const value)
{
  if (list == NULL)
    return NULL;

  list_node_t * new_node = MALLOC(list_node_t);

  if (new_node == NULL)
    return NULL;

  new_node->m_next     = NULL;
  new_node->m_previous = NULL;

  if (value == NULL)
  {
    new_node->m_value = NULL;
    return new_node;
  }
  new_node->m_value = value_copy(list, value);

  if (new_node->m_value == NULL)
  {
    free(new_node);
    return NULL;
  }
  return new_node;
}

static void node_destroy(const list_t * list, list_node_t * node)
{
  if (list == NULL || node == NULL)
    return;

  list->m_value_free_func(node->m_value);
  free(node);
}

static list_status_t list_init_with_one_node(list_t * list, const void * const value)
{
  list_node_t * new_node = node_create(list, value);

  if (new_node == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  new_node->m_next     = NULL;
  new_node->m_previous = NULL;

  list->m_first_node = new_node;
  list->m_last_node  = new_node;

  ++list->m_length;

  return LIST_SUCCESS | LIST_VALUE_INSERTED;
}

static list_iter_t * iter_create(list_node_t * node)
{
  list_iter_t * it = MALLOC(list_iter_t);

  if (it == NULL)
    return NULL;

  it->m_node = node;

  return it;
}

list_t * list_create(
  size_t                 value_size,
  list_value_copy_func_t value_copy_func,
  list_value_free_func_t value_free_func
)
{
  list_t * new_list = MALLOC(list_t);

  if (new_list == NULL)
    return NULL;

  new_list->m_length          = 0;
  new_list->m_value_size      = value_size;
  new_list->m_value_copy_func = value_copy_func;
  new_list->m_value_free_func = value_free_func ? value_free_func : free;
  new_list->m_first_node      = NULL;
  new_list->m_last_node       = NULL;

  return new_list;
}

list_t * list_copy(const list_t * other)
{
  if (other == NULL)
    return NULL;

  list_t * copy = list_create(
    other->m_value_size,
    other->m_value_copy_func,
    other->m_value_free_func
  );

  if (copy == NULL)
    return NULL;

  list_iter_t * it = list_begin(other);
  list_iter_t * end = list_end(other);

  while (list_iter_cmp(it, end))
  {
    list_status_t status = list_push_back(copy, list_iter_value(it));

    if (status < LIST_SUCCESS)
    {
      list_iter_destroy(it);
      list_iter_destroy(end);
      list_destroy(copy);
      return NULL;
    }
    list_iter_inc(&it);
  }
  list_iter_destroy(it);
  list_iter_destroy(end);
  return copy;
}

void list_destroy(list_t * list)
{
  if (list == NULL)
    return;

  list_node_t * current_node = list->m_first_node;
  list_node_t * next_node    = NULL;

  while (current_node != NULL)
  {
    next_node = current_node->m_next;
    node_destroy(list, current_node);
    current_node = next_node;
  }
  list->m_length = 0;
  
  free(list);
}

list_value_t list_front(const list_t * const list)
{
  if (list == NULL)
    return NULL;

  if (list->m_first_node == NULL)
    return NULL;

  return list->m_first_node->m_value;
}

list_value_t list_back(const list_t * const list)
{
  if (list == NULL)
    return NULL;

  if (list->m_last_node == NULL)
    return NULL;

  return list->m_last_node->m_value;
}

list_status_t list_push_front(list_t * list, const void * value)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (list->m_length == 0)
    return list_init_with_one_node(list, value);

  list_node_t * new_node = node_create(list, value);

  if (new_node == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  if (list->m_length == 1)
  {
    new_node->m_next = list->m_last_node;

    list->m_last_node->m_previous = new_node;

    list->m_first_node = new_node;

    ++list->m_length;

    return LIST_SUCCESS | LIST_VALUE_INSERTED;
  }
  new_node->m_next = list->m_first_node;

  list->m_first_node->m_previous = new_node;

  list->m_first_node = new_node;

  ++list->m_length;

  return LIST_SUCCESS | LIST_VALUE_INSERTED;
}

list_status_t list_push_back(list_t * list, const void * value)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (list->m_length == 0)
    return list_init_with_one_node(list, value);

  list_node_t * new_node = node_create(list, value);

  if (new_node == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  if (list->m_length == 1)
  {
    new_node->m_previous = list->m_first_node;

    list->m_first_node->m_next = new_node;

    list->m_last_node = new_node;

    ++list->m_length;

    return LIST_SUCCESS | LIST_VALUE_INSERTED;
  }
  new_node->m_previous = list->m_last_node;

  list->m_last_node->m_next = new_node;

  list->m_last_node = new_node;

  ++list->m_length;

  return LIST_SUCCESS | LIST_VALUE_INSERTED;
}

void list_pop_front(list_t * list)
{
  if (list == NULL)
    return;

  if (list->m_length == 0)
    return;

  if (list->m_length == 1)
  {
    node_destroy(list, list->m_first_node);

    list->m_first_node = NULL;
    list->m_last_node  = NULL;

    --list->m_length;

    return;
  }
  list_node_t * next_node = list->m_first_node->m_next;

  next_node->m_previous = NULL;

  node_destroy(list, list->m_first_node);

  list->m_first_node = next_node;

  --list->m_length;
}

void list_pop_back(list_t * list)
{
  if (list == NULL)
    return;

  if (list->m_length == 0)
    return;

  if (list->m_length == 1)
  {
    node_destroy(list, list->m_first_node);

    list->m_first_node = NULL;
    list->m_last_node  = NULL;

    --list->m_length;

    return;
  }
  list_node_t * previous_node = list->m_last_node->m_previous;

  previous_node->m_next = NULL;

  node_destroy(list, list->m_last_node);

  list->m_last_node = previous_node;

  --list->m_length;
}

int list_empty(const list_t * const list)
{
  return list->m_length == 0;
}

size_t list_size(const list_t * const list)
{
  return list->m_length;
}

list_status_t list_update_value(list_t * list, list_iter_t * it, const void * value)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (it == NULL)
    return LIST_FAILURE;

  if (it->m_node == NULL)
    return LIST_FAILURE;

  if (value == NULL)
    return LIST_FAILURE;

  list_node_t * current_node = it->m_node;

  list->m_value_free_func(current_node->m_value);

  current_node->m_value = list->m_value_copy_func ?
                          list->m_value_copy_func(value) :
                          value_copy(list, value);

  if (current_node->m_value == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  return LIST_SUCCESS | LIST_VALUE_UPDATED;
}

list_status_t list_insert_after_iter(list_t * list, list_iter_t * it, const void * value)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (it == NULL)
    return LIST_FAILURE;

  if (it->m_node == NULL)
    return LIST_FAILURE;

  if (value == NULL)
    return LIST_FAILURE;

  list_node_t * new_node     = node_create(list, value);
  list_node_t * current_node = it->m_node;

  if (new_node == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  new_node->m_previous = current_node;

  if (current_node == list->m_last_node)
  {
    current_node->m_next = new_node;
    list->m_last_node    = new_node;

    ++list->m_length;

    return LIST_SUCCESS | LIST_VALUE_INSERTED;
  }
  current_node->m_next->m_previous = new_node;

  new_node->m_next     = current_node->m_next;
  current_node->m_next = new_node;

  ++list->m_length;

  return LIST_SUCCESS | LIST_VALUE_INSERTED;
}

list_status_t list_insert_before_iter(list_t * list, list_iter_t * it, const void * value)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (it == NULL)
    return LIST_FAILURE;

  if (it->m_node == NULL)
    return LIST_FAILURE;

  list_node_t * new_node = node_create(list, value);
  list_node_t * current_node = it->m_node;

  if (new_node == NULL)
    return LIST_FAILURE | LIST_MALLOC_FAILURE;

  new_node->m_next = current_node;

  if (current_node == list->m_first_node)
  {
    current_node->m_previous = new_node;
    list->m_first_node       = new_node;

    ++list->m_length;

    return LIST_SUCCESS | LIST_VALUE_INSERTED;
  }
  current_node->m_previous->m_next = new_node;

  new_node->m_previous = current_node->m_previous;
  current_node->m_previous = new_node;

  ++list->m_length;

  return LIST_SUCCESS | LIST_VALUE_INSERTED;
}

list_status_t list_remove(list_t * list, list_iter_t * it)
{
  if (list == NULL)
    return LIST_FAILURE;

  if (it == NULL)
    return LIST_FAILURE;

  if (it->m_node == NULL)
    return LIST_FAILURE;

  list_node_t * node_to_remove = it->m_node;
  list_node_t * previous_node = node_to_remove->m_previous;
  list_node_t * next_node     = node_to_remove->m_next;

  if (node_to_remove == list->m_first_node)
    list->m_first_node = next_node;

  if (node_to_remove == list->m_last_node)
    list->m_last_node = previous_node;

  if (previous_node != NULL)
    previous_node->m_next = next_node;

  if (next_node != NULL)
    next_node->m_previous = previous_node;

  it->m_node = NULL;

  node_destroy(list, node_to_remove);

  --list->m_length;

  return LIST_SUCCESS | LIST_VALUE_REMOVED;
}

list_iter_t * list_begin(const list_t * list)
{
  if (list == NULL)
    return NULL;

  if (list->m_first_node == NULL)
    return NULL;

  return iter_create(list->m_first_node);
}

list_iter_t * list_end(const list_t * list)
{
  if (list == NULL)
    return NULL;

  if (list->m_last_node == NULL)
    return NULL;

  return iter_create(list->m_last_node->m_next);
}

list_value_t list_iter_value(const list_iter_t * it)
{
  if (it == NULL)
    return NULL;

  if (it->m_node == NULL)
    return NULL;

  return it->m_node->m_value;
}

void list_iter_inc(list_iter_t ** it)
{
  if (*it == NULL)
    return;

  if ((*it)->m_node == NULL)
    return;

  (*it)->m_node = (*it)->m_node->m_next;
}

void list_iter_dec(list_iter_t ** it)
{
  if (it == NULL)
    return;

  if ((*it)->m_node == NULL)
    return;

  (*it)->m_node = (*it)->m_node->m_previous;
}

int list_iter_cmp(const list_iter_t * const a, const list_iter_t * const b)
{
  int cmp_a = (a == NULL);
  int cmp_b = (b == NULL);

  if (cmp_a || cmp_b)
    return cmp_b - cmp_a;

  cmp_a = (a->m_node == NULL);
  cmp_b = (b->m_node == NULL);

  if (cmp_a || cmp_b)
    return cmp_b - cmp_a;

  if (a->m_node == b->m_node)
    return 0;

  // Note: there is no way to compare iterator indices.
  return 1;
}

void list_iter_destroy(list_iter_t * it)
{
  if (it == NULL)
    return;

  free(it);
}

void print(const list_t * list, void (*print_element)(const void *))
{
  assert(list != NULL);

  size_t        length = list->m_length;
  list_node_t * node   = list->m_first_node;

  while (length > 0)
  {
    print_element(node->m_value);
    node = node->m_next;
    --length;
  }
}

void print_back(const list_t * list, void (*print_element)(const void *))
{
  assert(list != NULL);

  size_t        length = list->m_length;
  list_node_t * node   = list->m_last_node;

  while (length > 0)
  {
    print_element(node->m_value);
    node = node->m_previous;
    --length;
  }
}
