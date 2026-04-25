BITS 64

default rel

section .data
msg db "Disassembler stress test running...", 0xA
msg_len equ $ - msg

align 16
lookup_table dq case0, case1, case2, case3, case4, case5, case6, case7

align 32
vector_data dd 1,2,3,4,5,6,7,8

section .bss
buffer resb 256

section .text
global _start

_start:

    ; syscall write(1, msg, msg_len)
    mov rax, 1
    mov rdi, 1
    lea rsi, [msg]
    mov rdx, msg_len
    syscall

    call demo_cpuid
    call demo_looping
    call demo_switch_jump
    call demo_simd
    call demo_indirect_calls

    mov rdi, 0
    call recursive_func

    ; exit syscall
    mov rax, 60
    xor rdi, rdi
    syscall

demo_cpuid:
    push rbp
    mov rbp, rsp

    xor eax, eax
    cpuid

    rdtsc
    shl rdx, 32
    or rax, rdx

    mov rsp, rbp
    pop rbp
    ret

demo_looping:
    mov rcx, 1000000
    xor rax, rax
    xor rbx, rbx

.loop_start:
    add rax, rcx
    xor rax, rbx
    inc rbx
    dec rcx
    jnz .loop_start

    ret

demo_switch_jump:
    mov rax, 3
    cmp rax, 7
    ja default_case

    lea rbx, [lookup_table]
    mov rdx, [rbx + rax*8]
    jmp rdx

case0:
    add rax, 10
    jmp end_switch

case1:
    sub rax, 10
    jmp end_switch

case2:
    imul rax, 2
    jmp end_switch

case3:
    xor rax, 0x55
    jmp end_switch

case4:
    shl rax, 1
    jmp end_switch

case5:
    shr rax, 1
    jmp end_switch

case6:
    neg rax
    jmp end_switch

case7:
    inc rax
    jmp end_switch

default_case:
    xor rax, rax

end_switch:
    ret

demo_simd:
    push rbp
    mov rbp, rsp

    ; load 256-bit vector
    vmovdqa ymm0, [vector_data]
    vmovdqa ymm1, [vector_data]

    vpaddd ymm2, ymm0, ymm1
    vpslld ymm3, ymm2, 2
    vpxor ymm4, ymm3, ymm2

    ; spill to memory
    vmovdqa [buffer], ymm4

    vzeroupper

    mov rsp, rbp
    pop rbp
    ret

demo_indirect_calls:
    lea rax, [func_a]
    call rax

    lea rax, [func_b]
    call rax

    lea rax, [func_c]
    call rax

    ret

func_a:
    add rax, 1
    ret

func_b:
    sub rax, 2
    ret

func_c:
    xor rax, rax
    ret

recursive_func:
    push rbp
    mov rbp, rsp

    cmp rdi, 0
    je .base_case

    dec rdi
    call recursive_func

    inc rax
    jmp .done

.base_case:
    mov rax, 1

.done:
    mov rsp, rbp
    pop rbp
    ret

weird_flow:
    mov rax, 5
    test rax, rax
    jz .skip

    jmp .middle

.skip:
    mov rbx, 1
    jmp .end

.middle:
    cmp rax, 5
    jne .alt

    lea rcx, [rax + rbx*2]
    jmp .end

.alt:
    xor rcx, rcx

.end:
    ret

align 8
db 0x90,0x90,0x90,0x90

align 16
nop
nop
nop
nop

opaque_block:
    mov rax, 1234
    mov rbx, 5678

    mov rcx, rax
    sub rcx, rbx
    add rcx, rbx
    sub rcx, rax

    test rcx, rcx
    jnz .always_taken
    jz .always_taken

.always_taken:
    xor rdx, rdx
    ret