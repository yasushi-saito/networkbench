cc_binary(
    name = "networkbench",
    srcs = ["tcp.cc", "tcp.h", "main.cc", "networkbench.h", "timer.h", "io_uring.h", "io_uring.cc", "util.cc", "util.h"],
    deps = [
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_github_gabime_spdlog//:spdlog",
        "@com_github_axboe_liburing//:liburing",
    ],
)
