# iistvmgt module
# 
add_library(${ModuleName} SHARED ${ModuleName}.cpp main.cpp ${BASE_SRCS})
set_property(TARGET ${ModuleName} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${ModuleName} PRIVATE UNICODE PRIVATE _UNICODE PRIVATE WIN32_LEAN_AND_MEAN PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(${ModuleName} PRIVATE "/Gz")
target_include_directories(${ModuleName} 
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PUBLIC ${PROJECT_SOURCE_DIR}/src/
    PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty/svandex/
)
target_link_options(${ModuleName} PRIVATE "/SUBSYSTEM:CONSOLE,10.00")
target_link_options(${ModuleName} PRIVATE "/GUARD:CF")
target_link_options(${ModuleName} PRIVATE "/EXPORT:RegisterModule")
target_link_libraries(${ModuleName} PUBLIC pathcch rpcrt4 windowsapp SQLiteCpp)

#add_custom_command(TARGET ${ModuleName} PRE_BUILD COMMAND powershell ARGS ${PROJECT_SOURCE_DIR}/tool/ModuleManagement.ps1 -UninstallModuleName ${ModuleName})
add_custom_target("Before_Building_${ModuleName}" COMMAND powershell ${PROJECT_SOURCE_DIR}/tool/ModuleManagement.ps1 -UninstallModuleName ${ModuleName})

add_custom_command(TARGET ${ModuleName} POST_BUILD COMMAND powershell ARGS ${PROJECT_SOURCE_DIR}/tool/ModuleManagement.ps1 -InstallModuleName ${ModuleName} -ModulePath ${PROJECT_BINARY_DIR}/module/${ModuleName}/${ModuleName}.dll) 

add_dependencies(${ModuleName} "Before_Building_${ModuleName}")