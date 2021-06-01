global		chaskey_decrypt

%define	v0	eax
%define v1	ebx
%define	v2	edx
%define	v3	ebp

; chaskey_decrypt(void *key, void *buff);
; key = rdi
; buf = rsi

chaskey_decrypt:
	push	rax
	push	rcx
	push	rdx
	push	rbx
	push	rsp
	push	rbp
	push	rsi
	push	rdi
	; register saved on stack

	push	rsi			; pushing the address of buff on the stack

	; load buf
	lodsd
	xchg    eax, v3		; storing 4 bytes in v3 (exchanged later)
	lodsd
	xchg    eax, v1		; storing 4 bytes in v1
	lodsd
	xchg    eax, v2		; storing 4 bytes in v2
	lodsd
	xchg	eax, v3		; storing 4 bytes in v3 (v0 is now filled)

	; pre-whiten
	xor		v0,	[rdi]
	xor		v1,	[rdi+4]
	xor		v2,	[rdi+8]
	xor		v3,	[rdi+12]

	; setting number of passes
	xor		rcx, rcx
	mov     cl, 16
	cld					; clearing direction flag

	ck_loop:
	pushfq				; saving flags

	; decrypt
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

	popfq				; restoring flags
	loop	ck_loop		; loop for n passes

	; post-whiten
	xor     v0, [rdi   ]
	xor     v1, [rdi+ 4]
	xor     v2, [rdi+ 8]
	xor     v3, [rdi+12]
	pop		rdi			; poping the address of buff from the stack

	; saving buffer
	stosd
	xchg    eax, v1
	stosd
	xchg    eax, v2
	stosd
	xchg    eax, v3
	stosd

	; restroring registers before returning
	pop		rdi
	pop		rsi
	pop		rbp
	pop		rsp
	pop		rbx
	pop		rdx
	pop		rcx
	pop		rax
	ret
