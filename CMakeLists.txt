cmake_minimum_required(VERSION 2.17)

project(kernell)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

set(CMAKE_CXX_STANDAED 17)

set(KERNEL_BIN "x86_64-kernel.bin")

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/arch/x86_64/linker.ld)
set(GRUB_CFG ${PROJECT_SOURCE_DIR}/src/arch/x86_64/grub.cfg)

set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-red-zone -mcmodel=large -mno-sse -ffreestanding -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-red-zone -mcmodel=large -mno-sse -ffreestanding -nostdlib")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_ASM_FLAGS "{CMAKE_ASM_FLAGS} -felf64")
set(CMAKE_LINKER_FLAGS "-n -T ${LINKER_SCRIPT}")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_CXX_COMPILER "/usr/bin/gcc")
set(CMAKE_C_COMPILER "/usr/bin/g++")
set(CMAKE_LINKER "/usr/bin/ld")

set(CMAKE_VERBOSE_MAKEFILE ON)

enable_language(ASM_NASM)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_executable(
    ${KERNEL_BIN}

    # Header files
    include/arch/x86_64/kernel_entry.h
    include/arch/x86_64/vga_text.h
    include/arch/x86_64/multiboot2.h

    # Source files
    src/arch/x86_64/kernel_entry.cpp
    src/arch/x86_64/multiboot2_boot32.asm
    src/arch/x86_64/multiboot2_boot64.asm
    src/arch/x86_64/multiboot2_header.asm
)

set_target_properties(${KERNEL_BIN} PROPERTIES CMAKE_LINKER "/usr/bin/ld")

add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/build/isofiles/boot/grub
    COMMAND cp ${KERNEL_BIN} ${CMAKE_BINARY_DIR}/build/isofiles/boot/${KERNEL_BIN}
    COMMAND cp ${GRUB_CFG} ${CMAKE_BINARY_DIR}/build/isofiles/boot/grub
    COMMAND grub-mkrescue -o kernel.iso ${CMAKE_BINARY_DIR}/build/isofiles
    COMMAND rm -r ${CMAKE_BINARY_DIR}/build/
)