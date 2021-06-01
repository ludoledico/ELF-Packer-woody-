/*******************************************************************************
* File name: insert_stub.c
*
* Description:
*		This file contain all the functions related to the insertion of our stub
*		on the to-be-created binary file. This stub contains magic values that
*		will be changed to offsets depending on the target executable. Two
*		different stubs are available depending on the CPU architecture (32bit
*		or 64bit).
*
* Public function(s):
*		void	insert_stub(t_elf_ehdr *elf_header, t_fdet *fdet);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * pointing to the first byte of the file in memory) and a pointer to a t_fdet
 * structure. It will insert as much physical padding to the file (physical
 * zeros that can be read even before memory mapping) as the initial "virtual"
 * padding produced by the difference between the memory size and file size of
 * the data segment. Which means that the resulting new file will be heavier by
 * at least as much as the initial bss size. This function returns nothing. */

static void	pad_bss(void *elf_header, t_fdet *fdet)
{
	t_uint64	remaining_size;

	remaining_size = fdet->bin_size - (fdet->stub_offset - fdet->bss_size);
	ft_memmove(elf_header + fdet->stub_offset, 
				elf_header + (fdet->stub_offset - fdet->bss_size), 
				remaining_size);
	ft_memset(elf_header + (fdet->stub_offset - fdet->bss_size), 
				'\0', 
				fdet->bss_size);
}

/* This function takes as arguments a pointer to the beginning of the stub that
 * needs to be fixed, the magic value that needs to be found, and the long
 * integer that is supposed to replace it. It will search the stub byte by byte 
 * until the magic the value magic value is found, and then replace it with the 
 * correct long integer. This function does not return anything. */

static void	fix_magic_long(t_uint8 *stub, long value, long target)
{
	t_uint64	qword;

	for (int i = 0; i < STUB_SIZE; i++)
	{
		qword = *(long *)(stub + i);
		if (!(value ^ qword))
			*((long *)(stub + i)) = target;
	}
}

/* This function takes as arguments a pointer to the beginning of the stub that
 * needs to be fixed, the magic value that needs to be found, and the integer 
 * that is supposed to replace it. It will search the stub byte by byte until 
 * the magic the value magic value is found, and then replace it with the 
 * correct integer. This function does not return anything. */

static void	fix_magic_int(t_uint8 *stub, int value, int target)
{
	t_uint32	word;

	for (int i = 0; i < STUB_SIZE; i++)
	{
		word = *(long *)(stub + i);
		if (!(value ^ word))
			*((int *)(stub + i)) = target;
	}
}

/* This function takes as arguments a pointer to the beginning of the stub that
 * needs fixing, the magic value that needs to be recognized (ideally defined in
 * woody.h), the target value that needs to be inserted, and the lenght of said
 * target. The stub will be iterated over byte by byte until a match with the
 * magic values happens. This value will then be replaced by the target array.
 * Once the iteration is done, nothing is returned. */

static void	fix_magic_array(t_uint8 *stub, long value,
							const t_uint8 *target, t_uint32 len)
{
	t_uint64	qword;

	for (int i = 0; i < STUB_SIZE; i++)
	{
		qword = *(long *)(stub + i);
		if (!(value ^ qword))
		{
			for (t_uint32 j = 0; j < len; j++)
				*((t_uint8 *)(stub + i + j)) = target[j];
		}
	}
}

/* This function takes as arguments a pointer to the ELF header (coincidentally
 * pointing to the first byte of the file in memory) and a pointer to a t_fdet
 * structure. It does not return any value. It will add paddding according to
 * the stub_size member of the t_fdet structure, at the offset declared in the
 * stub_offset member of that same structre. This padding is then zeroed out and
 * depending on the CPU architecture defined at compile time (see Makefile
 * options), either a 32bit or 64bit assembly stub will be inserted at the
 * beggining of this padding. Said stub contains magic values that are replaced
 * with correct ones calculated at the end of this function. Also, note that if
 * the ELF is not Position Independent, the stub will be inserted after the bss
 * instead of at the beging of the text, meaning that the pad_bss function needs
 * to be called. */

void		insert_stub(t_elf_ehdr *elf_header, t_fdet *fdet)
{
	void		*stub_addr;
	size_t		remaining_size;
	t_elf_xword	stub_size;

	stub_size = 0;
	fdet->bin_size += fdet->stub_size;
	stub_addr = (void *)elf_header + fdet->stub_offset;
	remaining_size = fdet->bin_size - fdet->stub_offset;
	if (elf_header->e_type == ET_EXEC)
		pad_bss(elf_header, fdet);
	ft_memmove(stub_addr + fdet->stub_size, stub_addr, remaining_size);
	ft_memset(stub_addr, '\0', fdet->stub_size);
#ifdef	WOODY_32BIT
	ft_memmove(stub_addr, g_stub_32, STUB_SIZE);
#else	/* not WOODY_32BIT */
	ft_memmove(stub_addr, g_stub_64, STUB_SIZE);
#endif	/* not WOODY_32BIT */
	if (elf_header->e_type == ET_DYN)
		stub_size = fdet->stub_size;
#ifdef	WOODY_32BIT
	fix_magic_int(stub_addr, STUB_JMP_MAGIC32, stub_size + (fdet->orig_ep - elf_header->e_entry));
	fix_magic_int(stub_addr, STUB_BUFF_MAGIC32, stub_size + (fdet->text_addr - elf_header->e_entry));
	fix_magic_int(stub_addr, STUB_SIZE_MAGIC32, fdet->text_size);
	(void)fix_magic_long;
#else	/* not WOODY_32BIT */
	fix_magic_long(stub_addr, STUB_JMP_MAGIC, stub_size + (fdet->orig_ep - elf_header->e_entry));
	fix_magic_long(stub_addr, STUB_BUFF_MAGIC, stub_size + (fdet->text_addr - elf_header->e_entry));
	fix_magic_long(stub_addr, STUB_SIZE_MAGIC, fdet->text_size);
	(void)fix_magic_int;
#endif	/* not WOODY_32BIT */
	fix_magic_array(stub_addr, STUB_KEY_MAGIC, g_key, KEY_LEN);
}
