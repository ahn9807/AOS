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

file(GLOB_RECURSE kernel_SRC "./include/*.h" "./src/*.c")
file(GLOB_RECURSE kernel_ASM "./src/*.asm" "./src/*.S" "./src/*.s")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arch/x86_64
    ${CMAKE_CURRENT_SOURCE_DIR}/include/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drivers/block_device
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drivers/char_device
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sync
    ${CMAKE_CURRENT_SOURCE_DIR}/include/context
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fs/ext2
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
    COMMAND cp ${CMAKE_BINARY_DIR}/../trace_stack ./
)

add_custom_target(run
    qemu-system-x86_64 -no-reboot -no-shutdown -cdrom kernel.iso -curses -m 4G -drive id=disk,format=raw,file=disk.img
)

add_custom_target(debug
    COMMAND touch .gdbinit
    COMMAND echo "target remote localhost:1234" > .gdbinit
    COMMAND echo "symbol-file ${KERNEL_BIN}" >> .gdbinit
    COMMAND qemu-system-x86_64 -no-shutdown -no-reboot -cdrom kernel.iso -curses -m 4G -drive id=disk,format=raw,file=disk.img -s -S
)

add_custom_target(debug_qemu
    COMMAND touch .gdbinit
    COMMAND echo "target remote localhost:1234" > .gdbinit
    COMMAND echo "symbol-file ${KERNEL_BIN}" >> .gdbinit
    COMMAND qemu-system-x86_64 -d int -no-shutdown -no-reboot -cdrom kernel.iso -vnc :1 -m 4G -drive id=disk,format=raw,file=disk.img -s -S
)

add_custom_target(count_line
    find ../ -name '*.[ch]' | xargs wc -l
)

add_custom_target(build_image
    COMMAND qemu-img create -f raw  -o size=500M disk.img
    COMMAND mkfs.ext2 disk.img
    COMMAND echo "copy initfs to disk..."
    COMMAND mkdir -p mnt
    COMMAND sudo mount disk.img ./mnt
    COMMAND sudo cp -r ./initfs/. ./mnt
)