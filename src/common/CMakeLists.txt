
add_library(common-lib
  utility/utility.h
  utility/utility.c

  hash_table/hash_table.h
  hash_table/hash_table.c
)

target_include_directories(common-lib
  PUBLIC
    .
  PRIVATE
    utility
)
