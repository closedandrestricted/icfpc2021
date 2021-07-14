load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

pybind_library(
    name = "solver_common_lib",
    hdrs = glob(["common/**/*.h"]),
    include_prefix=".",
)

pybind_library(
    name = "solver_common",
    hdrs = [
        "common/icfpc2021/solver/full_search.h",
    ],
    deps = [
        ":solver_common_lib",
        "@json//:json",
        "@xtensor//:xtensor",
        "@boost//:container",
    ],
)

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
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "cli",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":solver_common_lib",
        ":solver_common",
        ":solver",
        "@gflags//:gflags",
    ]
)
