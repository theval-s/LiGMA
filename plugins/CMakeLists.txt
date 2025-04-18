file(GLOB PLUGINS "*.cpp" )

foreach(plugin_src ${PLUGINS})
    get_filename_component(name ${plugin_src} NAME_WE)
    add_library(${name} SHARED ${plugin_src})
    target_link_libraries(${name} Qt6::Core)
endforeach ()
