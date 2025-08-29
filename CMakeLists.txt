# Set the minimum version of CMake required for modern features
cmake_minimum_required(VERSION 3.14)

# Define the project name and language
project(ASSET CENTRAL LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the FetchContent module for downloading dependencies
include(FetchContent)

# --- Fetch and include xlnt library ---
FetchContent_Declare(
  xlnt
  GIT_REPOSITORY https://github.com/tfussell/xlnt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(xlnt)

# --- Fetch and include tabulate library ---
FetchContent_Declare(
  tabulate
  GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
  GIT_TAG master
)
FetchContent_MakeAvailable(tabulate)

# Define all the source files in the project
set(SOURCE_FILES
    src/main.cpp
    src/User.cpp
    src/Stock.cpp
    src/ExcelUtil.cpp
    src/DisplayUtil.cpp
    src/Receipt.cpp
)

# Create the executable with a descriptive name
add_executable(ASSET CENTRAL ${SOURCE_FILES})

# Tell the compiler where to find the project's header files
target_include_directories(ASSET CENTRAL PRIVATE include)

# Link the libraries to the executable
target_link_libraries(ASSET CENTRAL PRIVATE xlnt tabulate)

# IMPORTANT: Fix the path for copying the DLL file
# This command ensures that the xlnt.dll is copied to the executable's directory
add_custom_command(TARGET ASSET CENTRAL POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "$<TARGET_FILE_DIR:xlnt>/xlnt.dll"
  $<TARGET_FILE_DIR:ASSET CENTRAL>
)
