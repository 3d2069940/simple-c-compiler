#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "hash_table.h"
#include "utility/utility.h"

#define MALLOC(__TYPE__) (__TYPE__*)malloc(sizeof(__TYPE__))
#define CALLOC(__SIZE__, __TYPE__) (__TYPE__*)calloc(__SIZE__, sizeof(__TYPE__))

#define ELEMENT_AT(__ARR__, __TYPE__, __ID__) ((__TYPE__*)__ARR__)[__ID__]
#define BYTE_AT(__ARR__, __ID__) ((unsigned char*)(__ARR__))[(__ID__)]

typedef struct hash_table_node_t hash_table_node_t;

struct hash_table_node_t
{
  hash_table_key_t   * m_key;
  hash_table_value_t * m_value;
  hash_table_node_t  * m_next;
};

typedef struct hash_table_bucket_t
{
  size_t              m_size;
  hash_table_node_t * m_start_node;
} hash_table_bucket_t;

struct hash_table_t
{
  hash_table_size_params_t     m_size_params;
  hash_table_bucket_t        * m_buckets;
  hash_table_hash_func_t       m_hash_func;
  hash_table_key_cmp_func_t    m_key_cmp_func;
  hash_table_key_copy_func_t   m_key_copy_func;
  hash_table_key_free_func_t   m_key_free_func;
  hash_table_value_copy_func_t m_value_copy_func;
  hash_table_value_free_func_t m_value_free_func;
  size_t                       m_size;
};

struct hash_table_iter_t
{
  hash_table_t * m_table;
  size_t         m_bucket_id;
  size_t         m_node_id;
};

static int is_prime(size_t num)
{
  if (num < 2)
    return 0;

  if (1 < num && num < 4)
    return 1;

  if ((num & 1) == 0)
    return 0;

  for (size_t i = 3; i * i <= num; i += 2)
  {
    if (num % i == 0)
      return 0;
  }
  return 1;
}

static size_t get_nearest_prime(size_t num)
{
  static const size_t max_prime = 
    (sizeof(size_t) == 8)   ?
    18446744073709551557ULL :
    4294967291U;

  if (num < 3)
    return 3;

  // if `num` is even then make it odd
  if ((num & 1) == 0)
    ++num;

  size_t upper = num + 2;

  for ( ; upper < max_prime - 2 ; upper += 2)
  {
    if (is_prime(upper))
      return upper;
  }
  return max_prime;
}

static size_t hash_func_default(const hash_table_key_t key, size_t key_size)
{
  size_t hash = ~key_size;
  size_t i    = 0;

  // TODO: safe comparation func to prevent size_t overflow
  for ( ; i + sizeof(size_t) < key_size; i += sizeof(size_t))
    hash ^= ELEMENT_AT(key, size_t, i);

  for ( ; i < key_size; ++i)
  {
    size_t tmp = BYTE_AT(key, i);
    hash ^= tmp;
  }
  return hash;
}

static int key_cmp_func_default(const hash_table_key_t a, const hash_table_key_t b, size_t key_size)
{
  data_buffer_t buff1 = {.m_data = a, .m_size = key_size};
  data_buffer_t buff2 = {.m_data = b, .m_size = key_size};

  return data_buffer_cmp_safe(&buff1, &buff2);
}

static void * copy_func_default(const void * src, size_t size)
{
  void * copy = malloc(size);

  if (copy == NULL)
    return NULL;

  memcpy(copy, src, size);
  return copy;
}

static int keys_compare(const hash_table_t * table, const hash_table_key_t a, const hash_table_key_t b)
{
  hash_table_key_cmp_func_t cmp_func = table->m_key_cmp_func;
  hash_table_size_params_t  params = table->m_size_params;
  
  int cmp = cmp_func ? 
            cmp_func(a, b) : 
            key_cmp_func_default(a, b, params.m_key_size);

  return cmp;
}

static hash_table_key_t key_copy(const hash_table_t * table, const hash_table_key_t key)
{
  hash_table_key_t * copy = table->m_key_copy_func ? 
                            table->m_key_copy_func(key) : 
                            copy_func_default(key, table->m_size_params.m_key_size);

  return copy;
}

static hash_table_value_t value_copy(const hash_table_t * table, const hash_table_value_t value)
{
  hash_table_value_t * copy = table->m_value_copy_func ? 
                              table->m_value_copy_func(value) : 
                              copy_func_default(value, table->m_size_params.m_value_size);

  return copy;
}

typedef struct node_pair_t
{
  hash_table_node_t * m_first;
  hash_table_node_t * m_second;

} node_pair_t;

static node_pair_t find_node_with_key(
  const hash_table_t   * table,
  const hash_table_key_t key,
  hash_table_node_t    * start_node,
  int                  * cmp
)
{
  node_pair_t pair = {.m_first=NULL, .m_second=NULL};

  *cmp = 1;
  pair.m_second = start_node;

  while (*cmp > 0)
  {
    *cmp = keys_compare(table, pair.m_second->m_key, key);

    if (*cmp <= 0)
      break;

    pair.m_first  = pair.m_second;
    pair.m_second = pair.m_second->m_next;    
  }
  return pair;
}

static hash_table_node_t * node_create(
  const hash_table_t     * table, 
  const hash_table_key_t   key, 
  const hash_table_value_t value
)
{
  hash_table_node_t * node = MALLOC(hash_table_node_t);

  if (node == NULL)
    return NULL;

  hash_table_size_params_t params = table->m_size_params;

  node->m_key   = key_copy(table, key);
  node->m_value = value_copy(table, value);
  node->m_next  = NULL;

  if (node->m_key == NULL || node->m_value == NULL)
  {
    table->m_key_free_func(node->m_key);
    table->m_value_free_func(node->m_value);
    free(node);
    return NULL;
  }
  return node;
}

static hash_table_status_t node_copy(
  const hash_table_t         * table,
  const hash_table_node_t    * src,
  hash_table_node_t          * dest
)
{
  if (dest == NULL)
  {
    dest = MALLOC(hash_table_node_t);

    if (dest == NULL)
      return HASH_TABLE_MALLOC_FAILURE;
  }

  dest->m_key   = key_copy(table, src->m_key);
  dest->m_value = value_copy(table, src->m_value);

  if (dest->m_key == NULL || dest->m_value == NULL)
    return HASH_TABLE_MALLOC_FAILURE;

  return HASH_TABLE_SUCCESS;
}

static hash_table_status_t node_chain_copy(
  const hash_table_t      * table,
  size_t                    nodes_count,
  const hash_table_node_t * src,
  hash_table_node_t       * dest
)
{
  while (src != NULL && nodes_count > 0)
  {
    hash_table_status_t status = node_copy(table, src, dest);

    if (status != HASH_TABLE_SUCCESS)
      return status;

    src  = src->m_next;
    dest = dest->m_next;

    --nodes_count;
  }
  return HASH_TABLE_SUCCESS;
}

static hash_table_status_t node_insert(
  hash_table_t           * table,
  hash_table_node_t     ** node,
  const hash_table_key_t   key,
  const hash_table_value_t value
)
{
  if (node == NULL)
    return HASH_TABLE_FAILURE;

  if (*node == NULL)
  {
    hash_table_node_t * new_head = node_create(table, key, value);
    if (new_head == NULL)
      return HASH_TABLE_FAILURE | HASH_TABLE_MALLOC_FAILURE;
    
    *node = new_head;
    return HASH_TABLE_SUCCESS | HASH_TABLE_NEW_VALUE_INSERTED;
  }

  int         cmp  = 1;
  node_pair_t pair = find_node_with_key(table, key, *node, &cmp);

  if (cmp == 0)
  {
    table->m_value_free_func(pair.m_second->m_value);
    pair.m_second->m_value = value_copy(table, value);

    return HASH_TABLE_SUCCESS | HASH_TABLE_VALUE_UPDATED;
  }
  // cmp < 0
  hash_table_node_t * new_node = node_create(table, key, value);

  if (pair.m_first != NULL)
    pair.m_first->m_next = new_node;
  else
    *node = new_node;

  new_node->m_next = pair.m_second;

  return HASH_TABLE_SUCCESS | HASH_TABLE_NEW_VALUE_INSERTED;
}

static hash_table_value_t node_at(
  const hash_table_t      * table,
  const hash_table_node_t * node,
  const hash_table_key_t    key
)
{
  int cmp = 1;

  while (node != NULL)
  {
    cmp = keys_compare(table, node->m_key, key);

    if (cmp <= 0)
      break;

    node = node->m_next;
  }
  if (cmp != 0 || node == NULL)
    return NULL;

  return node->m_value;
}

static void node_destroy(hash_table_t * table, hash_table_node_t * node)
{
  table->m_key_free_func(node->m_key);
  table->m_value_free_func(node->m_value);
  free(node);
}

static hash_table_status_t node_remove(
  hash_table_t         * table,
  hash_table_node_t   ** node,
  const hash_table_key_t key
)
{
  if (node == NULL)
    return HASH_TABLE_FAILURE;

  if ((*node) == NULL)
    return HASH_TABLE_FAILURE | HASH_TABLE_NO_VALUE_FOUND;
  
  int cmp = 1;
  node_pair_t pair = find_node_with_key(table, key, *node, &cmp);

  if (cmp != 0 || pair.m_second == NULL)
    return HASH_TABLE_FAILURE | HASH_TABLE_NO_VALUE_FOUND;

  // cmp == 0
  if (pair.m_first == NULL)
  {
    hash_table_node_t * next_node = pair.m_second->m_next;
    node_destroy(table, pair.m_second);
    *node = next_node;
    return HASH_TABLE_SUCCESS | HASH_TABLE_VALUE_REMOVED;
  }
  pair.m_first->m_next = pair.m_second->m_next;
  node_destroy(table, pair.m_second);
  return HASH_TABLE_SUCCESS | HASH_TABLE_VALUE_REMOVED;
}

static void node_chain_destroy(hash_table_t * table, hash_table_node_t * node)
{
  hash_table_node_t * current_node = NULL;

  while (node != NULL)
  {
    current_node = node->m_next;
    node_destroy(table, node);
    node = current_node;
  }
}

static hash_table_status_t bucket_copy(
  const hash_table_t         * table,
  const hash_table_bucket_t  * src,
        hash_table_bucket_t  * dest
)
{
  size_t capacity = table->m_size_params.m_bucket_capacity;

  for (size_t i = 0; i < capacity; ++i)
  {
    dest[i].m_size = src[i].m_size;
    
    hash_table_status_t status = node_chain_copy(table, dest[i].m_size, src[i].m_start_node, dest[i].m_start_node);

    if (status != HASH_TABLE_SUCCESS)
      return status;
  }
  return HASH_TABLE_SUCCESS;
}

static hash_table_status_t bucket_insert(
  hash_table_t           * table,
  hash_table_bucket_t    * bucket,
  const hash_table_key_t   key,
  const hash_table_value_t value
)
{
  hash_table_status_t status = node_insert(table, &bucket->m_start_node, key, value);
  
  if ((status & HASH_TABLE_NEW_VALUE_INSERTED) > 0)
    ++bucket->m_size;

  return status;
}

static hash_table_value_t bucket_at(
  const hash_table_t        * table,
  const hash_table_bucket_t * bucket,
  const hash_table_key_t      key
)
{
  return node_at(table, bucket->m_start_node, key);
}

static hash_table_status_t bucket_remove(
  hash_table_t         * table,
  hash_table_bucket_t  * bucket,
  const hash_table_key_t key
)
{
  hash_table_status_t status = node_remove(table, &bucket->m_start_node, key);

  if ((status & HASH_TABLE_VALUE_REMOVED) == HASH_TABLE_VALUE_REMOVED)
    --bucket->m_size;

  return status;
}

static void bucket_destroy(hash_table_t * table, hash_table_bucket_t * bucket)
{
  node_chain_destroy(table, bucket->m_start_node);
}

static size_t bucket_id_get(const hash_table_t * table, const hash_table_key_t key)
{
  hash_table_size_params_t params = table->m_size_params;
  size_t hash = (table->m_hash_func != NULL) ? table->m_hash_func(key) : hash_func_default(key, params.m_key_size);
  return hash % params.m_bucket_capacity;
}

hash_table_t * hash_table_create(
  hash_table_size_params_t     size_params,
  hash_table_key_cmp_func_t    key_cmp_func,
  hash_table_hash_func_t       hash_func,
  hash_table_key_copy_func_t   key_copy_func,
  hash_table_key_free_func_t   key_free_func,
  hash_table_value_copy_func_t value_copy_func,
  hash_table_value_free_func_t value_free_func
)
{
  if (size_params.m_bucket_capacity == 0) 
    size_params.m_bucket_capacity = 16;

  hash_table_t * table = MALLOC(hash_table_t);

  if (table == NULL) 
    return NULL;

  table->m_buckets = CALLOC(size_params.m_bucket_capacity, hash_table_bucket_t);

  if (table->m_buckets == NULL)
  {
    free(table);
    return NULL;
  }
  table->m_size            = 0;
  table->m_size_params     = size_params;
  table->m_hash_func       = hash_func;
  table->m_key_cmp_func    = key_cmp_func;
  table->m_key_copy_func   = key_copy_func;
  table->m_key_free_func   = key_free_func ? key_free_func : free;
  table->m_value_copy_func = value_copy_func;
  table->m_value_free_func = value_free_func ? value_free_func : free;

  return table;
}

hash_table_t * hash_table_copy(const hash_table_t * other)
{
  if (other == NULL)
    return NULL;

  hash_table_t * table = hash_table_create(
    other->m_size_params,
    other->m_key_cmp_func,
    other->m_hash_func,
    other->m_key_copy_func,
    other->m_key_free_func,
    other->m_value_copy_func,
    other->m_value_free_func
  );

  if (table == NULL)
    return NULL;

  hash_table_status_t status = bucket_copy(table, table->m_buckets, other->m_buckets);

  if (status != HASH_TABLE_SUCCESS)
  {
    hash_table_destroy(table);
    return NULL;
  }
  return table;
}

void hash_table_destroy(hash_table_t * table)
{
  if (table == NULL)
    return;

  if (table->m_buckets == NULL)
  {
    free(table);
    return;
  }

  size_t bucket_capacity = table->m_size_params.m_bucket_capacity;

  for (size_t i = 0; i < bucket_capacity; ++i)
    bucket_destroy(table, &table->m_buckets[i]);

  free(table->m_buckets);
  free(table);
}

hash_table_status_t hash_table_insert(hash_table_t * table, const hash_table_key_t key, const hash_table_value_t value)
{
  if(table == NULL)
    return HASH_TABLE_FAILURE;

  size_t id = bucket_id_get(table, key);

  hash_table_status_t status = bucket_insert(table, &table->m_buckets[id], key, value);

  if ((status & HASH_TABLE_NEW_VALUE_INSERTED) == HASH_TABLE_NEW_VALUE_INSERTED)
    ++table->m_size;

  return status;
}

hash_table_value_t hash_table_at(const hash_table_t * table, const hash_table_key_t key)
{
  if (table == NULL)
    return NULL;

  size_t id = bucket_id_get(table, key);

  return bucket_at(table, &table->m_buckets[id], key);
}

hash_table_status_t hash_table_remove(hash_table_t * table, const hash_table_key_t key)
{
  if (table == NULL)
    return HASH_TABLE_FAILURE;

  size_t id = bucket_id_get(table, key);

  hash_table_status_t status = bucket_remove(table, &table->m_buckets[id], key);

  if ((status & HASH_TABLE_VALUE_REMOVED) == HASH_TABLE_VALUE_REMOVED)
    --table->m_size;

  return status;
}

int hash_table_contains(const hash_table_t * table, const hash_table_key_t key)
{
  if (table == NULL)
    return -1;

  size_t id = bucket_id_get(table, key);

  return (bucket_at(table, &table->m_buckets[id], key) != NULL) ? 0 : 1;
}

size_t hash_table_size(const hash_table_t * table)
{
  if (table == NULL)
    return 0;

  return table->m_size;
}

#undef MALLOC
#undef CALLOC

#undef ELEMENT_AT
#undef BYTE_AT