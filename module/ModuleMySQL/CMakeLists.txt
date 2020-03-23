
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