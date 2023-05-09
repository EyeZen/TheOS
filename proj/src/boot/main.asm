global start
extern long_mode_start

section .text
bits 32
start:
    mov esp, stack_top

    ; switch to long mode for 64-bit 
    ; check if cpu supports long mode
    call check_multiboot    ; confirm that os is loaded by multiboot2 bootloader
    call check_cpuid
    call check_long_mode

    ; paging is automatically enabled by the long mode
    ; entering the long mode required paging / heap
    ; page table 4-level + leaf
    ; each level = 4KB = 9-bits * 4 = 36-bits | Level-4 => Level-3 => Level-2 => Level-1 => Physical-Page-Address + Offset
    ; Level-4 PageTable address stored in 'CR3' register
    ; leaf/offset page = 12-bits
    call setup_page_tables
    call enable_paging

    ; load global descriptor table
    lgdt [gdt64.pointer]
    ; load code segment into the code-selector
    jmp gdt64.code_segment:long_mode_start

    ; print 'OK'
    ; write 'OK'(0x2f4b2f4f) into video memory(at 0xb8000)
    ; mov dword [0xb8000], 0x2f4b2f4f

    hlt ; stop processing and wait for interrupts




check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    mov edi, ebx
    ret

.no_multiboot:
    mov al, "M"
    jmp error

check_cpuid:    ; check if cpu supports cpuid
    ; checked by attempting to flip id bit of flags register

    ; load flags register in eax, save a copy in ecx
    pushfd  ; push flags onto stack
    pop eax ; copy flags into eax
    mov ecx, eax

    xor eax, 1 << 21 ; attempt to flip id bit (bit 21)

    ; write back to flags register
    push eax
    popfd

    ; load flags for comparison
    pushfd
    pop eax
    
    ; reset flags register to initial value
    push ecx
    popfd

    ; compare to check if cpu reset the flags
    cmp eax, ecx    ; if equal, implies cpu reset flags, cpuid not available
    je .no_cpuid
    ret

.no_cpuid:
    mov al, "C"
    jmp error

check_long_mode:    ; check if 64-bit supported
    mov eax, 0x80000000
    cpuid ; implicitly loads eax, on seeing 0x80000000, returns in eax value = 0x80000000 + 1 if extended processor info supported
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid   ; this time, cpuid will set 'lm' bit (bit 29) in edx register, if long mode is supported
    test edx, 1 << 29
    jz .no_long_mode

    ret

.no_long_mode:
    mov al, "L"
    jmp error

setup_page_tables:
    ; identity mapping: mapping a physical address to exactly same virtual address
    ; - paging is enabled when the long mode is activated
    ; - the following instructions will be executed to setup pagetable, hence their addresses are from before the paging is enabled
    ; - but the cpu will now treat these as virtual addresses
    ; - a chunk of physical addresses can be mapped to virtual addresses to enable proper instruction execution
    ; Here, Identity mapping first 2MiB memory = 512-pages * 4KiB per page
    mov eax, page_table_l3
    or eax, 0b11    ; present, writable
    mov [page_table_l4], eax    ; first entry
    
    mov eax, page_table_l2
    or eax, 0b11    ; present, writable
    mov [page_table_l3], eax    ; first entry
    
    mov eax, page_table_l1
    or eax, 0b11    ; present, writable
    mov [page_table_l2], eax    ; first entry

    mov ecx, 0  ; counter
.loop:
    mov eax, 0x1000 ; 4KiB
    mul ecx ; next page address
    or eax, 0b00000011  ; present, writable
    mov [page_table_l1 + ecx * 8], eax

    inc ecx     ; increment counter
    cmp ecx, 512; check if whole Level-2 table is mapped
    jne .loop ; if not, continue

    ret

enable_paging:
    ; enable paging and enter long mode
    ; pass page table location value to cpu ( in CR3 register )
    mov eax, page_table_l4
    mov cr3, eax

    ; enable PAE (Physical Address Expansion) flag in CR4 register, neccessary for 64-bit paging
    mov eax, cr4
    or eax, 1 << 5 ; enable PAE flag (5th bit)
    mov cr4, eax

    ; enable long mode (model specific register)
    mov ecx, 0xC0000080
    rdmsr ; read-model-specific-register, loads `efer` register into `eax`
    or eax, 1 << 8  ; enable long mode (8th bit)
    wrmsr ; write-model-specific-register, `efer` register

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31 ; enable paging flag (31st bit)
    mov cr0, eax

    ; not in 64-bit mode yet, still in 32-bit compatibility sub-mode
    ; need to create global descriptor table

    ret


error:
    ; print "ERR: X" where X is the error code
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte [0xb800a], al
    hlt



section .bss    ; statically allocated variables
align 4096 ; every page is aligned to 4096 bytes, implies first address of every page has bits log2(4096) = 12-bits zero
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 4096
page_table_l1:
    resb 4096
stack_bottom:
    resb 4096 * 4   ; reserve 16KB for stack
stack_top:


section .rodata
gdt64:  ; 64-bit global descriptor table
    dq 0    ; zero entry
.code_segment: equ $ - gdt64   ; offset to code-segment
    ; code segment
    ; enable flags : (executable:bit43) | (descriptor-type=1:bit44) | (present-flag:bit47) | (64-bit flag:bit53)
    ; dq  (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
    dq (0b10011000 << 40) | (1 << 53)
.data_segment: equ $ - gdt64
    ; code segment
    ; enable flags : (descriptor-type=1:bit44) | (present-flag:bit47) | (64-bit flag:bit53)
    ; dq  (1 << 44) | (1 << 47) | (1 << 53)
    dq (0b10010010 << 40) | (1 << 53)
.pointer: ; gdt offset and pointer to gdt  
    dw $ - gdt64 - 1 ; length - 1
    dq gdt64