	.assume adl=1
	.org $b0000
	jp start
	.align $40

	.db "MOS"
	.db 0 ; version
	.db 1 ; ADL enabled (24-bit addressing)
start:
	xor a
	out (0),a
	ret
