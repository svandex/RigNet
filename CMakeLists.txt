cmake_minimum_required(VERSION 3.9)
project(rignet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4267")

set(snap_wrapper_dir "C:/Users/Public/Softwares/snap7-full-1.4.2/")
set(nidaqmx_dir "C:\\Program Files (x86)\\National Instruments\\Shared\\ExternalCompilerSupport\\C")
set(mysql_wrapper_dir "C:/Users/Public/Softwares/mysql-connector-cpp")
set(vcpkg_dir "C:/Users/Public/Repositories/vcpkg/installed/x64-windows/")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/CppWindowsService
    ${PROJECT_SOURCE_DIR}/include/NIdaqmx
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/CppWindowsService
    ${PROJECT_SOURCE_DIR}/../rapidjson/include
    ${PROJECT_SOURCE_DIR}/../websocketpp
    ${mysql_wrapper_dir}/include/
    ${snap_wrapper_dir}/examples/cpp/
    ${nidaqmx_base_dir}/include
    ${vcpkg_dir}/include
    )

link_directories(
    ${mysql_wrapper_dir}/Release
    ${mysql_wrapper_dir}/Debug
    ${snap_wrapper_dir}/lib64
    ${snap_wrapper_dir}/build/bin/win64/
    ${nidaqmx_dir}/lib64/msvc
    ${vcpkg_dir}/lib
    )

#add_executable(rignet src/RigNet.cpp src/NICard_s.cpp src/SimensPLC.cpp src/sub_test.cpp main.cpp ${snap_wrapper_dir}/examples/cpp/snap7.cpp)
#target_link_libraries(rignet visa pthread snap7 boost_system mysqlcppconn8 nidaqmx)

add_executable(RigNet src/RigNet.cpp src/NICard_s.cpp src/SimensPLC.cpp main.cpp ${snap_wrapper_dir}/examples/cpp/snap7.cpp)
target_link_libraries(RigNet snap7 boost_system-vc140-mt mysqlcppconn8 nidaqmx)

add_executable(RigNetService test.cpp src/RigNet.cpp)
target_link_libraries(RigNetService mysqlcppconn8 boost_system-vc140-mt)

#add_definitions(-DSvandexDebug)
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)
set(CMAKE_MFC_FLAG 2)
add_executable(MySQLService src/RigNet.cpp src/CppWindowsService/MySQLService.cpp src/CppWindowsService/ServiceBase.cpp src/CppWindowsService/ServiceInstaller.cpp src/CppWindowsService/CppWindowsService.cpp)
set_target_properties(MySQLService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(MySQLService RigNetLib mysqlcppconn8 boost_system-vc140-mt Pathcch)

add_executable(TestService src/CppWindowsService/SampleService.cpp src/CppWindowsService/ServiceBase.cpp src/CppWindowsService/ServiceInstaller.cpp src/CppWindowsService/CppWindowsService.cpp)
set_target_properties(TestService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS RigNet RigNetService MySQLService RUNTIME DESTINATION bin)
