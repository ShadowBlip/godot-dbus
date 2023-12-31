#!/usr/bin/env python
from SCons import __version__ as scons_raw_version
import os
import sys

ext_name = "dbus"

env = SConscript("godot-cpp/SConstruct")

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

# Include dependency libraries for dbus
env.Append(LIBS=["dbus-1"])
env.Append(
    CXXFLAGS=[
        "-I/usr/include/dbus-1.0",
        "-I/usr/lib/dbus-1.0/include",
        "-I/usr/lib/x86_64-linux-gnu/dbus-1.0/include",
    ]
)

# Generating the compilation DB (`compile_commands.json`) requires SCons 4.0.0 or later.

scons_ver = env._get_major_minor_revision(scons_raw_version)
if scons_ver < (4, 0, 0):
    print(
        "The `compiledb=yes` option requires SCons 4.0 or later, but your version is %s."
        % scons_raw_version
    )
    Exit(255)

env.Tool("compilation_db")
env.Alias("compiledb", env.CompilationDatabase())


# Build the shared library
library = env.SharedLibrary(
    "addons/{}/bin/lib{}{}{}".format(
        ext_name, ext_name, env["suffix"], env["SHLIBSUFFIX"]
    ),
    source=sources,
)

Default(library)
