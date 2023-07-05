cmake_minimum_required(VERSION 3.8)
project(database_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
ament_export_dependencies(rosidl_default_runtime)

set(msg_files
"msg/Log.msg"
"msg/Move.msg"
"msg/Waypoint.msg"
)

set(srv_files
  "srv/AddMove.srv"
  "srv/DeleteMove.srv"
  "srv/EditMove.srv"
  "srv/GetMove.srv"
  "srv/GetMoves.srv"

  "srv/AddLog.srv"
  "srv/GetLog.srv"
  "srv/GetLogs.srv"
  "srv/DeleteLog.srv"
  "srv/DeleteLogs.srv"

  "srv/CheckPassword.srv"
  "srv/UpdatePassword.srv"

  "srv/AddTileLayout.srv"
  "srv/GetTileLayout.srv"
  "srv/UpdateTileLayout.srv"

  "srv/AddTile.srv"
  "srv/GetTile.srv"
  "srv/UpdateTile.srv"

  "srv/SetLanguage.srv"
  "srv/GetLanguage.srv"
  )

rosidl_generate_interfaces(
  ${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
