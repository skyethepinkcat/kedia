qt_add_library(kedia_manager_lib STATIC
  ui/MainWindow.ui
  ui/CreateSeries.ui
  MainWindow.cpp
  ui/MainWindow.qrc
  manager_utils.cpp
  CreateSeries.cpp
  ui/CreateSeason.ui
  CreateSeason.cpp
  ui/CreateEpisode.ui
  CreateEpisode.cpp
  ui/CreateDrive.ui
  CreateDrive.cpp
)

qt_add_executable(kedia_manager
  main.cpp
)

include_directories(include)

target_link_libraries(kedia_manager_lib PRIVATE
  kedia kedia_store_lib Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent)

target_link_libraries(kedia_manager PRIVATE
  kedia kedia_store_lib kedia_manager_lib Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent)

# set_target_properties(kedia_manager PROPERTIES
#   WIN32_EXECUTABLE ON
#   MACOSX_BUNDLE ON
# )

target_include_directories(kedia_manager_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
