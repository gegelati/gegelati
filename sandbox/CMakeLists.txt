# Sandbox project CMakeLists.txt
cmake_minimum_required(VERSION 3.8...4.0)

# Create project sandbox 
set(SANDBOX_PROJECT_NAME sandbox)

# Create one executable with the source from the sandbox directory
# Find all source files in the sandbox directory and add them to the executable
file(GLOB_RECURSE SANDBOX_SOURCES *.cpp)

add_executable(${SANDBOX_PROJECT_NAME} ${SANDBOX_SOURCES})

# Link to GEGELATI
target_link_libraries(${SANDBOX_PROJECT_NAME} ${PROJECT_NAME}::${PROJECT_NAME})
