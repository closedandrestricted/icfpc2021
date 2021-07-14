cc_library(
    name = "imgui",
    hdrs = [
        "imgui.h",
        "imgui_internal.h",
        "imconfig.h",
        "imstb_textedit.h",
        "imstb_rectpack.h",
        "imstb_truetype.h",
        "backends/imgui_impl_sdl.h",
        "backends/imgui_impl_vulkan.h",
    ],
    srcs = [
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "backends/imgui_impl_sdl.cpp",
        "backends/imgui_impl_vulkan.cpp",
    ],
    deps = [
        "@sdl//:sdl",
    ],
    visibility = ["//visibility:public"],
)