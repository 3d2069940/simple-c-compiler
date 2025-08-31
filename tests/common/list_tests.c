
#include <stdio.h>
#include <assert.h>

#include "list/list.h"

void create_list_test(void)
{
  list_t * list = list_create(sizeof(int), NULL, NULL);

  assert(list != NULL);

  list_destroy(list);
}

void list_push_front_test(void)
{
  list_t * list = list_create(sizeof(int), NULL, NULL);

  assert(list != NULL);

  const int values[] = {0, 1, 2, 3, 4};

  for (size_t i = 0; i < 5; ++i)
  {
    assert(list_push_front(list, &(values[i])) >= LIST_SUCCESS);
    assert(list_front(list) != NULL);
    assert(*(int*)list_front(list) == (int)i);
  }
  list_destroy(list);
}

void list_push_back_test(void)
{
  list_t * list = list_create(sizeof(int), NULL, NULL);

  assert(list != NULL);

  const int values[] = {0, 1, 2, 3, 4};

  for (size_t i = 0; i < 5; ++i)
  {
    assert(list_push_back(list, &(values[i])) >= LIST_SUCCESS);
    assert(list_back(list) != NULL);
    assert(*(int*)list_back(list) == (int)i);
  }
  list_destroy(list);
}

void list_pop_front_test(void)
{
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
      assert(list_push_back(list, &(values[i])) >= LIST_SUCCESS);

    for (size_t i = 0; i < 5; ++i)
    {
      assert(list_front(list) != NULL);
      assert(*(int*)list_front(list) == (int)i);
      list_pop_front(list);
    }
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
      assert(list_push_front(list, &(values[i])) >= LIST_SUCCESS);

    assert(list_front(list) != NULL);
    assert(*(int*)list_front(list) == 4);

    list_pop_front(list);

    assert(list_front(list) != NULL);
    assert(*(int*)list_front(list) == 3);

    list_destroy(list);
  }
}

void list_pop_back_tests(void)
{
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
    {
      assert(list_push_back(list, &(values[i])) >= LIST_SUCCESS);
      assert(list_back(list) != NULL);
      assert(*(int*)list_back(list) == (int)i);
      list_pop_back(list);
    }
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
      list_push_back(list, &(values[i]));

    assert(list_back(list) != NULL);
    assert(*(int*)list_back(list) == 4);

    list_pop_back(list);

    assert(list_back(list) != NULL);
    assert(*(int*)list_back(list) == 3);

    list_destroy(list);
  }
}

void list_iter_tests(void)
{
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    list_iter_t * it  = list_begin(list);
    list_iter_t * end = list_end(list);

    while (list_iter_cmp(it, end))
    {
      assert(0);
    }
    list_iter_destroy(it);
    list_iter_destroy(end);
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    int value = 0;

    list_push_front(list, &value);

    list_iter_t * it  = list_begin(list);
    list_iter_t * end = list_end(list);

    while (list_iter_cmp(it, end))
    {
      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == value);

      it = list_iter_inc(it);
    }
    list_iter_destroy(it);
    list_iter_destroy(end);
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
      list_push_back(list, &values[i]);

    list_iter_t * it = list_begin(list);
    list_iter_t * end = list_end(list);

    int i = 0;

    while (list_iter_cmp(it, end))
    {
      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == i);

      ++i;
      it = list_iter_inc(it);
    }

    list_iter_destroy(it);
    list_iter_destroy(end);
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    list_push_front(list, &values[0]);

    list_iter_t * it = list_begin(list);

    for (size_t i = 1; i < 5; ++i)
    {
      assert(list_insert_after_iter(list, it, &values[i]) >= LIST_SUCCESS);
      
      it = list_iter_inc(it);

      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == values[i]);

      it = list_iter_dec(it);
    }
    list_iter_destroy(it);
    list_destroy(list);
  }
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    assert(list != NULL);

    const int values[] = {0, 1, 2, 3, 4};

    list_push_front(list, &values[0]);

    list_iter_t * it = list_begin(list);

    for (size_t i = 1; i < 5; ++i)
    {
      assert(list_insert_before_iter(list, it, &values[i]) >= LIST_SUCCESS);

      it = list_iter_dec(it);

      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == values[i]);

      it = list_iter_inc(it);
    }
    list_iter_destroy(it);
    list_destroy(list);
  }
}

void list_remove_tests(void)
{
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    const int values[] = {0, 1, 2, 3, 4};

    for (size_t i = 0; i < 5; ++i)
      list_push_back(list, &values[i]);

    list_iter_t * it = list_begin(list);

    list_remove(list, it);
    list_iter_destroy(it);

    it = list_begin(list);

    assert(list_iter_value(it) != NULL);
    assert(*(int*)list_iter_value(it) == 1);

    list_iter_destroy(it);
    list_destroy(list);
  }
}


void list_update_value_tests(void)
{
  {
    list_t * list = list_create(sizeof(int), NULL, NULL);

    const int values[] = {0, 1, 2, 3, 4};

    int default_value = 0;

    for (size_t i = 0; i < 5; ++i)
      list_push_back(list, &default_value);

    list_iter_t * it  = list_begin(list);

    for (size_t i = 0; i < 5; ++i)
    {
      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == 0);

      list_update_value(list, it, &values[i]);

      assert(list_iter_value(it) != NULL);
      assert(*(int*)list_iter_value(it) == values[i]);

      it = list_iter_inc(it);
    }
    list_iter_destroy(it);
    list_destroy(list);
  }
}

int main(void)
{
  printf("Testing is in progress\n");
  create_list_test();
  list_push_front_test();
  list_push_back_test();
  list_pop_front_test();
  list_pop_back_tests();
  list_iter_tests();
  list_remove_tests();
  list_update_value_tests();
  printf("Tests completed successfully\n");
}
