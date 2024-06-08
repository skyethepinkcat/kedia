qt_add_executable(kedia_test
  seriestest.cpp
)

include_directories(include)

target_link_libraries(kedia_test PRIVATE kedia Qt6::Core Qt6::Widgets Qt6::Test)
