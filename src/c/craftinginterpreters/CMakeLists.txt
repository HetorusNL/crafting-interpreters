cmake_minimum_required(VERSION 3.25)
project(crafting-interpreters-c VERSION 0.1.0)

# add some default compile options
add_compile_options(-g3 -fmessage-length=0)
# add several warnings and make them errors
add_compile_options(-Wall -Werror -Wimplicit-fallthrough -Wpedantic -Wextra -Wconversion -Wsign-conversion)
# silence some of the warnings enabled above
add_compile_options(-Wno-unused-parameter)

# add the main files to the executable
add_executable(crafting-interpreters-c
    src/chunk.c
    src/compiler.c
    src/debug.c
    src/main.c
    src/memory.c
    src/scanner.c
    src/value.c
    src/vm.c
)

# add the include directories for the target executable
target_include_directories(crafting-interpreters-c PRIVATE
    ${CMAKE_SOURCE_DIR}
)
