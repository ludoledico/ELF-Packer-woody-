/*******************************************************************************
* File name: output.c
*
* Description:
*		This file contains the function(s) that will help write the new packed
*		binary. The new file will be named woody and have the same access rights
*		as the original ELF.
*
* Public function(s):
*		-
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* See file header for details. */

void		write_new_file(char *packed_bin, t_fdet fdet)
{
	int		fd;

	fd = open("woody", O_WRONLY|O_CREAT, fdet.bin_mode);
	write(fd, packed_bin, fdet.bin_size);
	close(fd);
}
