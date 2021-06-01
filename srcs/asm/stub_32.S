DEFAULT REL
global		_stub_start

section		.text

; Defines for chaskey decryption
%define	v0	eax
%define	v1	ebx
%define	v2	edx
%define	v3	ebp

; Defines for syscall readability
%define	SYSCALL_WRITE		4
%define	SYSCALL_MPROTECT	125
%define PROT_RW				0x3
%define PROT_RX				0x5

; Defining the stub size (aligned to the page size)
%define STUB_SIZE	0x1000

;-------------------------------------------------------------------------------;
;								Prologue										;
;-------------------------------------------------------------------------------;
_stub_start:
	pushad
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							"...WOODY..." Banner								;
;-------------------------------------------------------------------------------;
    call	my_print
    db 		"...WOODY...", 10
    my_print:
    mov		ebx, 1					; int		fd
    pop		ecx						; char		*str
    mov		edx, 12					; size_t	len
    mov		eax, SYSCALL_WRITE
    int		0x80					; write(1, "...WOODY...\n", 12);
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							mprotecting ".text"	(RW)							;
;-------------------------------------------------------------------------------;
	mov		ecx, 0xcccccccc
	call	get_eip
	mov		ebx, eax
	sub		ebx, 0x2e
	add		ebx, ecx
	mov		ecx, ebx
	and		ecx, 0xfff
	sub		ebx, ecx					; unsigned long	addr
	mov		ecx, 0xdddddddd				; size_t		len
	mov		edx, PROT_RW				; unsigned long	prot
	mov		eax, SYSCALL_MPROTECT
	int		0x80						; mprotect(text_addr, text_size, MPROT);
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							Chaskey	decryption loop								;
;-------------------------------------------------------------------------------;
    call	get_key
    db		0xbb, 0xbb, 0xbb, 0xbb,\
			0xbb, 0xbb, 0xbb, 0xbb,\
			0xbb, 0xbb, 0xbb, 0xbb,\
			0xbb, 0xbb, 0xbb, 0xbb
    get_key:
    pop		edi					; Storing the decryption key in edi
	mov		ecx, 0xcccccccc
	call	get_eip
	mov		esi, eax
	sub		esi, 0x70
	add		esi, ecx			; Storing the address of .text in esi
    mov		ecx,  0xdddddddd	; Storing the lenght of .text in ecx
 
    decrypt_loop:
    push	edi
	push	ecx
    push	esi
; Loading the buffer
	lodsd
	xchg    eax, v3
	lodsd
	xchg    eax, v1
	lodsd
	xchg    eax, v2
	lodsd
	xchg	eax, v3
; Pre-whitenning
	xor		v0,	[edi   ]
	xor		v1,	[edi+4 ]
	xor		v2,	[edi+8 ]
	xor		v3,	[edi+12]
; Setting number of passes
	xor		ecx,	ecx
	mov		cl,		16
	cld

	ck_loop:
	pushfd
; Decrypt the loaded buffer
	ror     v2, 16
	xor     v1, v2
	ror     v1, 7
	sub     v2, v1
	xor     v3, v0
	ror     v3, 13
	sub     v0, v3
	xor     v3, v2
	ror     v3, 8
	sub     v2, v3
	ror     v0, 16
	xor     v1, v0
	ror     v1, 5
	sub     v0, v1
	popfd
	loop	ck_loop

; Post-whitenning
	xor     v0, [edi   ]
	xor     v1, [edi+ 4]
	xor     v2, [edi+ 8]
	xor     v3, [edi+12]
	pop     edi
; Saving the decrypted buffer
	stosd
	xchg    eax, v1
	stosd
	xchg    eax, v2
	stosd
	xchg    eax, v3
	stosd
	pop		ecx
	pop		edi			; Restoring the key
    sub		ecx, 16
	test	ecx, ecx
    jns		decrypt_loop
;-------------------------------------------------------------------------------;
 
;-------------------------------------------------------------------------------;
;							mprotecting ".text"	(RX)							;
;-------------------------------------------------------------------------------;
	mov		ecx, 0xcccccccc
	call	get_eip
	mov		ebx, eax
	sub		ebx, 0xe3
	add		ebx, ecx
	mov		ecx, ebx
	and		ecx, 0xfff
	sub		ebx, ecx					; unsigned long	addr
	mov		ecx, 0xdddddddd				; size_t		len
	mov		edx, PROT_RX				; unsigned long	prot
	mov		eax, SYSCALL_MPROTECT
	int		0x80						; mprotect(text_addr, text_size, MPROT);
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;								Epilogue										;
;-------------------------------------------------------------------------------;
	popad

; Jumping to the original entrypoint
	mov		ecx, 0xaaaaaaaa			; will be fixed with the oep
	call	get_eip
	sub		eax, 0x113
	add		eax, ecx					; will be fixed for oep
	jmp		eax
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							Wrapper to get eip									;
;-------------------------------------------------------------------------------;
	get_eip:
		mov eax, [esp]
		ret
;-------------------------------------------------------------------------------;
