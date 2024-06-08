qt_add_library(kedia STATIC
  media.cpp
  season.cpp
  series.cpp
  episode.cpp
  file.cpp
  drive.cpp
)

include_directories(include)

target_link_libraries(kedia PRIVATE Qt6::Widgets)
target_include_directories(kedia INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
