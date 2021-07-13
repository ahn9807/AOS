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

static int pt_load(struct ELF64_Phdr *phdr, file_t *file);

int elf_load(const char *file_name, struct intr_frame *if_)
{
	struct thread_info *th = thread_current_s();
	struct ELF64_Ehdr ehdr;
	struct ELF64_Phdr phdr;
	struct ELF64_Shdr shdr;
	file_t file;
	size_t cur_offset;
	int error_code = 0;

	th->p4 = vmm_new_p4();

	// To used in user process, we have to init tss->rsp and page table
	vmm_activate(th->p4);
	tss_update(th);

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

	elf_debug(&ehdr);

	if (elf_check_supported(&ehdr))
	{
		printf("invalid format\n");
		error_code = -1;
		goto done;
	}
cls();
	cur_offset = ehdr.e_phoff;
	for (int i = 0; i < ehdr.e_phnum; i++)
	{
		if (cur_offset < 0 || cur_offset > vfs_get_size(&file))
		{
			printf("offset error\n");
			goto done;
		}

		vfs_seek(&file, cur_offset, SEEK_SET);
		cur_offset += sizeof(phdr);
		if (vfs_read(&file, &phdr, sizeof(phdr)) != sizeof(phdr))
		{
			printf("read failed");
			error_code = -FS_INVALID;
			goto done;
		}

		switch (phdr.p_type)
		{
			case PT_LOAD:
				if(error_code = pt_load(&phdr, &file)) {
					goto done;
				}
				break;

			default:
				break;
		}
	}

done:
	PANIC("NOT IMPLEMENTED");
	return error_code;
}

static int pt_load(struct ELF64_Phdr *phdr, file_t *file) {
	if(elf_check_segment(phdr)) {
		printf("validation failed");
		return -1;
	}

	uint64_t file_offset = phdr->p_offset % phdr->p_align;
	uint64_t mem_vaddr = phdr->p_vaddr % phdr->p_align;
	uint64_t mem_remaning = phdr->p_vaddr - mem_vaddr;
	uint64_t read_bytes, zero_bytes;

	uint16_t flags = 0;
	flags |= phdr->p_flags == PF_W ? PAGE_WRITE : 0;

	// Data / Code section
	if(phdr->p_filesz >= phdr->p_memsz) {
		read_bytes = mem_remaning + phdr->p_filesz;
		zero_bytes = (read_bytes + PAGE_SIZE - (read_bytes) % PAGE_SIZE) - read_bytes;
	} 
	// Bss Section
	else {
		read_bytes = 0;
		zero_bytes = mem_remaning + phdr->p_memsz + PAGE_SIZE - (mem_remaning + phdr->p_memsz + PAGE_SIZE) % PAGE_SIZE; 
	}

	ASSERT((read_bytes + zero_bytes) % PAGE_SIZE == 0);
	ASSERT(file_offset % PAGE_SIZE == 0);

	while(read_bytes > 0 || zero_bytes > 0) {
		size_t page_read_bytes = read_bytes < PAGE_SIZE ? read_bytes : PAGE_SIZE;
		size_t page_zero_bytes = PAGE_SIZE - page_read_bytes;

		uint8_t* kpage = P2V(pmm_alloc());
		if(kpage == NULL) {
			return -1;
		}

		// if(vfs_read(file, kpage, page_read_bytes) != (int)page_read_bytes) {
		// 	pmm_free(kpage);
		// 	return -1;
		// }

		// memset(kpage + page_read_bytes, 0, page_zero_bytes);

		// if(vmm_set_page(thread_current()->p4, mem_vaddr, kpage, flags)) {
		// 	printf("failed to allocate at vm\n");
		// 	pmm_free(kpage);
		// 	return -1;
		// }

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