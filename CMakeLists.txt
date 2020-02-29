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
find_package(SQLiteCpp CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/iistvmgt
    ${PROJECT_SOURCE_DIR}/include/iistvmda
    ${PROJECT_SOURCE_DIR}/include/iismysql
    ${PROJECT_SOURCE_DIR}/include/iissqlite
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

# MYSQL MODULE
# send request with json format to execute mysql engine to query data
add_library(iistvmysql SHARED src/config.cpp src/iismysql/iismysqlpp.cpp src/iismysql/main.cpp)
target_compile_definitions(iistvmysql PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(iistvmysql PRIVATE "/Gz")
target_link_options(iistvmysql PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iistvmysql PRIVATE "/GUARD:CF")
target_link_options(iistvmysql PRIVATE "/EXPORT:RegisterModule")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_link_libraries(iistvmysql PUBLIC pathcch rpcrt4 windowsapp mysqlpp_d libmysql)
else()
    target_link_libraries(iistvmysql PUBLIC pathcch rpcrt4 windowsapp mysqlpp libmysql)
endif()

# mysql xdevapi module
# 
add_library(xdevapi SHARED src/config.cpp src/iismysql/iisxdevapi.cpp src/iismysql/main.cpp)
set_property(TARGET xdevapi PROPERTY CXX_STANDARD 17)
target_compile_definitions(xdevapi PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(xdevapi PRIVATE "/Gz")
target_link_options(xdevapi PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(xdevapi PRIVATE "/GUARD:CF")
target_link_options(xdevapi PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(xdevapi PUBLIC pathcch rpcrt4 windowsapp mysqlcppconn8)

# iistvmgt module
# 
add_library(iistvmgt SHARED src/config.cpp src/iistvmgt/iistvmgt.cpp src/iistvmgt/main.cpp src/Svandex.cpp)
set_property(TARGET iistvmgt PROPERTY CXX_STANDARD 11)
target_compile_definitions(iistvmgt PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(iistvmgt PRIVATE "/Gz")
target_link_options(iistvmgt PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iistvmgt PRIVATE "/GUARD:CF")
target_link_options(iistvmgt PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(iistvmgt PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)

# iistvmda module
# 
add_library(iistvmda SHARED src/config.cpp src/iistvmda/iistvmda.cpp src/iistvmda/main.cpp src/Svandex.cpp src/iisutl.cpp)
set_property(TARGET iistvmda PROPERTY CXX_STANDARD 11)
target_compile_definitions(iistvmda PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
target_compile_options(iistvmda PRIVATE "/Gz")
target_link_options(iistvmda PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iistvmda PRIVATE "/GUARD:CF")
target_link_options(iistvmda PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(iistvmda PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)

# iissqlite module
# 
add_library(iissqlite SHARED src/config.cpp src/iissqlite/iissqlite.cpp src/iissqlite/main.cpp src/Svandex.cpp src/iisutl.cpp)
set_property(TARGET iissqlite PROPERTY CXX_STANDARD 11)
target_compile_definitions(iissqlite PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
target_compile_options(iissqlite PRIVATE "/Gz")
target_link_options(iissqlite PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iissqlite PRIVATE "/GUARD:CF")
target_link_options(iissqlite PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(iissqlite PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)