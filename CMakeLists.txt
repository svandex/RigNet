cmake_minimum_required(VERSION 3.9)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /wd4267")

set(MYSQL_SOURCE_DIR "$ENV{USERPROFILE}\\Documents\\Repositories\\mysql-connector-cpp")
set(MYSQL_INSTALL_DIR "$ENV{USERPROFILE}\\Documents\\Softwares\\mysql-connector-cpp")
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

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DGLOG_NO_ABBREVIATED_SEVERITIES)
set(CMAKE_MFC_FLAG 2)

#Windows Service
add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/RigNet.cpp src/Svandex.cpp)
set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RigNetService mysqlcppconn8 boost_system-vc140-mt  pathcch glog)

#Test
add_executable(testapp testMain.cpp src/Svandex.cpp src/RigNet.cpp)
target_link_libraries(testapp mysqlcppconn8 boost_system-vc140-mt  pathcch glog)

#IIS MODULE 
add_library(ReqLMod SHARED src/IISModule/main.cpp src/IISModule/precomp.cpp src/IISModule/mymodule.cpp)
target_compile_options(ReqLMod PRIVATE "/Gz")
#string(APPEND DEF_DIR "/def:" ${PROJECT_SOURCE_DIR} "/data/iisdef.def")
#set_target_properties(ReqLMod PROPERTIES LINK_FLAGS ${DEF_DIR})
target_link_options(ReqLMod PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(ReqLMod PRIVATE "/GUARD:CF")
target_link_options(ReqLMod PRIVATE "/VERSION:10.00")
target_link_options(ReqLMod PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(ReqLMod glog::glog pathcch)