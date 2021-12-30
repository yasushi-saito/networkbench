genrule(
    name = "liburing_configure",
    srcs = glob(["**"]),
    outs = [
        "include/liburing.h",
        "include/liburing/barrier.h",
        "include/liburing/compat.h",
        "include/liburing/io_uring.h",
        "lib/liburing.a",
    ],
    cmd = (
        "SRC_DIR=$$(dirname $(location configure))&& echo XXX $$SRC_DIR YYY && " +
        "BUILD_DIR=/tmp/uring_build && " +
        "INSTALL_DIR=/tmp/uring_install && " +
        "DEST_DIR=$$(realpath $$(dirname $$(dirname $(location include/liburing.h)))) && " +
        "cp -r $$SRC_DIR $$BUILD_DIR && " +
        "cd $$BUILD_DIR && ./configure --prefix=$$INSTALL_DIR && " +
        "make -C src install && " +
        "mkdir -p $$DEST_DIR && " +
        "cp -r $$INSTALL_DIR/{include,lib} $$DEST_DIR"
    ),
)

cc_library(
    name = "liburing",
    hdrs = [
        "include/liburing.h",
        "include/liburing/barrier.h",
        "include/liburing/compat.h",
        "include/liburing/io_uring.h",
    ],
    includes = ["include"],
    srcs = ["lib/liburing.a"],
    visibility = ["//visibility:public"],
)
