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
include modules/highgui/CMakeFiles/opencv_test_highgui.dir/depend.make

# Include the progress variables for this target.
include modules/highgui/CMakeFiles/opencv_test_highgui.dir/progress.make

# Include the compile flags for this target's objects.
include modules/highgui/CMakeFiles/opencv_test_highgui.dir/flags.make

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o: modules/highgui/CMakeFiles/opencv_test_highgui.dir/flags.make
modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o: modules/highgui/test/test_main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/boney/opencvprog/opencv-source/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -o CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o -c /home/boney/opencvprog/opencv-source/modules/highgui/test/test_main.cpp

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.i"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -E /home/boney/opencvprog/opencv-source/modules/highgui/test/test_main.cpp > CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.i

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.s"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -S /home/boney/opencvprog/opencv-source/modules/highgui/test/test_main.cpp -o CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.s

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.requires:
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.requires

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.provides: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.requires
	$(MAKE) -f modules/highgui/CMakeFiles/opencv_test_highgui.dir/build.make modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.provides.build
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.provides

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.provides.build: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o: modules/highgui/CMakeFiles/opencv_test_highgui.dir/flags.make
modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o: modules/highgui/test/test_gui.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/boney/opencvprog/opencv-source/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -o CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o -c /home/boney/opencvprog/opencv-source/modules/highgui/test/test_gui.cpp

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.i"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -E /home/boney/opencvprog/opencv-source/modules/highgui/test/test_gui.cpp > CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.i

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.s"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -include "/home/boney/opencvprog/opencv-source/modules/highgui/test_precomp.hpp"  -Winvalid-pch  -S /home/boney/opencvprog/opencv-source/modules/highgui/test/test_gui.cpp -o CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.s

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.requires:
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.requires

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.provides: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.requires
	$(MAKE) -f modules/highgui/CMakeFiles/opencv_test_highgui.dir/build.make modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.provides.build
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.provides

modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.provides.build: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o

# Object files for target opencv_test_highgui
opencv_test_highgui_OBJECTS = \
"CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o" \
"CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o"

# External object files for target opencv_test_highgui
opencv_test_highgui_EXTERNAL_OBJECTS =

bin/opencv_test_highgui: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o
bin/opencv_test_highgui: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o
bin/opencv_test_highgui: modules/highgui/CMakeFiles/opencv_test_highgui.dir/build.make
bin/opencv_test_highgui: lib/libopencv_ts.a
bin/opencv_test_highgui: lib/libopencv_highgui.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgcodecs.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_videoio.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_core.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgproc.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgcodecs.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_videoio.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_core.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgproc.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgcodecs.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_videoio.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_highgui.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_core.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgproc.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgcodecs.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_videoio.so.3.1.0
bin/opencv_test_highgui: 3rdparty/ippicv/unpack/ippicv_lnx/lib/intel64/libippicv.a
bin/opencv_test_highgui: lib/libopencv_imgcodecs.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_imgproc.so.3.1.0
bin/opencv_test_highgui: lib/libopencv_core.so.3.1.0
bin/opencv_test_highgui: modules/highgui/CMakeFiles/opencv_test_highgui.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../bin/opencv_test_highgui"
	cd /home/boney/opencvprog/opencv-source/modules/highgui && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/opencv_test_highgui.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
modules/highgui/CMakeFiles/opencv_test_highgui.dir/build: bin/opencv_test_highgui
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/build

modules/highgui/CMakeFiles/opencv_test_highgui.dir/requires: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_main.cpp.o.requires
modules/highgui/CMakeFiles/opencv_test_highgui.dir/requires: modules/highgui/CMakeFiles/opencv_test_highgui.dir/test/test_gui.cpp.o.requires
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/requires

modules/highgui/CMakeFiles/opencv_test_highgui.dir/clean:
	cd /home/boney/opencvprog/opencv-source/modules/highgui && $(CMAKE_COMMAND) -P CMakeFiles/opencv_test_highgui.dir/cmake_clean.cmake
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/clean

modules/highgui/CMakeFiles/opencv_test_highgui.dir/depend:
	cd /home/boney/opencvprog/opencv-source && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/highgui /home/boney/opencvprog/opencv-source /home/boney/opencvprog/opencv-source/modules/highgui /home/boney/opencvprog/opencv-source/modules/highgui/CMakeFiles/opencv_test_highgui.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/highgui/CMakeFiles/opencv_test_highgui.dir/depend

