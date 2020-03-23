# iissqlite module
 
add_library(iissqlite SHARED iissqlite.cpp main.cpp ${BASIC_FILES})
set_property(TARGET iissqlite PROPERTY CXX_STANDARD 11)
target_compile_definitions(iissqlite PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN)
target_compile_options(iissqlite PRIVATE "/Gz")
target_link_options(iissqlite PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iissqlite PRIVATE "/GUARD:CF")
target_link_options(iissqlite PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(iissqlite PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)