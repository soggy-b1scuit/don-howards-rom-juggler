
add_rules("mode.debug", "mode.release")

set_runtimes("MT")

add_requires("xxhash")
add_requires("zlib-ng")
add_requires("vcpkg::wxwidgets")
add_requires("libopenmpt")
add_requires("soloud")


target("romjuggler")
    set_kind("binary")

    set_languages("c++23")

    add_packages("xxhash")
    add_packages("zlib-ng")
    add_packages("vcpkg::wxwidgets")
    add_packages("libopenmpt")
    add_packages("soloud")

    if is_plat("windows") then
        set_toolchains("clang")
        add_files("src/res.rc")
        add_syslinks("kernel32", "user32", "gdi32", "comdlg32", "comctl32", "advapi32", "shell32", "ole32", "oleaut32", "rpcrt4", "uuid", "winspool")
    end

    add_embeddirs("src/asset")
    add_includedirs("include")
    add_files("src/*.cpp")

    add_cxxflags("-flto", "-Os")
    add_ldflags("-static", "-Wl,--strip-all")