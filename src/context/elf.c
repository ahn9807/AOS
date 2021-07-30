#include <stddef.h>
#include <stdint.h>
#include "elf.h"
#include "debug.h"
#include "vfs.h"
#include "thread.h"
#include "vmm.h"
#include "tss.h"
#include "string.h"
#include "pmm.h"
#include "layout.h"
#include "kmalloc.h"
#include "process.h"
#include "msr_flags.h"

#define ALIGN_ADDR(src) ((src) - (src) % (alignment))
#define ROUND_UP(X, STEP) (((X) + (STEP) - 1) / (STEP) * (STEP))

static int pt_load(struct ELF64_Phdr *phdr, file_t *file);
static int pt_tls(struct ELF64_Phdr *phdr, file_t *file);

int elf_load(struct process_info *proc, const char *file_name, struct intr_frame *if_)
{
	struct thread_info *th = thread_current_s();
	struct ELF64_Ehdr ehdr;
	struct ELF64_Shdr shdr;
	struct ELF64_Phdr* phdrs;
	file_t file;
	size_t cur_offset;
	int error_code = 0;

	th->p4 = vmm_new_p4();

	if (vfs_open_by_path(file_name, &file))
	{
		printf("open failed");
		error_code = -FS_NO_ENTRY;
		goto done;
	}

	vfs_seek(&file, 0, SEEK_SET);
	if (vfs_read(&file, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
	{
		printf("read failed");
		error_code = -FS_INVALID;
		goto done;
	}

	if (elf_check_supported(&ehdr))
	{
		printf("invalid format\n");
		error_code = -1;
		goto done;
	}

	phdrs = kmalloc(ehdr.e_phentsize * ehdr.e_phnum);

	cur_offset = ehdr.e_phoff;
	for (int i = 0; i < ehdr.e_phnum; i++)
	{
		if (cur_offset < 0 || cur_offset > vfs_get_size(&file))
		{
			printf("offset error\n");
			goto done;
		}

		vfs_seek(&file, cur_offset, SEEK_SET);
		cur_offset += ehdr.e_phentsize;
		if (vfs_read(&file, &phdrs[i], sizeof(struct ELF64_Phdr)) != sizeof(struct ELF64_Phdr))
		{
			printf("read failed");
			error_code = -FS_INVALID;
			goto done;
		}

		switch (phdrs[i].p_type)
		{
			case PT_LOAD:
				if(error_code = pt_load(&phdrs[i], &file)) {
					goto done;
				}
				break;
			case PT_TLS:
				if(error_code = pt_tls(&phdrs[i], &file)) {
					goto done;
				}
				break;
			default:
				break;
		}
	}

	if_->rip = ehdr.e_entry;

	proc->ehdr = kmalloc(sizeof(struct ELF64_Ehdr));
	memcpy(proc->ehdr, &ehdr, sizeof(struct ELF64_Ehdr));
	proc->phdrs = kmalloc(sizeof(struct ELF64_Phdr) * ehdr.e_phnum);
	memcpy(proc->phdrs, phdrs, ehdr.e_phentsize * ehdr.e_phnum);

done:
	kfree(phdrs);
	return error_code;
}

static int pt_tls(struct ELF64_Phdr *phdr, file_t *file) {
	if(elf_check_segment(phdr)) {
		printf("validation failed");
		return -1;
	}

	uint64_t alignment = phdr->p_align;
	uint64_t file_offset = ALIGN_ADDR(phdr->p_offset);
	uint64_t mem_vaddr = ALIGN_ADDR(phdr->p_vaddr);
	uint64_t mem_remaning = phdr->p_vaddr - mem_vaddr;
	uint64_t read_bytes, zero_bytes;
	uint16_t flags = (phdr->p_flags & PF_W ? PAGE_WRITE | PAGE_USER_ACCESSIBLE | PAGE_PRESENT : PAGE_USER_ACCESSIBLE | PAGE_PRESENT);

	// Data / Code section
	if(phdr->p_filesz > 0) {
		read_bytes = mem_remaning + phdr->p_filesz;
		zero_bytes = ROUND_UP(mem_remaning + phdr->p_memsz, alignment) - read_bytes;
	} 
	// Bss Section
	else {
		read_bytes = 0;
		zero_bytes = ROUND_UP(mem_remaning + phdr->p_memsz, alignment);
	}

	ASSERT((read_bytes + zero_bytes) % alignment == 0);
	ASSERT(file_offset % alignment == 0);

	while((int)read_bytes > 0 || (int)zero_bytes > 0) {
		size_t page_read_bytes = read_bytes < PAGE_SIZE ? read_bytes : PAGE_SIZE;
		size_t page_zero_bytes = PAGE_SIZE - page_read_bytes;

		uint8_t* kpage = pmm_alloc();
		if(kpage == NULL) {
			return -1;
		}

		if(vmm_set_page(thread_current()->p4, mem_vaddr, kpage, flags)) {
			printf("failed to allocate at vm\n");
			pmm_free(kpage);
			return -1;
		}

		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		mem_vaddr += PAGE_SIZE;
	}

	// Set FS base for TLS Section
	// Short... But pretty, very important!
	// Wait... is there any race condition at hear?? <-FIX!
	write_msr(MSR_FS_BASE, mem_vaddr);

	return 0;
}

static int pt_load(struct ELF64_Phdr *phdr, file_t *file) {
	if(elf_check_segment(phdr)) {
		printf("validation failed");
		return -1;
	}

	uint64_t alignment = phdr->p_align;
	uint64_t file_offset = ALIGN_ADDR(phdr->p_offset);
	uint64_t mem_vaddr = ALIGN_ADDR(phdr->p_vaddr);
	uint64_t mem_remaning = phdr->p_vaddr - mem_vaddr;
	uint64_t read_bytes, zero_bytes;
	uint16_t flags = (phdr->p_flags & PF_W ? PAGE_WRITE | PAGE_USER_ACCESSIBLE | PAGE_PRESENT : PAGE_USER_ACCESSIBLE | PAGE_PRESENT);

	// Data / Code section
	if(phdr->p_filesz > 0) {
		read_bytes = mem_remaning + phdr->p_filesz;
		zero_bytes = ROUND_UP(mem_remaning + phdr->p_memsz, PAGE_SIZE) - read_bytes;
	} 
	// Bss Section
	else {
		read_bytes = 0;
		zero_bytes = ROUND_UP(mem_remaning + phdr->p_memsz, PAGE_SIZE);
	}

	ASSERT((read_bytes + zero_bytes) % PAGE_SIZE == 0);
	ASSERT(file_offset % PAGE_SIZE == 0);

	vfs_seek(file, file_offset, SEEK_SET);
	while((int)read_bytes > 0 || (int)zero_bytes > 0) {
		size_t page_read_bytes = read_bytes < PAGE_SIZE ? read_bytes : PAGE_SIZE;
		size_t page_zero_bytes = PAGE_SIZE - page_read_bytes;

		uint8_t* kpage = pmm_alloc();
		if(kpage == NULL) {
			return -1;
		}

		if(vfs_read(file, P2V(kpage), page_read_bytes) != (int)page_read_bytes) {
			pmm_free(kpage);
			return -1;
		}

		memset(P2V(kpage + page_read_bytes), 0, page_zero_bytes);

		if(vmm_set_page(thread_current()->p4, mem_vaddr, kpage, flags)) {
			printf("failed to allocate at vm\n");
			pmm_free(kpage);
			return -1;
		}

		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		mem_vaddr += PAGE_SIZE;
	}

	return 0;
}

int elf_check_supported(struct ELF64_Ehdr *ehdr)
{
	if (
		ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
		ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
		ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
		ehdr->e_ident[EI_MAG3] != ELFMAG3 ||
		ehdr->e_type != ET_EXEC ||
		ehdr->e_machine != EM_X86_64 ||
		ehdr->e_version != 1 ||
		// ehdr->e_phentsize != sizeof(ELF64_Phdr) ||
		ehdr->e_phnum > 1024)
	{
		return -1;
	}

	return 0;
}

int elf_check_segment(struct ELF64_Phdr *phdr) {
	return 0;
}

void elf_debug(struct ELF64_Ehdr *ehdr)
{
	printf("ELF MACHINE: %x\n", ehdr->e_machine);
	printf("ELF TYPE: %d\n", ehdr->e_type);
	printf("ELF VERSION: %s\n", ehdr->e_version);
	printf("ELF PHNUM: %d\n", ehdr->e_phnum);
}