qt_add_executable(kedia_test_manager
  entrytest.cpp
)

include_directories(include)

target_link_libraries(kedia_test_manager PRIVATE kedia kedia_manager_lib Qt6::Core Qt6::Widgets Qt6::Test)
