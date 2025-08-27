#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  HASH_TABLE_FAILURE        = 1,
  HASH_TABLE_MALLOC_FAILURE = 2,
  HASH_TABLE_NO_VALUE_FOUND = 4,

  HASH_TABLE_SUCCESS            = (1 << 8),
  HASH_TABLE_VALUE_UPDATED      = (1 << 9),
  HASH_TABLE_NEW_VALUE_INSERTED = (1 << 10),
  HASH_TABLE_VALUE_REMOVED      = (1 << 11)

} hash_table_status_t;

typedef struct
{
  size_t m_key_size;
  size_t m_value_size;
  size_t m_bucket_capacity;

} hash_table_size_params_t;

typedef struct hash_table_t hash_table_t;

typedef void* hash_table_key_t;
typedef void* hash_table_value_t;

typedef hash_table_key_t (*hash_table_key_copy_func_t)(const void * key);
typedef hash_table_value_t (*hash_table_value_copy_func_t)(const void * value);

typedef void (*hash_table_key_free_func_t)(hash_table_key_t key);
typedef void (*hash_table_value_free_func_t)(hash_table_value_t value);

typedef size_t (*hash_table_hash_func_t)(const void * key);
typedef int    (*hash_table_key_cmp_func_t)(const void * a, const void * b);

typedef struct hash_table_iter_t hash_table_iter_t;

hash_table_t * hash_table_create(
  hash_table_size_params_t     size_params,
  hash_table_key_cmp_func_t    key_cmp_func,
  hash_table_hash_func_t       hash_func,
  hash_table_key_copy_func_t   key_copy_func,
  hash_table_key_free_func_t   key_free_func,
  hash_table_value_copy_func_t value_copy_func,
  hash_table_value_free_func_t value_free_func
);

hash_table_t * hash_table_copy(const hash_table_t * other);

void hash_table_destroy(hash_table_t * table);

hash_table_status_t hash_table_insert(hash_table_t * table, const hash_table_key_t key, const hash_table_value_t value);

hash_table_value_t hash_table_at(const hash_table_t * table, const hash_table_key_t key);

hash_table_status_t hash_table_remove(hash_table_t * table, const hash_table_key_t key);

int hash_table_contains(const hash_table_t * table, const hash_table_key_t key);

size_t hash_table_size(const hash_table_t * table);

void hash_table_clear(hash_table_t * table);

#endif // __HASH_TABLE_H__