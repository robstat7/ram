# format ELF64

# extrn interrupt_handler
# extrn exception_handler

isr_stub_0:
    # pushad
    # mov [tmp], rsp
    # push rax
    # push rcx
    # push rdx
    # push rbx
    # push [tmp]
    # push rbp
    # push rsi
    # push rdi
    # cld    # C code following the sysV ABI requires DF to be clear on function entry
    call interrupt_handler
    # popad
    # pop rdi
    # pop rsi
    # pop rbp
    # add rsp, 16
    # pop rbx
    # pop rdx
    # pop rcx
    # pop rax
    # iretq

isr_stub_1:
    # pushad
    # mov [tmp], rsp
    # push rax
    # push rcx
    # push rdx
    # push rbx
    # push [tmp]
    # push rbp
    # push rsi
    # push rdi
    # cld    # C code following the sysV ABI requires DF to be clear on function entry
    call exception_handler
    # popad
    # pop rdi
    # pop rsi
    # pop rbp
    # add rsp, 16
    # pop rbx
    # pop rdx
    # pop rcx
    # pop rax
    # iretq

# public isr_stub_table

.global isr_stub_table

# tmp dq ?

isr_stub_table:
     .quad isr_stub_0
     .quad isr_stub_1
    # dq isr_stub_0
    # dq isr_stub_1
