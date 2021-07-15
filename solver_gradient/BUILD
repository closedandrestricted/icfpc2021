load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

cc_binary(
    name = "solver_gradient",
    srcs = [
        "main.cpp",
    ],
    deps = [
        "//solver:solver",
        "@gflags//:gflags",
        "@json//:json",
        "@boost//:container",
    ],
    copts = select({"@bazel_tools//src/conditions:windows": ["/std:c++17"], "//conditions:default": []}),
)
