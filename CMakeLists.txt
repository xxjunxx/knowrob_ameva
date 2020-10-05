cmake_minimum_required(VERSION 2.8.3)
project(knowrob_ameva)

find_package(catkin REQUIRED roscpp std_msgs)
find_package (Threads)
find_package(PkgConfig)
find_package(Protobuf REQUIRED)

pkg_check_modules(SWIPL REQUIRED swipl)
link_directories(${SWIPL_LIBRARY_DIRS})
message(STATUS "SWIPL found.")

pkg_check_modules(PROTOBUF REQUIRED protobuf)
link_directories(${PROTOBUF_LIBRARY_DIRS})
message(STATUS "PROTOBUF found.")

pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
link_directories(${LIBWEBSOCKETS_LIBRARY_DIRS})
message(STATUS "LIBWEBSOCKETS found.")

set(proto_dir ${PROJECT_SOURCE_DIR}/src/proto)
file(GLOB proto_files "${proto_dir}/*.proto")
message(STATUS "Proto Source Dir: ${proto_dir}")
message(STATUS "Proto Source Files: ${proto_files}")

# Set up destination directories
catkin_destinations()
set(proto_gen_cpp_dir ${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_INCLUDE_DESTINATION})
file(MAKE_DIRECTORY ${proto_gen_cpp_dir})
set(protogen_include_dirs ${proto_gen_cpp_dir}/../)
message(STATUS "Proto Include Dirs: ${protogen_include_dirs}")
message(STATUS "Proto Gen Dirs:${proto_gen_cpp_dir}")

catkin_package(
  DEPENDS
   roscpp
  INCLUDE_DIRS ${protogen_include_dirs}
  LIBRARIES knowrob_ameva knowrob_ameva_proto 
)


# Create lists of files to be generated.
set(proto_gen_cpp_files "")
foreach(proto_file ${proto_files})
    get_filename_component(proto_name ${proto_file} NAME_WE)
    list(APPEND proto_gen_cpp_files ${proto_gen_cpp_dir}/${proto_name}.pb.h ${proto_gen_cpp_dir}/${proto_name}.pb.cc)
endforeach(proto_file ${proto_files})


message(STATUS "Proto Gen Files :${proto_gen_cpp_dir}")
# Run protoc and generate language-specific headers.
add_custom_command(
    OUTPUT ${proto_gen_cpp_files}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${proto_dir} --cpp_out=${proto_gen_cpp_dir}  ${proto_files}
    DEPENDS ${PROTOBUF_PROTOC_EXECUTABLE} ${proto_files}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(knowrob_ameva_generate_headers ALL
    COMMAND echo "Creating proto generated cpp files"
    DEPENDS ${proto_gen_cpp_files}
)

# Create proto library for linking.
include_directories(include ${PROTOBUF_INCLUDE_DIRS})
add_library(knowrob_ameva_proto 
  ${proto_gen_cpp_files})
target_link_libraries(knowrob_ameva_proto ${PROTOBUF_LIBRARIES})
add_dependencies(knowrob_ameva_proto knowrob_ameva_generate_headers)


include_directories(include ${SWIPL_INCLUDE_DIRS} ${LIBWEBSOCKETS_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS} ${proto_gen_cpp_dir})

add_library(knowrob_ameva SHARED 
    src/knowrob_ameva.cpp
    )
target_link_libraries(knowrob_ameva  knowrob_ameva_proto ${LIBWEBSOCKETS_LIBRARIES} ${SWIPL_LIBRARIES} ${catkin_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_proto)