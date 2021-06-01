/*******************************************************************************
* File name: misc.c
*
* Description:
*		This file contanins functions that have been wrote to replicate the
*		behaviour of some libC functions. This is only due to the limitations of
*		being a school project, in which the use of external libraies is very
*		limited (the list of function I am authorized to use without rewriting
*		them myself is available in the README of this repository).
*
* Public function(s):
*		void	*ft_memmove(void *dest, void const *src, size_t n);
*		void	*ft_memset(void *address, int byte_value, size_t size);
*		int     ft_strcmp(char const *s1, char const *s2);
*		size_t	ft_strlen(const char *s);
*		size_t	align_to_n(size_t size, t_uint32 n);
*		size_t	align_to_psize(size_t size);
*
* Notes:
*		While these functions behave like you would expect. They may be way less
*		optimized than their homonyms from the standard C library. For personal
*		use, feel free to replace calls to "ft_' functions by standard ones.
*
* Author: Ludovic Montagnier (lmontagn)
*******************************************************************************/

#include "woody.h"

/* This sub-function is only called by ft_memmove. */

static void	memcpy_rev(void *dest, void const *src, size_t n)
{
	char const	*s;
	char		*d;
	int			i;

	s = src;
	d = dest;
	i = n;
	while (i > 0)
	{
		d[i - 1] = s[i - 1];
		i--;
	}
}

/* This sub-function is only called by ft_memmove. */

static void	*ft_memcpy(void *dest, void const *src, size_t n)
{
	char const	*s;
	char		*d;
	int			i;

	s = src;
	d = dest;
	i = 0;
	while (n)
	{
		*d = s[i];
		i++;
		d++;
		n--;
	}
	return (dest);
}

/* This function aims to follow the same behaviour as string.h's memmove. */

void		*ft_memmove(void *dest, void const *src, size_t n)
{
	if (dest > src)
		memcpy_rev(dest, src, n);
	else
		ft_memcpy(dest, src, n);
	return (dest);
}

/* This function takes a size_t "size", and an unsigned integer "n" as
 * arguments. It returns the first multiple of n that is superior of equal to
 * size. */

size_t		align_to_n(size_t size, t_uint32 n)
{
	return ((size + (n - 1)) & ~(n - 1));
}

/* This function takes a size_t "size" as its only argument. It returns the 
 * first multiple of size that is superior of equal to the page size that is
 * currently defined. */

size_t		align_to_psize(size_t size)
{
	size_t	pagesize;

	pagesize = getpagesize();
	return ((size + (pagesize - 1)) & ~(pagesize - 1));
}

/* This function aims to follow the same behaviour as string.h's strcmp. */

int     ft_strcmp(char const *s1, char const *s2)
{
	char    *str1;
	char    *str2;
	int     i;

	str1 = (char *)s1;
	str2 = (char *)s2;
	i = 0;
	while (str1[i] || str2[i])
	{
		if (str1[i] != str2[i])
			return ((t_uint8)str1[i] - (t_uint8)str2[i]);
		i++;
	}
	return (0);
}

/* This function aims to follow the same behaviour as string.h's strlen. */

size_t		ft_strlen(const char *s)
{
	size_t i;

	i = 0;
	while (s[i] != '\0')
		i++;
	return (i);
}

/* This function aims to follow the same behaviour as string.h's memset. */

void		*ft_memset(void *address, int byte_value, size_t size)
{
	char	*byte_string;
	size_t	offset;

	offset = 0;
	byte_string = address;
	while (offset < size)
	{
		*(byte_string + offset) = byte_value;
		offset++;
	}
	return (address);
}
