cmake_minimum_required(VERSION 3.9)
project(rignet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mfma -g")

set(snap_wrapper_dir /home/svandex/Softwares/snap7-full-1.4.2)
set(nidaqmx_base_dir /usr/local/natinst/nidaqmxbase)

set(centos_cpp_include_dir 
/usr/include/
/usr/include/gnu
/usr/include/c++/4.8.5/
/usr/include/c++/4.8.5/x86_64-redhat-linux/
/usr/local/include
/usr/include/ni-visa/
)

set(centos_cpp_link_dir
/usr/lib/x86_64-linux-gnu/
/usr/lib
/usr/lib64
    )
elseif(WIN32)
endif()

include_directories(
    ${centos_cpp_dir}
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/src
${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include
${PROJECT_SOURCE_DIR}/thirdparty/websocketpp
${mysql_wrapper_dir}/include/
${snap_wrapper_dir}/examples/cpp/
${nidaqmx_base_dir}/include
)

link_directories(
    ${centos_cpp_link_dir}
${mysql_wrapper_dir}/lib64
${snap_wrapper_dir}/lib64
${mysql_wrapper_dir}/lib
${snap_wrapper_dir}/lib
${snap_wrapper_dir}/build/bin/x86_64-linux/
${snap_wrapper_dir}/build/bin/i386-linux/
${nidaqmx_base_dir}/lib
${nidaqmx_base_dir}/lib64
)

#add_executable(rignet src/RigNet.cpp src/NICard_s.cpp src/SimensPLC.cpp src/sub_test.cpp main.cpp ${snap_wrapper_dir}/examples/cpp/snap7.cpp)
#target_link_libraries(rignet visa pthread snap7 boost_system mysqlcppconn8 nidaqmx)

add_executable(rignet src/RigNet.cpp src/NICard_s.cpp src/SimensPLC.cpp main.cpp ${snap_wrapper_dir}/examples/cpp/snap7.cpp)
target_link_libraries(rignet pthread snap7 boost_system mysqlcppconn8 nidaqmxbase)

add_executable(wss test.cpp src/RigNet.cpp)
target_link_libraries(wss mysqlcppconn8 boost_system)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS rignet wss RUNTIME DESTINATION bin)
