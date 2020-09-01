cmake_minimum_required(VERSION 2.8.3)
project(knowrob_ameva)

find_package(catkin REQUIRED roscpp std_msgs)

catkin_package(
  DEPENDS
   roscpp
)

find_package (Threads)

find_package(PkgConfig)

pkg_check_modules(SWIPL REQUIRED swipl)
link_directories(${SWIPL_LIBRARY_DIRS})
message(STATUS "SWIPL found.")

pkg_check_modules(PROTOBUF REQUIRED protobuf)
link_directories(${PROTOBUF_LIBRARY_DIRS})
message(STATUS "PROTOBUF found.")

pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
link_directories(${LIBWEBSOCKETS_LIBRARY_DIRS})
message(STATUS "LIBWEBSOCKETS found.")

include_directories(include ${SWIPL_INCLUDE_DIRS} ${LIBWEBSOCKETS_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS})


add_library(knowrob_ameva SHARED 
    src/knowrob_ameva.cpp
    src/knowrob_ameva.pb.cc)
target_link_libraries(knowrob_ameva ${LIBWEBSOCKETS_LIBRARIES} ${PROTOBUF_LIBRARIES} ${SWIPL_LIBRARIES} ${catkin_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
