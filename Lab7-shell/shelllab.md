#### TASK

Implement 7 functions: 

+ `eval`: Main routine that **parses** and **interprets** the command line.
  + 解析字符串，检查第一个命令是built-in command还是可执行文件，如果是built-in command则调用`built-in command`来处理，如果是可执行文件，则调用`Fork`函数创建子进程，并在新的上下文的子进程中执行。
+ `builtin_cmd`: Recognizes and interprets the built-in commands: `quit`, `fg`, `bg` and `jobs`
  + `jobs`: List the running and stopped background jobs.
  + `bg <job>`: Change a stopped background job to a running background job, send `SIGCONT` signal to the job
  + `fg <job>`: Change a stopped or running background job to a running foreground job, send `SIGCONT` signal to the job
  + `quit`：terminate the ash
+ `do_bgfg`:  Implements the `bg` and `fg` built-in command.
+ `waitfg`:  Waits for a foreground job to complete.
+ `sigchld_handler`: Catches `SIGCHILD` signals.
+ `sigint_handler`: Catches `SIGINT(ctrl-c)` signals.
+ `sigtstp_handler`: Catches `SIGTSTP(ctrl-z)` signals

#### Hints

+ `ctrl-c`发送`SIGINT`信号给**前台运行的**进程和其所有子进程
+ `ctrl-z`发送`SIGSTP`信号给**前台运行的**进程和其所有子进程，收到该的信号的进程停止，直到收到`SIGCONT`信号才恢复运行
+ tsh不用支持管道和重定向功能
+ 最后的字符是&的话，则在后台运行
+ 注意并发编程错误，使用`sigprocmask`来保证操作的原子性，要操作全局变量时需要注意
+ 考虑实际情况，我们在standard UNIX shell上运行我们的tsh，tsh的进程在standard UNIX shell的前台，基于tsh所fork的子进程已都在同一个进程组，如果我们按下ctrl+c，会向tsh以及其所有的子进程发送SIGINT信号，很显然这与我们的想法冲突了，我们的想法是ctrl+c的SIGINT信号只发送给tsh的前台，并不发给后台。所以解决办法是，在tsh fork子进程后，execve之前，利用`setgpid(0,0)`将fork的子进程置于一个新的进程组中，不受SIGINT signal影响
+ 代码中每个printf后面都有一个`fflush(stdout)`，因为printf是把我们的内容传到输出缓冲区中，并不会在运行时就输出到屏幕，而是在程序结束时导致的缓冲区刷新而把缓冲区的内容输出到屏幕上，我们肯定不能等到tsh结束再打印屏幕输出，我们要在tsh运行时跟其互动，tsh遇到问题也要及时输出，所以我们在每次printf后都会用fflush来人为的刷新缓冲区

+ Parseline function: Characters enclosed in single quotes are treated as a single

+ 当UNIX系统和函数遇到错误的时候通常会设置全局`errno`，定义在`<errno.h>`标准C库中，同了简化这些函数的错误处理，可以对其进行封装

+ `waitpid(-1, NULL, 0)`的行为是：waitpid会默认挂起调用进程，直到它的等待集合中的一个子进程终止。如果等待集合中的一个子进程在刚被调用的时候就被终止，那么就会立即返回，返回的是终止的子进程的pid，如果没有子进程了就会返回-1，并且设置errno为ECHILD。

  `waitpid(-1, NULL, WNOHANG|WUNTRACED)`的行为是立即返回，如果子进程中没有终止或者停止的，则返回值为0，如果有则返回该子进程的pid

  这两者的区别就在前者一旦被调用就会一直挂起调用进程直到子进程全部终止，而后者不会。

+ 注意在键盘上输入 Ctrl+C 会导致内核发送一个 SIGINT 信号到前台进程组中的每个进
  程。默认情况下，结果是终止前台作业。类似地，输人 Ctrl+Z会发送一个 SIGTSTP 信
  号到前台进程组中的每个进程。默认情况下，结果是停止(挂起)前台作业。

+ 在test08遇到了一个问题：

  ![image-20230213141621335](https://cdn.jsdelivr.net/gh/linengcs/note-img@main/uPic/image-20230213141621335.png)

  左边是自己写的，右边是标准，明显看到在前台运行的./myspin的暂停信息在jobs命令中输出了，因为我是把信号终止和暂停的信息写在SIGCHLD_handler中，也就是说在`SIGCHLD_handler`处理SIGCHLD信号前，waitfg函数就结束了tsh进入了下一个迭代，而waitfd函数结束的条件就是没有前台进程。这里要注意的是SIGCHLD信号在什么时候会被发送，定义是在子进程**终止或者暂停**时就会发生，**主程序收到SIGINT和SIGSTOP signal会发给子进程，子进程收到后调用SIGSINT_handler和SIGSTOP_handler信号处理程序（继承自父进程），再发送SIGCHLD signal给父进程**。我再顺藤摸瓜就发现我在SIGSTOP_handler中就修改了job的state为ST，导致在子进程向父进程发送SIGCHLD时，父进程调用SIGCHLD_handler的同时，该信号也被waitfg中的sigsuspend捕捉到，跳出循环开始tsh下一个循环，所以暂停信息就缓存到了下一条命令中。

+ SIGSTOP(19) vs SIGTSTP(20)

  Both signals are designed to suspend a process which will be eventually resumed with `SIGCONT`. The main differences between them are:

  - `SIGSTOP` is a signal sent programmatically (eg: `kill -STOP pid` ) while `SIGTSTP` (for signal - terminal stop) may also be sent through the `tty` driver by a user typing on a keyboard, usually Control-Z.
  - `SIGSTOP` cannot be ignored. `SIGTSTP` might be.

+ 在调用waitfg函数（内部有sigsuspend函数）前，要将SIGCHLD给blocked掉，避免在while条件通过后在调用sigsuspend前收到了sigchld signal导致进程被永远挂起

+ 注意根据tshref的输出来调整tsh在遇到非法输入的输出，记得要及时退出

