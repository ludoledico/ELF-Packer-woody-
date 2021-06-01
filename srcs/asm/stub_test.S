DEFAULT REL

global		_stub_start
section		.text

%define	v0	eax
%define v1	ebx
%define	v2	edx
%define	v3	ebp

_stub_start:

	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	rsp
	push	rbp
	; registers saved

    call	my_print
    db 		"...WOOPY...", 10
 
    my_print:
    mov		rdx, 12
    pop		rsi
    mov		rdi, 1
    mov		rax, 1
    syscall				; write(1, "...WOODY...\n", 12);

    ; restoring registers
    pop		rbp
    pop		rsp
    pop		rdx
    pop		rcx
    pop		rbx
    pop		rax
 
	; jumping to original entrypoint
	mov		r9, 0xaaaaaaaaaaaaaaaa		; will be fixed with the oep
	lea		r8, [_stub_start]
	add		r8, r9						; will be fixed for oep
	;int3
	jmp		r8
