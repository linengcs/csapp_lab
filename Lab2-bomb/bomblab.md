### Reference

1. [GDB Command Reference - disassemble command](https://visualgdb.com/gdbreference/commands/disassemble)：怎么使用GDB进行反汇编调试
2. [GDB X Command](https://visualgdb.com/gdbreference/commands/x)
3. [GDB and Debugging](https://web.stanford.edu/class/archive/cs/cs107/cs107.1202/resources/gdb)：GDB上手教程和如何进行debug
4. [GDB Reference pdf](https://web.stanford.edu/class/archive/cs/cs107/cs107.1202/resources/gdb_refcard.pdf)：GDB命令参考手册



bomblab提供的文件是bomb和bomb.c

+ bomb是已经编译好的可执行目标文件
+ bomb.c中只有main函数，让你了解bomb的运行逻辑，即按照顺序依次在shell中输入一行正确的字符串（也可以以文件的形式），拆除对应的phase。

很显然，bomb.c中隐藏了所有的phase函数实现，我们无法从其中获取更多的信息，只能通过反编译bomb来查看完整的汇编代码去推断原来的C函数是如何实现的。

我是用gdb来对bomb进行调试，通过layout asm来显示汇编代码，用si/ni指令像调试C语句一样一句句执行汇编语句。

```bash
gdb bomb	// 用gdb逐条汇编语言去调试bomb 
...
break main 	// 设置main函数为入口断点
run
n
...
layout asm	// 展示assemble布局
si
...
```

### Phase1

phase1的汇编代码如下：

```bash
Dump of assembler code for function phase_1:
   0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:	call   0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:	test   %eax,%eax
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:	call   0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp
   0x0000000000400efb <+27>:	ret    
End of assembler dump.
```



在main函数中调用每个`phase_n(input)`函数时，传递的input参数就是输入的字符串，因为只有一个参数，所以字符串就在寄存器%rdi中。

> x86-64中，可以通过寄存器传递最多6个整型（即整数和指针）参数。寄存器的使用是有特殊顺序的，寄存器使用的名字取决于要传递的数据类型的大小。以64位为例，用于传参的寄存器顺序为：%rdi, %rsi, %rdx, %rcx, %r8, %r9。如果一个函数有大于6个整型数据，超过6个的部分就要通过栈来传递。——CSAPP

<phase_1+4> 将一个立即数`$0x402400`传递给%esi，然后call`string_not equal`函数，可以猜这个函数的作用是比较两个字符串是否相等，即要传递了两个参数给函数，第一个参数存储在%rdi就是我们输入的字符串的指针，第二个参数在%rsi(%esi)，说明刚刚传递的立即数`$0x402400`应该是一个字符串的指针。通过x command可以查看以某种数据形式来读取对应地址的数据，例如`x/s address`就是读取address处的数据，并当作字符串来展示，读取到'\0'字节截止。

![image-20221214213653913](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221214213653913.png)

确实读取到了一条字符串，我们把它复制作为phase_1的输入

Border relations with Canada have never been better.

![image-20221214213728054](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221214213728054.png)

拆除成功！

### Phase2

```bash
Dump of assembler code for function phase_2:
   0x0000000000400efc <+0>:     push   %rbp
   0x0000000000400efd <+1>:     push   %rbx
   0x0000000000400efe <+2>:     sub    $0x28,%rsp
   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
   0x0000000000400f05 <+9>:     callq  0x40145c <read_six_numbers>
```

> phase_2的前两句是push %rbp和%rbx，这两个寄存器和%r12~-%r15都划为调用者保存寄存器，当过程P调用过程Q时，Q如果需要用到其中的寄存器，在过程开始必须保存这些寄存器，在结束时再还原，以确保过程P在调用Q的前后这些寄存器的值没有被破坏——CSAPP

call `read_six_numbers`函数之前修改了%rsi的值，说明该函数有两个参数，一个%rdi——输入字符串，%rsi——%rsp的值，也就是栈顶指针。

进入`read_six_numbers`函数中查看，首先从函数命名来看可以猜到函数的作用是读取六个数字。 

```bash
Dump of assembler code for function read_six_numbers:
   0x000000000040145c <+0>:     sub    $0x18,%rsp
   0x0000000000401460 <+4>:     mov    %rsi,%rdx
   0x0000000000401463 <+7>:     lea    0x4(%rsi),%rcx
   0x0000000000401467 <+11>:    lea    0x14(%rsi),%rax
   0x000000000040146b <+15>:    mov    %rax,0x8(%rsp)
   0x0000000000401470 <+20>:    lea    0x10(%rsi),%rax
   0x0000000000401474 <+24>:    mov    %rax,(%rsp)
   0x0000000000401478 <+28>:    lea    0xc(%rsi),%r9
   0x000000000040147c <+32>:    lea    0x8(%rsi),%r8
   0x0000000000401480 <+36>:    mov    $0x4025c3,%esi
   0x0000000000401485 <+41>:    mov    $0x0,%eax   
   0x000000000040148a <+46>:    callq  0x400bf0 <__isoc99_sscanf@plt>
   0x000000000040148f <+51>:    cmp    $0x5,%eax
   0x0000000000401492 <+54>:    jg     0x401499 <read_six_numbers+61>
   0x0000000000401494 <+56>:    callq  0x40143a <explode_bomb>
   0x0000000000401499 <+61>:    add    $0x18,%rsp
   0x000000000040149d <+65>:    retq
End of assembler dump.
```

先介绍一下`sscanf`函数

```c++
int sscanf(const char *str, const char *format, ...)
```

将str字符串以format形式赋值给后面的参数，返回的值是被赋值参数的个数，从<read_six_numbers>~<read_six_bumbers+41>这部分就是在设置sscanf函数的参数：
	其中第一个参数str是%rdi中输入的字符串，格式字符串在%rsi中，通过`x/s 0x4025c3`查到是`"%d %d %d %d %d %d"`，也就是说我们输入的字符串应该是6个数字，这样才能赋值给后面的参数。`...`包含6个指针，整个sscanf已经用去了两个寄存器，还剩%rdx, %rcx, %8, %9四个寄存器可以用来存储d1~d4的指针，而d5和d6的指针也就只能通过栈来传递，占用的是调用者栈空间。

+ %rdx=%rsi                    ->   %rdx = %rsi = &d1   ->   M[%rdx] = d1

+ %rcx=0x4(%rsi)            ->   %rcx=%rdx+4=&d2  ->   M[%rcx]=d2

+ %r8=0x8(%rsi)              ->   %r8=%rdx+8=&d3   ->   M[%r8]=d3

+ %r9=0xc(%rsi)              ->   %r9=%rdx+12=&d4  ->   M[%r9]=d4

+ %rsp=0x10(%rsi)          ->   s1=%rsp=&d5           ->   M[s1]=d5

+ 0x8(%rsp)=0x14(%rsi)  ->   s2=%rsp+8=&d6       ->   M[s2]=d6

  --备注：%rsi指的是phase_2函数的栈指针，%rsp指的是当前read_six_numbers函数的栈指针

在下面的示意图中两个名为s1和s2的变量是d5和d6的指针（8个字节）。下面这个图展示栈情况，在刚调用`read_six_numbers`时，%rsi中存的是phase_1函数的栈帧的位置：

![image-20221217235836043](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221217235836043.png)

调用完sscanf函数后，如果成功的话，phase_1的栈空间中相应的位置都已经写入了数据，函数return的值保存在%rax中，下面的

````assembly
cmp $0x5, %eax
jg 0x401499 <read_six_numbers+61>
callq 0x40143a <explode_bomb>
````

条件跳转会进行判断，如果%eax的值大于5（6）的话，就跳转到相对位置61的指令处，否则会`explode_bomb`

返回到phase_2后，%rsp重新指向了`phase_2`的栈帧，并且%rsp的值为d1

```bash
0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)
0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
0x0000000000400f10 <+20>:    callq  0x40143a <explode_bomb>
```

如果%rsp的值不等于1，将会触发`explode_bomb`，所以d1的值为1。跳转到<phase_2+52>

```bash
0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
```

让%rbx的值为d2的地址，%rbp的值为数据d6之后的地址，再跳转到<phase_2+27>

```bash
0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
0x0000000000400f1a <+30>:    add    %eax,%eax
0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
0x0000000000400f20 <+36>:    callq  0x40143a <explode_bomb>
0x0000000000400f25 <+41>:    add    $0x4,%rbx
0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>
0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>
```

%eax的值改为d1，再加上自己，此时值为2d1，将`（%rbx）`与`%eax`的值进行比较，如果不相等则触发`explode_bomb`，即说明等式$d2=2d1$。

> 这里要注意%rbx %rbp寄存器中的值为一个地址，前者是d2~ d6的地址，后者是d6的地址+4之后的地址，这个值是未知的，所以在用%rbx表示d2~d6时，需要加个括号

然后%rbx指向下一位d3，并且与%rbp比较一下，看是否超过了d6，如果是则可以跳转到<phase_2+64>拆弹成功；否则将再次回到<phase_2+27>判断d3是否等于两倍d2，不等于则触发炸弹，一直这样循环到判断d6是否等于两倍d5后结束。

所以可以得到**phase_2的字符串**是：

1 2 4 8 16 32

![image-20221214213926305](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221214213926305.png)

### Phase3

这个phase和phase_2很相似

我们先看第一部分

```bash
0x0000000000400f43 <+0>:     sub    $0x18,%rsp
0x0000000000400f47 <+4>:     lea    0xc(%rsp),%rcx
0x0000000000400f4c <+9>:     lea    0x8(%rsp),%rdx
0x0000000000400f51 <+14>:    mov    $0x4025cf,%esi
0x0000000000400f56 <+19>:    mov    $0x0,%eax
0x0000000000400f5b <+24>:    callq  0x400bf0 <__isoc99_sscanf@plt>
0x0000000000400f60 <+29>:    cmp    $0x1,%eax
0x0000000000400f63 <+32>:    jg     0x400f6a <phase_3+39>
0x0000000000400f65 <+34>:    callq  0x40143a <explode_bomb>
```

根据phase_2中的经验，看到`sscanf`函数，就知道str是输入字符串，在%rdi中，格式字符串format在%rsi中，R[%rsi]=$0x4025cf，通过`x/s 0x4025cf`得到format为`"%d %d"`，与<+29>处的条件传送指令一致，赋值参数大于1（2）即可跳转，否则引爆。

对于应该输入的两个数据d1和d2，这次存储的位置在

+ %rdx=%rsp+8 -> %rdx=&d1 -> M[%rdx]=d1
+ %rcx=%rsp+12 -> %rcx=&d2 -> M[%rcx]=d2

> 这个地方需要注意lea命令和mov命令的区别

```bash
0x0000000000400f6a <+39>:    cmpl   $0x7,0x8(%rsp)
0x0000000000400f6f <+44>:    ja     0x400fad <phase_3+106>
0x0000000000400f71 <+46>:    mov    0x8(%rsp),%eax
0x0000000000400f75 <+50>:    jmpq   *0x402470(,%rax,8)
```

读取成功后，又是一个条件跳转，如果0x8(%rsp)即d1大于7的话会引爆，说明**d1一定小于等于7**，因为后面有用到具体的d1，所以我们可以先假设d1的值为1。

> 这里我想再复习一下条件跳转指令，做的时候因为搞混过几次浪费了比较多的时间。
>
> |      |                              |
> | ---- | ---------------------------- |
> | jg   | 有符合大于 greater           |
> | jge  | 有符合大于等于 greater equal |
> | jl   | 有符号小于 less              |
> | jle  | 有符号小于等于 less equal    |
> | ja   | 无符号大于                   |
> | jae  | 无符号大于等于               |
> | jb   | 无符号小于 below             |
> | jbe  | 无符号小于等于 below equal   |

<+50>的指令是`jmpq   *0x402470(,%rax,8)`，关于`jmp *`有两种形式

+ jmp *%rax：用寄存器%rax中的值作为跳转目标
+ jmp *(%rax)：以%rax中的值作为读地址，从内存中读出跳转目标

很显然，这条是以M[%rax*8+0x402470]的值作为跳转目标，此时%rax为1，即跳转目标为：0x402478

```bash
(gdb) x 0x402478
0x402478:	0x00400fb9
```

可以知道跳转至0x00400fb9地址处，此处的指令为`mov    $0x137,%eax`，即%rax的值为311，：

```bash
0x0000000000400f7c <+57>:	mov    $0xcf,%eax
0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f83 <+64>:	mov    $0x2c3,%eax
0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f8a <+71>:	mov    $0x100,%eax
0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f91 <+78>:	mov    $0x185,%eax
0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f98 <+85>:	mov    $0xce,%eax
0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
0x0000000000400f9f <+92>:	mov    $0x2aa,%eax
0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fa6 <+99>:	mov    $0x147,%eax
0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fad <+106>:	call   0x40143a <explode_bomb>
0x0000000000400fb2 <+111>:	mov    $0x0,%eax
0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
0x0000000000400fb9 <+118>:	mov    $0x137,%eax
```

最后几句的指令是将%rax的值和d2相比较

```bash
0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax
0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
0x0000000000400fc4 <+129>:	call   0x40143a <explode_bomb>
0x0000000000400fc9 <+134>:	add    $0x18,%rsp
0x0000000000400fcd <+138>:	ret
```

如果d2不等于%rax就会引爆炸弹，到此我们就可以得到phase3的key为`1 311`。

很显然d1=1是我们假设的，这意味着phase3的key并不唯一，d1取不同的值（<=7）会决定%rax的值，间接决定了d2的值。



### Phase4

phase_4的汇编代码如下：

```bash
Dump of assembler code for function phase_4:
    0x000000000040100c <+0>:	sub    $0x18,%rsp
    0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
    0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
    0x000000000040101a <+14>:	mov    $0x4025cf,%esi
    0x000000000040101f <+19>:	mov    $0x0,%eax
    0x0000000000401024 <+24>:	call   0x400bf0 <__isoc99_sscanf@plt>
    0x0000000000401029 <+29>:	cmp    $0x2,%eax
    0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
    0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)
    0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
    0x0000000000401035 <+41>:	call   0x40143a <explode_bomb>
    0x000000000040103a <+46>:	mov    $0xe,%edx
    0x000000000040103f <+51>:	mov    $0x0,%esi
    0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
    0x0000000000401048 <+60>:	call   0x400fce <func4>
    0x000000000040104d <+65>:	test   %eax,%eax
    0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
    0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
    0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
    0x0000000000401058 <+76>:	call   0x40143a <explode_bomb>
    0x000000000040105d <+81>:	add    $0x18,%rsp
    0x0000000000401061 <+85>:	ret    
End of assembler dump.
```

1. **<+0>——<+32>**：和phase_3的一样，都是用sscanf从输入字符串中读出两个数字d1和d2分别存在0x8(%rsp)和0xc(%rsp)

2. **<+34>——<+41>**：读成功后判断d1是否小于等于14，大于14就引爆

3. **<+46>——<+67>**：调用函数`func4`前的参数准备`%rdi %rsi %rdx`，并且要求返回的值%rax必须为0，否则引爆（test就是&操作 只有0&0才会等于0）。func4的汇编代码如下：

   ```bash
   Dump of assembler code for function func4:
       0x0000000000400fce <+0>:	sub    $0x8,%rsp
       0x0000000000400fd2 <+4>:	mov    %edx,%eax
       0x0000000000400fd4 <+6>:	sub    %esi,%eax
       0x0000000000400fd6 <+8>:	mov    %eax,%ecx
       0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx
       0x0000000000400fdb <+13>:	add    %ecx,%eax
       0x0000000000400fdd <+15>:	sar    %eax
       0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx
       0x0000000000400fe2 <+20>:	cmp    %edi,%ecx
       0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36>
       0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
       0x0000000000400fe9 <+27>:	call   0x400fce <func4>
       0x0000000000400fee <+32>:	add    %eax,%eax
       0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
       0x0000000000400ff2 <+36>:	mov    $0x0,%eax
       0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
       0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57>
       0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
       0x0000000000400ffe <+48>:	call   0x400fce <func4>
       0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
       0x0000000000401007 <+57>:	add    $0x8,%rsp
       0x000000000040100b <+61>:	ret    
   End of assembler dump.
   ```

   在通读一遍func4的汇编后，可以发现func4内部有多个条件跳转和嵌套调用，可以尝试先画出大致流程图，再写出对应的C语言伪代码版本：

   ```c
   int func4(int edi, int esi, int edx) // func4(d1, 0, 14)
   {
       eax = edx - esi; 		// <+4>-<+6>	// eax=14-0
       ecx = eax >> (31);		// <+8>-<+10>	// ecx=0 
       eax = (eax + ecx) >> 1;	// <+13>-<+15>  // eax=14/2=7
       ecx = eax+ esi;			// <+17>		// ecx=7+0=0
       if (ecx <= edi){		// <+20>-<+22>	// if(7<=d1)
           eax = 0;			// <+36>		// eax=0
           if(ecx >= edi)		// <+41>-<+43>	// if(7>=d1)
               return eax;		// <+57>-<+61>	// return 0;
           else{
               return func4(edi, ecx+1, edx); //<+45>-<+48>
           }
       }
       else
           return func4(edi, esi, ecx+1);	  	//<+24>-<+27>
   }
   ```

   根据要求返回0，从上面C语言来看要return0，只有在第一次7<=d1 && 7>=d1成立时才会得到。即**d1=7**

4. **<+69>——<+76>**：判断d2是否等于0，不等于则引爆

综上，phase_4的key为`7 0`

### Phase5

我们把完整的汇编代码分成几个部分，先看第一部分的汇编

```bash
0x0000000000401062 <+0>:	push   %rbx
0x0000000000401063 <+1>:	sub    $0x20,%rsp
0x0000000000401067 <+5>:	mov    %rdi,%rbx
0x000000000040106a <+8>:	mov    %fs:0x28,%rax
0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
0x0000000000401078 <+22>:	xor    %eax,%eax
0x000000000040107a <+24>:	call   0x40131b <string_length>
0x000000000040107f <+29>:	cmp    $0x6,%eax
0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
0x0000000000401084 <+34>:	call   0x40143a <explode_bomb>
0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
```

1. **<+8>——<+17>**：在<+8>处的`%fs:0x28`是指明金丝雀值是用段寻址从内存读入的，段寻址的方式可以追溯到8086，现代处理器很少用这种方式寻址了。<+17>处意思是将金丝雀值植入在栈帧任何局部缓冲区和栈状态之间，用作栈破坏检测，这是一种对抗缓冲区溢出攻击的方式，具体介绍可以查阅CSAPP3.10.4节部分

2. **<+22>**：xor异或操作，自己与自己异或得0，这里将%eax赋值为0

3. **<+24>——<+39>**：调用`string_length`函数判断输入字符串的长度是否为6，否则引爆，是的话则跳转到<+112>

   ```bash
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   ```

   将eax=0，再跳转到<+41>

再看第二部分：

```bash
0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
0x000000000040108f <+45>:	mov    %cl,(%rsp)
0x0000000000401092 <+48>:	mov    (%rsp),%rdx
0x0000000000401096 <+52>:	and    $0xf,%edx
0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
0x00000000004010a4 <+66>:	add    $0x1,%rax
0x00000000004010a8 <+70>:	cmp    $0x6,%rax
0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
```

通读后可以发现这一部分是一个循环体，从<+66>——<+70>来看循环次数为6，刚好是输入字符串的长度，可以猜是对每个字符的一种操作。**在这之前请先看对第三部分的解析，再回来看第二部分你会有更清晰。**

1. <+41>：%rbx在一开始被赋值为%rdi也就是输入字符串，当%rax=0时，赋给%ecx的值是输入字符串s的首个字符。随着%rax值的增加，%ecx的值也改变为s[%rax]。
2. <+45>：%cl是%rcx寄存器最小单元，大小为一个字节，因为`movzbl`是无符号拓展的，实际上%ecx和%cl的值都为s[%rax]。并把该字符传给%rsp指向的地址
3. <+48>——<+52>：将%rsp指向的地址的值传递给%edx，对%edx和`0xf`与操作，取%edx的低四位，为何取第四位，从后面的操作来看是因为后续所需要的整数大小不超过15。
4. <+55>：假设0x4024b0处的字符串为t，通过`mobzbl 0x4024b0(%rdx)`将t[%rdx]的字符传给%edx，通过x/s可以查到t为`"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"`
5. <+62>：将%edx的最小单元%dl的值赋值给0x10(%rsp, %rax,1)所指的地址。结合第三部分来看，`flyers`在t中分别对应为`t[9]t[15]t[14]t[5]t[6][7]`，我们就可以知道%edx取四位后的值应该为`9 f e 5 6 7`，相应的通过查找ascii表，**可以找到后四位与之相匹配的字符分别是：`ionefg`，这也就是phase5的key！**

第三部分：

```bash
 0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
 0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
 0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
 0x00000000004010bd <+91>:	call   0x401338 <strings_not_equal>
 0x00000000004010c2 <+96>:	test   %eax,%eax
 0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
```

给0x16(%rsp)处的值赋0，0对应的就是字符串的截止符。<+81>到<+91>是对函数`strings_not_equal`参数的准备和调用，从名字来看是对两个字符串的比较，看是否相等，相等返回0。

+ 第一个参数%rdi：值是0x10(%rsp)，会被编译器读成字符串的地址，因为0x16(%rsp)的字符串终止符所以长度为6。字符串的数值就是0x10(%rsp)-0x15(%rsp)
+ 第二个参数%rsi：参考字符串，0x401338，通过x/s查得"flyers"

这说明：0x10(%rsp)的值为0x66（‘f’的ASCII码值），0x11(%rsp)的值为0x6c(‘l’的ASCII码值)，0x12(%rsp)的值为0x79(‘y’的ASCII码值)，0x13(%rsp)的值为0x65(‘e’的ASCII码值)，0x14(%rsp)的值为0x72(‘r’的ASCII码值)，0x15(%rsp)的值为0x73(‘s’的ASCII码值).

### Phase6

因为phase6很长并且有大量的跳转循环，建议大家在写这个phase时，根据大致功能分成不同部分并一句句画出大致流程图。

#### Part1

```bash
0x00000000004010f4 <+0>:	push   %r14
0x00000000004010f6 <+2>:	push   %r13
0x00000000004010f8 <+4>:	push   %r12
0x00000000004010fa <+6>:	push   %rbp
0x00000000004010fb <+7>:	push   %rbx
0x00000000004010fc <+8>:	sub    $0x50,%rsp
0x0000000000401100 <+12>:	mov    %rsp,%r13
0x0000000000401103 <+15>:	mov    %rsp,%rsi
0x0000000000401106 <+18>:	call   0x40145c <read_six_numbers>
```

对于`read_six_numbers`函数的解析在phase2中已经很详细了，如果有忘记的可以返回去再看一遍，d1~d6按照顺序存储在%rsp ~ %rsp+0x14地址块中

#### Part2

```bash
0x000000000040110b <+23>:	mov    %rsp,%r14
0x000000000040110e <+26>:	mov    $0x0,%r12d
0x0000000000401114 <+32>:	mov    %r13,%rbp
0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
0x000000000040111b <+39>:	sub    $0x1,%eax
0x000000000040111e <+42>:	cmp    $0x5,%eax
0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
0x0000000000401123 <+47>:	call   0x40143a <explode_bomb>
0x0000000000401128 <+52>:	add    $0x1,%r12d
0x000000000040112c <+56>:	cmp    $0x6,%r12d
0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
0x0000000000401132 <+62>:	mov    %r12d,%ebx
0x0000000000401135 <+65>:	movslq %ebx,%rax
0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)
0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
0x0000000000401140 <+76>:	call   0x40143a <explode_bomb>
0x0000000000401145 <+81>:	add    $0x1,%ebx
0x0000000000401148 <+84>:	cmp    $0x5,%ebx
0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
0x000000000040114d <+89>:	add    $0x4,%r13
0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
```

+ <+23>——<+47>：%r13在part1中被赋值为%rsp，即(%r13)=d1，这部分，将d1mov给%eax，减1后比较是否小于等于5，否的话引爆炸弹，说明d1<=6
+ <+52>——<+87>：这部分是一个循环，判断d1是否不等于其他5个数，
+ <+89>——<+93>：这部分，将前面两个部分重新循环在一起，(%r13)会遍历一遍d1 ~ d6，每个都和其他五个数做一轮比较确认不相等。即d1~d6都彼此互不相同，再结合每个数都小于等于6，可以猜到最终答案是1 ~ 6的一个排列组合

#### Part3

```bash
0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
0x0000000000401158 <+100>:	mov    %r14,%rax
0x000000000040115b <+103>:	mov    $0x7,%ecx
0x0000000000401160 <+108>:	mov    %ecx,%edx
0x0000000000401162 <+110>:	sub    (%rax),%edx
0x0000000000401164 <+112>:	mov    %edx,(%rax)
0x0000000000401166 <+114>:	add    $0x4,%rax
0x000000000040116a <+118>:	cmp    %rsi,%rax
0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
```

通过一个循环来遍历d1到d6，并且改变值为`di=7-di`。其中%r14存放的是%rsp是循环的开始d1，%rsi存的是d6后面接着的地址，是循环结束的标志。

#### Part4

```bash
0x000000000040116f <+123>:	mov    $0x0,%esi
0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
0x000000000040117a <+134>:	add    $0x1,%eax
0x000000000040117d <+137>:	cmp    %ecx,%eax
0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
0x000000000040118d <+153>:	add    $0x4,%rsi
0x0000000000401191 <+157>:	cmp    $0x18,%rsi
0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
0x000000000040119a <+166>:	cmp    $0x1,%ecx
0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
0x000000000040119f <+171>:	mov    $0x1,%eax
0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
```

**先总结这部分汇编代码的作用是按照d1 ~ d6的值来顺序存储node**

node是什么结构？我们发现<+143>和<+176>这两句指令把一个地址常量存进寄存器，通过对`x 0x401176`可以发现

![image-20221215180424594](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221215180424594.png)

0x401176是node1的地址。因为node1的地址是存在%edx寄存器上，并且每次更新都是在原地址上+8，我们猜测+8的后地址应该是另一个node的地址，就用`x/8x`来查看，再把得到的地址用`x`解析，果不其然是node2的地址。这就让我想到了数据结构中链表的节点结构，**一个节点存着本身的数据和下一个节点的地址**，虽然在汇编语言面前讨论数据类型没有太多意义，但是对看到第五部分时，代码中已经明示数据是int类型（用%eax存储），重复同样的方式可以得到完整的链表如下：

![image-20221215190304224](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221215190304224.png)

node的结构为：

```c
struct node{
    int val;
    int num;
    struct node *next;
}
```

并且node之间的关系为(图画得有点丑…省去了自身num的部分…见谅）

<img src="https://raw.githubusercontent.com/linengcs/note-img/main/image-20221215190830852.png" alt="image-20221215190830852" style="zoom: 33%;" />

大小关系是node3>node4>node5>node6>node1>node2

实现效果如图所示：

<img src="https://raw.githubusercontent.com/linengcs/note-img/main/image-20221215191807818.png" alt="image-20221215191807818" style="zoom: 33%;" />

#### Part5

```bash
0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx
0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
0x00000000004011ba <+198>:	mov    %rbx,%rcx
0x00000000004011bd <+201>:	mov    (%rax),%rdx
0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)
0x00000000004011c4 <+208>:	add    $0x8,%rax
0x00000000004011c8 <+212>:	cmp    %rsi,%rax
0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
0x00000000004011cd <+217>:	mov    %rdx,%rcx
0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
```

这部分汇编代码的作用是按照d1~d6的值重新调整了链表的顺序。例如原本的链表顺序是node1->node2->node3->node4->node5->node6，输入的d1 ~ d6的值是：3 2 1  4 5 6，那么经过这段代码后链表的顺序就变成了node3->node2->node1->node4->node5->node6。

> 这里要注意mov和lea指令，0x20(%rsp)指就是存有node[d1]地址的栈地址，在mov命令下，会被解析为内存引用再传给%rbx，即%rbx=node[d1]；而lea命令下，传递给%rax的0x28(%rsp)就只是存有node[d2]地址的栈地址，而不会解析为内存引用，即%rax=&node[d2]。

#### Part6

```bash
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
   0x00000000004011da <+230>:	mov    $0x5,%ebp
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	call   0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	ret    
End of assembler dump.
```

+ <+235>——<+245>：在这之前，%rbx的值就是node[d1]，把node[d2]的val值传给eax，又与node[d1]进行比较，如果`node[d1].val >= node[d2].val`即成功，否则引爆
+ <+250>——<+257>：%rbx的值改为node[d2]，回到<+235>，比较node[d3].value >= node[2].value，以此不断循环，直到比较完所有节点，如果node链表的大小是按照从大到小排列的就拆弹成功。

即根据node的大小，我们重新排列后链表应为：node3->node4->node5->node6->node1->node2，因为在part3中改变过di的值(di=7-di)，所有**最终的key为：`4 3 2 1 6 5`**

### SecretBomb

在bomb.c文件的结尾有这么一句话：

```
/* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha! */
```

告诉我们还有一个隐藏的bomb还没拆掉呢！

在哪呢？我们在每个phase的汇编代码中也没有找到有关这个bomb的入口，但还有一个函数对我们是不可见的——`phase_defused()`！对这个函数反汇编：

```bash
Dump of assembler code for function phase_defused:
   0x00000000004015c4 <+0>:		sub    $0x78,%rsp
   0x00000000004015c8 <+4>:		mov    %fs:0x28,%rax
   0x00000000004015d1 <+13>:	mov    %rax,0x68(%rsp)
   0x00000000004015d6 <+18>:	xor    %eax,%eax
   0x00000000004015d8 <+20>:	cmpl   $0x6,0x202181(%rip)        # 0x603760 <num_input_strings>
   0x00000000004015df <+27>:	jne    0x40163f <phase_defused+123>
   0x00000000004015e1 <+29>:	lea    0x10(%rsp),%r8
   0x00000000004015e6 <+34>:	lea    0xc(%rsp),%rcx
   0x00000000004015eb <+39>:	lea    0x8(%rsp),%rdx
   0x00000000004015f0 <+44>:	mov    $0x402619,%esi
   0x00000000004015f5 <+49>:	mov    $0x603870,%edi
   0x00000000004015fa <+54>:	call   0x400bf0 <__isoc99_sscanf@plt>
   0x00000000004015ff <+59>:	cmp    $0x3,%eax
   0x0000000000401602 <+62>:	jne    0x401635 <phase_defused+113>
   0x0000000000401604 <+64>:	mov    $0x402622,%esi
   0x0000000000401609 <+69>:	lea    0x10(%rsp),%rdi
   0x000000000040160e <+74>:	call   0x401338 <strings_not_equal>
   0x0000000000401613 <+79>:	test   %eax,%eax
   0x0000000000401615 <+81>:	jne    0x401635 <phase_defused+113>
   0x0000000000401617 <+83>:	mov    $0x4024f8,%edi
   0x000000000040161c <+88>:	call   0x400b10 <puts@plt>
   0x0000000000401621 <+93>:	mov    $0x402520,%edi
   0x0000000000401626 <+98>:	call   0x400b10 <puts@plt>
   0x000000000040162b <+103>:	mov    $0x0,%eax
   0x0000000000401630 <+108>:	call   0x401242 <secret_phase>
   0x0000000000401635 <+113>:	mov    $0x402558,%edi
   0x000000000040163a <+118>:	call   0x400b10 <puts@plt>
   0x000000000040163f <+123>:	mov    0x68(%rsp),%rax
   0x0000000000401644 <+128>:	xor    %fs:0x28,%rax
   0x000000000040164d <+137>:	je     0x401654 <phase_defused+144>
   0x000000000040164f <+139>:	call   0x400b30 <__stack_chk_fail@plt>
   0x0000000000401654 <+144>:	add    $0x78,%rsp
   0x0000000000401658 <+148>:	ret    
End of assembler dump.
```

果不其然，在<+108>发现了这个`secret_phase`的入口！现在该解决的问题是怎么调用这个phase:

+ <+0>——<+27>：初始化phase_defused的栈空间，并且判断输入了多少条字符串（num_input_string的作用），如果不等于6，则跳到<+123>,栈破坏检测完后结束。如果等于6，则执行<+29>——<+118>的部分——这部分为`secret_phase`触发部分。

+ <+29>——<+62>：熟悉的sscanf函数，%edi中的0x603870通过`x/s`查看后就是我们所输入的**phase_4的key**，在没到phase4之前这部分是空的。%esi中的0x402619查看是`“%d %d %s”`，d1,d2,s分别存在0x8(%rsp)，0xc(%rsp)和0x10(%rsp)中，读取成功后判断%eax是否是3，不是的话则跳到<+113>即跳过触发secret_phase的部分。

+ <+64>——<+81>：同样是熟悉的`strings_not_equal`函数，%rdi是从我们输入字符串中读取的s，存放在0x10(%rsp)；%rsi是目标字符串，我们通过`x/s 0x402622 `查得为`DrEvil`，如果不相等则跳到<+113>即跳过触发secret_phase的部分。

  **如何触发就很明细了，在phase4之后再加上`DrEvil`就行。**

现在让我们看`secret_phase`的汇编：

```bash
Dump of assembler code for function secret_phase:
   0x0000000000401242 <+0>:		push   %rbx
   0x0000000000401243 <+1>:		call   0x40149e <read_line>
   0x0000000000401248 <+6>:		mov    $0xa,%edx
   0x000000000040124d <+11>:	mov    $0x0,%esi
   0x0000000000401252 <+16>:	mov    %rax,%rdi
   0x0000000000401255 <+19>:	call   0x400bd0 <strtol@plt>
   0x000000000040125a <+24>:	mov    %rax,%rbx
   0x000000000040125d <+27>:	lea    -0x1(%rax),%eax
   0x0000000000401260 <+30>:	cmp    $0x3e8,%eax
   0x0000000000401265 <+35>:	jbe    0x40126c <secret_phase+42>
   0x0000000000401267 <+37>:	call   0x40143a <explode_bomb>
   0x000000000040126c <+42>:	mov    %ebx,%esi
   0x000000000040126e <+44>:	mov    $0x6030f0,%edi
   0x0000000000401273 <+49>:	call   0x401204 <fun7>
   0x0000000000401278 <+54>:	cmp    $0x2,%eax
   0x000000000040127b <+57>:	je     0x401282 <secret_phase+64>
   0x000000000040127d <+59>:	call   0x40143a <explode_bomb>
   0x0000000000401282 <+64>:	mov    $0x402438,%edi
   0x0000000000401287 <+69>:	call   0x400b10 <puts@plt>
   0x000000000040128c <+74>:	call   0x4015c4 <phase_defused>
   0x0000000000401291 <+79>:	pop    %rbx
   0x0000000000401292 <+80>:	ret    
End of assembler dump.
```

+ <+0>——<+24>：在<+1>行调用`read_line`函数从命令行读取一行字符串赋值到%rax返回，在<+19>调用`strtol`函数，将%rdi所指向的字符串转换成long类型，%rdx的值为0xa，表示用10进制表示，转换的long值在%rax中，返回后又把值传给%rbx

+ <+27>——<+37>：%eax减1后和0x3e8(400)比较，如果大于400则爆炸

+ <+42>——<+54>：调用`fun7`函数，并且期待返回2，第一个参数%edi，通过x查得是一个数据结构名字叫n1，第二个参数%esi，从%ebx传来，就是准换后的long值d1

  对于n1结构我们通过查看更多的字节得到以下情况：

  ![image-20221216152757712](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221216152757712.png)

  每个n节点占有32个字节，以n1为例，前8个字节是n1的value值为36，8-16字节是n21的地址，16-24是n22的地址，24-32为空，用于对齐。对其他节点做同样操作可以得到一棵树结构：

  ![image-20221216154044068](https://raw.githubusercontent.com/linengcs/note-img/main/image-20221216154044068.png)

  根据每个节点值大小的关系，很明显这是一个AVL二叉平衡树，即每个节点的左节点小于父节点，右节点大于父节点。接下来我们看看fun7函数：

  `fun7`的汇编代码如下：

  ```bash
  Dump of assembler code for function fun7:
     0x0000000000401204 <+0>:		sub    $0x8,%rsp
     0x0000000000401208 <+4>:		test   %rdi,%rdi
     0x000000000040120b <+7>:		je     0x401238 <fun7+52>
     0x000000000040120d <+9>:		mov    (%rdi),%edx
     0x000000000040120f <+11>:	cmp    %esi,%edx
     0x0000000000401211 <+13>:	jle    0x401220 <fun7+28>
     0x0000000000401213 <+15>:	mov    0x8(%rdi),%rdi
     0x0000000000401217 <+19>:	call   0x401204 <fun7>
     0x000000000040121c <+24>:	add    %eax,%eax
     0x000000000040121e <+26>:	jmp    0x40123d <fun7+57>
     0x0000000000401220 <+28>:	mov    $0x0,%eax
     0x0000000000401225 <+33>:	cmp    %esi,%edx
     0x0000000000401227 <+35>:	je     0x40123d <fun7+57>
     0x0000000000401229 <+37>:	mov    0x10(%rdi),%rdi
     0x000000000040122d <+41>:	call   0x401204 <fun7>
     0x0000000000401232 <+46>:	lea    0x1(%rax,%rax,1),%eax
     0x0000000000401236 <+50>:	jmp    0x40123d <fun7+57>
     0x0000000000401238 <+52>:	mov    $0xffffffff,%eax
     0x000000000040123d <+57>:	add    $0x8,%rsp
     0x0000000000401241 <+61>:	ret    
  End of assembler dump.
  ```

  对fun7函数我们改写成C代码会更加清晰：

  ```c
  // 定义AVL的树节点结构
  struct node{
      int val;
      struct node *lchild;
      struct node *rchild;
  }
  
  int fun7(rdi, rsi)	//n1 d1
  {
      if(rdi!=NULL){
          edx = rdi.val;
          if(edx <= esi){
              eax=0;
              if(edx == esi)
                  return eax;
              else{
                  eax = fun7(rdi.rchild, rsi);
                  return 2*eax+1;
              }
          }
          else{
              eax = fun7(rdi,lchild, rsi);
              return 2*eax;
          }
      }else{
          return INT_MAX;
      }
  }
  ```

  对于要返回2的结果，并且eax的值初始化为0开始，那么只有一条执行路径先利用`2*eax+1`从0变成1，再利用`2*eax`从1变成2，注意因为递归调用的自下而上的特性，也就是我们需要从n1出发先访问左节点，再访问其右节点，即n1->n21->n32，因为访问n32就是递归的结束所以`rsi=n32.val`，**即secret_phase的key为22**



### Summary

到此为止终于把所有的phase都拆掉了，每个phase的key如下：

```
phase_1: Border relations with Canada have never been better.
phase_2: 1 2 4 8 16 32
phase_3: 1 311
phase_4: 7 0 DrEvil
phase_5：ionefg
phase_6：4 3 2 1 6 5
secret_phase: 22
```

bomblab给我带来的收货是很大的，最明显的就是阅读汇编代码的能力提高了不少，不会再想之前一样看到就畏惧。同样这个lab也让我使用gdb更加的熟练。

bomblab写完已经有好一阵子了，当时是每天晚上下课后跑到教室里拆完一个bomb就回宿舍，随着难度的越来越大我回宿舍的时间也越来越晚hhh。虽然写这篇文章也花了很长时间，大部分phase都要重新推一遍，但是第二遍也有很大的收货。

