/*******************************************************************************
* File name: encrypt.c
*
* Description:
*		This file contains functions needed for the encryption part of the
*		packer.
*
* Public function(s):
*		void	encrypt_text(t_elf_ehdr *elf_header);
*
* Notes:
*		Currently, the only encryption algorithm availble is "Chaskey-LTS".
*		Also, note that the algorithm is implemented in assembly. The sources
*		for the assembly code can be found in "srcs/asm/".
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This function takes as its only argument a pointer to the ELF header
 * (coincidentally pointing to the first byte of the file). It will call a 
 * "chaskey_encrypt" functiont (in chaskey_encrypt.S), while iterating over the
 * ".text" section. This will result in a fully encrypted ".text" section. */

void		encrypt_text(t_elf_ehdr *elf_header)
{
	t_elf_xword	n;
	t_elf_shdr	*text_shdr;
	void		*text_buff;
	t_uint8		data[17];

	text_shdr = get_shdr(elf_header, ".text");
	text_buff = (void *)elf_header + text_shdr->sh_offset;
	n = text_shdr->sh_size;
	while (n)
	{
		ft_memset(data, '\0', 17);
		ft_memmove(data, text_buff, 16);
		chaskey_encrypt(g_key, data);
		ft_memmove(text_buff, data, 16);
		if (n < 16)
			break ;
		text_buff += 16;
		n -= 16;
	}
}
