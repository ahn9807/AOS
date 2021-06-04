#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "device.h"
#include "spin_lock.h"
#include "list.h"

// Status Register I/O Base + 7 (R)
#define ATA_SR_BSY     0x80 // Busy
#define ATA_SR_DRDY    0x40 // Device Ready
#define ATA_SR_DF      0x20 // Driver Fault Error
#define ATA_SR_DSC     0x10 // Overlapped Mode Service Request
#define ATA_SR_DRQ     0x08 // Device Request
#define ATA_SR_CORR    0x04 // Corrected data, always zero
#define ATA_SR_IDX     0x02 // Index, always zero
#define ATA_SR_ERR     0x01 // Error occurred

// ATA Error code I/O Base + 1 (R)
#define ATA_ER_BBK      0x80 // Bad Block detected. 
#define ATA_ER_UNC      0x40 // Uncorrectable data error. 
#define ATA_ER_MC       0x20 // Media changed
#define ATA_ER_IDNF     0x10 // ID not found
#define ATA_ER_MCR      0x08 // Aborted command
#define ATA_ER_ABRT     0x04 // Aborted command
#define ATA_ER_TK0NF    0x02 // Track zero not found
#define ATA_ER_AMNF     0x01 // Address mark not found. 

// ATA Command I/O Base + 7 (W)
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// ATAPI Command I/O Base + 7 (W)
#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

// Offset from I/O Base
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define ATA_PRIMARY      0x00
#define ATA_SECONDARY    0x01

// Directions:
#define ATA_READ      0x00
#define ATA_WRITE     0x01

// Device Control Register (Control base  + 0)
#define ATA_CR_NA 0x0       // Always set to zero
#define ATA_CR_NIEN 0x1    // Stop sending Interrupt
#define ATA_CR_SRST 0x04   // Software Reset

// This represent type of disk
// Currently, SATA is emulated by ATA
enum ata_disk_type {
    SATA,
    SATAPI,
    PATA,
    PATAPI,
};

// COMMEND_IDENTIFY struct
struct ata_identity {
    uint16_t flags;
	uint16_t unused1[9];
	char     serial[20];
	uint16_t unused2[3];
	char     firmware[8];
	char     model[40];
	uint16_t sectors_per_int;
	uint16_t unused3;
	uint16_t capabilities[2];
	uint16_t unused4[2];
	uint16_t valid_ext_data;
	uint16_t unused5[5];
	uint16_t size_of_rw_mult;
	uint32_t sectors_28;
	uint16_t unused6[38];
	uint64_t sectors_48;
	uint16_t unused7[152];
} __attribute__((packed));

// This represent actual ata disk we used
// Primary channel is io_base 0x1F0, control 0x3F6
// Secondary channel is io_base 0x170, control 0x376
// Each channel consist of two different disk
// First is Master and sencond is Slave.
struct ata_disk {
    char name[8];
    uint16_t io_base;
    uint16_t control_base;
    uint8_t irq_num;
    bool is_intr_enabled;
    bool is_master;
    bool is_active;
    enum ata_disk_type type; // 1 for atapi 0 for ata
    struct ata_identity info;
    spinlock_t lock;
    struct list_elem elem;
};

void ata_init();
static void ata_disk_read(struct ata_disk *d, size_t offset, size_t len, void* buffer);
static void ata_disk_write(struct ata_disk *d, size_t offset, size_t len, const void *buffer);