global		chaskey_encrypt

; Defines for chaskey encryption
%define	v0	eax
%define v1	ebx
%define	v2	edx
%define	v3	ebp

; Prototype:
;	void chaskey_encrypt(void *key, void *buff);

; Arguments:
;	key = rdi
;	buf = rsi

;-------------------------------------------------------------------------------;
;								Prologue										;
;-------------------------------------------------------------------------------;
chaskey_encrypt:
	push	rax
	push	rcx
	push	rdx
	push	rbx
	push	rsp
	push	rbp
	push	rsi
	push	rdi
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;								Chaskey-LTS										;
;-------------------------------------------------------------------------------;
	push	rsi
; Loading the buffer to be encrypted
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
; Setting the number of passes
	xor		rcx, rcx
	mov     cl, 16
	cld

	ck_loop:
	pushfq
; Encryption pass
	add     v0, v1
	rol     v1, 5
	xor     v1, v0
	rol     v0, 16
	add     v2, v3
	rol     v3, 8
	xor     v3, v2
	add     v0, v3
	rol     v3, 13
	xor     v3, v0
	add     v2, v1
	rol     v1, 7
	xor     v1, v2
	rol     v2,	16
	popfq
	loop	ck_loop

; Post-whitenning
	xor     v0, [rdi   ]
	xor     v1, [rdi+ 4]
	xor     v2, [rdi+ 8]
	xor     v3, [rdi+12]
	pop		rdi			; poping buff from the stack
; Saving the encrypted buffer
	stosd
	xchg    eax, v1
	stosd
	xchg    eax, v2
	stosd
	xchg    eax, v3
	stosd
;-------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;								Epilogue										;
;-------------------------------------------------------------------------------;
	pop		rdi
	pop		rsi
	pop		rbp
	pop		rsp
	pop		rbx
	pop		rdx
	pop		rcx
	pop		rax
	ret
