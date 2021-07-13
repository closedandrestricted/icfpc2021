load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

cmake(
    name = "sdl",
    cache_entries = {
    },
    build_args = ["-j24"],
    lib_source = ":all_srcs",
    out_static_libs = ["libSDL2.a"],
    out_include_dir = "include/SDL2",
    visibility = ["//visibility:public"],
)