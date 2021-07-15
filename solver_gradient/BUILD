load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

cc_binary(
    name = "solver_gradioent",
    srcs = [
        "main.cpp",
    ],
    deps = [
        "@solver//:solver",
        "@gflags//:gflags",
    ]
)
