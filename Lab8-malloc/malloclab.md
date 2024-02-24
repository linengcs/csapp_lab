### 实验要求

1. 只需要修改一个文件——mm.c

2. 我们的分配器实现要求：书本已经明确指出空闲块的组织方式为隐式空闲链表，合并方式为立即边界标记合并，放置策略采取首次适配搜索，放置策略和分割分别实现在`find_first`和`place`函数中

3. 需要完成的函数

   ```c
   int mm_init(void);
   void *mm_malloc(size_t size);
   void mm_free(void *ptr);
   void *mm_realloc(void *ptr, size_t size);
   ```

4. 我们所写的allocator的隐式空闲链表中的块格式如下：

   ![image-20230223202858304](https://cdn.jsdelivr.net/gh/linengcs/note-img@main/uPic/image-20230223202858304.png)

   隐式空闲链表的第一个字(4 bytes)是一个用于双字边界对齐的的没有使用的填充字；填充字之后跟着的就是一个特殊的**Prologue block**，由两个字构成——header and footer；跟在prologue block之后的是零或更多的常规块，由malloc、realloc、free函数创建修改。堆区通常以一个特殊的**epilogue block**结尾，这个块分配字节大小为0，只有一个header。设置这两个特殊块的作用就是简化块之间进行coalesce时的边界条件。

   > 边界器使用一个单独的static全局变量heap_listp，它总是指向prologue block

   

5. 上述函数的实现功能要求

   1. mm_init：
      + **使用背景**：在调用mm_malloc、mm_free或者mm_realloc之前需要先调用mm_init来初始化分配器
      + **功能**：初始化堆区，创建padding block、prolugue block和epilogue block，以及给堆申请一个默认大小空间
      + 如果初始化有问题则返回-1，否则返回0，需要讨论的问题有：分配空间不足等
   2. mm_malloc：
      1. 返回8字节(DWORD)对齐的指针——指向已分配块payload的第一个字节
      2. 功能：先检查请求合法，根据实际情况修改block的实际size，从可用空闲块中寻找，如果没有则申请
   3. mm_free：
      1. It returns nothing.
      2. he passed pointer (ptr) was returned by an earlier call to mm malloc or mm realloc and has not yet been freed
      3. 释放后立即进行合并

6. 在memlib.c中的补充函数

7. 
