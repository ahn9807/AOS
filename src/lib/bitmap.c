#include "lib/bitmap.h"
#include "kmalloc.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "lib/types.h"
#include "vga_text.h"

#define CHAR_BIT 8

/* Yields X rounded up to the nearest multiple of STEP.
 * For X >= 0, STEP >= 1 only. */
#define ROUND_UP(X, STEP) (((X) + (STEP)-1) / (STEP) * (STEP))

/* Yields X divided by STEP, rounded up.
 * For X >= 0, STEP >= 1 only. */
#define DIV_ROUND_UP(X, STEP) (((X) + (STEP)-1) / (STEP))

/* Yields X rounded down to the nearest multiple of STEP.
 * For X >= 0, STEP >= 1 only. */
#define ROUND_DOWN(X, STEP) ((X) / (STEP) * (STEP))

/* Element type.
   This must be an unsigned integer type at least as wide as int.
   Each bit represents one bit in the bitmap.
   If bit 0 in an element represents bit K in the bitmap,
   then bit 1 in the element represents bit K+1 in the bitmap,
   and so on. */
typedef uint64_t elem_type;

/* Number of bits in an element. */
#define ELEM_BITS (sizeof(elem_type) * CHAR_BIT)

/* From the outside, a bitmap is an array of bits.  From the
   inside, it's an array of elem_type (defined above) that
   simulates an array of bits. */
struct bitmap {
	size_t bit_cnt; /* Number of bits. */
	elem_type *bits; /* Elements that represent bits. */
};

/* Returns the index of the element that contains the bit
   numbered BIT_IDX. */
static inline size_t elem_idx(size_t bit_idx)
{
	return bit_idx / ELEM_BITS;
}

/* Returns an elem_type where only the bit corresponding to
   BIT_IDX is turned on. */
static inline elem_type bit_mask(size_t bit_idx)
{
	return (elem_type)1 << (bit_idx % ELEM_BITS);
}

/* Returns the number of elements required for BIT_CNT bits. */
static inline size_t elem_cnt(size_t bit_cnt)
{
	return DIV_ROUND_UP(bit_cnt, ELEM_BITS);
}

/* Returns the number of bytes required for BIT_CNT bits. */
static inline size_t byte_cnt(size_t bit_cnt)
{
	return sizeof(elem_type) * elem_cnt(bit_cnt);
}

/* Returns a bit mask in which the bits actually used in the last
   element of B's bits are set to 1 and the rest are set to 0. */
static inline elem_type last_mask(const struct bitmap *b)
{
	int last_bits = b->bit_cnt % ELEM_BITS;
	return last_bits ? ((elem_type)1 << last_bits) - 1 : (elem_type)-1;
}

/* Creation and destruction. */

/* Initializes B to be a bitmap of BIT_CNT bits
   and sets all of its bits to false.
   Returns true if success, false if memory allocation
   failed. */
struct bitmap *bitmap_create(size_t bit_cnt)
{
	struct bitmap *b = kmalloc(sizeof *b);
	if (b != NULL) {
		b->bit_cnt = bit_cnt;
		b->bits = kcalloc(1, byte_cnt(bit_cnt));
		if (b->bits != NULL || bit_cnt == 0) {
			bitmap_set_all(b, false);
			return b;
		}
		kfree(b);
	}
	return NULL;
}

/* With given bit array, set bitmap */
struct bitmap *bitmap_create_from_buf(size_t bit_cnt, uint8_t *buf)
{
	struct bitmap *b = bitmap_create(bit_cnt);

	ASSERT(buf != NULL);
	ASSERT(b != NULL);
	ASSERT(b->bits != NULL);

	memcpy(b->bits, buf, byte_cnt(bit_cnt));

	return b;
}

/* With given bit array, set bitmap */
size_t bitmap_set_from_buf(struct bitmap *b, size_t bit_cnt, uint8_t *buf)
{
	ASSERT(b != NULL);
	ASSERT(buf != NULL);

	bit_cnt = b->bit_cnt >= bit_cnt ? bit_cnt : b->bit_cnt;

	memcpy(b->bits, buf, byte_cnt(bit_cnt));

	return bit_cnt;
}

/* Creates and returns a bitmap with BIT_CNT bits in the
   BLOCK_SIZE bytes of storage preallocated at BLOCK.
   BLOCK_SIZE must be at least bitmap_needed_bytes(BIT_CNT). */
struct bitmap *bitmap_create_in_buf(size_t bit_cnt, void *block, size_t block_size UNUSED)
{
	struct bitmap *b = block;

	ASSERT(block_size >= bitmap_buf_size(bit_cnt));

	b->bit_cnt = bit_cnt;
	b->bits = (elem_type *)(b + 1);
	bitmap_set_all(b, false);
	return b;
}

/* Returns the number of bytes required to accomodate a bitmap
   with BIT_CNT bits (for use with bitmap_create_in_buf()). */
size_t bitmap_buf_size(size_t bit_cnt)
{
	return sizeof(struct bitmap) + byte_cnt(bit_cnt);
}

/* get bitmap buffer */
void *bitmap_get_raw(struct bitmap *b)
{
	return b->bits;
}

/* Destroys bitmap B, freeing its storage.
   Not for use on bitmaps created by
   bitmap_create_preallocated(). */
void bitmap_destroy(struct bitmap *b)
{
	if (b != NULL) {
		kfree(b->bits);
		kfree(b);
	}
}

/* Bitmap size. */

/* Returns the number of bits in B. */
size_t bitmap_size(const struct bitmap *b)
{
	return b->bit_cnt;
}

/* Setting and testing single bits. */

/* Atomically sets the bit numbered IDX in B to VALUE. */
void bitmap_set(struct bitmap *b, size_t idx, bool value)
{
	ASSERT(b != NULL);
	ASSERT(idx < b->bit_cnt);
	if (value)
		bitmap_mark(b, idx);
	else
		bitmap_reset(b, idx);
}

/* Atomically sets the bit numbered BIT_IDX in B to true. */
void bitmap_mark(struct bitmap *b, size_t bit_idx)
{
	size_t idx = elem_idx(bit_idx);
	elem_type mask = bit_mask(bit_idx);

	/* This is equivalent to `b->bits[idx] |= mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the OR instruction in [IA32-v2b]. */
	asm("lock orq %1, %0" : "=m"(b->bits[idx]) : "r"(mask) : "cc");
}

/* Atomically sets the bit numbered BIT_IDX in B to false. */
void bitmap_reset(struct bitmap *b, size_t bit_idx)
{
	size_t idx = elem_idx(bit_idx);
	elem_type mask = bit_mask(bit_idx);

	/* This is equivalent to `b->bits[idx] &= ~mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the AND instruction in [IA32-v2a]. */
	asm("lock andq %1, %0" : "=m"(b->bits[idx]) : "r"(~mask) : "cc");
}

/* Atomically toggles the bit numbered IDX in B;
   that is, if it is true, makes it false,
   and if it is false, makes it true. */
void bitmap_flip(struct bitmap *b, size_t bit_idx)
{
	size_t idx = elem_idx(bit_idx);
	elem_type mask = bit_mask(bit_idx);

	/* This is equivalent to `b->bits[idx] ^= mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the XOR instruction in [IA32-v2b]. */
	asm("lock xorq %1, %0" : "=m"(b->bits[idx]) : "r"(mask) : "cc");
}

/* find first index of the value
   return -1 (size_t MAX_VAL) if not found */
size_t bitmap_find(struct bitmap *b, bool val)
{
	for (int i = 0; i < b->bit_cnt; i++) {
		if (bitmap_test(b, i) == val) {
			return i;
		}
	}

	return -1;
}

/* Returns the value of the bit numbered IDX in B. */
bool bitmap_test(const struct bitmap *b, size_t idx)
{
	ASSERT(b != NULL);
	ASSERT(idx < b->bit_cnt);
	return (b->bits[elem_idx(idx)] & bit_mask(idx)) != 0;
}

/* Setting and testing multiple bits. */

/* Sets all bits in B to VALUE. */
void bitmap_set_all(struct bitmap *b, bool value)
{
	ASSERT(b != NULL);

	bitmap_set_multiple(b, 0, bitmap_size(b), value);
}

/* Sets the CNT bits starting at START in B to VALUE. */
void bitmap_set_multiple(struct bitmap *b, size_t start, size_t cnt, bool value)
{
	size_t i;

	ASSERT(b != NULL);
	ASSERT(start <= b->bit_cnt);
	ASSERT(start + cnt <= b->bit_cnt);

	for (i = 0; i < cnt; i++)
		bitmap_set(b, start + i, value);
}

/* Returns the number of bits in B between START and START + CNT,
   exclusive, that are set to VALUE. */
size_t bitmap_count(const struct bitmap *b, size_t start, size_t cnt, bool value)
{
	size_t i, value_cnt;

	ASSERT(b != NULL);
	ASSERT(start <= b->bit_cnt);
	ASSERT(start + cnt <= b->bit_cnt);

	value_cnt = 0;
	for (i = 0; i < cnt; i++)
		if (bitmap_test(b, start + i) == value)
			value_cnt++;
	return value_cnt;
}

/* Returns true if any bits in B between START and START + CNT,
   exclusive, are set to VALUE, and false otherwise. */
bool bitmap_contains(const struct bitmap *b, size_t start, size_t cnt, bool value)
{
	size_t i;

	ASSERT(b != NULL);
	ASSERT(start <= b->bit_cnt);
	ASSERT(start + cnt <= b->bit_cnt);

	for (i = 0; i < cnt; i++)
		if (bitmap_test(b, start + i) == value)
			return true;
	return false;
}

/* Returns true if any bits in B between START and START + CNT,
   exclusive, are set to true, and false otherwise.*/
bool bitmap_any(const struct bitmap *b, size_t start, size_t cnt)
{
	return bitmap_contains(b, start, cnt, true);
}

/* Returns true if no bits in B between START and START + CNT,
   exclusive, are set to true, and false otherwise.*/
bool bitmap_none(const struct bitmap *b, size_t start, size_t cnt)
{
	return !bitmap_contains(b, start, cnt, true);
}

/* Returns true if every bit in B between START and START + CNT,
   exclusive, is set to true, and false otherwise. */
bool bitmap_all(const struct bitmap *b, size_t start, size_t cnt)
{
	return !bitmap_contains(b, start, cnt, false);
}

/* Finding set or unset bits. */

/* Finds and returns the starting index of the first group of CNT
   consecutive bits in B at or after START that are all set to
   VALUE.
   If there is no such group, returns BITMAP_ERROR. */
size_t bitmap_scan(const struct bitmap *b, size_t start, size_t cnt, bool value)
{
	ASSERT(b != NULL);
	ASSERT(start <= b->bit_cnt);

	if (cnt <= b->bit_cnt) {
		size_t last = b->bit_cnt - cnt;
		size_t i;
		for (i = start; i <= last; i++)
			if (!bitmap_contains(b, i, cnt, !value))
				return i;
	}
	return BITMAP_ERROR;
}

/* Finds the first group of CNT consecutive bits in B at or after
   START that are all set to VALUE, flips them all to !VALUE,
   and returns the index of the first bit in the group.
   If there is no such group, returns BITMAP_ERROR.
   If CNT is zero, returns 0.
   Bits are set atomically, but testing bits is not atomic with
   setting them. */
size_t bitmap_scan_and_flip(struct bitmap *b, size_t start, size_t cnt, bool value)
{
	size_t idx = bitmap_scan(b, start, cnt, value);
	if (idx != BITMAP_ERROR)
		bitmap_set_multiple(b, idx, cnt, !value);
	return idx;
}

/* Dumps the contents of B to the console as hexadecimal. */
#define BITMAP_DUMP_WIDTH 8
void bitmap_dump(const struct bitmap *b)
{
	for (int i = 0; i < byte_cnt(b->bit_cnt); i++) {
		for (int j = 0; j < 8; j++) {
			printf("%d", bitmap_test(b, i * 8 + j));
		}
		if ((i + 1) % BITMAP_DUMP_WIDTH == 0) {
			printf("\n");
		} else {
			printf(" ");
		}
	}
}