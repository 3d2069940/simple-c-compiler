
add_library(common-lib
  list/list.h
  list/list.c
)

target_include_directories(common-lib
  PUBLIC
    .
  PRIVATE
    utility
)
