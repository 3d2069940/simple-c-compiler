#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hash_table/hash_table.h"


struct complex_struct;

typedef struct complex_struct
{
  struct complex_struct * m_ptr;

  int m_a;

} complex_struct;

static void complex_struct_destroy(void * ptr)
{
  if (ptr == NULL)
    return;

  complex_struct * s = ptr;
  complex_struct * current_ptr;

  while (s != NULL)
  {
    current_ptr = s->m_ptr;

    free(s);

    s = current_ptr;
  }
}

static complex_struct * complex_struct_create(size_t count, ...)
{
  if (count == 0)
    return NULL;

  va_list args;
  va_start(args, count);

  complex_struct * s = malloc(sizeof(complex_struct));

  if (s == NULL)
    return NULL;

  complex_struct * current_ptr = s;

  for (size_t i = 0; i < count; ++i)
  {
    int arg = va_arg(args, int);
    current_ptr->m_a = arg;

    if (i + 1 == count)
    {
      current_ptr->m_ptr = NULL;
      break;
    }

    current_ptr->m_ptr = malloc(sizeof(complex_struct));
    current_ptr = current_ptr->m_ptr;

    if (current_ptr == NULL)
    {
      complex_struct_destroy(s);
      return NULL;
    }
  }
  va_end(args);
  return s;
}

static void * complex_struct_copy(const void * ptr)
{
  const complex_struct * other = ptr;
  if (other == NULL)
    return NULL;

  complex_struct * copy = malloc(sizeof(complex_struct));

  if (copy == NULL)
    return NULL;

  complex_struct * current_ptr = copy;

  while (other != NULL)
  {
    current_ptr->m_a = other->m_a;

    if (other->m_ptr == NULL)
      break;

    current_ptr->m_ptr = malloc(sizeof(complex_struct));
    current_ptr        = current_ptr->m_ptr;

    other = other->m_ptr;
  }
  current_ptr->m_ptr = NULL;
  return copy;
}

static void init_deinit_test(void)
{
  hash_table_size_params_t size_params = {.m_key_size=4, .m_value_size=4, .m_bucket_capacity=16};
  hash_table_t * hash_table = hash_table_create(size_params, NULL, NULL, NULL, NULL, NULL, NULL);

  hash_table_destroy(hash_table);
}

static void insert_test(void)
{
  hash_table_size_params_t size_params = {.m_key_size=4, .m_value_size=4, .m_bucket_capacity=16};
  hash_table_t * hash_table = hash_table_create(size_params, NULL, NULL, NULL, NULL, NULL, NULL);

  int key   = 5;
  int value = 5;

  assert(hash_table_insert(hash_table, &key, &value) >= HASH_TABLE_SUCCESS);

  int * table_value = hash_table_at(hash_table, &key);

  assert(table_value != NULL);
  assert(*table_value == value);

  hash_table_destroy(hash_table);
}

static void init_copy_test(void)
{
  hash_table_size_params_t size_params = {.m_key_size=4, .m_value_size=4, .m_bucket_capacity=16};
  hash_table_t * table = hash_table_create(size_params, NULL, NULL, NULL, NULL, NULL, NULL);

  int key   = 5;
  int value = 5;

  assert(hash_table_insert(table, &key, &value) >= HASH_TABLE_SUCCESS);

  int * table_value = hash_table_at(table, &key);

  hash_table_t * table_copy = hash_table_copy(table);

  int * table_copy_value = hash_table_at(table, &key);

  assert(table_value != NULL);
  assert(table_copy_value != NULL);
  assert(*table_value == * table_copy_value);

  hash_table_destroy(table);
  hash_table_destroy(table_copy);
}

static void insert_pod_test(void)
{
  typedef struct
  {
    char  m_a;
    int   m_b;
    float m_c;
  } A;

  hash_table_size_params_t size_params = {.m_key_size=sizeof(int), .m_value_size=sizeof(A), .m_bucket_capacity=16};
  hash_table_t * table = hash_table_create(size_params, NULL, NULL, NULL, NULL, NULL, NULL);

  int key   = 5;
  A   value = {.m_a = '5', .m_b=5, .m_c=5.0};

  assert(hash_table_insert(table, &key, &value) >= HASH_TABLE_SUCCESS);

  A * table_value = hash_table_at(table, &key);

  assert(table_value != NULL);
  assert(table_value->m_a == (char)'5');
  assert(table_value->m_b == 5);
  assert(table_value->m_c == 5.0);

  hash_table_destroy(table);
}

static void insert_complex_struct_test(void)
{
  complex_struct * ptr = complex_struct_create(5, 1, 2, 3, 4, 5);

  hash_table_size_params_t params = {.m_key_size=sizeof(int), .m_value_size=sizeof(complex_struct), .m_bucket_capacity=16};
  hash_table_t           * table  = hash_table_create(
    params,
    NULL,
    NULL,
    NULL,
    NULL,
    complex_struct_copy,
    complex_struct_destroy
  );
  int key = 5;

  assert(hash_table_insert(table, &key, ptr) >= HASH_TABLE_SUCCESS);

  complex_struct_destroy(ptr);

  complex_struct * table_value = hash_table_at(table, &key);

  assert(table_value->m_a == 1);
  table_value = table_value->m_ptr;
  assert(table_value->m_a == 2);
  table_value = table_value->m_ptr;
  assert(table_value->m_a == 3);
  table_value = table_value->m_ptr;
  assert(table_value->m_a == 4);
  table_value = table_value->m_ptr;
  assert(table_value->m_a == 5);

  hash_table_destroy(table);
}

static size_t hash_func(const void * key)
{
  (void)key;
  return 0;
}

static void insert_with_same_hash(void)
{
  hash_table_size_params_t params = {.m_key_size=sizeof(int), .m_value_size=sizeof(int), .m_bucket_capacity=16};
  hash_table_t           * table  = hash_table_create(
    params, 
    NULL, 
    hash_func, 
    NULL, 
    NULL, 
    NULL, 
    NULL
  );

  int key_a = 5;
  int value_a = 1;

  int key_b   = 6;
  int value_b = 2;

  assert(hash_table_insert(table, &key_a, &value_a) >= HASH_TABLE_SUCCESS);
  assert(hash_table_insert(table, &key_b, &value_b) >= HASH_TABLE_SUCCESS);

  int * table_value_a = hash_table_at(table, &key_a);
  int * table_value_b = hash_table_at(table, &key_b);

  assert(table_value_a != NULL);
  assert(table_value_b != NULL);

  assert(*table_value_a == 1);
  assert(*table_value_b == 2);

  hash_table_destroy(table);
}

static void remove_value(void)
{
  complex_struct * ptr = complex_struct_create(5, 1, 2, 3, 4, 5);

  hash_table_size_params_t params = {.m_key_size=sizeof(int), .m_value_size=sizeof(complex_struct), .m_bucket_capacity=16};
  hash_table_t           * table  = hash_table_create(
    params,
    NULL,
    NULL,
    NULL,
    NULL,
    complex_struct_copy,
    complex_struct_destroy
  );
  assert(table != NULL);
  
  int key = 5;

  assert(hash_table_insert(table, &key, ptr) >= HASH_TABLE_SUCCESS);

  complex_struct_destroy(ptr);
  {
    complex_struct * table_value = hash_table_at(table, &key);
    assert(table_value != NULL);
  }
  assert(hash_table_remove(table, &key) >= HASH_TABLE_SUCCESS);
  {
    complex_struct * table_value = hash_table_at(table, &key);
    assert(table_value == NULL);
  }
  hash_table_destroy(table);
}

int main(void)
{
  printf("Testing is in progress\n");
  init_deinit_test();
  insert_test();
  init_copy_test();
  insert_pod_test();
  insert_complex_struct_test();
  insert_with_same_hash();
  remove_value();
  printf("Tests completed successfully\n");
}