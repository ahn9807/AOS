#include "ata.h"
#include "port.h"
#include "vga_text.h"
#include "interrupt.h"
#include "debug.h"
#include "list.h"
#include "kmalloc.h"
#include "string.h"
#include "device.h"

// #define ASSERT_NO_ERR 0

static struct ata_disk ata_disk_primary_master = {.io_base = 0x1F0, .control_base = 0x3F6, .irq_num = 14 + 0x20, .is_master = true, .name = "hd0:0"};
static struct ata_disk ata_disk_primary_slave = {.io_base = 0x1F0, .control_base = 0x3F6, .irq_num = 14 + 0x20, .is_master = false, .name = "hd0:1"};
static struct ata_disk ata_disk_secondary_master = {.io_base = 0x170, .control_base = 0x376, .irq_num = 15 + 0x20, .is_master = true, .name = "hd1:0"};
static struct ata_disk ata_disk_secondary_slave = {.io_base = 0x170, .control_base = 0x376, .irq_num = 15 + 0x20, .is_master = false, .name = "hd1:1"};

static void disk_init(struct ata_disk *disk);
static void select_disk(const struct ata_disk *disk);
static void wait_io(const struct ata_disk *disk);
static int wait_busy(const struct ata_disk *disk);
static int wait_transfer(const struct ata_disk *disk);
static int wait_idle(const struct ata_disk *disk);
static void soft_reset(const struct ata_disk *disk);
static void intr_handler(struct intr_frame *);
static void ata_intr_enable(struct ata_disk *disk);
static void ata_intr_disable(struct ata_disk *disk);
static void read_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer);
static void write_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer);
static void identify_disk(struct ata_disk *disk);
static void flush_cache(struct ata_disk *disk);
static uint64_t max_offset(struct ata_disk *disk);
// static void read_one_sector_48(struct ata_disk *disk, uint32_t lba28, void *buffer); // To do...
// static void write_one_sector_48(struct ata_disk *disk, uint32_t lba28, void *buffer); / To do...
static void debug_device();

static struct list active_ata_devices;

// Initialize PATA, SATA, PATAPI and SATAPI Disk
void ata_init()
{
    list_init(&active_ata_devices);

    bind_interrupt_with_name(0x20 + 14, &intr_handler, "ATA HD0");
    bind_interrupt_with_name(0x20 + 15, &intr_handler, "ATA HD1");

    disk_init(&ata_disk_primary_master);
    disk_init(&ata_disk_primary_slave);
    disk_init(&ata_disk_secondary_master);
    disk_init(&ata_disk_secondary_slave);

    char buffer[1024];
    ata_disk_read(&ata_disk_primary_master, 1024, 1024, buffer);

    printf("SUPER BLOCK\n");
    for(int i=0;i<1024;i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");

    debug_device();
}

// Get MAX Offset of disk
uint64_t max_offset(struct ata_disk* disk) {
    return disk->info.sectors_28 * 512;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
 * Returns the number of bytes actually read, which may be less
 * than SIZE if an error occurs or end of file is reached. */
void ata_disk_read(struct ata_disk *disk, size_t offset, size_t len, void *buffer)
{
    uint32_t start_block = offset / 512;
    uint32_t end_block = (offset + len - 1) / 512;

    uint64_t x_offset = 0;

    uint8_t *buffer_8 = (uint8_t*)buffer;

    char tmp[512];

    if(offset > max_offset(disk)) {
        return 0;
    } else if(offset + len > max_offset(disk)) {
        len = max_offset(disk) - offset;
    }

    if(offset % 512) {
        uint32_t prefix_size = (512 - (offset % 512));
        read_one_sector_28(disk, 0, tmp);
		memcpy(buffer_8, (void *)((uintptr_t)tmp + ((uintptr_t)offset % 512)), prefix_size);

		x_offset += prefix_size;
		start_block++;
    }
    if((offset + len) % 512 && start_block < end_block) {
        uint32_t postfix_size = (offset + len) % 512;
        read_one_sector_28(disk, end_block, tmp);
		memcpy((void *)((uintptr_t)buffer_8 + len - postfix_size), tmp, postfix_size);

		end_block--;
    }

    while(start_block <= end_block) {
        read_one_sector_28(disk, start_block, (void*)((uintptr_t)buffer_8 + x_offset));
        x_offset += 512;
        start_block++;
    }

    return len;
}

/* Writes SIZE bytes from INODE into BUFFER, starting at position OFFSET.
 * Returns the number of bytes actually write, which may be less
 * than SIZE if an error occurs or end of file is reached. */
void ata_disk_write(struct ata_disk *disk, size_t offset, size_t len, const void *buffer)
{
    uint32_t start_block = offset / 512;
    uint32_t end_block = (offset + len - 1) / 512;

    uint32_t x_offset = 0;

    char tmp[512];

    if(offset > max_offset(disk)) {
        return 0;
    } else if(offset + len > max_offset(disk)) {
        len = max_offset(disk) - offset;
    }

    if(offset % 512) {
        uint32_t prefix_size = (512 - (offset % 512));
        read_one_sector_28(disk, start_block, tmp);
		memcpy(buffer, (void *)((uintptr_t)tmp + ((uintptr_t)offset % 512)), prefix_size);
        write_one_sector_28(disk, start_block, tmp);

		x_offset += prefix_size;
		start_block++;
    }
    if((offset + len) % 512 && start_block < end_block) {
        uint32_t postfix_size = (offset + len) % 512;
        read_one_sector_28(disk, end_block, tmp);
		memcpy((void *)((uintptr_t)buffer + len - postfix_size), tmp, postfix_size);
        write_one_sector_28(disk, end_block, tmp);

		end_block--;
    }

    while(start_block <= end_block) {
        write_one_sector_28(disk, start_block, (void*)((uintptr_t)buffer + x_offset));
        x_offset += 512;
        start_block++;
    }

    return len;
}

static void read_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer)
{
    ASSERT(disk->is_active == true);
    ASSERT(disk->type == PATA);

    printf("lba28: %d, buffer: 0x%x\n", lba28, buffer);
    spin_lock(&disk->lock);
    wait_idle(disk);

    uint16_t *buffer_ptr = (uint16_t *)buffer;

    outb(disk->io_base + ATA_REG_HDDEVSEL, (disk->is_master == true ? 0xe0 : 0xf0) | (uint8_t)((lba28 & 0x0f000000) >> 24));
    outb(disk->io_base + ATA_REG_FEATURES, 0x0);
    outb(disk->io_base + ATA_REG_SECCOUNT0, 1);
    outb(disk->io_base + ATA_REG_LBA0, (uint8_t)((lba28 & 0x000000ff) >> 0));
    outb(disk->io_base + ATA_REG_LBA1, (uint8_t)((lba28 & 0x0000ff00) >> 8));
    outb(disk->io_base + ATA_REG_LBA2, (uint8_t)((lba28 & 0x00ff0000) >> 16));
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    wait_io(disk);
    wait_busy(disk);

    enum intr_level prev_level = intr_disable();
    insw(disk->io_base + ATA_REG_DATA, buffer, 256);
    wait_io(disk);
    wait_busy(disk);
    wait_idle(disk);
    intr_set_level(prev_level);
    spin_unlock(&disk->lock);
}

// Write 512Byte (1 sector) to the disk
static void write_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer)
{
    ASSERT(disk->is_active == true);
    ASSERT(disk->type == PATA);

    spin_lock(&disk->lock);

    uint16_t *buffer_ptr = (uint16_t *)buffer;

    outb(disk->io_base + ATA_REG_HDDEVSEL, (disk->is_master == true ? 0xe0 : 0xf0) | (uint8_t)(lba28 & 0x0f000000) >> 24);
    outb(disk->io_base + ATA_REG_FEATURES, 0x0);
    outb(disk->io_base + ATA_REG_SECCOUNT0, 0x1);
    outb(disk->io_base + ATA_REG_LBA0, (lba28 & 0x000000ff) >>  0);
    outb(disk->io_base + ATA_REG_LBA1, (lba28 & 0x0000ff00) >>  8);
    outb(disk->io_base + ATA_REG_LBA2, (lba28 & 0x00ff0000) >> 16);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    wait_io(disk);
    wait_busy(disk);

    enum intr_level prev_level = intr_disable();
    outsw(disk->io_base + ATA_REG_DATA, buffer, 256);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    wait_io(disk);
    wait_busy(disk);
    intr_set_level(prev_level);
    spin_unlock(&disk->lock);
}

// FLush Cache
static void flush_cache(struct ata_disk *disk)
{
    select_disk(disk);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    wait_busy(disk);
}

// Initialize ATA device if it exists.
static void disk_init(struct ata_disk *disk)
{
    spin_lock(&disk->lock);
    soft_reset(disk);
    wait_io(disk);

    // detect device
    select_disk(disk);
    wait_io(disk);
    wait_busy(disk);

    uint8_t cl = inb(disk->io_base + ATA_REG_LBA1);
    uint8_t ch = inb(disk->io_base + ATA_REG_LBA2);

    if (cl == 0xFF && ch == 0xFF)
    {
        disk->is_active = false;
        return;
    }
    else if (cl == 0x00 && ch == 0x00)
    {
        // This is ATA Device (eg. HardDisk)
        disk->is_active = true;
        disk->type = PATA;
        ata_intr_disable(disk);
        identify_disk(disk);
        list_push_back(&active_ata_devices, &disk->elem);
    }
    else if (cl == 0x3c && ch == 0xc3)
    {
        // This is SATA Device (eg. HardDisk)
        disk->is_active = true;
        disk->type = SATA;
        ata_intr_disable(disk);
        identify_disk(disk);
        list_push_back(&active_ata_devices, &disk->elem);
    }
    else if (cl == 0x14 && ch == 0xeb)
    {
        // This is ATAPI Device (eg. CDROM)
        // disk->is_active = true;
        // disk->type = PATAPI;
        // ata_intr_disable(disk);
        // identify_disk(disk);
        // list_push_back(&active_ata_devices, &disk->elem);
    }
    else if (cl == 0x69 && ch == 0x96)
    {
        // This is SATAPI Device (eg. CDROM)
        // disk->is_active = true;
        // disk->type = SATAPI;
        // ata_intr_disable(disk);
        // identify_disk(disk);
        // list_push_back(&active_ata_devices, &disk->elem);
    }

    wait_io(disk);
    wait_idle(disk);

    spin_unlock(&disk->lock);
}

// Identify ATA Device
static void identify_disk(struct ata_disk *disk) {
    outb(disk->io_base + ATA_REG_ERROR, 0x1);
    outb(disk->control_base, 0x0);

    select_disk(disk);

    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    wait_io(disk);

    uint16_t status = inb(disk->io_base + ATA_REG_COMMAND);
    wait_io(disk);
    wait_busy(disk);
    wait_transfer(disk);

    memset(&disk->info, 0, sizeof(struct ata_identity));
    uint16_t *buf = (uint16_t*)&disk->info;

    for(int i=0;i<256;i++) {
        buf[i] = inw(disk->io_base);
    }

    uint8_t *ptr = (uint8_t*)&disk->info.model;
    for(int i=0;i<39;i+=2) {
        uint8_t tmp = ptr[i+1];
        ptr[i+1] = ptr[i];
        ptr[i] = tmp;
    }
    ptr[40] = '\0';
}

// Select ATA Device to use.
static void select_disk(const struct ata_disk *disk)
{
    uint8_t reg = disk->io_base + ATA_REG_HDDEVSEL;
    uint8_t select_bit = disk->is_master ? 0xA0 : 0xB0; // 0x40 | (slave_bit << 4)
    outb(reg, select_bit);
    wait_io(disk);
}

// Each inb to IO port takes about 30ns, so in order to slepp at least 400ns,
// We have to check status bit about 14times.
// Alternatively, we can sleep this thread about 400ns.
static void wait_io(const struct ata_disk *disk)
{
    // This can be changed as like
    // inb(disk->io_base + ATA_REG_ATLSTATUS);
    // timer_nsleep(400);
    inb(disk->io_base + ATA_REG_ALTSTATUS);
    inb(disk->io_base + ATA_REG_ALTSTATUS);
    inb(disk->io_base + ATA_REG_ALTSTATUS);
    inb(disk->io_base + ATA_REG_ALTSTATUS);
}

// ATA Device is busy...
// Technically, when BSY is set, the other bits in the Status byte are meaningless. Wait BSY == 0
static int wait_busy(const struct ata_disk *disk)
{
    int return_val = 0;
    while ((return_val = inb(disk->io_base + ATA_REG_STATUS)) & ATA_SR_BSY)
        ;
#ifdef ASSERT_NO_ERR
    ASSERT(return_val & ATA_SR_ERR == 1);
#endif
    if(return_val & ATA_SR_ERR) {
        int16_t error = inb(disk->io_base + ATA_REG_ERROR);
        return error;
    }
    return 0;
}

// ATA Device is now ready for data transfer.
// Check the drive has PIO data to transfer, or is ready to accept PIO data. Wait DRQ == 1 and BSY == 0
static int wait_transfer(const struct ata_disk *disk)
{
    int return_val = 0;
    while (!(return_val = inb(disk->io_base + ATA_REG_STATUS)) & (ATA_SR_DRQ | ~ATA_SR_BSY))
        ;
#ifdef ASSERT_NO_ERR
    ASSERT(return_val & ATA_SR_ERR == 1);
#endif
    if(return_val & ATA_SR_ERR) {
        int16_t error = inb(disk->io_base + ATA_REG_ERROR);
        return error;
    }
    return 0;
}

// ATA Device is now idle
// Reading the status register clears any pending interrupt. Wait DRQ == 0 and BSY == 0
static int wait_idle(const struct ata_disk *disk)
{
    int return_val = 0;
    while ((return_val = inb(disk->io_base + ATA_REG_STATUS)) & (ATA_SR_DRQ | ATA_SR_BSY) != 0)
        ;
#ifdef ASSERT_NO_ERR
    ASSERT(return_val & ATA_SR_ERR == 1);
#endif
    if(return_val & ATA_SR_ERR) {
        int16_t error = inb(disk->io_base + ATA_REG_ERROR);
        return error;
    }
    return 0;
}

// This will reset both ATA devices on the bus.
// The master drive on the bus is automatically selected.
// ATAPI drives set values on their LBA_LOW and LBA_HIGH IO port.
static void soft_reset(const struct ata_disk *disk)
{
    outb(disk->control_base, ATA_CR_SRST);
    wait_io(disk);
    // Have to clear bit manually.
    outb(disk->control_base, ATA_CR_NA);
}

static void intr_handler(struct intr_frame *frame)
{
    // printf("Interrupt form [%s] disk!\n", "hd0:0");
    //do nothing
}

// enable interrupt of this ata device
static void ata_intr_enable(struct ata_disk *disk)
{
    outb(disk->control_base, ATA_CR_NA);
    wait_io(disk);
    disk->is_intr_enabled = true;
}

// disable interrupt of this ata device
static void ata_intr_disable(struct ata_disk *disk)
{
    outb(disk->control_base, ATA_CR_NIEN);
    wait_io(disk);
    disk->is_intr_enabled = false;
}

// Print debug information of active ata devices
static void debug_device()
{
    for (struct list_elem *e = list_begin(&active_ata_devices); e != list_end(&active_ata_devices); e = list_remove(e))
    {
        struct ata_disk *disk = list_entry(e, struct ata_disk, elem);
        printf("%s (%s)\n",
            disk->info.model,
            disk->type == (PATA) ? "PATA" : "PATAPI"
        );
    }
}