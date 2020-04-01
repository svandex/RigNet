cmake_minimum_required(VERSION 3.14)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(TestValidationNet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /EHsc")

set(VCPKG_INSTALL_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/")
set(MYSQL_C_CNT "$ENV{MYSQL_ROOT}/MySQL\ Connector\ C\ 6.1/")
set(MYSQL_CPP_CNT "$ENV{MYSQL_ROOT}/Connector C++\ 8.0/")
set(MYSQLPP_DIR "C:/Users/svandex/Repositories/mysqlpp/")

find_package(RapidJSON CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/thirdparty/svandex
    ${PROJECT_SOURCE_DIR}/module/**
    ${GMIME_INCLUDE_DIRS}
    #$ENV{NIEXTCCOMPILERSUPP}/include
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
    #$ENV{NIEXTCCOMPILERSUPP}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
    ${MYSQLPP_DIR}/vc2008/x64/Release
    ${MYSQLPP_DIR}/vc2008/x64/Debug
    )

#2 means shared mfc library
set(CMAKE_MFC_FLAG 2)

set(BASE_SRCS ${PROJECT_SOURCE_DIR}/src/HeaderPrecompilation.cpp ${PROJECT_SOURCE_DIR}/src/User.cpp ${PROJECT_SOURCE_DIR}/thirdparty/svandex/Svandex.cpp ${PROJECT_SOURCE_DIR}/src/ModuleBase.cpp)

enable_testing()
add_subdirectory(test)
add_test(NAME TestSQLiteOperation COMMAND SQLiteOperation)

list(APPEND ModuleList ModuleSQLite ModuleMediaManagement ModuleStaffManagement)#ModuleMySQL 

list(APPEND ServiceList)

foreach(ModuleName IN LISTS ModuleList)
    add_subdirectory(module/${ModuleName})
endforeach()

foreach(ServiceName IN LISTS ServiceList)
    add_subdirectory(service/${Servicename})
endforeach()
