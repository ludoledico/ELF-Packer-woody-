/*******************************************************************************
* File name: check.c
*
* Description:
*		This file contains the function that check the validity of the target
*		file.
*
* Public function(s):
*		t_bool	check_binary(int fd);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This function takes the (non-null) file descriptor of the target binary file
 * as argument. If the file has already been marked by our packer, it will
 * return FALSE. */

static t_bool	is_marked(int fd)
{
	char	buff[9];

	buff[8] = '\0';
	if (lseek(fd, 8, SEEK_SET) < 0)
	{
		perror("failed to reset the file offset: ");
		return (TRUE);
	}
	if (read(fd, buff, 8) < 8)
		return (TRUE);
	if (!ft_strcmp(PACKER_MARK, buff))
		return (TRUE);
	if (lseek(fd, 0, SEEK_SET) < 0)
	{
		perror("failed to reset the file offset: ");
		return (TRUE);
	}
	return (FALSE);
}

/* This function takes the (non-null) file descriptor of the target binary file 
 * as argument. It checks the validity for the magic number located at the 
 * beginning of the file, and return a boolean accordingly. */

static t_bool	is_elf_magic(int fd)
{
	char	buff[6];

	buff[5] = '\0';
	if (read(fd, buff, 5) < 5)
		return (FALSE);
	if (buff[0] != ELFMAG0)
		return (FALSE);
	if (buff[1] != ELFMAG1)
		return (FALSE);
	if (buff[2] != ELFMAG2)
		return (FALSE);
	if (buff[3] != ELFMAG3)
		return (FALSE);
#ifdef WOODY_32BIT
	if (buff[4] != ELFCLASS32)
		return (FALSE);
#else
	if (buff[4] != ELFCLASS64)
		return (FALSE);
#endif
	if (lseek(fd, 0, SEEK_SET) < 0)
	{
		perror("failed to reset the file offset: ");
		return (FALSE);
	}
	return (TRUE);
}

/* This function take the (non-null) file descriptor of the target binary file.
 * It returns a boolean value: TRUE if the target is a valid ELF target, FALSE 
 * otherwise */

t_bool			check_binary(int fd)
{
	if (is_elf_magic(fd) == FALSE
		|| is_marked(fd) == TRUE)
		return (FALSE);
	return (TRUE);
}
