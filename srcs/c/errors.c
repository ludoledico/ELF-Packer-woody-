/*******************************************************************************
* File name: error.c
*
* Description:
*		This file contains the necessary function to print error messages.
*
* Public function(s):
*		void	print_err(int errcode);
*
* Notes:
*		...
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* See details in file header */

void		print_err(int errcode)
{
	if (errcode == ERR_NO_ARG)
		write(2, "(ERR_NO_ARG) woody_woodpacker: No arguments were provided.\n", 59);
	else if (errcode == ERR_BAD_BIN)
		write(2, "(ERR_BAD_BIN) woody_woodpacker: Invalid magic number at the start of the file.\n", 79);
	else
	{
		write(2, g_err_strings[errcode - 1], ft_strlen(g_err_strings[errcode - 1]));
		perror(" woody_woodpacker");
	}
}
