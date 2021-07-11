load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

pybind_library(
    name = "solver",
    hdrs = [
        "solver.h",
    ],
    deps = [
        "@json//:json",
        "@xtensor//:xtensor",
        "@boost//:container",
    ],
)

cc_binary(
    name = "cli",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":solver",
        "@gflags//:gflags",
    ]
)
