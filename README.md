# AOS
osdev project for studying purposes. First target is pintos and second is run gcc at my os.

# Under Developing
1. Now palloc and malloc is implemented. But, palloc have to be changed to bitmap allocator.
2. Start Implement ext2 File system!!! First to do! VFS.

# Basic Todo List
## X86_64
- [x] GDT
- [x] TSS
- [x] LDT
- [x] Detecting CPU
- [ ] Multitasking
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
- [x] Elf Loader
- [ ] Dynamic Linker
- [x] Argument Passing
- [ ] User Memory
- [x] System calls - Have to implement POSIX
- [ ] Process Termination Messages
- [ ] Safty
- [ ] Extend File Descriptor
- [ ] IPI
## Thread
- [ ] Implement Clock
- [ ] Implement RTC
- [x] Basic Scheduler (FIFO)
- [x] Basic thread implementation
## Sync
- [x] Spin lock
- [x] Mutex
- [x] Semaphore
## Driver
- [ ] Serial
- [ ] Graphic Driver
- [ ] Soudn Driver
- [x] APIC
- [x] ACPI
- [ ] HPET
- [x] ATA
- [ ] ATAPI, SATA, SATAPI
- [ ] USB
- [ ] NVME
- [ ] PCI
## Library
- [ ] Small LibC
- [ ] kernel stdlib
- [x] List, Queue
- [x] Bitmap
- [x] Hash
- [ ] Btree+
## Filesystem
- [ ] Ext2 - under development
- [ ] Ext4
- [ ] VFS - under development
- [ ] Journaling
- [ ] RamDisk
## ETC
- [ ] PerCPU
- [x] Assert, Panic, Debug
- [ ] Minish (Bash Like Shell)
