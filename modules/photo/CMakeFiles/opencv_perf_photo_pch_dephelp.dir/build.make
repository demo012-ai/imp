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

# Include any dependencies generated for this target.
include modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/depend.make

# Include the progress variables for this target.
include modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/progress.make

# Include the compile flags for this target's objects.
include modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/flags.make

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/flags.make
modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o: modules/photo/opencv_perf_photo_pch_dephelp.cxx
	$(CMAKE_COMMAND) -E cmake_progress_report /home/boney/opencvprog/opencv-source/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o"
	cd /home/boney/opencvprog/opencv-source/modules/photo && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o -c /home/boney/opencvprog/opencv-source/modules/photo/opencv_perf_photo_pch_dephelp.cxx

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.i"
	cd /home/boney/opencvprog/opencv-source/modules/photo && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/boney/opencvprog/opencv-source/modules/photo/opencv_perf_photo_pch_dephelp.cxx > CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.i

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.s"
	cd /home/boney/opencvprog/opencv-source/modules/photo && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/boney/opencvprog/opencv-source/modules/photo/opencv_perf_photo_pch_dephelp.cxx -o CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.s

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.requires:
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.requires

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.provides: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.requires
	$(MAKE) -f modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/build.make modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.provides.build
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.provides

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.provides.build: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o

# Object files for target opencv_perf_photo_pch_dephelp
opencv_perf_photo_pch_dephelp_OBJECTS = \
"CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o"

# External object files for target opencv_perf_photo_pch_dephelp
opencv_perf_photo_pch_dephelp_EXTERNAL_OBJECTS =

lib/libopencv_perf_photo_pch_dephelp.a: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o
lib/libopencv_perf_photo_pch_dephelp.a: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/build.make
lib/libopencv_perf_photo_pch_dephelp.a: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library ../../lib/libopencv_perf_photo_pch_dephelp.a"
	cd /home/boney/opencvprog/opencv-source/modules/photo && $(CMAKE_COMMAND) -P CMakeFiles/opencv_perf_photo_pch_dephelp.dir/cmake_clean_target.cmake
	cd /home/boney/opencvprog/opencv-source/modules/photo && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/opencv_perf_photo_pch_dephelp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/build: lib/libopencv_perf_photo_pch_dephelp.a
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/build

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/requires: modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/opencv_perf_photo_pch_dephelp.cxx.o.requires
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/requires

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/clean:
	cd /home/boney/opencvprog/opencv-source/modules/photo && $(CMAKE_COMMAND) -P CMakeFiles/opencv_perf_photo_pch_dephelp.dir/cmake_clean.cmake
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/clean

modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/depend:
	cd /home/boney/opencvprog/opencv-source && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/photo /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/photo /home/boney/opencvprog/opencv-source/modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/photo/CMakeFiles/opencv_perf_photo_pch_dephelp.dir/depend

