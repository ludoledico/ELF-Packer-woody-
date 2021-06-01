/*******************************************************************************
* File name: main.c
*
* Description:
*		This program takes a single ELF binary as argument. A clone of this
*		binary will be created, with its ".text" section encrypted. When
*		executed, said clone will first decrypt its own ".text" section and
*		proceed with a normal execution.
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

/* This function takes the (non-zero) file descriptor of the target binary, and
 * fills a new t_fdet structure according to its stats. */

static t_fdet	*get_file_details(const int fd)
{
	t_fdet	*fdet;
	t_stat	stats;

	if (!(fdet = (t_fdet *)malloc(sizeof(t_fdet))))
	{
		perror("malloc error: ");
		free(fdet);
		close(fd);
		exit(ERR_MEM_ALLOC);
	}
	ft_memset(fdet, '\0', sizeof(t_fdet));
	if (fstat(fd, &stats) < 0)
	{
		perror("fstat error: ");
		free(fdet);
		close(fd);
		exit(ERR_NO_STATS);
	}
	if (!(stats.st_mode & S_IFREG))
	{
		write(2, "error: bad file type.\n", 22); 
		free(fdet);
		close(fd);
		exit(ERR_BAD_FILE);
	}
	fdet->bin_size = stats.st_size;
	fdet->bin_mode = stats.st_mode;
	fdet->stub_size = align_to_psize(STUB_SIZE);
	return (fdet);
}

/* See details for the program's behaviour in the file header. */

int				main(int argc, char **argv)
{
	int			return_value;
	int			fd;
	t_fdet		*file_details;

	return_value = 0;
	fd = 0;
	file_details = NULL;
	if (argc == 1)
	{
		print_err(ERR_NO_ARG);
		return (ERR_NO_ARG);
	}
	if ((fd = open(argv[1], O_RDONLY)) < 0)
	{
		print_err(ERR_OPEN);
		return_value = ERR_OPEN;
	}
	else if (check_binary(fd) != TRUE)
	{
		print_err(ERR_BAD_BIN);
		return_value = ERR_BAD_BIN;
	}
	else if ((file_details = get_file_details(fd))->bin_size <= 0)
	{
		print_err(ERR_NO_STATS);
		return_value = ERR_NO_STATS;
	}
	else if ((return_value = pack_binary(fd, file_details)) > 0)
	{
		print_err(return_value);
	}
	if (file_details)
		free(file_details);
	if (fd >= 0)
		close(fd);
	return (return_value);
}
