#include "ata.h"
#include "port.h"
#include "vga_text.h"
#include "interrupt.h"

static struct ata_disk ata_disk_primary_master = {.io_base = 0x1F0, .control = 0x3F6, .is_master = true, .name = "hd0:0"};
static struct ata_disk ata_disk_primary_slave = {.io_base = 0x1F0, .control = 0x3F6, .is_master = false, .name = "hd0:1"};
static struct ata_disk ata_disk_secondary_master = {.io_base = 0x170, .control = 0x376, .is_master = true, .name = "hd1:0"};
static struct ata_disk ata_disk_secondary_slave = {.io_base = 0x170, .control = 0x376, .is_master = false, .name = "hd1:1"};

static void disk_init(struct ata_disk *disk);
static void select_disk(const struct ata_disk *disk);
static void wait_io(const struct ata_disk *disk);
static int wait_busy(const struct ata_disk *disk);
static int wait_transfer(const struct ata_disk *disk);
static int wait_idle(const struct ata_disk *disk);
static void soft_reset(const struct  ata_disk *disk);
static void intr_handler(struct intr_frame*);

void ata_init() {
    disk_init(&ata_disk_primary_master);
}

void ata_disk_read(struct ata_disk *d, size_t offset, size_t len, void* buffer) {

}

void ata_disk_write(struct ata_disk *d, size_t offset, size_t len, const void *buffer) {

}

// Initialize ATA device if it exists.
static void disk_init(struct ata_disk *disk) {
    // set basic of struct ata_disk
    if(disk->io_base == 0x1F0) {
        disk->irq_num = 14 + 0x20;
    } else {
        disk->irq_num = 15 + 0x20;
    }

    bind_interrupt_with_name(disk->irq_num, &intr_handler, disk->name);

    spin_lock(&disk->lock);

    // detect device
    select_disk(disk);
    outb(disk->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    wait_io(disk);
    int status = inb(disk->io_base + ATA_REG_COMMAND);
    printf("[%s] status %d\n", disk->name, status);
    wait_io(disk);
    wait_idle(disk);

    spin_unlock(&disk->lock);
}

// Select ATA Device to use.
static void select_disk(const struct ata_disk *disk) {
    uint8_t reg = disk->io_base + ATA_REG_HDDEVSEL;
    uint8_t select_bit = disk->is_master ? 0x50 : 0x40; // 0x40 | (slave_bit << 4)
    outb(reg, select_bit);
    wait_io(disk);
}

// Each inb to IO port takes about 30ns, so in order to slepp at least 400ns,
// We have to check status bit about 14times.
// Alternatively, we can sleep this thread about 400ns.
static void wait_io(const struct ata_disk *disk) {
    // This can be changed as like
    // inb(disk->io_base + ATA_REG_ATLSTATUS);
    // timer_nsleep(400);
    for(int i=0;i<14;i++) {
        inb(disk->io_base + ATA_REG_ALTSTATUS);
    }
}

// ATA Device is busy...
// Technically, when BSY is set, the other bits in the Status byte are meaningless.
static int wait_busy(const struct ata_disk *disk) {
    int return_val = 0;
    while((return_val = inb(disk->io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
    return return_val;
}

// Check the drive has PIO data to transfer, or is ready to accept PIO data. Wait DRQ == 1
static int wait_transfer(const struct ata_disk *disk) {
    int return_val = 0;
    while(!(return_val = inb(disk->io_base + ATA_REG_STATUS)) & ATA_SR_DRQ);
    return return_val;
}

// ATA Device is now idle
// Reading the status register clears any pending interrupt. Wait DRQ == 0 and BSY == 0
static int wait_idle(const struct ata_disk *disk) {
    int return_val = 0;
    while((return_val = inb(disk->io_base + ATA_REG_STATUS)) & (ATA_SR_DRQ | ATA_SR_BSY) != 0);
    return return_val;
}

// This will reset both ATA devices on the bus.
// The master drive on the bus is automatically selected.
// ATAPI drives set values on their LBA_LOW and LBA_HIGH IO port.
static void soft_reset(const struct ata_disk *disk) {
    outb(disk->control, ATA_CR_SRST);
    wait_busy(disk);
    // Have to clear bit manually.
    outb(disk->control, ATA_CR_NA);
}

static void intr_handler(struct intr_frame *frame) {
    printf("Interrupt form [%s] disk!\n", "hd0:0");
    //do nothing
}