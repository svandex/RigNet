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
    ${PROJECT_SOURCE_DIR}/thirdparty/**
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

set(BASIC_FILES "src/HeaderPrecompilation.cpp src/ProjectUtility.cpp src/User.cpp")

add_subdirectory(test)
enable_testing()
add_test(NAME TestSQLiteOperation COMMAND SQLiteOperation)

add_subdirectory(module/ModuleSQLite)
add_subdirectory(module/ModuleMySQL)
add_subdirectory(module/ModuleMediaManagement)
add_subdirectory(module/ModuleStaffManagement)

#add_subdirectory(service/example)