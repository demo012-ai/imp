# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/boney/opencvprog/opencv-source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/boney/opencvprog/opencv-source

# Utility rule file for pch_Generate_opencv_ts.

# Include the progress variables for this target.
include modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/progress.make

modules/ts/CMakeFiles/pch_Generate_opencv_ts: modules/ts/precomp.hpp.gch/opencv_ts_Release.gch

modules/ts/precomp.hpp.gch/opencv_ts_Release.gch: modules/ts/src/precomp.hpp
modules/ts/precomp.hpp.gch/opencv_ts_Release.gch: modules/ts/precomp.hpp
modules/ts/precomp.hpp.gch/opencv_ts_Release.gch: lib/libopencv_ts_pch_dephelp.a
	$(CMAKE_COMMAND) -E cmake_progress_report /home/boney/opencvprog/opencv-source/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating precomp.hpp.gch/opencv_ts_Release.gch"
	cd /home/boney/opencvprog/opencv-source/modules/ts && /usr/bin/cmake -E make_directory /home/boney/opencvprog/opencv-source/modules/ts/precomp.hpp.gch
	cd /home/boney/opencvprog/opencv-source/modules/ts && /usr/bin/c++ -O3 -DNDEBUG -DNDEBUG "-D__OPENCV_BUILD=1" -I"/home/boney/opencvprog/opencv-source/3rdparty/ippicv/unpack/ippicv_lnx/include" -I"/home/boney/opencvprog/opencv-source" -I"/home/boney/opencvprog/opencv-source/3rdparty/ippicv/unpack/ippicv_lnx/include" -I"/home/boney/opencvprog/opencv-source" -I"/home/boney/opencvprog/opencv-source/modules/ts/include" -I"/home/boney/opencvprog/opencv-source/modules/ts/src" -I"/home/boney/opencvprog/opencv-source/modules/ts" -I"/home/boney/opencvprog/opencv-source/modules/core/include" -I"/home/boney/opencvprog/opencv-source/modules/imgproc/include" -I"/home/boney/opencvprog/opencv-source/modules/imgcodecs/include" -I"/home/boney/opencvprog/opencv-source/modules/videoio/include" -I"/home/boney/opencvprog/opencv-source/modules/highgui/include" -fsigned-char -W -Wall -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -Wformat -Werror=format-security -Wmissing-declarations -Winit-self -Wpointer-arith -Wshadow -Wsign-promo -Wno-narrowing -Wno-delete-non-virtual-dtor -Wno-comment -fdiagnostics-show-option -Wno-long-long -pthread -fomit-frame-pointer -msse -msse2 -mno-avx -msse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -ffunction-sections -fvisibility=hidden -fvisibility-inlines-hidden -Wno-undef -x c++-header -o /home/boney/opencvprog/opencv-source/modules/ts/precomp.hpp.gch/opencv_ts_Release.gch /home/boney/opencvprog/opencv-source/modules/ts/precomp.hpp

modules/ts/precomp.hpp: modules/ts/src/precomp.hpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/boney/opencvprog/opencv-source/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating precomp.hpp"
	cd /home/boney/opencvprog/opencv-source/modules/ts && /usr/bin/cmake -E copy_if_different /home/boney/opencvprog/opencv-source/modules/ts/src/precomp.hpp /home/boney/opencvprog/opencv-source/modules/ts/precomp.hpp

pch_Generate_opencv_ts: modules/ts/CMakeFiles/pch_Generate_opencv_ts
pch_Generate_opencv_ts: modules/ts/precomp.hpp.gch/opencv_ts_Release.gch
pch_Generate_opencv_ts: modules/ts/precomp.hpp
pch_Generate_opencv_ts: modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/build.make
.PHONY : pch_Generate_opencv_ts

# Rule to build all files generated by this target.
modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/build: pch_Generate_opencv_ts
.PHONY : modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/build

modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/clean:
	cd /home/boney/opencvprog/opencv-source/modules/ts && $(CMAKE_COMMAND) -P CMakeFiles/pch_Generate_opencv_ts.dir/cmake_clean.cmake
.PHONY : modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/clean

modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/depend:
	cd /home/boney/opencvprog/opencv-source && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/ts /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/ts /home/boney/opencvprog/opencv-source/modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/ts/CMakeFiles/pch_Generate_opencv_ts.dir/depend

