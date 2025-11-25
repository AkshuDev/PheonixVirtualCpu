section .bss
    align 8
    heap_space: resb 65536 ; Heap of size 64KiB

section .data
    heap_ptr dq 0
    heap_size dq 65536
    heap_end dq  heap_size

section .text
    global malloc
    global free
    global get_fheap

; malloc(long size) -> rax (void*)
malloc:
    push rbp
    mov rbp, rsi

    mov rax, [rbp + 16] ; size

    ; align to 8 bytes
    add rax, 7
    and rax, -8

    mov rbx, [rel heap_ptr]
    mov rcx, [rel heap_end]

    ; check if enough memory
    lea rdx, [rbx + rax]
    cmp rdx, rcx
    ja .no_mem

    ; allocate
    mov rax, rbx
    mov [rel heap_ptr], rdx
    jmp .done
.no_mem:
    xor rax, rax ; NULL
.done:
    pop rbp
    ret

; free(long size)
free:
    push rbp
    mov rbp, rsp

    mov rax, [rbp + 16] ; size
    add rax, 7
    and rax, -8

    mov rbx, [rel heap_ptr]
    mov rcx, [rel heap_end]

    mov rdx, rbx
    sub rdx, rax

    cmp rdx, 0
    jl .done

    cmp rdx, rcx
    jg .done

    mov [rel heap_ptr], rdx
.done:
    pop rbp
    ret

get_fheap: ; rax (void*) -> rax (char*)
    mov rdi, rax
    lea rax, [heap_space + rax]
    ret