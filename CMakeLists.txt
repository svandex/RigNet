cmake_minimum_required(VERSION 3.14)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /EHsc")

set(VCPKG_INSTALL_DIR "${VCPKG_ROOT}/installed/x64-windows/")

find_package(RapidJSON CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/IISModule
    ${PROJECT_SOURCE_DIR}/include/RigNetService
    $ENV{NIEXTCCOMPILERSUPP}/include
    ${RAPIDJSON_INCLUDE_DIRS}
    ${VCPKG_INSTALL_DIR}/include
    )

link_directories(
    $ENV{NIEXTCCOMPILERSUPP}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
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
add_library(RigNetModule SHARED src/IISModule/precomp.cpp src/Svandex.cpp src/IISModule/RigNetModule.cpp src/IISModule/RigNetMain.cpp)
target_compile_definitions(RigNetModule PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(RigNetModule PRIVATE "/Gz")
target_link_options(RigNetModule PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(RigNetModule PRIVATE "/GUARD:CF")
target_link_options(RigNetModule PRIVATE "/VERSION:1.00")
target_link_options(RigNetModule PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(RigNetModule PUBLIC pathcch rpcrt4 windowsapp sqlite3)
