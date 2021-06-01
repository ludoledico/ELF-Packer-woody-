#ifndef WOODY_H

/****************************************************************************/
/*							INCLUDED HEADER FILES							*/
/****************************************************************************/

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <sys/mman.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <elf.h>

/****************************************************************************/
/*							DEFINES AND TYPEDEFS							*/
/****************************************************************************/

/* Error codes */
enum		e_error {
	ERR_NO_ARG=1,
	ERR_MEM_ALLOC,
	ERR_OPEN,
	ERR_NO_STATS,
	ERR_MMAP,
	ERR_BAD_FILE,
	ERR_BAD_BIN,
	ERR_UNK,
	NB_OF_ERROR_CODES	/* Always keep last */
};

/* Memory mapping flags */
# define	MMAP_PROT		(PROT_READ|PROT_WRITE)
# define	MMAP_FLAGS		(MAP_PRIVATE)

/* Name of a new section to be created for the stub */
# define	NEW_SHNAME		".stub"
# define	NEW_SHNAME_LEN	5

/* Necessary informations to insert and fix the stub */
# define	STUB_SIZE			512L
# define	STUB_JMP_MAGIC32	0xaaaaaaaa
# define	STUB_KEY_MAGIC32	0xbbbbbbbb
# define	STUB_BUFF_MAGIC32	0xcccccccc
# define	STUB_SIZE_MAGIC32	0xdddddddd
# define	STUB_JMP_MAGIC		0xaaaaaaaaaaaaaaaa
# define	STUB_KEY_MAGIC		0xbbbbbbbbbbbbbbbb
# define	STUB_BUFF_MAGIC		0xcccccccccccccccc
# define	STUB_SIZE_MAGIC		0xdddddddddddddddd
# define	KEY_LEN				16

/* Mark to recognize a packed file (max length: 7) */
# define	PACKER_MARK		"lmontagn"

/* Boolean type */
typedef	unsigned char	t_bool;
# define	TRUE		1
# define	FALSE		0

/* Shorthand types for unsigned integers */
# define	t_uint8		unsigned char
# define	t_uint16	unsigned short
# define	t_uint32	unsigned int
# define	t_uint64	unsigned long

/* Shorthand type for the stat structure */
# define	t_stat		struct stat

/* Custom types for 32bit compatibility. */
# ifdef WOODY_32BIT

	/* Data types for 32bit */
	typedef	Elf32_Addr		t_elf_addr;		// (Size 4) Unsigned program addresses
	typedef	Elf32_Off		t_elf_off;		// (Size 4) Unsigned file offsets
	typedef	Elf32_Half		t_elf_half;		// (Size 2) Unsigned medium integers
	typedef	Elf32_Word		t_elf_word;		// (Size 4) Unsigned large integers
	typedef	Elf32_Sword		t_elf_sword;	// (Size 4) Signed large integers
	typedef	Elf32_Word		t_elf_xword;	// Defined for compatibility with Elf64_Xword
	typedef	Elf32_Sword		t_elf_sxword;	// Defined for compatibility with Elf64_Sxword

	/* Data structures for 32bit */
	typedef	Elf32_Ehdr		t_elf_ehdr;		// Elf header located at file offset 0
	typedef	Elf32_Phdr		t_elf_phdr;		// Program headers contaning segment informations
	typedef	Elf32_Shdr		t_elf_shdr;		// Section header containing section informations
	typedef Elf32_Sym		t_elf_sym;		// Symbol table entries
	typedef Elf32_Rel		t_elf_rel;		// Relocation entries
	typedef Elf32_Rela		t_elf_rela;		// Relocation entries with an r_addend member
	typedef Elf32_Dyn		t_elf_dyn;		// Dynamic table entries

# else /* not WOODY_32BIT */

	/* Data types for 64bit */
	typedef	Elf64_Addr		t_elf_addr;		// (Size 8) Unsigned program addresses
	typedef	Elf64_Off		t_elf_off;		// (Size 8) Unsigned file offsets
	typedef	Elf64_Half		t_elf_half;		// (Size 2) Unsigned medium integers
	typedef	Elf64_Word		t_elf_word;		// (Size 4) Unsigned integers
	typedef	Elf64_Sword		t_elf_sword;	// (Size 4) Signed integers
	typedef	Elf64_Xword		t_elf_xword;	// (Size 8) Unsigned long integers
	typedef	Elf64_Sxword	t_elf_sxword;	// (Size 8) Signed long integers

	/* Data structures for 64bit */
	typedef	Elf64_Ehdr		t_elf_ehdr;		// Elf header located at file offset 0
	typedef	Elf64_Phdr		t_elf_phdr;		// Program headers contaning segment informations
	typedef	Elf64_Shdr		t_elf_shdr;		// Section header containing section informations
	typedef Elf64_Sym		t_elf_sym;		// Symbol table entries
	typedef Elf64_Rel		t_elf_rel;		// Relocation entries
	typedef Elf64_Rela		t_elf_rela;		// Relocation entries with an r_addend member
	typedef Elf64_Dyn		t_elf_dyn;		// Dynamic table entries

# endif /* not WOODY_32BIT */

/* Data structure used to store details about the target binary */
typedef		struct s_fdet
{
	/* informations needed to create the new file */
	size_t		bin_size;
	mode_t		bin_mode;

	/* informations needed by the packer */
	Elf64_Addr	orig_ep;
	Elf64_Addr	stub_offset;
	Elf64_Addr	stub_addr;
	size_t		stub_size;
	Elf64_Xword	bss_size;

	/* informations needed for the stub */
	Elf64_Xword	text_size;
	Elf64_Addr	text_addr;
}			t_fdet;

/****************************************************************************/
/*							GLBOAL		VARIABLES							*/
/****************************************************************************/

/* Error string */
static const char		*g_err_strings[NB_OF_ERROR_CODES] = {
	"(ERR_NO_ARG)",
	"(ERR_MEM_ALLOC)",
	"(ERR_OPEN)",
	"(ERR_NO_STATS)",
	"(ERR_MMAP)",
	"(ERR_BAD_FILE)",
	"(ERR_BAD_BIN)",
	"(ERR_UNK)",
};

/* 128bit master key */
static const t_uint8	g_key[KEY_LEN] =
{
	0x42, 0x42, 0x42, 0x42,
	0x42, 0x42, 0x42, 0x44,
	0x44, 0x42, 0x42, 0x42,
	0x42, 0x42, 0x42, 0x00
};

/* Decryption stub for 32bit binaries */
static const uint8_t	g_stub_32[STUB_SIZE] = {
	0x60, 0xe8, 0x0c, 0x00, 0x00, 0x00, 0x2e, 0x2e, 0x2e, 0x57,
	0x4f, 0x4f, 0x44, 0x59, 0x2e, 0x2e, 0x2e, 0x0a, 0xbb, 0x01,
	0x00, 0x00, 0x00, 0x59, 0xba, 0x0c, 0x00, 0x00, 0x00, 0xb8,
	0x04, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xb9, 0xcc, 0xcc, 0xcc,
	0xcc, 0xe8, 0xee, 0x00, 0x00, 0x00, 0x89, 0xc3, 0x83, 0xeb,
	0x2e, 0x01, 0xcb, 0x89, 0xd9, 0x81, 0xe1, 0xff, 0x0f, 0x00,
	0x00, 0x29, 0xcb, 0xb9, 0xdd, 0xdd, 0xdd, 0xdd, 0xba, 0x03,
	0x00, 0x00, 0x00, 0xb8, 0x7d, 0x00, 0x00, 0x00, 0xcd, 0x80,
	0xe8, 0x10, 0x00, 0x00, 0x00, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
	0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
	0xbb, 0x5f, 0xb9, 0xcc, 0xcc, 0xcc, 0xcc, 0xe8, 0xac, 0x00,
	0x00, 0x00, 0x89, 0xc6, 0x83, 0xee, 0x70, 0x01, 0xce, 0xb9,
	0xdd, 0xdd, 0xdd, 0xdd, 0x57, 0x51, 0x56, 0xad, 0x95, 0xad,
	0x93, 0xad, 0x92, 0xad, 0x95, 0x33, 0x07, 0x33, 0x5f, 0x04,
	0x33, 0x57, 0x08, 0x33, 0x6f, 0x0c, 0x31, 0xc9, 0xb1, 0x10,
	0xfc, 0x9c, 0xc1, 0xca, 0x10, 0x31, 0xd3, 0xc1, 0xcb, 0x07,
	0x29, 0xda, 0x31, 0xc5, 0xc1, 0xcd, 0x0d, 0x29, 0xe8, 0x31,
	0xd5, 0xc1, 0xcd, 0x08, 0x29, 0xea, 0xc1, 0xc8, 0x10, 0x31,
	0xc3, 0xc1, 0xcb, 0x05, 0x29, 0xd8, 0x9d, 0xe2, 0xda, 0x33,
	0x07, 0x33, 0x5f, 0x04, 0x33, 0x57, 0x08, 0x33, 0x6f, 0x0c,
	0x5f, 0xab, 0x93, 0xab, 0x92, 0xab, 0x95, 0xab, 0x59, 0x5f,
	0x83, 0xe9, 0x10, 0x85, 0xc9, 0x79, 0xa3, 0xb9, 0xcc, 0xcc,
	0xcc, 0xcc, 0xe8, 0x39, 0x00, 0x00, 0x00, 0x89, 0xc3, 0x81,
	0xeb, 0xe3, 0x00, 0x00, 0x00, 0x01, 0xcb, 0x89, 0xd9, 0x81,
	0xe1, 0xff, 0x0f, 0x00, 0x00, 0x29, 0xcb, 0xb9, 0xdd, 0xdd,
	0xdd, 0xdd, 0xba, 0x05, 0x00, 0x00, 0x00, 0xb8, 0x7d, 0x00,
	0x00, 0x00, 0xcd, 0x80, 0x61, 0xb9, 0xaa, 0xaa, 0xaa, 0xaa,
	0xe8, 0x09, 0x00, 0x00, 0x00, 0x2d, 0x13, 0x01, 0x00, 0x00,
	0x01, 0xc8, 0xff, 0xe0, 0x8b, 0x04, 0x24, 0xc3, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff
};

/* Decryption stub for 64bit binaries */
static const uint8_t	g_stub_64[STUB_SIZE] = {
	0x50, 0x53, 0x51, 0x52, 0x54, 0x55, 0xe8, 0x0c, 0x00, 0x00,
	0x00, 0x2e, 0x2e, 0x2e, 0x57, 0x4f, 0x4f, 0x44, 0x59, 0x2e,
	0x2e, 0x2e, 0x0a, 0xbf, 0x01, 0x00, 0x00, 0x00, 0x5e, 0xba,
	0x0c, 0x00, 0x00, 0x00, 0xb8, 0x01, 0x00, 0x00, 0x00, 0x0f,
	0x05, 0x49, 0xb8, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
	0xcc, 0x48, 0x8d, 0x3d, 0xc6, 0xff, 0xff, 0xff, 0x4c, 0x01,
	0xc7, 0x49, 0x89, 0xf8, 0x49, 0x81, 0xe0, 0xff, 0x0f, 0x00,
	0x00, 0x4c, 0x29, 0xc7, 0x48, 0xbe, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xba, 0x03, 0x00, 0x00, 0x00, 0xb8,
	0x0a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xe8, 0x10, 0x00, 0x00,
	0x00, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
	0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x5f, 0x49, 0xb8,
	0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x48, 0x8d,
	0x35, 0x79, 0xff, 0xff, 0xff, 0x4c, 0x01, 0xc6, 0x49, 0xb8,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x57, 0x56,
	0xad, 0x95, 0xad, 0x93, 0xad, 0x92, 0xad, 0x95, 0x33, 0x07,
	0x33, 0x5f, 0x04, 0x33, 0x57, 0x08, 0x33, 0x6f, 0x0c, 0x48,
	0x31, 0xc9, 0xb1, 0x10, 0xfc, 0x9c, 0xc1, 0xca, 0x10, 0x31,
	0xd3, 0xc1, 0xcb, 0x07, 0x29, 0xda, 0x31, 0xc5, 0xc1, 0xcd,
	0x0d, 0x29, 0xe8, 0x31, 0xd5, 0xc1, 0xcd, 0x08, 0x29, 0xea,
	0xc1, 0xc8, 0x10, 0x31, 0xc3, 0xc1, 0xcb, 0x05, 0x29, 0xd8,
	0x9d, 0xe2, 0xda, 0x33, 0x07, 0x33, 0x5f, 0x04, 0x33, 0x57,
	0x08, 0x33, 0x6f, 0x0c, 0x5f, 0xab, 0x93, 0xab, 0x92, 0xab,
	0x95, 0xab, 0x5f, 0x49, 0x83, 0xe8, 0x10, 0x4d, 0x85, 0xc0,
	0x79, 0xa2, 0x49, 0xb8, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
	0xcc, 0xcc, 0x48, 0x8d, 0x3d, 0xfd, 0xfe, 0xff, 0xff, 0x4c,
	0x01, 0xc7, 0x49, 0x89, 0xf8, 0x49, 0x81, 0xe0, 0xff, 0x0f,
	0x00, 0x00, 0x4c, 0x29, 0xc7, 0x48, 0xbe, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xba, 0x05, 0x00, 0x00, 0x00,
	0xb8, 0x0a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0x5d, 0x5c, 0x5a,
	0x59, 0x5b, 0x58, 0x49, 0xb9, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0x4c, 0x8d, 0x05, 0xc0, 0xfe, 0xff, 0xff,
	0x4d, 0x01, 0xc8, 0x41, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff
};


/****************************************************************************/
/*							FUNCTION	PROTOTYPES							*/
/****************************************************************************/

/* Error checking */
t_bool			check_binary(int fd);

/* Packer functions */
int				pack_binary(int fd, t_fdet *file_details);
void    		fix_symbols(t_elf_ehdr *elf_header, t_fdet *fdet);
void    		fix_offsets(t_elf_ehdr *elf_header, t_fdet *fdet);
void			fix_text_phdr(t_elf_ehdr *elf_header, t_elf_phdr *phdr, t_fdet *fdet);
void			fix_bss_phdr(t_elf_phdr *phdr, t_fdet *fdet);
void			fix_syms(t_elf_sym *section, t_elf_shdr *shdr, t_fdet *fdet);
void			insert_stub(t_elf_ehdr *elf_header, t_fdet *fdet);
void			mark_file(char *bin_content);

/* Chaskey */
void			encrypt_text(t_elf_ehdr *elf_header);
void			chaskey_encrypt(const t_uint8 key[16], void *buff);

/* Output functions */
void			write_new_file(char *packed_bin, t_fdet file_details);

/* Get functions */
t_elf_shdr		*get_shdr(t_elf_ehdr *elf_header, const char *target);
t_elf_phdr		*get_phdr(t_elf_ehdr *elf_header, t_elf_shdr *shdr);
t_elf_shdr		*get_shtable(t_elf_ehdr *elf_header);
t_elf_phdr		*get_phtable(t_elf_ehdr *elf_header);

/* Error handling functions */
void			print_err(int errcode);

/* Misc. functions */
int				ft_strcmp(char const *s1, char const *s2);
void			*ft_memmove(void *dest, void const *src, size_t n);
void			*ft_memset(void *address, int byte_value, size_t size);
size_t			align_to_psize(size_t size);
size_t			align_to_n(size_t size, t_uint32 n);
size_t			ft_strlen(const char *s);

/*****************************************************************************/

#endif /* not WOODY_H */