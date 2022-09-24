add_rules("mode.debug", "mode.release")
add_includedirs(".","net","xop","mongoose")
add_cxxflags("-shared -fPIC")
add_cflags("-shared -fPIC")
add_syslinks("dl", "pthread", "m")

target("etp_static_log")
    set_kind("static")
    add_files("log.cpp")
    on_install(function (target)end)
    on_uninstall(function (target)end)

target("deps")
    set_kind("static")
    add_deps("etp_static_log")
    add_files("net/*.cpp")
    add_files("xop/*.cpp")
    add_files("mongoose/*.c")
    add_files("modulesdk.cpp")
    on_install(function (target)end)
    on_uninstall(function (target)end)
    
target("etp_module_rtmp")
    set_kind("shared")
    add_deps("deps", "etp_static_log")
    set_filename("etp_module_rtmp.so")
    add_files("main.cpp")


