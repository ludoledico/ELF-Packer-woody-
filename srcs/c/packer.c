/*******************************************************************************
* File name: packer.c
*
* Description:
*		This file contains the function for the main packing engine.
*
* Public function(s):
*		int	pack_binary(int fd, t_fdet *file_details);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This function takes as arguments the (non-null) file descriptor of the target
 * binary file, and its size. The target file is copied into a new writeable 
 * memory projection, which is then returned. If an error occurs, MAP_FAILES is
 * returned instead. */

static char		*get_file_content(int fd, size_t file_size)
{
	char	*file_content;
	char	*packed_map;

	file_content = mmap(NULL, file_size, MMAP_PROT, MMAP_FLAGS, fd, 0);
	if (file_content == MAP_FAILED)
		return(MAP_FAILED);
	packed_map = mmap(NULL, file_size + align_to_psize(STUB_SIZE) + getpagesize(), MMAP_PROT, MMAP_FLAGS|MAP_ANON, -1, 0);
	if (packed_map == MAP_FAILED)
	{
		munmap(file_content, file_size);
		return(MAP_FAILED);
	}
	ft_memmove(packed_map, file_content, file_size);
	if (munmap(file_content, file_size) == -1)
		return(MAP_FAILED);
	return (packed_map);
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * pointing to the first byte of the file in memory) and a t_fdet structure.
 * It will fill all the members of the t_fdet structure that were not already
 * filled by the "get_file_details" function (in main.c). In case of errors, it
 * will return ERR_UNK, or 0 otherwise. */

static t_uint8	get_elf_details(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	t_elf_shdr *text_shdr;
	t_elf_shdr *bss_shdr;
	t_elf_phdr *text_phdr;
	t_elf_phdr *bss_phdr;

	text_shdr = get_shdr(elf_header, ".text");
	bss_shdr = get_shdr(elf_header, ".bss");
	text_phdr = get_phdr(elf_header, text_shdr);
	bss_phdr = get_phdr(elf_header, bss_shdr);
	if (text_phdr == NULL || bss_phdr == NULL)
		return (ERR_UNK);
	fdet->orig_ep = elf_header->e_entry;
	if (elf_header->e_type == ET_DYN)
	{
		fdet->stub_offset = text_phdr->p_offset;
		fdet->stub_addr = text_phdr->p_vaddr;
	}
	else
	{
		fdet->stub_offset = bss_phdr->p_offset + bss_phdr->p_memsz;
		fdet->stub_addr = bss_phdr->p_vaddr + bss_phdr->p_memsz;
		fdet->bss_size = bss_phdr->p_memsz - bss_phdr->p_filesz;
	}
	fdet->stub_size = align_to_psize(STUB_SIZE);
	fdet->text_size = text_shdr->sh_size;
	fdet->text_addr = text_shdr->sh_addr;
	fdet->bin_size += fdet->bss_size;
	return (0);
}

/* This functions takes as arguments the (non-zero) file descriptor of the
 * valid target binary, and a t_fdet structure with its bin_mode, bin_size, and
 * stub_size already filled. It return 0 or an error code. */

int				pack_binary(int fd, t_fdet *file_details)
{
	char		*bin_content;

	if ((bin_content = get_file_content(fd, file_details->bin_size)) == MAP_FAILED)
		return (ERR_MMAP);
	if (get_elf_details((t_elf_ehdr *)bin_content, file_details) != 0)
		return (ERR_UNK);
	((t_elf_ehdr *)bin_content)->e_entry = file_details->stub_addr;
	encrypt_text((t_elf_ehdr *)bin_content);
	fix_offsets((t_elf_ehdr *)bin_content, file_details);
	insert_stub((t_elf_ehdr *)bin_content, file_details);
	mark_file(bin_content);
	write_new_file(bin_content, *file_details);
	munmap(bin_content, file_details->bin_size);
	return (0);
}
