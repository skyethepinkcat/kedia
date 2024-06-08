qt_add_library(kedia_store_lib STATIC
  database.cpp
  store.cpp
)

qt_add_executable(kedia_store
  main.cpp
)

qt_add_resources(kedia_store_lib "SQL Scripts"
  PREFIX "/"
  BASE "resources"
  FILES
  resources/init.sql
)

include_directories(include)

target_link_libraries(kedia_store_lib PRIVATE kedia Qt6::Widgets Qt6::Sql)
target_link_libraries(kedia_store PRIVATE kedia
  kedia_store_lib Qt6::Widgets Qt6::Sql)
target_include_directories(kedia_store_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

