# AOS
osdev project for studying purposes. First target is pintos and second is run gcc at my os.

# Current Developing
kmalloc (sync, bitmap allocator), list, queue, thread

Have to change pmm to whatever support pmm_malloc and pmm_free
Also have to change kmalloc.c and implement sync

# Basic Todo List
## X86_64
- [ ] GDT
- [ ] TSS
- [ ] LDT
## Memory
- [ ] Implement kmalloc
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
- [ ] Basic Scheduler
- [ ] Basic thread implementation
## Sync
- [ ] Spin lock
- [ ] Mutex
- [ ] Semaphore
## Driver
- [ ] Serial
- [ ] Graphic Driver
- [ ] ACPI
## Library
- [ ] Small LibC
- [ ] kernel stdlib
- [ ] List, Queue
- [ ] Bitmap
- [ ] Btree+
## Filesystem
- [ ] Ext2
- [ ] Ext4
- [ ] VFS
- [ ] Journaling
## ETC
- [x] Assert, Panic, Debug