// All commented code is in .AOL not in .C. You can understand it either way

.section .data
st_asm_msg: .asciz "Starting Pheonix Virtual Cpu Assembler...\n"
st_em_msg: .asciz "Starting Pheonix Virtual Cpu Emulator...\n"
err_arg_msg: .asciz "Error, Usage: pv-cpu <filename>\n"

.section .bss
argc: .skip 8
filename_in: .skip 8
filename_out: .skip 8

.section .text
.global _start

exit: // nreturn exit(int code)
	mov x8, #93
	svc #0
	
write: // nreturn write(int fd, char* buffer, int size)
	mov x8, #64
	svc #0
	ret
	
strlen: // int strlen(None, char* buffer)
	mov x9, #0
	mov x2, x1
.loop:
	ldrb w3, [x2], #1
	cbz w3, .done
	b .loop
.done:
	sub x9, x2, x1
	ret

error_arg: // nreturn error_arg()
	mov x0, #1
	adr x1, err_arg_msg
	mov x2, #32
	mov x8, #64
	svc #0
	b exit

_start: // int _start(int argc, char** argv)
	adr x1, st_asm_msg
	mov x0, #1
	bl strlen
	mov x2, x9
	mov x8, #64
	svc #0

	// Get argc
	ldr x10, [sp]
	// Compare argc to 2
	mov x11, #2
	cmp x10, x11
	b.lt error_arg
	
	// auto argc = argv[1]
	mov x11, #0
	ldr x11, [sp, #16]
	adr x12, argc
	str x10, [x12]
	adr x12, filename_in
	str x11, [x12]
	mov x11, #0
	mov x12, #3
	
	cmp x10, x12
	b.ne .phase_ii
	// Get filename_out
	ldr x11, [sp, #24]
	adr x12, filename_out
	str x11, [x12]
	mov x11, #0
	mov x12, #0
.phase_ii:
	// Clear x10
	mov x10, #0
	// Print arg1
	mov x0, #1
	adr x1, filename_in
	bl strlen
	mov x2, x9
	mov x9, #0
	bl write
	b exit
	
