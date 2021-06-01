/*******************************************************************************
* File name: segments.c
*
* Description:
*		This files contain functions needed to update to contents of different
*		program headers. These functions are called by the fix_phdr function (in
*		packer.c).
*
* Public function(s):
*		void	fix_text_phdr(t_elf_ehdr *elf_header, 
*							t_elf_phdr *phdr, 
*							t_fdet *fdet);
*		void	fix_bss_phdr(t_elf_phdr *phdr, t_fdet *fdet);
*
* Notes:
*		In the case of a post-bss injection, the data segment will be granted
*		with both execution and write rights. Please note that this may not work
*		on some hardened or custom kernels which have the (W^E option set).
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* In the case of a pre-text stub injection, this function will be called in
 * order to make sure that the whole stub is included inside of the text
 * segment. */

void	fix_text_phdr(t_elf_ehdr *elf_header, t_elf_phdr *phdr, t_fdet *fdet)
{
	if (elf_header->e_type == ET_DYN)
	{
		phdr->p_memsz	+=	fdet->stub_size;
		phdr->p_filesz	+=	fdet->stub_size;
	}
}

/* In the case of a post-bss stub injection, this function will be called in
 * order to make sure that the whole stub is included inside of the data
 * segment. We also make sure that the data segment is executable. */

void	fix_bss_phdr(t_elf_phdr *phdr, t_fdet *fdet)
{
	phdr->p_memsz	+=	fdet->stub_size;
	phdr->p_filesz	=	phdr->p_memsz;
	phdr->p_flags	|=	PF_X;
}
