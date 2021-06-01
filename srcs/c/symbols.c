/*******************************************************************************
* File name: symbols.c
*
* Description:
*		This file contains the neccessary functions to fix the offsets contained
*		in the different symbol tables of the binary file. These offsets will be
*		changed depending on the address at which our stub will be inserted.
*
* Public function(s):
*		void	fix_symbols(t_elf_ehdr *elf_header, t_fdet *fdet);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This small function returns a boolean depending on the "d_un" union member
 * used by the dynamic entry passed as argument. The return value will be TRUE
 * if the entry uses "d_un.d_ptr", and FALSE if it uses "d_un.d_val". */

static t_bool	dt_contains_ptr(t_elf_dyn *dyn)
{
	if (dyn->d_tag == DT_PLTGOT || dyn->d_tag == DT_HASH
		|| dyn->d_tag == DT_STRTAB || dyn->d_tag == DT_SYMTAB
		|| dyn->d_tag == DT_RELA || dyn->d_tag == DT_INIT
		|| dyn->d_tag == DT_FINI || dyn->d_tag == DT_REL
		|| dyn->d_tag == DT_FINI_ARRAY || dyn->d_tag == DT_INIT_ARRAY
		|| dyn->d_tag == DT_DEBUG || dyn->d_tag == DT_JMPREL
		|| (dyn->d_tag & DT_LOOS))
	{
		return (TRUE);
	}
	return (FALSE);
}

/* This function takes as arguments a pointer to the relocation dynamic section
 * (SHT_DYNAMIC) and our t_fdet structure. It will iterate over the table and 
 * fix pointers depending on the address where our stub will be inserted. */

static void	fix_dynamic(t_elf_dyn *dynamic, t_fdet *fdet)
{
	while (dynamic->d_tag != DT_NULL)
	{
		if (dt_contains_ptr(dynamic) == TRUE)
		{
			if (dynamic->d_un.d_ptr >= fdet->stub_addr)
				dynamic->d_un.d_ptr += fdet->stub_size;
		}
		dynamic = (void *)dynamic + sizeof(t_elf_dyn);
	}
}

/* This function takes as arguments a pointer to the relocation table
 * (SHT_RELA), the number of entries inside it, and our t_fdet structure. It 
 * will iterate over the table and fix offsets depending on the address where 
 * our stub will be inserted. */

static void	fix_rela(t_elf_rela *rela, t_elf_half relnum, t_fdet *fdet)
{
	while (relnum--)
	{
		if (rela->r_offset >= fdet->stub_addr)
			rela->r_offset += fdet->stub_size;
		if (rela->r_addend >= (t_elf_sxword)fdet->stub_size)
			rela->r_addend += fdet->stub_size;
		rela = (void *)rela + sizeof(t_elf_rela);
	}
}

/* This function takes as arguments a pointer to the relocation table (SHT_REL), 
 * the number of entries inside it, and our t_fdet structure. It will iterate 
 * over the table and fix offsets depending on the address where our stub will
 * be inserted. */

static void	fix_rel(t_elf_rel *rel, t_elf_half relnum, t_fdet *fdet)
{
	while (relnum--)
	{
		if (rel->r_offset >= fdet->stub_addr)
			rel->r_offset += fdet->stub_size;
		rel = (void *)rel + sizeof(t_elf_rel);
	}
}

/* This function takes as arguments a pointer to the dynamic symbol table
 * (SHT_DYNSYM) the number of symbols inside it, and our t_fdet structure. It 
 * will iterate over the symbol table and fix values depending on the address 
 * where our stub will be inserted. */

static void	fix_dynsym(t_elf_sym *sym, t_elf_half symcount, t_fdet *fdet)
{
	while (symcount--)
	{
		if (sym->st_value >= fdet->stub_addr)
			sym->st_value += fdet->stub_size;
		sym = (void *)sym + sizeof(t_elf_sym);
	}
}

/* This function takes as arguments a pointer to the symbol table (SHT_SYMTAB),
 * the number of symbols inside it, and our t_fdet structure. It will iterate
 * over the symbol table and fix values depending on the address where our stub
 * will be iserted. */

static void fix_symtab(t_elf_sym *sym, t_elf_half symcount, t_fdet *fdet)
{
	while (symcount--)
	{
		if (sym->st_value >= fdet->stub_addr)
			sym->st_value += fdet->stub_size;
		sym = (void *)sym + sizeof(t_elf_sym);
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * the first byte of the file in memory) and a t_fdet structure. It will iterate
 * over section headers to find symbol tables. When found, a function will be
 * called to fix the offsets and/or addresses contained in said symbol table,
 * depending on its type. */

void		fix_symbols(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	t_elf_shdr			*section;
	t_elf_half			shnum = elf_header->e_shnum;
	const t_elf_half	shentsize = elf_header->e_shentsize;

	section = get_shtable(elf_header);
	if (section == NULL || shentsize <= 0)
		return ;
	while (shnum)
	{
		if (section->sh_type == SHT_SYMTAB)
		{
			fix_symtab((void *)elf_header + section->sh_offset,
						section->sh_size / section->sh_entsize,
						fdet);
		}
		else if (section->sh_type == SHT_DYNSYM)
		{
			fix_dynsym((void *)elf_header + section->sh_offset,
						section->sh_size / section->sh_entsize,
						fdet);
		}
		else if (section->sh_type == SHT_REL)
		{
			fix_rel((void *)elf_header + section->sh_offset, 
					section->sh_size / section->sh_entsize, 
					fdet);
		}
		else if (section->sh_type == SHT_RELA)
		{
			fix_rela((void *)elf_header + section->sh_offset, 
					section->sh_size / section->sh_entsize, 
					fdet);
		}
		else if (section->sh_type == SHT_DYNAMIC)
			fix_dynamic((void *)elf_header + section->sh_offset, fdet);
		section = (void *)section + shentsize;
		shnum--;
	}
}
