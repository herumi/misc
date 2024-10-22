const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const mkdir_bls_step = b.addSystemCommand(&.{
        "mkdir",
        "-p",
        "include/bls",
    });
    const mkdir_mcl_step = b.addSystemCommand(&.{
        "mkdir",
        "-p",
        "include/mcl",
    });
    const mkdir_lib_step = b.addSystemCommand(&.{
        "mkdir",
        "-p",
        "lib",
    });

    // Download header files
    const header_files = [_][]const u8{
        "bls/bls.h",
        "bls/bls384_256.h",
        "mcl/bn.h",
        "mcl/bn_c384_256.h",
        "mcl/curve_type.h",
    };

    const base_url = "https://raw.githubusercontent.com/herumi/bls-eth-go-binary/v1.36.1/bls/include/";

    // Create download steps for each header file
    var header_steps = std.ArrayList(*std.Build.Step.Run).init(b.allocator);
    for (header_files) |header| {
        const download_header = b.addSystemCommand(&.{
            "curl",
            "-L",
            b.fmt("{s}{s}", .{ base_url, header }),
            "-o",
            b.fmt("include/{s}", .{header}),
        });
        download_header.step.dependOn(&mkdir_mcl_step.step);
        download_header.step.dependOn(&mkdir_bls_step.step);
        download_header.step.dependOn(&mkdir_lib_step.step);
        header_steps.append(download_header) catch unreachable;
    }

    // Download the library file
    const lib_download_step = b.addSystemCommand(&.{
        "curl",
        "-L",
        "https://github.com/herumi/bls-eth-go-binary/raw/refs/tags/v1.36.1/bls/lib/linux/amd64/libbls384_256.a",
        "-o",
        "lib/libbls384_256.a",
    });

    // Make library download depend on header downloads
    for (header_steps.items) |header_step| {
        lib_download_step.step.dependOn(&header_step.step);
    }

    // Create a static library
    //    const lib = b.addStaticLibrary(.{
    //        .name = "bls384_256",
    //        .target = target,
    //        .optimize = optimize,
    //    });
    //
    //    // Add the downloaded library as a pre-compiled object
    //    lib.addObjectFile(b.path("libbls384_256.a"));
    //
    //    // Add include path
    //    lib.addIncludePath(b.path("include"));
    //
    //    // Make sure download happens before library compilation
    //    lib.step.dependOn(&lib_download_step.step);
    //
    //    // Make the library installable
    //    b.installArtifact(lib);

    // Install header files
    const install_headers = b.addInstallDirectory(.{
        .source_dir = b.path("include"),
        .install_dir = .header,
        .install_subdir = "",
    });

    install_headers.step.dependOn(&lib_download_step.step);

    // Create a test step
    const main_tests = b.addTest(.{
        .root_source_file = b.path("test.zig"),
        .target = target,
        .optimize = optimize,
    });

    const run_main_tests = b.addRunArtifact(main_tests);
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_main_tests.step);

    // Create executable
    const exe = b.addExecutable(.{
        .name = "bls-example",
        .root_source_file = b.path("main.zig"),
        .target = target,
        .optimize = optimize,
    });

    exe.step.dependOn(&lib_download_step.step);
    // Link with BLS library
    exe.addIncludePath(b.path("include"));
    //exe.addLibraryPath(b.path("lib"));
    exe.linkLibC();
    exe.linkSystemLibrary("stdc++");
    exe.addObjectFile(b.path("lib/libbls384_256.a"));

    // Make the executable installable
    b.installArtifact(exe);

    // Create a run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    const run_step = b.step("run", "Run the example");
    run_step.dependOn(&run_cmd.step);
}
