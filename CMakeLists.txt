cmake_minimum_required(VERSION 3.10)
project(MultiTargetProject VERSION 1.0)

set(CMAKE_C_COMPILER gcc)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Define source files for different targets
set(SOURCES_TARGET1
    src/main.c
    src/animations.c
    src/fft.c
    src/miniaudio.c
)

# Define the first executable target
add_executable(release ${SOURCES_TARGET1})
add_executable(debug ${SOURCES_TARGET1})

target_precompile_headers(release PRIVATE src/miniaudio.h)
target_precompile_headers(debug PRIVATE src/miniaudio.h)

# Define the second executable target


# Add include directories common to both targets
include_directories(
    /usr/local/include/notcurses
)

# Link libraries common to both targets
target_link_libraries(release
    -pthread
    -lm
    -lnotcurses
    -lnotcurses-core
#    -lunistring
#    -ltinfo
#    -lavcodec
#    -lavformat
#    -lavutil
)

target_link_libraries(debug
    -pthread
    -lm
    -lnotcurses
    -lnotcurses-core
)

# Set different compiler flags for each target
target_compile_options(release PRIVATE -Wall -Wextra -O2 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -I/usr/local/include -L/usr/local/lib -D_XOPEN_SOURCE=600)
target_compile_options(debug PRIVATE -Wall -Wextra -O2 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -I/usr/local/include -L/usr/local/lib -D_XOPEN_SOURCE=600 -ggdb -g)



# Default build type to Release if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
