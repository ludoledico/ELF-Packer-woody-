/*******************************************************************************
* File name: fix_offsets.c
*
* Description:
*		In the case of a pre-text stub injection, some offsets contained in
*		tables or symbols need to be fixed. The same goes for the section header
*		table and program header table. This file contains all the necessary
*		code to do so.
*
* Public function(s):
*		void	fix_offsets(t_elf_ehdr *elf_header, t_fdet *fdet);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. If need be,
 * this function will change the offset of some program headers (Phdr) depending
 * on where the stub will inserted. It may also change the offset of the table
 * itself if necessary. */

static void		fix_phdr_offsets(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	const t_elf_half	phentsize = elf_header->e_phentsize;
	t_elf_half			phnum = elf_header->e_phnum;
	t_elf_phdr			*phtable = (void *)elf_header + elf_header->e_phoff;

	if (phentsize <= 0 || phtable == NULL)
		return ;
	if (elf_header->e_phoff > fdet->stub_offset)
		elf_header->e_phoff	+=	fdet->stub_size;
	while (phnum)
	{
		if (phtable->p_vaddr < fdet->orig_ep 
			&& phtable->p_vaddr + phtable->p_memsz > fdet->orig_ep)
		{
			fix_text_phdr(elf_header, phtable, fdet);
		}
		else if (elf_header->e_type == ET_EXEC
				&& phtable->p_offset <= fdet->stub_offset
				&& (phtable->p_offset + phtable->p_memsz) >= fdet->stub_offset)
		{
			fix_bss_phdr(phtable, fdet);
		}
		else if (phtable->p_offset >= fdet->stub_offset)
		{
			phtable->p_offset	+=	fdet->stub_size;
			phtable->p_paddr	+=	fdet->stub_size;
			phtable->p_vaddr	+=	fdet->stub_size;
		}
		phtable = (void *)phtable + phentsize;
		phnum--;
	}
}


/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. If need be,
 * this function will change the offset of some section headers (Shdr) depending
 * on where the stub will inserted. It may also change the offset of the table
 * itself if necessary. */

static void		fix_shdr_offsets(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	const t_elf_half	shentsize = elf_header->e_shentsize;
	t_elf_half			shnum = elf_header->e_shnum;
	t_elf_shdr			*sectable;
	t_elf_xword			size_to_add;

	size_to_add = fdet->stub_size;
	if (elf_header->e_type == ET_EXEC)
		size_to_add += fdet->bss_size;
	sectable = get_shtable(elf_header);
	if (sectable == NULL || shentsize <= 0)
		return ;
	if (elf_header->e_shoff > fdet->stub_offset)
		elf_header->e_shoff	+=	size_to_add;
	while (shnum)
	{
		if (sectable->sh_offset >= fdet->stub_offset)
		{
			sectable->sh_offset += size_to_add;
			if (sectable->sh_addr)
				sectable->sh_addr += size_to_add;
		}
		sectable = (void *)sectable + shentsize;
		shnum--;
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. If need be,
 * this function will update the offsets of GOT (Global Offset Table) entries
 * depending on the address where the stub will be inserted. This function
 * returns nothing. */

static void	fix_got(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	t_elf_shdr	*shdr;
	t_elf_addr	*entry;
	t_elf_half	entries_count;

	if (!(shdr = get_shdr(elf_header, ".got")))
		return ;
	entries_count = shdr->sh_size / sizeof(t_elf_xword);
	entry = (void *)elf_header + shdr->sh_offset;
	while (entries_count--)
	{
		if (*entry >= fdet->stub_addr)
			*entry += fdet->stub_size;
		entry = (void *)entry + sizeof(t_elf_xword);
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. If need be,
 * this function will update the offsets of GOT_PLT (Global Offset Table, 
 * Procedure Linkage Table) entries depending on the address where the stub will 
 * be inserted. This function returns nothing. */

static void	fix_got_plt(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	t_elf_shdr	*shdr;
	t_elf_addr	*entry;
	t_elf_half	entries_count;

	if (!(shdr = get_shdr(elf_header, ".got.plt")))
		return ;
	entries_count = shdr->sh_size / sizeof(t_elf_xword);
	entry = (void *)elf_header + shdr->sh_offset;
	while (entries_count--)
	{
		if (*entry >= fdet->stub_addr)
			*entry += fdet->stub_size;
		entry = (void *)entry + sizeof(t_elf_xword);
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. If need be,
 * this function will fix the value of some offsets in the PREINIT_ARRAY, 
 * INIT_ARRAY, and FINI_ARRAY sections depending on where the stub will
 * inserted. */

static void		fix_arrays(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	t_elf_shdr			*section;
	t_elf_half			shnum = elf_header->e_shnum;
	const t_elf_half	shentsize = elf_header->e_shentsize;
	t_elf_half			pointer_count;
	t_elf_addr			*array;

	section = get_shtable(elf_header);
	if (section == NULL || shentsize <= 0)
		return ;
	while (shnum)
	{
		if (section->sh_type == SHT_PREINIT_ARRAY
			|| section->sh_type == SHT_INIT_ARRAY
			|| section->sh_type == SHT_FINI_ARRAY)
		{
			pointer_count = section->sh_size / section->sh_entsize;
			array = (void *)elf_header + section->sh_offset;
			while (pointer_count)
			{
				--pointer_count;
				if (array[pointer_count] >= fdet->stub_addr)
					array[pointer_count] += fdet->stub_size;
			}
		}
		section = (void *)section + shentsize;
		shnum--;
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. It contains all
 * the necessary function calls to fix all of the offsets invalidated by our
 * stub injection. Beware, the order of those calls may be important. */

void			fix_offsets(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	fix_symbols((t_elf_ehdr *)elf_header, fdet);
	fix_got_plt((t_elf_ehdr *)elf_header, fdet);
	fix_got((t_elf_ehdr *)elf_header, fdet);
	fix_arrays((t_elf_ehdr *)elf_header, fdet);
	fix_shdr_offsets((t_elf_ehdr *)elf_header, fdet);
	fix_phdr_offsets((t_elf_ehdr *)elf_header, fdet);
}
