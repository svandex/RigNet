cmake_minimum_required(VERSION 3.14)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")

set(MYSQL_SOURCE_DIR "$ENV{USERPROFILE}\\Documents\\Repositories\\mysql-connector-cpp")
set(MYSQL_INSTALL_DIR "$ENV{USERPROFILE}\\Documents\\Softwares\\mysql-connector-cpp-14-13")
set(VCPKG_INSTALL_DIR "${VCPKG_ROOT}/installed/x64-windows/")
set(SQLITE_INSTALL_DIR "$ENV{USERPROFILE}\\Documents\\Repositories\\sqlite")

find_package(RapidJSON CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/IISModule
    ${PROJECT_SOURCE_DIR}/include/RigNetService
    $ENV{NIEXTCCOMPILERSUPP}/include
    ${RAPIDJSON_INCLUDE_DIRS}
    ${VCPKG_INSTALL_DIR}/include
	${MYSQL_SOURCE_DIR}/include
	${SQLITE_INSTALL_DIR}/sqlite-amalgamation
    )

link_directories(
	${MYSQL_INSTALL_DIR}/RelWithDebInfo
    $ENV{NIEXTCCOMPILERSUPP}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
	${SQLITE_INSTALL_DIR}
    )

set(CMAKE_MFC_FLAG 2)#2 means shared mfc library

#Windows Service
#add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/Svandex.cpp)
#target_compile_definitions(RigNetService PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
#target_compile_options(RigNetService PRIVATE "/permissive")
#set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
#target_link_libraries(RigNetService mysqlcppconn8 pathcch rpcrt4)

#IIS MODULE 
add_library(RigNetModule SHARED src/IISModule/precomp.cpp src/Svandex.cpp src/IISModule/RigNetModule.cpp src/IISModule/RigNetMain.cpp)
target_compile_definitions(RigNetModule PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(RigNetModule PRIVATE "/Gz")
target_link_options(RigNetModule PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(RigNetModule PRIVATE "/GUARD:CF")
target_link_options(RigNetModule PRIVATE "/VERSION:1.00")
target_link_options(RigNetModule PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(RigNetModule PUBLIC mysqlcppconn8 pathcch rpcrt4 windowsapp)