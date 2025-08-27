#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "utility.h"

data_buffer_t * data_buffer_init(const void * data, size_t size)
{
  data_buffer_t * buffer = (data_buffer_t *)malloc(sizeof(data_buffer_t));

  if (buffer == NULL)
    return NULL;

  buffer->m_data = malloc(size);

  if (buffer->m_data == NULL)
  {
    free(buffer);
    return NULL; 
  }
  memcpy(buffer->m_data, data, size);
  return buffer;
}

data_buffer_t * data_buffer_copy(const data_buffer_t * other)
{
  if (other == NULL)
    return NULL;

  return data_buffer_init(other->m_data, other->m_size);
}

void data_buffer_free(data_buffer_t * buffer)
{
  if (buffer == NULL)
    return;

  free(buffer->m_data);
  free(buffer);
}

int data_buffer_cmp(const data_buffer_t * a, const data_buffer_t * b)
{
  assert(a != NULL && b != NULL);

  if (a->m_size == b->m_size)
    return memcmp(a->m_data, b->m_data, a->m_size);

  size_t min_size = (a->m_size > b->m_size) ? 
                    b->m_size               :
                    a->m_size;

  return memcmp(a->m_data, b->m_data, min_size);
}

int data_buffer_cmp_safe(const data_buffer_t * a, const data_buffer_t * b)
{
  int cmp_a = (a == NULL);
  int cmp_b = (b == NULL);

  if (cmp_a || cmp_b)
    return cmp_b - cmp_a; 

  return data_buffer_cmp(a, b);
}

bytes_buffer_t * bytes_buffer_init(const unsigned char * data, size_t size)
{
  bytes_buffer_t * buffer = (bytes_buffer_t *)malloc(sizeof(bytes_buffer_t));

  if (buffer == NULL)
    return NULL;

  buffer->m_bytes = malloc(size);

  if (buffer->m_bytes == NULL)
  {
    free(buffer);
    return NULL;
  }
  memcpy(buffer->m_bytes, data, size);
  return buffer;
}

bytes_buffer_t * bytes_buffer_copy(const bytes_buffer_t * other)
{
  assert(other != NULL);
  return bytes_buffer_init(other->m_bytes, other->m_size);
}

void bytes_buffer_free(bytes_buffer_t * buffer)
{
  if (buffer == NULL)
    return;

  free(buffer->m_bytes);
  free(buffer);
}

int bytes_buffer_cmp(const bytes_buffer_t * a, const bytes_buffer_t * b)
{
  assert(a != NULL && b != NULL);

  size_t min_size = (a->m_size > b->m_size) ? b->m_size : a->m_size;
  int    cmp      = memcmp(a->m_bytes, b->m_bytes, min_size);

  if (cmp > 0)
    return 1;

  if (cmp < 0)
    return -1;

  return (a->m_size == min_size) ? 1 : -1;
}

int bytes_buffer_cmp_safe(const bytes_buffer_t * a, const bytes_buffer_t * b)
{
  int cmp_a = (a == NULL);
  int cmp_b = (b == NULL);

  if (cmp_a || cmp_b)
    return cmp_b - cmp_a; 

  return bytes_buffer_cmp(a, b);
}