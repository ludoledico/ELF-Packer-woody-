/*******************************************************************************
* File name: get.c
*
* Description:
*		This file contain function that help retrieving informations (like
*		specific headers) from an ELF file. These can be called from anywhere as
*		long as they are given the right arguments.
*
* Public function(s):
*		t_elf_shdr	*get_shtable(t_elf_ehdr *elf_header);
*		t_elf_shdr	*get_shdr(t_elf_ehdr *elf_header, const char *target);
*		t_elf_phdr	*get_phdr(t_elf_ehdr *elf_header, t_elf_shdr *shdr);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This functions takes as arguments a pointer to the ELF header, and a pointer
 * to the secion header of the section table. It return the offset of the
 * section header string table (shstrtab), or 0 if an error occurs. */

static t_elf_off	get_strtab_off(t_elf_ehdr *elf_header, t_elf_shdr *sectable)
{
	t_elf_shdr	*strtab_header;
	t_elf_off	offset;

	if (elf_header->e_shstrndx == SHN_UNDEF)
		return (0);
	offset = (elf_header->e_shstrndx * elf_header->e_shentsize);
	strtab_header = (void *)sectable + offset;
	return (strtab_header->sh_offset);
}

/* This function takes as its single argument, a pointer to the ELF header. It
 * either return the section header of the section header table, or NULL if an
 * error occurs. */

t_elf_shdr	*get_shtable(t_elf_ehdr *elf_header)
{
	const t_elf_off	sectable_offset = elf_header->e_shoff;

	if (!sectable_offset)
		return (NULL);
	return ((void *)elf_header + sectable_offset);
}

/* This functions takes as arguments a pointer to the target ELF file header
 * (coincidentally pointing to the first byte of the file mapped in memory), and
 * a string. This function will return a pointer to the section header (Shdr)
 * which name correpsonds to the string passed as its second argument. To do so,
 * it will get section names in the Section Header String Table (shstrtab). If
 * an error occurs or if there is no such section to be found, the following
 * function will return NULL. */

t_elf_shdr	*get_shdr(t_elf_ehdr *elf_header, const char *target)
{
	t_elf_half			shnum = elf_header->e_shnum;
	const t_elf_half	shentsize = elf_header->e_shentsize;
	t_elf_shdr			*sectable;
	const char			*strtab;
	char	  			*secname;

	sectable = get_shtable(elf_header);
	strtab = (char *)elf_header + get_strtab_off(elf_header, sectable);
	if (strtab == NULL || sectable == NULL || shnum == 0)
		return (NULL);
	while (shnum)
	{
		secname = (char *)strtab + sectable->sh_name;
		if (secname)
		{
			if (!ft_strcmp(secname, target))
				return (sectable);
		}
		sectable = (void *)sectable + shentsize;
		shnum--;
	}
	return (NULL);
}

/* This functions takes as arguments a pointer to the target ELF file header
 * (coincidentally pointing to the first byte of the file mapped in memory), and
 * a pointer to a section header (Shdr). This function will then return a
 * pointer to the program header (Phdr) of the segment that contains the section
 * related to the header passed as its second argument. If an error occurs or if
 * there is no such header to be found, the following function will return NULL
 * instead. */

t_elf_phdr	*get_phdr(t_elf_ehdr *elf_header, t_elf_shdr *shdr)
{
	t_elf_half	phnum;
	t_elf_half	phentsize;
	t_elf_phdr	*phtable;

	phtable = (void *)elf_header + elf_header->e_phoff;
	phnum = elf_header->e_phnum;
	phentsize = elf_header->e_phentsize;
	if (phnum == 0 || shdr == NULL)
		return (NULL);
	while (phnum)
	{
		if (phtable->p_offset <= shdr->sh_offset
			&& (phtable->p_offset + phtable->p_memsz) > shdr->sh_offset)
			return (phtable);
		phtable = (void *)phtable + phentsize;
		phnum--;
	}
	return (NULL);
}
