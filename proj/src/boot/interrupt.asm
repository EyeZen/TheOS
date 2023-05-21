; extern interrupt_dispatch

; interrupt_stub:
;     push rax
;     push rbx
;     push rcx
;     push rdx
;     push rsi
;     push rdi
;     push rbp
;     push r8
;     push r9
;     push r10
;     push r11
;     push r12
;     push r13
;     push r14
;     push r15

;     mov rdi, rsp
;     call interrupt_dispatch
;     mov rsp, rax

;     pop r15
;     pop r14
;     pop r13
;     pop r12
;     pop r11
;     pop r10
;     pop r9
;     pop r8
;     pop rbp
;     pop rdi
;     pop rsi
;     pop rdx
;     pop rcx
;     pop rbx
;     pop rax

;     ; remove the vector number + error code
;     add rsp, 16

;     iret


; align 16
; global vector_0_handler
; vector_0_handler:
;     ; vector 0 has no error code
;     push 0
;     ; the vector number
;     push 0
;     jmp interrupt_stub

; align 16
; vector_1_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 1
;     jmp interrupt_stub

; align 16
; vector_2_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 2
;     jmp interrupt_stub

; align 16
; vector_3_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 3
;     jmp interrupt_stub

; align 16
; vector_4_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 4
;     jmp interrupt_stub

; align 16
; vector_5_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 5
;     jmp interrupt_stub

; align 16
; vector_6_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 6
;     jmp interrupt_stub

; align 16
; vector_7_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 7
;     jmp interrupt_stub

; align 16
; vector_8_handler:
;     ; error code:  (always 0)
;     ; push 0
;     ; vector number
;     push 8
;     jmp interrupt_stub

; align 16
; vector_9_handler:
;     ; vector number
;     push 9
;     jmp interrupt_stub

; align 16
; vector_10_handler:
;     ; vector number
;     push 10
;     jmp interrupt_stub

; align 16
; vector_11_handler:
;     ; vector number
;     push 11
;     jmp interrupt_stub

; align 16
; vector_12_handler:
;     ; vector number
;     push 12
;     jmp interrupt_stub
    
; align 16
; vector_13_handler:
;     ; vector number
;     push 13
;     jmp interrupt_stub
    
; align 16
; vector_14_handler:
;     ; vector number
;     push 14
;     jmp interrupt_stub
    
; align 16
; vector_15_handler:
;     push 0
;     ; vector number
;     push 15
;     jmp interrupt_stub
    
; align 16
; vector_16_handler:
;     ; vector number
;     push 16
;     jmp interrupt_stub
    
; align 16
; vector_17_handler:
;     ; vector number
;     push 17
;     jmp interrupt_stub
    
; align 16
; vector_18_handler:
;     push 0
;     ; vector number
;     push 18
;     jmp interrupt_stub

; align 16
; vector_19_handler:
;     ; also needs a dummy error code
;     push 0
;     ; vector number
;     push 19
;     jmp interrupt_stub

; ; 20 - 31 unused

