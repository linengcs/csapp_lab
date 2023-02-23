movabs $0x6166373939623935, %rcx
movq %rcx,(%rdi)
movq $0x0, %rcx
movq %rcx, 0x8(%rdi)
pushq $0x4018fa
ret
