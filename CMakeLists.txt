cmake_minimum_required(VERSION 2.8)

project(kernell)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

set(CMAKE_CXX_STANDAED 17)

set(KERNEL_BIN "x86_64-kernel.bin")

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/arch/x86_64/linker.ld)
set(GRUB_CFG ${PROJECT_SOURCE_DIR}/src/arch/x86_64/grub.cfg)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-red-zone -mcmodel=large -mno-sse -ffreestanding -fno-exceptions -fno-rtti -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-red-zone -mcmodel=large -mno-sse -ffreestanding -nostdlib -fno-pic -fno-pie")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -felf64")
set(CMAKE_LINKER_FLAGS "-n -T ${LINKER_SCRIPT}")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_LINKER "/usr/bin/ld")

set(CMAKE_VERBOSE_MAKEFILE ON)

enable_language(ASM_NASM)

file(GLOB_RECURSE kernel_SRC "*.h" "*.c")
file(GLOB_RECURSE kernel_ASM "*.asm" "*.S" "*.s")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arch/x86_64
    ${CMAKE_CURRENT_SOURCE_DIR}/include/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/include/util
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sync
    ${CMAKE_CURRENT_SOURCE_DIR}/include/process
    ${CMAKE_CURRENT_SOURCE_DIR}/include/thread
)

add_executable(
    ${KERNEL_BIN}

    ${kernel_SRC}
    ${kernel_ASM}
)

set_target_properties(${KERNEL_BIN} PROPERTIES CMAKE_LINKER "/usr/bin/ld")

add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/build/isofiles/boot/grub
    COMMAND cp ${KERNEL_BIN} ${CMAKE_BINARY_DIR}/build/isofiles/boot/${KERNEL_BIN}
    COMMAND cp ${GRUB_CFG} ${CMAKE_BINARY_DIR}/build/isofiles/boot/grub
    COMMAND grub-mkrescue -o kernel.iso ${CMAKE_BINARY_DIR}/build/isofiles
    COMMAND rm -r ${CMAKE_BINARY_DIR}/build/
)

add_custom_target(run
    qemu-system-x86_64 -cdrom kernel.iso -vnc localhost:1 -m 500M
)