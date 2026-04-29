#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/", "src/thirdparty/kissfft/"])

# Collect C++ sources
sources = Glob("src/*.cpp")
sources += Glob("src/thirdparty/kissfft/kiss_fft.c")

# if env["target"] in ["editor", "template_debug"]:
#     try:
#         doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
#         sources.append(doc_data)
#     except AttributeError:
#         print("Not including class reference as we're targeting a pre-4.3 baseline.")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "addons/audio_visualizer/bin/libaudio_visualizer.{}.{}.framework/libaudio_visualizer.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "addons/audio_visualizer/bin/libaudio_visualizer.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "addons/audio_visualizer/bin/libaudio_visualizer.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )

elif env["platform"] == "android":
    library = env.SharedLibrary(
        "addons/audio_visualizer/bin/libaudio_visualizer{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

else:
    library = env.SharedLibrary(
        "addons/audio_visualizer/bin/audio_visualizer{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
