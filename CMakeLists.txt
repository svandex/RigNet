cmake_minimum_required(VERSION 3.9)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()
project(RigNet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4267")

set(MY_DOCUMENTS_DIR "C:\\Users\\svandex\\Documents")

set(NIDAQMX_INSTALL_DIR "C:\\Program Files (x86)\\National Instruments\\Shared\\ExternalCompilerSupport\\C")
#set(mysql_wrapper_dir "C:\\Program Files\\MySQL\\Connector C++ 8.0\\")
set(MYSQL_SOURCE_DIR "C:\\Users\\saictv\\Documents\\Repositories\\mysql-connector-cpp")
set(MYSQL_BIN_DIR "C:\\Users\\saictv\\Documents\\Softwares\\mysql-connector-cpp")
set(VCPKG_INSTALL_DIR "${MY_DOCUMENTS_DIR}/github/vcpkg/installed/x64-windows/")

find_package(glog CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/RigNetService
	${NIDAQMX_INSTALL_DIR}/include
    ${PROJECT_SOURCE_DIR}/../rapidjson/include
    ${PROJECT_SOURCE_DIR}/../websocketpp
    ${VCPKG_INSTALL_DIR}/include
	${MYSQL_SOURCE_DIR}/include
    )

link_directories(
	${MYSQL_BIN_DIR}
    ${NIDAQMX_INSTALL_DIR}/lib64/msvc
    ${VCPKG_INSTALL_DIR}/lib
    )

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DGLOG_NO_ABBREVIATED_SEVERITIES)
set(CMAKE_MFC_FLAG 2)
add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/RigNet.cpp src/Svandex.cpp)
set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RigNetService mysqlcppconn8 boost_system-vc140-mt  pathcch glog::glog)

#add_executable(testSvandex testSvandex.cpp src/Svandex.cpp)
#target_link_libraries(testSvandex pathcch)
add_executable(testapp testMain.cpp src/Svandex.cpp src/RigNet.cpp)
target_link_libraries(testapp mysqlcppconn8 boost_system-vc140-mt  pathcch)

#IIS MODULE 
string(APPEND DEF_DIR "/def:" ${PROJECT_SOURCE_DIR} "/data/iisdef.def")
add_library(gModuleTest SHARED src/IISModule/globalmodule.cpp src/Svandex.cpp)
target_compile_options(gModuleTest PRIVATE "/Gz")
set_target_properties(gModuleTest PROPERTIES LINK_FLAGS ${DEF_DIR})
target_link_libraries(gModuleTest glog::glog pathcch)