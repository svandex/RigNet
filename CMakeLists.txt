cmake_minimum_required(VERSION 3.14)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /wd4267")

set(MYSQL_SOURCE_DIR "$ENV{USERPROFILE}\\Documents\\Repositories\\mysql-connector-cpp")
set(MYSQL_INSTALL_DIR "$ENV{USERPROFILE}\\Documents\\Softwares\\mysql-connector-cpp-14-13")
set(VCPKG_INSTALL_DIR "${VCPKG_ROOT}/installed/x64-windows/")

find_package(glog CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/IISModule
    $ENV{NIEXTCCOMPILERSUPP}/include
    ${RAPIDJSON_INCLUDE_DIRS}
    ${WEBSOCKETPP_INCLUDE_DIRS}
    ${VCPKG_INSTALL_DIR}/include
	${MYSQL_SOURCE_DIR}/include
    )

link_directories(
	${MYSQL_INSTALL_DIR}
    $ENV{NIEXTCCOMPILERSUPP}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
    )

set(CMAKE_MFC_FLAG 2)#2 means shared mfc library

#Windows Service
add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/RigNet.cpp src/Svandex.cpp)
target_compile_definitions(RigNetService PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RigNetService mysqlcppconn8 boost_system-vc140-mt  pathcch glog)

#Test
add_executable(testapp testMain.cpp src/Svandex.cpp src/RigNet.cpp)
target_link_libraries(testapp mysqlcppconn8 boost_system-vc140-mt  pathcch glog)

#IIS MODULE 
add_library(RigNetModule SHARED src/IISModule/precomp.cpp src/Svandex.cpp src/IISModule/RigNetModule.cpp src/IISModule/RigNetMain.cpp)
target_compile_definitions(RigNetModule PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE GLOG_NO_ABBREVIATED_SEVERITIES)
target_compile_options(RigNetModule PRIVATE "/Gz")
target_link_options(RigNetModule PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(RigNetModule PRIVATE "/GUARD:CF")
target_link_options(RigNetModule PRIVATE "/VERSION:10.00")
target_link_options(RigNetModule PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(RigNetModule PUBLIC pathcch mysqlcppconn8)
#target_link_libraries(RigNetModule PRIVATE "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/libeay32.lib" "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/ssleay32.lib")