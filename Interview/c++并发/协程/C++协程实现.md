# C++协程及其原理

## 协程的几种实现方式及原理

协程又可以称为用户线程,微线程，可以将其理解为单个进程或线程中的多个**用户态线程**，这些微线程在用户态进程控制和调度.协程的实现方式有很多种，包括

1. 使用glibc中的ucontext库实现
2. 利用汇编代码切换上下文
3. 利用C语言语法中的switch-case的奇淫技巧实现(protothreads)
4. 利用C语言的setjmp和longjmp实现

实际上，无论是上述那种方式实现协程,其原理是相同的，都是通过保存和恢复寄存器的状态，来进行各协程上下文的保存和切换。

## 协程较于函数和线程的优点

- 相比于函数:协程避免了传统的函数调用栈，几乎可以无限地递归
- 相比与线程:协程没有内核态的上下文切换，近乎可以无限并发。协程在用户态进程显式的调度，可以把异步操作转换为同步操作，也意味着不需要加锁,避免了加锁过程中不必要的开销。

进程,线程以及协程的设计都是为了并发任务可以更好的利用CPU资源，他们之间最大的区别在于CPU资源的使用上:

- 进程和线程的任务调度是由内核控制的，是抢占式的；
- 协程的任务调度是在用户态完成,需要代码里显式地将CPU交给其他协程,是协作式的

由于我们可以在用户态调度协程任务，所以我们可以把**一组相互依赖的任务设计为协程。这样,当一个协程任务完成之后,可以手动的进行任务切换，把当前任务挂起(yield),切换到另一个协程区工作**.由于我们可以控制程序主动让出资源，很多情况下将不需要对资源进行加锁。

