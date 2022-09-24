add_rules("mode.debug", "mode.release")
add_includedirs(".","net","xop","mongoose")
add_cxxflags("-shared -fPIC")
add_cflags("-shared -fPIC")
add_syslinks("dl", "pthread", "m")

target("deps")
    set_kind("static")
    add_files("net/*.cpp")
    add_files("xop/*.cpp")
    add_files("mongoose/*.c")
    add_files("./modulesdk.cpp")
    on_install(function (target)                         end)                                                 on_uninstall(function (target)                       end)
                                                on_uninstall(function (target)                       end)

target("etp_module_rtmp")
    set_kind("shared")
    add_deps("deps")
    set_filename("etp_module_rtmp.so")
    add_files("main.cpp","log.c")


