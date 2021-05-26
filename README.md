# AOS
osdev project for studying purposes. First target is pintos and second is run gcc at my os.

# Under Developing
1. Now palloc and malloc is implemented. But, palloc have to be changed to bitmap allocator.
2. Start Implement ext2 File system!!! First to do! VFS.

# Basic Todo List
## X86_64
- [ ] GDT
- [ ] TSS
- [ ] LDT
## Memory
- [x] Implement kmalloc
- [ ] User, Kernel Pool
- [ ] Bitmap Allocator
- [ ] Buddy Allocator
- [ ] Anonymouse page
- [ ] Stack / Heap
- [ ] MMAP
- [ ] Swap in and out
- [ ] Cpoy on write
## User Process
- [ ] Elf Loader
- [ ] Dynamic Linker
- [ ] Argument Passing
- [ ] User Memory
- [ ] System calls
- [ ] Process Termination Messages
- [ ] Safty
- [ ] Extend File Descriptor
## Thread
- [ ] Implement Clock
- [x] Basic Scheduler
- [x] Basic thread implementation
## Sync
- [x] Spin lock
- [ ] Mutex
- [ ] Semaphore
## Driver
- [ ] Serial
- [ ] Graphic Driver
- [ ] ACPI
## Library
- [ ] Small LibC
- [ ] kernel stdlib
- [x] List, Queue
- [ ] Bitmap
- [ ] Btree+
## Filesystem
- [ ] Ext2
- [ ] Ext4
- [ ] VFS
- [ ] Journaling
## ETC
- [x] Assert, Panic, Debug
- [ ] Minish (Bash Like Shell)