#include <stddef.h>
#include <stdint.h>
#include "elf.h"
#include "debug.h"
#include "vfs.h"
#include "thread.h"
#include "vmm.h"
#include "tss.h"
#include "string.h"

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
				if(elf_check_segment(&phdr)) {
					printf("segment validation failed\n");
					error_code = -1;
					goto done;
				}

				uint64_t align = phdr.p_offset % PAGE_SIZE;
				uint64_t aling_p = (phdr.p_vaddr + phdr.p_filesz) % PAGE_SIZE;

				int flags = 0;
				break;

			default:
				break;
		}
	}

done:
	PANIC("NOT IMPLEMENTED");
	return error_code;
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