add_executable(list_tests
  common/list_tests.c
)

target_link_libraries(list_tests
  PRIVATE
    common-lib
)

add_dependencies(list_tests
  common-lib
)