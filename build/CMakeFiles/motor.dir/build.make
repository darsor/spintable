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
CMAKE_SOURCE_DIR = /home/pi/spintable

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/spintable/build

# Include any dependencies generated for this target.
include CMakeFiles/motor.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/motor.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/motor.dir/flags.make

CMakeFiles/motor.dir/gps/gps.cpp.o: CMakeFiles/motor.dir/flags.make
CMakeFiles/motor.dir/gps/gps.cpp.o: ../gps/gps.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/spintable/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/motor.dir/gps/gps.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/motor.dir/gps/gps.cpp.o -c /home/pi/spintable/gps/gps.cpp

CMakeFiles/motor.dir/gps/gps.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/motor.dir/gps/gps.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/spintable/gps/gps.cpp > CMakeFiles/motor.dir/gps/gps.cpp.i

CMakeFiles/motor.dir/gps/gps.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/motor.dir/gps/gps.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/spintable/gps/gps.cpp -o CMakeFiles/motor.dir/gps/gps.cpp.s

CMakeFiles/motor.dir/gps/gps.cpp.o.requires:
.PHONY : CMakeFiles/motor.dir/gps/gps.cpp.o.requires

CMakeFiles/motor.dir/gps/gps.cpp.o.provides: CMakeFiles/motor.dir/gps/gps.cpp.o.requires
	$(MAKE) -f CMakeFiles/motor.dir/build.make CMakeFiles/motor.dir/gps/gps.cpp.o.provides.build
.PHONY : CMakeFiles/motor.dir/gps/gps.cpp.o.provides

CMakeFiles/motor.dir/gps/gps.cpp.o.provides.build: CMakeFiles/motor.dir/gps/gps.cpp.o

CMakeFiles/motor.dir/cosmos/cosmos.cpp.o: CMakeFiles/motor.dir/flags.make
CMakeFiles/motor.dir/cosmos/cosmos.cpp.o: ../cosmos/cosmos.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/spintable/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/motor.dir/cosmos/cosmos.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/motor.dir/cosmos/cosmos.cpp.o -c /home/pi/spintable/cosmos/cosmos.cpp

CMakeFiles/motor.dir/cosmos/cosmos.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/motor.dir/cosmos/cosmos.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/spintable/cosmos/cosmos.cpp > CMakeFiles/motor.dir/cosmos/cosmos.cpp.i

CMakeFiles/motor.dir/cosmos/cosmos.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/motor.dir/cosmos/cosmos.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/spintable/cosmos/cosmos.cpp -o CMakeFiles/motor.dir/cosmos/cosmos.cpp.s

CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.requires:
.PHONY : CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.requires

CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.provides: CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.requires
	$(MAKE) -f CMakeFiles/motor.dir/build.make CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.provides.build
.PHONY : CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.provides

CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.provides.build: CMakeFiles/motor.dir/cosmos/cosmos.cpp.o

CMakeFiles/motor.dir/motorpi.cpp.o: CMakeFiles/motor.dir/flags.make
CMakeFiles/motor.dir/motorpi.cpp.o: ../motorpi.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/spintable/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/motor.dir/motorpi.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/motor.dir/motorpi.cpp.o -c /home/pi/spintable/motorpi.cpp

CMakeFiles/motor.dir/motorpi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/motor.dir/motorpi.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/spintable/motorpi.cpp > CMakeFiles/motor.dir/motorpi.cpp.i

CMakeFiles/motor.dir/motorpi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/motor.dir/motorpi.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/spintable/motorpi.cpp -o CMakeFiles/motor.dir/motorpi.cpp.s

CMakeFiles/motor.dir/motorpi.cpp.o.requires:
.PHONY : CMakeFiles/motor.dir/motorpi.cpp.o.requires

CMakeFiles/motor.dir/motorpi.cpp.o.provides: CMakeFiles/motor.dir/motorpi.cpp.o.requires
	$(MAKE) -f CMakeFiles/motor.dir/build.make CMakeFiles/motor.dir/motorpi.cpp.o.provides.build
.PHONY : CMakeFiles/motor.dir/motorpi.cpp.o.provides

CMakeFiles/motor.dir/motorpi.cpp.o.provides.build: CMakeFiles/motor.dir/motorpi.cpp.o

CMakeFiles/motor.dir/motor/dcmotor.cpp.o: CMakeFiles/motor.dir/flags.make
CMakeFiles/motor.dir/motor/dcmotor.cpp.o: ../motor/dcmotor.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/spintable/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/motor.dir/motor/dcmotor.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/motor.dir/motor/dcmotor.cpp.o -c /home/pi/spintable/motor/dcmotor.cpp

CMakeFiles/motor.dir/motor/dcmotor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/motor.dir/motor/dcmotor.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/spintable/motor/dcmotor.cpp > CMakeFiles/motor.dir/motor/dcmotor.cpp.i

CMakeFiles/motor.dir/motor/dcmotor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/motor.dir/motor/dcmotor.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/spintable/motor/dcmotor.cpp -o CMakeFiles/motor.dir/motor/dcmotor.cpp.s

CMakeFiles/motor.dir/motor/dcmotor.cpp.o.requires:
.PHONY : CMakeFiles/motor.dir/motor/dcmotor.cpp.o.requires

CMakeFiles/motor.dir/motor/dcmotor.cpp.o.provides: CMakeFiles/motor.dir/motor/dcmotor.cpp.o.requires
	$(MAKE) -f CMakeFiles/motor.dir/build.make CMakeFiles/motor.dir/motor/dcmotor.cpp.o.provides.build
.PHONY : CMakeFiles/motor.dir/motor/dcmotor.cpp.o.provides

CMakeFiles/motor.dir/motor/dcmotor.cpp.o.provides.build: CMakeFiles/motor.dir/motor/dcmotor.cpp.o

CMakeFiles/motor.dir/motor/pwm.cpp.o: CMakeFiles/motor.dir/flags.make
CMakeFiles/motor.dir/motor/pwm.cpp.o: ../motor/pwm.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/spintable/build/CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/motor.dir/motor/pwm.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/motor.dir/motor/pwm.cpp.o -c /home/pi/spintable/motor/pwm.cpp

CMakeFiles/motor.dir/motor/pwm.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/motor.dir/motor/pwm.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/spintable/motor/pwm.cpp > CMakeFiles/motor.dir/motor/pwm.cpp.i

CMakeFiles/motor.dir/motor/pwm.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/motor.dir/motor/pwm.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/spintable/motor/pwm.cpp -o CMakeFiles/motor.dir/motor/pwm.cpp.s

CMakeFiles/motor.dir/motor/pwm.cpp.o.requires:
.PHONY : CMakeFiles/motor.dir/motor/pwm.cpp.o.requires

CMakeFiles/motor.dir/motor/pwm.cpp.o.provides: CMakeFiles/motor.dir/motor/pwm.cpp.o.requires
	$(MAKE) -f CMakeFiles/motor.dir/build.make CMakeFiles/motor.dir/motor/pwm.cpp.o.provides.build
.PHONY : CMakeFiles/motor.dir/motor/pwm.cpp.o.provides

CMakeFiles/motor.dir/motor/pwm.cpp.o.provides.build: CMakeFiles/motor.dir/motor/pwm.cpp.o

# Object files for target motor
motor_OBJECTS = \
"CMakeFiles/motor.dir/gps/gps.cpp.o" \
"CMakeFiles/motor.dir/cosmos/cosmos.cpp.o" \
"CMakeFiles/motor.dir/motorpi.cpp.o" \
"CMakeFiles/motor.dir/motor/dcmotor.cpp.o" \
"CMakeFiles/motor.dir/motor/pwm.cpp.o"

# External object files for target motor
motor_EXTERNAL_OBJECTS =

motor: CMakeFiles/motor.dir/gps/gps.cpp.o
motor: CMakeFiles/motor.dir/cosmos/cosmos.cpp.o
motor: CMakeFiles/motor.dir/motorpi.cpp.o
motor: CMakeFiles/motor.dir/motor/dcmotor.cpp.o
motor: CMakeFiles/motor.dir/motor/pwm.cpp.o
motor: CMakeFiles/motor.dir/build.make
motor: CMakeFiles/motor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable motor"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/motor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/motor.dir/build: motor
.PHONY : CMakeFiles/motor.dir/build

CMakeFiles/motor.dir/requires: CMakeFiles/motor.dir/gps/gps.cpp.o.requires
CMakeFiles/motor.dir/requires: CMakeFiles/motor.dir/cosmos/cosmos.cpp.o.requires
CMakeFiles/motor.dir/requires: CMakeFiles/motor.dir/motorpi.cpp.o.requires
CMakeFiles/motor.dir/requires: CMakeFiles/motor.dir/motor/dcmotor.cpp.o.requires
CMakeFiles/motor.dir/requires: CMakeFiles/motor.dir/motor/pwm.cpp.o.requires
.PHONY : CMakeFiles/motor.dir/requires

CMakeFiles/motor.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/motor.dir/cmake_clean.cmake
.PHONY : CMakeFiles/motor.dir/clean

CMakeFiles/motor.dir/depend:
	cd /home/pi/spintable/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/spintable /home/pi/spintable /home/pi/spintable/build /home/pi/spintable/build /home/pi/spintable/build/CMakeFiles/motor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/motor.dir/depend

