workspace(name = "networkbench")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_absl",
    sha256 = "fde167d2238376278bdbf0081b42e9feb34fad11a6a77642c7dde64596a65aff",
    strip_prefix = "abseil-cpp-04610889a913d29037205ca72e9d7fd7acc925fe",
    urls = ["https://github.com/abseil/abseil-cpp/archive/04610889a913d29037205ca72e9d7fd7acc925fe.zip"],
)

http_archive(
    name = "com_github_fmtlib_fmt",
    build_file = "//bazel:fmtlib.BUILD",
    sha256 = "bec489fad190689659b07183ab3e14950a3bc3b4ad64c7859b393e47adbefc1c",
    strip_prefix = "fmt-5aa5c9873877b79c2588d5f33ad9910b02488b00",
    urls = ["https://github.com/fmtlib/fmt/archive/5aa5c9873877b79c2588d5f33ad9910b02488b00.zip"],
)

http_archive(
    name = "com_github_gabime_spdlog",
    build_file = "//bazel:spdlog.BUILD",
    sha256 = "f0114a4d3c88be9e696762f37a7c379619443ce9d668546c61b21d41affe5b62",
    strip_prefix = "spdlog-1.7.0",
    urls = ["https://github.com/gabime/spdlog/archive/v1.7.0.tar.gz"],
)

http_archive(
    name = "com_github_axboe_liburing",
    build_file = "//bazel:liburing.BUILD",
    sha256 = "40ce5797ed6d58cb7f79b6baad94315aebff75a20bde1aedfdda1711f36ced5e",
    strip_prefix = "liburing-515e03ef1a82e1b866209756bb18bea053988c9d",
    urls = ["https://github.com/axboe/liburing/archive/515e03ef1a82e1b866209756bb18bea053988c9d.zip"],
)
