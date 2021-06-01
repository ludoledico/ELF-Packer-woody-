/*******************************************************************************
* File name: mark.c
*
* Description:
*		This file contains functions that will mark the file as packed. This
*		mark is used in order to make sure that the target is not packed more
*		than once (which would be pointless loss of disk space).
*
* Public function(s):
*		void	mark_file(char *bin_content);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* See file header for details. */

void	mark_file(char *bin_content)
{
	ft_memmove(bin_content + 8, PACKER_MARK, 8);
}
