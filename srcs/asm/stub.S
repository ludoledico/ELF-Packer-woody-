DEFAULT REL
global		_stub_start

section		.text

; Defines for chaskey decryption
%define	v0	eax
%define v1	ebx
%define	v2	edx
%define	v3	ebp

; Defines for syscall readability
%define	SYSCALL_WRITE		1
%define	SYSCALL_MPROTECT	10
%define PROT_RW				0x3
%define PROT_RX				0x5

; Defining the stub size (aligned to the page size)
%define STUB_SIZE	0x1000

;-------------------------------------------------------------------------------;
;								Prologue										;
;-------------------------------------------------------------------------------;
_stub_start:
	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	rsp
	push	rbp
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							"...WOODY..." Banner								;
;-------------------------------------------------------------------------------;
    call	my_print
    db 		"...WOODY...", 10
    my_print:
    mov		rdi, 1				; int		fd
    pop		rsi					; char		*str
    mov		rdx, 12				; size_t	len
    mov		rax, SYSCALL_WRITE
    syscall						; write(1, "...WOODY...\n", 12);
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							mprotecting ".text"	(RW)							;
;-------------------------------------------------------------------------------;
	mov		r8, 0xcccccccccccccccc
    lea		rdi, [_stub_start]
	add		rdi, r8
	mov		r8, rdi
	and		r8, 0xfff
	sub		rdi, r8						; unsigned long	addr
	mov		rsi, 0xdddddddddddddddd		; size_t		len
	mov		rdx, PROT_RW				; unsigned long	prot
	mov		rax, SYSCALL_MPROTECT
	syscall								; mprotect(text_addr, text_size, MPROT);
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
    pop		rdi							; Storing the decryption key in rdi
	mov		r8, 0xcccccccccccccccc
    lea		rsi, [_stub_start]
	add		rsi, r8						; Storing the address of .text in rsi
    mov		r8, 0xdddddddddddddddd		; Storing the size of .text in r8

    decrypt_loop:
    push	rdi
    push	rsi
; Loading the buffer
    lodsd
    xchg    eax, v3		; storing 4 bytes in v3 (exchanged later)
    lodsd
    xchg    eax, v1		; storing 4 bytes in v1
    lodsd
    xchg    eax, v2		; storing 4 bytes in v2
    lodsd
    xchg	eax, v3		; storing 4 bytes in v3 (v0 is now filled)
; Pre-whitenning
    xor		v0,	[rdi]
    xor		v1,	[rdi+4]
    xor		v2,	[rdi+8]
    xor		v3,	[rdi+12]
; Setting number of passes
    xor		rcx, rcx
    mov     cl, 16
    cld
 
    ck_loop:
    pushfq
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
    popfq
    loop	ck_loop

; Post-whitenning
    xor     v0, [rdi   ]
    xor     v1, [rdi+ 4]
    xor     v2, [rdi+ 8]
    xor     v3, [rdi+12]
    pop		rdi
; Saving the decrypted buffer
    stosd
    xchg    eax, v1
    stosd
    xchg    eax, v2
    stosd
    xchg    eax, v3
    stosd
	pop		rdi			; Restoring the key
    sub 	r8, 16
	test	r8, r8
    jns		decrypt_loop
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;							mprotecting ".text"	(RX)							;
;-------------------------------------------------------------------------------;
	mov		r8, 0xcccccccccccccccc
    lea		rdi, [_stub_start]
	add		rdi, r8
	mov		r8, rdi
	and		r8, 0xfff
	sub		rdi, r8						; unsigned long	addr
	mov		rsi, 0xdddddddddddddddd		; size_t		len
	mov		rdx, PROT_RX				; unsigned long	prot
	mov		rax, SYSCALL_MPROTECT
	syscall								; mprotect(text_addr, text_size, MPROT);
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;								Epilogue										;
;-------------------------------------------------------------------------------;
    pop		rbp
    pop		rsp
    pop		rdx
    pop		rcx
    pop		rbx
    pop		rax
 
; Jumping to the original entrypoint
	mov		r9, 0xaaaaaaaaaaaaaaaa
	lea		r8, [_stub_start]
	add		r8, r9
	jmp		r8
;-------------------------------------------------------------------------------;
