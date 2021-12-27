#include "keyboard.h"
#include "interrupt.h"
#include "port.h"
#include "pic8259.h"
#include "vfs.h"
#include "kmalloc.h"
#include "memory.h"

static uint8_t *kbd_buffer;
static int cur_offset = 0;

static size_t read_kbd_buffer(void *__attribute__ ((unused)) aux, size_t __attribute__ ((unused)) offset, size_t size, void *buf);

static int8_t get_char(void)
{
    static int shift;
    static int8_t *charcode[4] = {
        normalmap, shiftmap, ctlmap, ctlmap};
    int st, data, c;

    st = inb(KBSTATP);
    if ((st & KBS_DIB) == 0)
        return -1;
    data = inb(KBDATAP);

    if (data == 0xE0)
    {
        shift |= E0ESC;
        return 0;
    }
    else if (data & 0x80)
    {
        // Key released
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shiftcode[data] | E0ESC);
        return 0;
    }
    else if (shift & E0ESC)
    {
        // Last character was an E0 escape; or with 0x80
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shiftcode[data];
    shift ^= togglecode[data];
    c = charcode[shift & (CTL | SHIFT)][data];
    if (shift & CAPSLOCK)
    {
        if ('a' <= c && c <= 'z')
            c += 'A' - 'a';
        else if ('A' <= c && c <= 'Z')
            c += 'a' - 'A';
    }
    return c;
}

static enum irq_handler_result keyboard_handler(struct intr_frame *f)
{
    char c = (char)get_char();
    if(c) {
        printf("%c", c);
        kbd_buffer[(cur_offset++) % KBD_BUFFER_LEN] = c;
    }
    // For debug
    if(c == 'p') {
        printf("\nkbd_buffer\n");
        char buf[256];
        memset(buf, 0, 256);
        read_kbd_buffer(NULL, 0, 256, buf);
        for(int i=0;i<256;i++) {
            if(buf[i] == 0) {
                return OK;
            } else {
                printf("%c", buf[i]);
            }
        }
        printf("\n");
    }
}

static size_t read_kbd_buffer(void *__attribute__ ((unused)) aux, size_t __attribute__ ((unused)) offset, size_t size, void *buf) {
    size = cur_offset > size ? size : cur_offset;

    memcpy(buf, kbd_buffer, size);

    cur_offset -= cur_offset;
    return size;
}

void keyboard_init()
{
    bind_interrupt_with_name(0x21, &keyboard_handler, "Keyboard");
    kbd_buffer = kcalloc(KBD_BUFFER_LEN, sizeof(uint8_t));

    device_t *kbd_device = (device_t *)kmalloc(sizeof(device_t));
    kbd_device->device_type = DEVICE_CHARACTER;

    struct device_operations *dev_ops = (struct device_operations *)kmalloc(sizeof(struct device_operations));
    dev_ops->block_size = 0;
    dev_ops->read = &read_kbd_buffer;
    kbd_device->dev_op = dev_ops;

    dev_install(kbd_device, NULL);
}

DEVICE_INSTALL(keyboard, &keyboard_init);