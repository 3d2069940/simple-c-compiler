add_executable(hash_table_tests
  common/hash_table_tests.c
)

target_link_libraries(hash_table_tests
  PRIVATE
    common-lib
)

add_dependencies(hash_table_tests
  common-lib
)