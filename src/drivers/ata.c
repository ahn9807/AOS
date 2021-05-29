#include "ata.h"
#include "port.h"
#include "vga_text.h"
#include "interrupt.h"
#include "debug.h"
#include "list.h"

static struct ata_disk ata_disk_primary_master = {.io_base = 0x1F0, .control = 0x3F6, .irq_num = 14 + 0x20, .is_master = true, .name = "hd0:0"};
static struct ata_disk ata_disk_primary_slave = {.io_base = 0x1F0, .control = 0x3F6, .irq_num = 14 + 0x20, .is_master = false, .name = "hd0:1"};
static struct ata_disk ata_disk_secondary_master = {.io_base = 0x170, .control = 0x376, .irq_num = 15 + 0x20, .is_master = true, .name = "hd1:0"};
static struct ata_disk ata_disk_secondary_slave = {.io_base = 0x170, .control = 0x376, .irq_num = 15 + 0x20, .is_master = false, .name = "hd1:1"};

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
static void flush_cache(struct ata_disk *disk);
// static void read_one_sector_48(struct ata_disk *disk, uint32_t lba28, void *buffer); // To do...
// static void write_one_sector_48(struct ata_disk *disk, uint32_t lba28, void *buffer); / To do...
static void debug_device();

static struct list active_ata_devices;

void ata_init()
{
    list_init(&active_ata_devices);

    bind_interrupt_with_name(0x20 + 14, &intr_handler, "ATA HD0");
    bind_interrupt_with_name(0x20 + 15, &intr_handler, "ATA HD1");

    disk_init(&ata_disk_primary_master);
    disk_init(&ata_disk_primary_slave);
    disk_init(&ata_disk_secondary_master);
    disk_init(&ata_disk_secondary_slave);

    debug_device();

    char buffer[512];
    char buffer_read[512] = {
        0,
    };
    for (int i = 0; i < 512; i++)
    {
        buffer[i] = 'a';
        buffer_read[i] = 'b';
        // printf("%c", buffer[i]);
    }
    write_one_sector_28(&ata_disk_primary_master, 512, buffer);
    read_one_sector_28(&ata_disk_primary_master, 512, buffer_read);
    for (int i = 0; i < 512; i++)
    {
        printf("%x", buffer_read[i]);
    }
}

void ata_disk_read(struct ata_disk *d, size_t offset, size_t len, void *buffer)
{
    PANIC("NOT IMPLEMENTED");
}

void ata_disk_write(struct ata_disk *d, size_t offset, size_t len, const void *buffer)
{
    PANIC("NOT IMPLEMENTED");
}

static void read_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer)
{
    ASSERT(disk->is_active == true);
    ASSERT(disk->type == PATA);

    spin_lock(&disk->lock);
    wait_idle(disk);

    uint16_t *buffer_ptr = (uint16_t *)buffer;

    outb(disk->io_base + ATA_REG_CONTROL, 0x02);
    wait_busy(disk);

    outb(disk->io_base + ATA_REG_HDDEVSEL, (disk->is_master == true ? 0xe0 : 0xf0) | (lba28 & 0x0f000000) >> 24);
    outb(disk->io_base + ATA_REG_FEATURES, 0x0);
    outb(disk->io_base + ATA_REG_SECCOUNT0, 1);
    outb(disk->io_base + ATA_REG_LBA0, (lba28 & 0x000000ff) >> 0);
    outb(disk->io_base + ATA_REG_LBA1, (lba28 & 0x0000ff00) >> 8);
    outb(disk->io_base + ATA_REG_LBA2, (lba28 & 0x00ff0000) >> 16);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    wait_io(disk);
    wait_transfer(disk);

    insw(disk->io_base + ATA_REG_DATA, buffer, 256);
    wait_io(disk);
    wait_busy(disk);

    spin_unlock(&disk->lock);

    printf("Read one sector...Done!\n");
}

// Write 512Byte (1 sector) to the disk
static void write_one_sector_28(struct ata_disk *disk, uint32_t lba28, void *buffer)
{
    ASSERT(disk->is_active == true);
    ASSERT(disk->type == PATA);

    spin_lock(&disk->lock);
    wait_idle(disk);

    uint16_t *buffer_ptr = (uint16_t *)buffer;

    outb(disk->io_base + ATA_REG_CONTROL, 0x02);
    wait_busy(disk);

    outb(disk->io_base + ATA_REG_HDDEVSEL, (disk->is_master == true ? 0xe0 : 0xf0) | (lba28 & 0x0f000000) >> 24);
    outb(disk->io_base + ATA_REG_FEATURES, 0x0);
    outb(disk->io_base + ATA_REG_SECCOUNT0, 0x1);
    outb(disk->io_base + ATA_REG_LBA0, (lba28 & 0x000000ff) >>  0);
    outb(disk->io_base + ATA_REG_LBA1, (lba28 & 0x0000ff00) >>  8);
    outb(disk->io_base + ATA_REG_LBA2, (lba28 & 0x00ff0000) >> 16);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    wait_io(disk);
    wait_busy(disk);

    outsw(disk->io_base + ATA_REG_DATA, buffer, 256);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    wait_io(disk);
    wait_busy(disk);

    spin_unlock(&disk->lock);

    printf("Wirte one sector...Done!\n");
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

    outb(disk->io_base + ATA_REG_SECCOUNT0, 0);
    outb(disk->io_base + ATA_REG_LBA0, 0);
    outb(disk->io_base + ATA_REG_LBA1, 0);
    outb(disk->io_base + ATA_REG_LBA2, 0);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    wait_transfer(disk);
    uint8_t status = inb(disk->io_base + ATA_REG_STATUS);

    if (status != 0x0 || status != 0xff)
    {
        wait_busy(disk);
        status = inb(disk->io_base + ATA_REG_STATUS);
        if (status & ATA_SR_ERR)
        {
            spin_unlock(&disk->lock);
            return;
        }
        wait_transfer(disk);
        status = inb(disk->io_base + ATA_REG_STATUS);
        if (status & ATA_SR_ERR)
        {
            spin_unlock(&disk->lock);
            return;
        }

        uint8_t error = inb(disk->io_base + ATA_REG_ERROR);
        uint8_t cl = inb(disk->io_base + ATA_REG_LBA1);
        uint8_t ch = inb(disk->io_base + ATA_REG_LBA2);
        status = inb(disk->io_base + ATA_REG_STATUS);

        if (cl == 0xFF || ch == 0xFF)
        {
            disk->is_active = false;
            return;
        }
        else if (cl == 0x00 && ch == 0x00)
        {
            // This is ATA Device (eg. HardDisk)
            disk->is_active = true;
            disk->type = PATA;
            list_push_back(&active_ata_devices, &disk->elem);
        }
        else if (cl == 0x3c && ch == 0xc3)
        {
            // This is SATA Device (eg. HardDisk)
            disk->is_active = true;
            disk->type = SATA;
            list_push_back(&active_ata_devices, &disk->elem);
        }
        else if (cl == 0x14 && ch == 0xeb)
        {
            // This is ATAPI Device (eg. CDROM)
            disk->is_active = true;
            disk->type = PATAPI;
            list_push_back(&active_ata_devices, &disk->elem);
        }
        else if (cl == 0x69 && ch == 0x96)
        {
            // This is SATAPI Device (eg. CDROM)
            disk->is_active = true;
            disk->type = SATAPI;
            list_push_back(&active_ata_devices, &disk->elem);
        }
    }
    else
    {
        disk->is_active = false;
    }

    wait_io(disk);
    wait_idle(disk);

    spin_unlock(&disk->lock);
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
    for (int i = 0; i < 4; i++)
    {
        inb(disk->io_base + ATA_REG_ALTSTATUS);
    }
}

// ATA Device is busy...
// Technically, when BSY is set, the other bits in the Status byte are meaningless. Wait BSY == 0
static int wait_busy(const struct ata_disk *disk)
{
    int return_val = 0;
    while ((return_val = inb(disk->io_base + ATA_REG_STATUS)) & ATA_SR_BSY)
        ;
    return return_val;
}

// ATA Device is now ready for data transfer.
// Check the drive has PIO data to transfer, or is ready to accept PIO data. Wait DRQ == 1 and BSY == 0
static int wait_transfer(const struct ata_disk *disk)
{
    int return_val = 0;
    while (!(return_val = inb(disk->io_base + ATA_REG_STATUS)) & (ATA_SR_DRQ | ~ATA_SR_BSY))
        ;
    return return_val;
}

// ATA Device is now idle
// Reading the status register clears any pending interrupt. Wait DRQ == 0 and BSY == 0
static int wait_idle(const struct ata_disk *disk)
{
    int return_val = 0;
    while ((return_val = inb(disk->io_base + ATA_REG_STATUS)) & (ATA_SR_DRQ | ATA_SR_BSY) != 0)
        ;
    return return_val;
}

// This will reset both ATA devices on the bus.
// The master drive on the bus is automatically selected.
// ATAPI drives set values on their LBA_LOW and LBA_HIGH IO port.
static void soft_reset(const struct ata_disk *disk)
{
    outb(disk->control, ATA_CR_SRST);
    wait_io(disk);
    // Have to clear bit manually.
    outb(disk->control, ATA_CR_NA);
}

static void intr_handler(struct intr_frame *frame)
{
    // printf("Interrupt form [%s] disk!\n", "hd0:0");
    //do nothing
}

// enable interrupt of this ata device
static void ata_intr_enable(struct ata_disk *disk)
{
    outb(disk->control, ATA_CR_NA);
    wait_io(disk);
    disk->is_intr_enabled = true;
}

// disable interrupt of this ata device
static void ata_intr_disable(struct ata_disk *disk)
{
    outb(disk->control, ATA_CR_NIEN);
    wait_io(disk);
    disk->is_intr_enabled = false;
}

// Print debug information of active ata devices
static void debug_device()
{
    for (struct list_elem *e = list_begin(&active_ata_devices); e != list_end(&active_ata_devices); e = list_remove(e))
    {
        struct ata_disk *disk = list_entry(e, struct ata_disk, elem);
        printf("Disk %s %s is %s (%s)\n",
               disk->io_base == 0x1f0 ? "First" : "Second",
               disk->is_master ? "mastser" : "slave",
               disk->is_active ? "active" : "deactive",
               disk->type == (PATA) ? "ATA" : "ATAPI");
    }
}