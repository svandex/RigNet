cmake_minimum_required(VERSION 3.9)
project(rignet)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mfma -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(snap_wrapper_dir /home/svandex/Softwares/snap7-full-1.4.2)
set(mysql_wrapper_dir /home/svandex/Documents/mysql-connector-c++)

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/src
${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include
${PROJECT_SOURCE_DIR}/thirdparty/websocketpp
/usr/include/
/usr/include/c++/4.8.5/
/usr/include/c++/4.8.5/x86_64-redhat-linux/
/usr/local/include
/usr/include/ni-visa/
${mysql_wrapper_dir}/include/
${snap_wrapper_dir}/examples/cpp/
)

link_directories(
/usr/lib/x86_64-linux-gnu/
/usr/lib
${mysql_wrapper_dir}/lib64
${snap_wrapper_dir}/lib64
${snap_wrapper_dir}/build/bin/x86_64-linux/)

add_executable(rignet src/RigNet.cpp src/NICard.cpp src/SimensPLC.cpp src/sub_test.cpp main.cpp ${snap_wrapper_dir}/examples/cpp/snap7.cpp)
target_link_libraries(rignet visa pthread snap7 boost_system mysqlcppconn8 nidaqmx)

add_executable(wss test.cpp src/RigNet.cpp)
target_link_libraries(wss mysqlcppconn8 boost_system)