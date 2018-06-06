# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /usr/local/cmake-3.9.2/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake-3.9.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/travis/build/Alkud/OTUS_HOMEWORK_10

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/travis/build/Alkud/OTUS_HOMEWORK_10

# Include any dependencies generated for this target.
include command_processor/CMakeFiles/command_processor.dir/depend.make

# Include the progress variables for this target.
include command_processor/CMakeFiles/command_processor.dir/progress.make

# Include the compile flags for this target's objects.
include command_processor/CMakeFiles/command_processor.dir/flags.make

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o: command_processor/CMakeFiles/command_processor.dir/flags.make
command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o: command_processor/input_processor.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_10/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/command_processor.dir/input_processor.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_processor.cpp

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/command_processor.dir/input_processor.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_processor.cpp > CMakeFiles/command_processor.dir/input_processor.cpp.i

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/command_processor.dir/input_processor.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_processor.cpp -o CMakeFiles/command_processor.dir/input_processor.cpp.s

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.requires:

.PHONY : command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.requires

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.provides: command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.requires
	$(MAKE) -f command_processor/CMakeFiles/command_processor.dir/build.make command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.provides.build
.PHONY : command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.provides

command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.provides.build: command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o


command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o: command_processor/CMakeFiles/command_processor.dir/flags.make
command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o: command_processor/input_reader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_10/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/command_processor.dir/input_reader.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_reader.cpp

command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/command_processor.dir/input_reader.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_reader.cpp > CMakeFiles/command_processor.dir/input_reader.cpp.i

command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/command_processor.dir/input_reader.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/input_reader.cpp -o CMakeFiles/command_processor.dir/input_reader.cpp.s

command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.requires:

.PHONY : command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.requires

command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.provides: command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.requires
	$(MAKE) -f command_processor/CMakeFiles/command_processor.dir/build.make command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.provides.build
.PHONY : command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.provides

command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.provides.build: command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o


command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o: command_processor/CMakeFiles/command_processor.dir/flags.make
command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o: command_processor/publisher_mt.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_10/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/command_processor.dir/publisher_mt.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/publisher_mt.cpp

command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/command_processor.dir/publisher_mt.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/publisher_mt.cpp > CMakeFiles/command_processor.dir/publisher_mt.cpp.i

command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/command_processor.dir/publisher_mt.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/publisher_mt.cpp -o CMakeFiles/command_processor.dir/publisher_mt.cpp.s

command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.requires:

.PHONY : command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.requires

command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.provides: command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.requires
	$(MAKE) -f command_processor/CMakeFiles/command_processor.dir/build.make command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.provides.build
.PHONY : command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.provides

command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.provides.build: command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o


# Object files for target command_processor
command_processor_OBJECTS = \
"CMakeFiles/command_processor.dir/input_processor.cpp.o" \
"CMakeFiles/command_processor.dir/input_reader.cpp.o" \
"CMakeFiles/command_processor.dir/publisher_mt.cpp.o"

# External object files for target command_processor
command_processor_EXTERNAL_OBJECTS =

command_processor/libcommand_processor.a: command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o
command_processor/libcommand_processor.a: command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o
command_processor/libcommand_processor.a: command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o
command_processor/libcommand_processor.a: command_processor/CMakeFiles/command_processor.dir/build.make
command_processor/libcommand_processor.a: command_processor/CMakeFiles/command_processor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_10/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library libcommand_processor.a"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && $(CMAKE_COMMAND) -P CMakeFiles/command_processor.dir/cmake_clean_target.cmake
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/command_processor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
command_processor/CMakeFiles/command_processor.dir/build: command_processor/libcommand_processor.a

.PHONY : command_processor/CMakeFiles/command_processor.dir/build

command_processor/CMakeFiles/command_processor.dir/requires: command_processor/CMakeFiles/command_processor.dir/input_processor.cpp.o.requires
command_processor/CMakeFiles/command_processor.dir/requires: command_processor/CMakeFiles/command_processor.dir/input_reader.cpp.o.requires
command_processor/CMakeFiles/command_processor.dir/requires: command_processor/CMakeFiles/command_processor.dir/publisher_mt.cpp.o.requires

.PHONY : command_processor/CMakeFiles/command_processor.dir/requires

command_processor/CMakeFiles/command_processor.dir/clean:
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor && $(CMAKE_COMMAND) -P CMakeFiles/command_processor.dir/cmake_clean.cmake
.PHONY : command_processor/CMakeFiles/command_processor.dir/clean

command_processor/CMakeFiles/command_processor.dir/depend:
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_10 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/travis/build/Alkud/OTUS_HOMEWORK_10 /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor /home/travis/build/Alkud/OTUS_HOMEWORK_10 /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor /home/travis/build/Alkud/OTUS_HOMEWORK_10/command_processor/CMakeFiles/command_processor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : command_processor/CMakeFiles/command_processor.dir/depend

