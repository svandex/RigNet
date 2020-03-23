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