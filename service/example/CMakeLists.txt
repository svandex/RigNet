# Windows Service
# Get rigt-test data from NI card or Simens PLC, use IPC to interact with IIS Module
add_executable(RigNetService src/RigNetService/CppWindowsService.cpp src/RigNetService/RigNetService.cpp src/RigNetService/ServiceBase.cpp src/RigNetService/ServiceInstaller.cpp src/Svandex.cpp)
target_compile_definitions(RigNetService PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
target_compile_options(RigNetService PRIVATE "/permissive")
set_target_properties(RigNetService PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
target_link_libraries(RigNetService mysqlcppconn8 pathcch rpcrt4)