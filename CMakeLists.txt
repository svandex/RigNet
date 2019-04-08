cmake_minimum_required(VERSION 3.9)
project(rignet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4267")

set(nidaqmx_dir "C:\\Program Files (x86)\\National Instruments\\Shared\\ExternalCompilerSupport\\C")
#set(mysql_wrapper_dir "C:\\Program Files\\MySQL\\Connector C++ 8.0\\")
set(mysql_wrapper_dir "C:\\Users\\saictv\\Documents\\Repositories\\mysql-connector-cpp")
set(mysql_bin "C:\\Users\\saictv\\Documents\\Softwares\\mysql-connector-cpp")
set(vcpkg_dir "C:/Users/saictv/Documents/Repositories/vcpkg/installed/x64-windows/")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/RigNetService
	${nidaqmx_dir}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/RigNetService
    ${PROJECT_SOURCE_DIR}/../rapidjson/include
    ${PROJECT_SOURCE_DIR}/../websocketpp
    ${vcpkg_dir}/include
	${mysql_wrapper_dir}/include
    )

link_directories(
#    ${mysql_wrapper_dir}/lib64/vs14
	${mysql_bin}
    ${nidaqmx_dir}/lib64/msvc
    ${vcpkg_dir}/lib
    )

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)
set(CMAKE_MFC_FLAG 2)
add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/RigNet.cpp src/Svandex.cpp)
set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RigNetService mysqlcppconn8 boost_system-vc140-mt  pathcch)

#add_executable(testSvandex testSvandex.cpp src/Svandex.cpp)
#target_link_libraries(testSvandex pathcch)
add_executable(testapp testMain.cpp src/Svandex.cpp src/RigNet.cpp)
target_link_libraries(testapp mysqlcppconn8 boost_system-vc140-mt  pathcch)