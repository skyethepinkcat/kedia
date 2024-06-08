
qt_add_executable(kedia_store_test
  storetest.cpp
)

include_directories(include)

target_link_libraries(kedia_store_test PRIVATE kedia kedia_store_lib Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Test)
