section .text
    global print
    global input
    global strlen


; print(char* str)
print:
    push rbp
    mov rbp, rsp

    mov rsi, [rbp + 16] ; rsi = string ptr
    push rsi ; save str ptr for later

    ; call strlen to get string length
    mov rdi, rsi ; strlen(str_ptr)
    call strlen
    mov rdx, rax ; rdx = length
    pop rsi ; restore str ptr

    mov rax, 1 ; syscall write
    mov rdi, 1 ; stdout
    syscall

    pop rbp
    ret

; input(void* buf, int maxlen)
input:
    push rbp
    mov rbp, rsp

    mov rsi, [rsp + 16] ; get void*
    mov rdx, [rsp + 24] ; get int

    mov rax, 0 ; read syscall
    mov rdi, 0 ; stdin
    syscall

    pop rbp
    ret

; Returns in rax (len)
strlen:
    push rbp
    mov rbp, rsp

    mov rsi, [rbp + 16] ; argument from stack: str pointer

    xor rax, rax ; length counter = 0
.next:
    cmp byte [rsi + rax], 0
    je .done
    inc rax
    jmp .next
.done:
    pop rbp
    ret
