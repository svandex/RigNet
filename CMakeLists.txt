cmake_minimum_required(VERSION 3.14)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /EHsc")

set(VCPKG_INSTALL_DIR "${VCPKG_ROOT}/installed/x64-windows/")
set(MYSQL_C_CNT "$ENV{MYSQL_ROOT}/MySQL\ Connector\ C\ 6.1/")
set(MYSQL_CPP_CNT "$ENV{MYSQL_ROOT}/Connector C++\ 8.0/")
set(MYSQLPP_DIR "C:/Users/svandex/Repositories/mysqlpp/")

find_package(RapidJSON CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(JPEG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/IISModule
    ${PROJECT_SOURCE_DIR}/include/mysqlmodule
    ${PROJECT_SOURCE_DIR}/include/RigNetService
    $ENV{NIEXTCCOMPILERSUPP}/include
    ${RAPIDJSON_INCLUDE_DIRS}
    ${VCPKG_INSTALL_DIR}/include
	${JPEG__INCLUDE_DIR}
    ${MYSQL_C_CNT}/include
    ${MYSQL_CPP_CNT}/include
    ${MYSQLPP_DIR}/lib
    )

link_directories(
    ${MYSQL_C_CNT}/lib
    ${MYSQL_C_CNT}/lib/vs14
    ${MYSQL_CPP_CNT}/lib64/vs14
    $ENV{NIEXTCCOMPILERSUPP}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
    ${MYSQLPP_DIR}/vc2008/x64/Release
    ${MYSQLPP_DIR}/vc2008/x64/Debug
    )

set(CMAKE_MFC_FLAG 2)#2 means shared mfc library

# Windows Service
# Get rigt-test data from NI card or Simens PLC, use IPC to interact with IIS Module
#add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/Svandex.cpp)
#target_compile_definitions(RigNetService PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
#target_compile_options(RigNetService PRIVATE "/permissive")
#set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
#target_link_libraries(RigNetService mysqlcppconn8 pathcch rpcrt4)

# IIS MODULE
# Provide HTTP service
add_library(RigNetModule SHARED src/precomp.cpp src/Svandex.cpp src/IISModule/RigNetModule.cpp src/IISModule/RigNetMain.cpp)
target_compile_definitions(RigNetModule PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(RigNetModule PRIVATE "/Gz")
target_link_options(RigNetModule PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(RigNetModule PRIVATE "/GUARD:CF")
target_link_options(RigNetModule PRIVATE "/VERSION:1.00")
target_link_options(RigNetModule PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(RigNetModule PUBLIC pathcch rpcrt4 windowsapp sqlite3 ${JPEG_LIBRARIES})

# MYSQL MODULE
# send request with json format to execute mysql engine to query data
add_library(mysqlmodule SHARED src/precomp.cpp src/mysqlmodule/mysqlmodulec.cpp src/mysqlmodule/main.cpp)
target_compile_definitions(mysqlmodule PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(mysqlmodule PRIVATE "/Gz")
target_link_options(mysqlmodule PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(mysqlmodule PRIVATE "/GUARD:CF")
target_link_options(mysqlmodule PRIVATE "/EXPORT:RegisterModule")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_link_libraries(mysqlmodule PUBLIC pathcch rpcrt4 windowsapp mysqlpp_d libmysql)
else()
    target_link_libraries(mysqlmodule PUBLIC pathcch rpcrt4 windowsapp mysqlpp libmysql)
endif()

# mysql xdevapi module
# 
add_library(xdevapi SHARED src/precomp.cpp src/mysqlmodule/xdevapimodule.cpp src/mysqlmodule/main.cpp)
set_property(TARGET xdevapi PROPERTY CXX_STANDARD 17)
target_compile_definitions(xdevapi PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(xdevapi PRIVATE "/Gz")
target_link_options(xdevapi PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(xdevapi PRIVATE "/GUARD:CF")
target_link_options(xdevapi PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(xdevapi PUBLIC pathcch rpcrt4 windowsapp mysqlcppconn8)