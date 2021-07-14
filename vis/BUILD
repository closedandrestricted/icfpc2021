cc_binary(
    name = "vis",
    srcs = [
        "vis.cc",
        "vis.h",
    ],
    linkopts = ["-lvulkan", "-lpthread", "-ldl"],
    deps = [
        "@imgui//:imgui",
        "@implot//:implot",
        "@gflags//:gflags",
        "//solver:solver",
    ],
)
