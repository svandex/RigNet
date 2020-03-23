# iistvmgt module
# 
add_library(iistvmgt SHARED src/iistvmgt/iistvmgt.cpp src/iistvmgt/main.cpp ${BASIC_FILES})
set_property(TARGET iistvmgt PROPERTY CXX_STANDARD 11)
target_compile_definitions(iistvmgt PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(iistvmgt PRIVATE "/Gz")
target_link_options(iistvmgt PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(iistvmgt PRIVATE "/GUARD:CF")
target_link_options(iistvmgt PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(iistvmgt PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)