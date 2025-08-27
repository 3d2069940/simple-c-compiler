#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stddef.h>

typedef struct
{
  void * m_data;
  size_t m_size;

} data_buffer_t;

data_buffer_t * data_buffer_init(const void * data, size_t size);
data_buffer_t * data_buffer_copy(const data_buffer_t * other);

void data_buffer_free(data_buffer_t * buffer);

int data_buffer_cmp(const data_buffer_t * a, const data_buffer_t * b);
int data_buffer_cmp_safe(const data_buffer_t * a, const data_buffer_t * b);

typedef struct
{
  unsigned char * m_bytes;
  size_t          m_size;

} bytes_buffer_t;

bytes_buffer_t * bytes_buffer_init(const unsigned char * data, size_t size);
bytes_buffer_t * bytes_buffer_copy(const bytes_buffer_t * other);

void bytes_buffer_free(bytes_buffer_t * buffer);

int bytes_buffer_cmp(const bytes_buffer_t * a, const bytes_buffer_t * b);
int bytes_buffer_cmp_safe(const bytes_buffer_t * a, const bytes_buffer_t * b);

#endif // __UTILITY_H__