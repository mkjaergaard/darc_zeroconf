cmake_minimum_required(VERSION 2.8)
project(darc_zeroconf)

find_package(catkin)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES darc_zeroconf
)

find_package(catkin REQUIRED COMPONENTS iris darc_common)

include_directories(include)
include_directories(${catkin_INCLUDE_DIRS})

add_library(darc_zeroconf
  src/client.cpp
  src/service.cpp)
target_link_libraries(darc_zeroconf ${catkin_LIBRARIES} -lavahi-client)

add_executable(avahi_client_test
  test/test.cpp)
target_link_libraries(avahi_client_test darc_zeroconf)
