# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /tmp/MQTTSNPacket/samples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/MQTTSNPacket/samples

# Include any dependencies generated for this target.
include CMakeFiles/pub0sub1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pub0sub1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pub0sub1.dir/flags.make

CMakeFiles/pub0sub1.dir/pub0sub1.o: CMakeFiles/pub0sub1.dir/flags.make
CMakeFiles/pub0sub1.dir/pub0sub1.o: pub0sub1.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/MQTTSNPacket/samples/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/pub0sub1.dir/pub0sub1.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pub0sub1.dir/pub0sub1.o   -c /tmp/MQTTSNPacket/samples/pub0sub1.c

CMakeFiles/pub0sub1.dir/pub0sub1.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pub0sub1.dir/pub0sub1.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/MQTTSNPacket/samples/pub0sub1.c > CMakeFiles/pub0sub1.dir/pub0sub1.i

CMakeFiles/pub0sub1.dir/pub0sub1.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pub0sub1.dir/pub0sub1.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/MQTTSNPacket/samples/pub0sub1.c -o CMakeFiles/pub0sub1.dir/pub0sub1.s

CMakeFiles/pub0sub1.dir/pub0sub1.o.requires:

.PHONY : CMakeFiles/pub0sub1.dir/pub0sub1.o.requires

CMakeFiles/pub0sub1.dir/pub0sub1.o.provides: CMakeFiles/pub0sub1.dir/pub0sub1.o.requires
	$(MAKE) -f CMakeFiles/pub0sub1.dir/build.make CMakeFiles/pub0sub1.dir/pub0sub1.o.provides.build
.PHONY : CMakeFiles/pub0sub1.dir/pub0sub1.o.provides

CMakeFiles/pub0sub1.dir/pub0sub1.o.provides.build: CMakeFiles/pub0sub1.dir/pub0sub1.o


CMakeFiles/pub0sub1.dir/transport.o: CMakeFiles/pub0sub1.dir/flags.make
CMakeFiles/pub0sub1.dir/transport.o: transport.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/MQTTSNPacket/samples/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/pub0sub1.dir/transport.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pub0sub1.dir/transport.o   -c /tmp/MQTTSNPacket/samples/transport.c

CMakeFiles/pub0sub1.dir/transport.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pub0sub1.dir/transport.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/MQTTSNPacket/samples/transport.c > CMakeFiles/pub0sub1.dir/transport.i

CMakeFiles/pub0sub1.dir/transport.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pub0sub1.dir/transport.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/MQTTSNPacket/samples/transport.c -o CMakeFiles/pub0sub1.dir/transport.s

CMakeFiles/pub0sub1.dir/transport.o.requires:

.PHONY : CMakeFiles/pub0sub1.dir/transport.o.requires

CMakeFiles/pub0sub1.dir/transport.o.provides: CMakeFiles/pub0sub1.dir/transport.o.requires
	$(MAKE) -f CMakeFiles/pub0sub1.dir/build.make CMakeFiles/pub0sub1.dir/transport.o.provides.build
.PHONY : CMakeFiles/pub0sub1.dir/transport.o.provides

CMakeFiles/pub0sub1.dir/transport.o.provides.build: CMakeFiles/pub0sub1.dir/transport.o


# Object files for target pub0sub1
pub0sub1_OBJECTS = \
"CMakeFiles/pub0sub1.dir/pub0sub1.o" \
"CMakeFiles/pub0sub1.dir/transport.o"

# External object files for target pub0sub1
pub0sub1_EXTERNAL_OBJECTS =

pub0sub1: CMakeFiles/pub0sub1.dir/pub0sub1.o
pub0sub1: CMakeFiles/pub0sub1.dir/transport.o
pub0sub1: CMakeFiles/pub0sub1.dir/build.make
pub0sub1: CMakeFiles/pub0sub1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/MQTTSNPacket/samples/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable pub0sub1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pub0sub1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pub0sub1.dir/build: pub0sub1

.PHONY : CMakeFiles/pub0sub1.dir/build

CMakeFiles/pub0sub1.dir/requires: CMakeFiles/pub0sub1.dir/pub0sub1.o.requires
CMakeFiles/pub0sub1.dir/requires: CMakeFiles/pub0sub1.dir/transport.o.requires

.PHONY : CMakeFiles/pub0sub1.dir/requires

CMakeFiles/pub0sub1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pub0sub1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pub0sub1.dir/clean

CMakeFiles/pub0sub1.dir/depend:
	cd /tmp/MQTTSNPacket/samples && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/MQTTSNPacket/samples /tmp/MQTTSNPacket/samples /tmp/MQTTSNPacket/samples /tmp/MQTTSNPacket/samples /tmp/MQTTSNPacket/samples/CMakeFiles/pub0sub1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pub0sub1.dir/depend

