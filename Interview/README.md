# c++ 面经 与 项目

# 项目
## IPv4_Media
udp多播流媒体服务端、客户端
### 多播
组播也可以称之为多播这也是 UDP 的特性之一。组播是主机间一对多的通讯模式，是一种允许一个或多个组播源发送同一报文到多个接收者的技术。组播源将一份报文发送到特定的组播地址，组播地址不同于单播地址，它并不属于特定某个主机，而是属于一组主机。一个组播地址表示一个群组，需要接收组播报文的接收者都加入这个群组。

- 广播只能在局域网访问内使用，组播既可以在局域网中使用，也可以用于广域网
- 在发送广播消息的时候，连接到局域网的客户端不管想不想都会接收到广播数据，组播可以控制发送端的消息能够被哪些接收端接收，更灵活和人性化。
- 广播使用的是广播地址，组播需要使用组播地址。
- 广播和组播属性默认都是关闭的，如果使用需要通过 setsockopt () 函数进行设置。

单播用于两个主机之间的端对端通信，广播用于一个主机对整个局域网上所有主机上的数据通信。单播和广播是两个极端，要么对一个主机进行通信，要么对整个局域网上的主机进行通信。实际情况下，经常需要对一组特定的主机进行通信，而不是整个局域网上的所有主机，这就是多播的用途。

多播，也称为“组播”，将局域网中同一业务类型主机进行了逻辑上的分组，进行数据收发的时候其数据仅仅在同一分组中进行，其他的主机没有加入此分组不能收发对应的数据。

多播的地址是特定的，D类地址用于多播。D类IP地址就是多播IP地址，即224.0.0.0至239.255.255.255之间的IP地址，并被划分为局部连接多播地址、预留多播地址和管理权限多播地址3类：

### 内核中fork，pthread_create，还有两者的区别。
fork用来穿用来创建一个子进程，内核会赋值父进程的代码，数据，寄存器，堆栈以及文件句柄等所有资源给子进程（先忽略写时拷贝），基本上刚fork出来的子进程就是父进程的镜像，拥有进程的该有的所有资源。

而pthread_create创建线程时，只是给线程分配了自己的栈内存、寄存器，而代码段，数据段，文件句柄，堆内存等是进程内所有线程共享的资源。

Linux通过clone系统调用实现fork.调用通过一系列的参数标志来指明父、子进程需要共享的资源。fork、vfork、和__clone的库函数都根据各自需要的参数标志去调用clone，然后由clone()去调用do_fork()。

调用fork和 pthread_create,然后利用strace跟踪两者的调用过程，发现果然都是调用的clone。

### 线程间同步与互斥方法

- 互斥：通过保证同一时间只有一个执行流可以对临界资源进行访问（一个执行流访问期间，其它执行流不能访问），来保证数据访问的安全性。

同步：通过一些条件判断来实现多个执行流对临界资源访问的合理性（有资源则访问，没有资源则等待，等有了资源再被唤醒）。

### 线程间通信的方式:

线程间的通信目的主要是用于线程同步，所以线程没有像进程通信中的用于数据交换的通信机制

### 锁机制

包括互斥锁/量（mutex）、读写锁（reader-writer lock）、自旋锁（spin lock）、条件变量（condition）
- 互斥锁/量（mutex）：提供了以排他方式防止数据结构被并发修改的方法。
- 读写锁（reader-writer lock）：允许多个线程同时读共享数据，而对写操作是互斥的。
- 自旋锁（spin lock）与互斥锁类似，都是为了保护共享资源。互斥锁是当资源被占用，申请者进入睡眠状态；而自旋锁则循环检测保持者是否已经释放锁。
- 条件变量（condition）：可以以原子的方式阻塞进程，直到某个特定条件为真为止。对条件的测试是在互斥锁的保护下进行的。条件变量始终与互斥锁一起使用。

### 临界区：

通过多线程的串行化来访问公共资源或一段代码，速度快，适合控制数据访问；

### 互斥量Synchronized/Lock：

采用互斥对象机制，只有拥有互斥对象的线程才有访问公共资源的权限。因为互斥对象只有一个，所以可以保证公共资源不会被多个线程同时访问

互斥量又称互斥锁，主要用于线程互斥，不能保证按序访问，可以和条件锁一起实现同步。当进入临界区 时，需要获得互斥锁并且加锁；当离开临界区时，需要对互斥锁解锁，以唤醒其他等待该互斥锁的线程。其主要的系统调用如下：

pthread_mutex_init:初始化互斥锁

pthread_mutex_destroy：销毁互斥锁

pthread_mutex_lock：以原子操作的方式给一个互斥锁加锁，如果目标互斥锁已经被上锁，pthread_mutex_lock调用将阻塞，直到该互斥锁的占有者将其解锁。

pthread_mutex_unlock:以一个原子操作的方式给一个互斥锁解锁。

### 条件变量，

又称条件锁，用于在线程之间同步共享数据的值。条件变量提供一种线程间通信机制：当某个共享数据达到某个值时，唤醒等待这个共享数据的一个/多个线程。即，当某个共享变量等于某个值时，调用 signal/broadcast。此时操作共享变量时需要加锁。其主要的系统调用如下：

pthread_cond_init:初始化条件变量

pthread_cond_destroy：销毁条件变量

pthread_cond_signal：唤醒一个等待目标条件变量的线程。哪个线程被唤醒取决于调度策略和优先级。

pthread_cond_wait：等待目标条件变量。需要一个加锁的互斥锁确保操作的原子性。该函数中在进入wait状态前首先进行解锁，然后接收到信号后会再加锁，保证该线程对共享资源正确访问。

### 信号量Semphare：

- 信号量机制(Semaphore)
  - 无名线程信号量
  - 命名线程信号量
为控制具有有限数量的用户资源而设计的，它允许多个线程在同一时刻去访问同一个资源，但一般需要限制同一时刻访问此资源的最大线程数目。 信号量是一种特殊的变量，可用于线程同步。它只取自然数值，并且只支持两种操作：

- P(SV):如果信号量SV大于0，将它减一；如果SV值为0，则挂起该线程。

- V(SV)：如果有其他进程因为等待SV而挂起，则唤醒，然后将SV+1；否则直接将SV+1。

其系统调用为：

sem_wait（sem_t *sem）：以原子操作的方式将信号量减1，如果信号量值为0，则sem_wait将被阻塞，直到这个信号量具有非0值。

sem_post（sem_t *sem)：以原子操作将信号量值+1。当信号量大于0时，其他正在调用sem_wait等待信号量的线程将被唤醒。

### 事件(信号)

信号机制(Signal)：类似进程间的信号处理 Wait/Notify：通过通知操作的方式来保持多线程同步，还可以方便的实现多线程优先级的比较操作

### 屏障

屏障（barrier）：屏障允许每个线程等待，直到所有的合作线程都达到某一点，然后从该点继续执行。


### 进程间同步与互斥方法
当多个进程协同完成一些任务时，不同进程的执行进度不一致，这便产生了进程的同步问题。需要操作系统干预，在特定的同步点对所有进程进行同步，这种协作进程之间相互等待对方消息或信号的协调关系称为进程同步。进程互斥本质上也是一种进程同步。

### 进程的同步方法：

互斥锁
读写锁
条件变量
？记录锁(record locking)
信号量
屏障（barrier）

### 简述进程间通信方法
每个进程各自有不同的用户地址空间,任何一个进程的全局变量在另一个进程中都看不到，所以进程之间要交换数据必须通过内核,在内核中开辟一块缓冲区,进程A把数据从用户空间拷到内核缓冲区,进程B再从内核缓冲区把数据读走,内核提供的这种机制称为进程间通信。

不同进程间的通信本质：进程之间可以看到一份公共资源；而提供这份资源的形式或者提供者不同，造成了通信方式不同。

进程间通信主要包括管道、系统IPC（包括消息队列、信号量、信号、共享内存等）、以及套接字socket。

### 管道
- 无名管道(普通管道)(PIPE)，是 UNIX 系统IPC最古老的形式。
    - 它是半双工的（即数据只能在一个方向上流动），具有固定的读端和写端
    - 它只能用于具有亲缘关系的进程之间的通信（也是父子进程或者兄弟进程之间）
    - 它可以看成是一种特殊的文件，对于它的读写也可以使用普通的read、write等函数。但是它不是普通的文件，并不属于其他任何文件系统，并且只存在于内存中

- 优点：简单方便
- 缺点：
  - 局限于单向通信
  - 只能创建在它的进程以及其有亲缘关系的进程之间
  - 缓冲区有限
- FIFO，也称为命名管道，它是一种文件类型。
    - FIFO可以在无关的进程之间交换数据,与无名管道不同。
    - FIFO有路径名与之相关联，它以一种特殊设备文件形式存在于文件系统中。

- 优点：可以实现任意关系的进程间的通信
- 缺点：
  - 长期存于系统中，使用不当容易出错
  - 缓冲区有限
### 系统IPC
- 消息队列，是消息的链接表，存放在内核中。一个消息队列由一个标识符（即队列ID）来标识。 (消息队列克服了信号传递信息少，管道只能承载无格式字节流以及缓冲区大小受限等特点)具有写权限得进程可以按照一定得规则向消息队列中添加新信息；对消息队列有读权限得进程则可以从消息队列中读取信息；
> 特点
1. 消息队列是面向记录的，其中的消息具有特定的格式以及特定的优先级。
2. 消息队列独立于发送与接收进程。进程终止时，消息队列及其内容并不会被删除。
3. 消息队列可以实现消息的随机查询,消息不一定要以先进先出的次序读取,也可以按消息的类型读取。

- **信号量（semaphore）**与已经介绍过的 IPC 结构不同，它是一个计数器，可以用来控制多个线程对共享资源的访问。信号量用于实现进程间的互斥与同步，而不是用于存储进程间通信数据。
> 特点
1. 信号量用于进程间同步，若要在进程间传递数据需要结合共享内存。
2. 信号量基于操作系统的 PV 操作，程序对信号量的操作都是原子操作。
3. 每次对信号量的 PV 操作不仅限于对信号量值加 1 或减 1，而且可以加减任意正整数。
4. 支持信号量组。

- 信号（Signal）：一种比较复杂的通信方式，用于通知接收进程某个事件已经发生

- 套接字（Socket）： socket也是一种进程间通信机制，与其他通信机制不同的是，它可用于不同主机之间的进程通信。
  - 优点：
    - 传输数据为字节级，传输数据可自定义，数据量小效率高
    - 传输数据时间短，性能高
    - 适合于客户端和服务器端之间信息实时交互
    - 可以加密,数据安全性强
  - 缺点：需对传输的数据进行解析，转化成应用级的数据。
- 共享内存（Shared Memory）
它使得多个进程可以访问同一块内存空间（这段共享内存由一个进程创建），不同进程可以及时看到对方进程中对共享内存中数据得更新。这种方式需要依靠某种同步操作，如互斥锁和信号量等 特点：

1. 共享内存是最快的一种IPC，因为进程是直接对内存进行存取
2. 因为多个进程可以同时操作，所以需要进行同步
3. 信号量+共享内存通常结合在一起使用，信号量用来同步对共享内存的访问
   - 优点：无须复制，快捷，信息量大
   - 缺点：通信是通过将共享空间缓冲区直接附加到进程的虚拟地址空间中来实现的，因此进程间的读写操作的同步问题
利用内存缓冲区直接交换信息，内存的实体存在于计算机中，只能同一个计算机系统中的诸多进程共享，不方便网络通信

### mnidb
bitcast存储引擎
启动：先恢复内存数据，循环读取文件，将键值对保存到map中。mark为PUT的保存，mark为DEL的跳过。

单次读取：传入一个读取开始偏移量，将文件指针定位到该处，读取4个字节作为数据总长度，在根据这个长度读取相应的数据并反序列化。
单次写入：传入要写入的键值对entry{mark, key, value} 将之序列化：总长度 mark key value 然后把这个序列化的string写入文件尾部。

merge: 遍历文件，并根据map中是否有该键值对判断是否将之重写到新文件。

对外只保留一个抽象类头文件和一个工厂类，工厂类有一个`MiniDB *get()`，在静态库中的db.cpp中实现，`MiniDB* db = new db(path); return db;` 这样外界就无法窥见具体如何实现。

### serialize
内存数据序列化 为minidb提供序列化支持

> 对于基本数据类型

定义 serialize 和 deserialize 函数模板，然后针对不同的基本数据类型进行特化，使用宏定义减少代码编写。
序列化：传入一个string&和Type& 将传入的数据强转为`char *` 然后使用string的append方法将长度为sizeof(Type)的字节追加到字符串中。然后返回这个string。
反序列化：复制传入string的前sizeof(Type)个字节到接收数据中。

string的序列化: 不光要append字符串内容，还需要string.size() 先存长度再存内容
string的反序列化: 先反序列化一个int的内容，然后根据这个长度len，使用string.substr 作为string的内容

另外为了方便使用我构造了了两个输入输出类，分别重载了 `>>` `<<`操作符

对于input，重载了`>>`，调用反序列化函数，然后`str = str.substr()`截短string

对于output,重载了 `<<`,调用序列化函数,然后将序列化的结果写到optput对象的ostringstream中，使用时使用`os.str()`即可获取序列化的字符串。

> 对于自定义类型

struct或class 需要继承抽象类，并重写序列化/反序列化虚函数。

### timer
定时器 为minidb提供定时任务支持

### rwlocker
无锁实现

- 当「写锁」没有被线程持有时，多个线程能够并发地持有读锁，这大大提高了共享资源的访问效率，因为「读锁」是用于读取共享资源的场景，所以多个线程同时持有读锁也不会破坏共享资源的数据。
- 但是，一旦「写锁」被线程持有后，读线程的获取读锁的操作会被阻塞，而且其他写线程的获取写锁的操作也会被阻塞

以下代码利用atomic实现了一个轻量级的读写资源锁，相比利用互斥锁实现的读写锁，本锁更类似自旋锁，通过 CPU 提供的 CAS 函数（Compare And Swap），在「用户态」完成加锁和解锁操作，不会主动产生线程上下文切换，所以相比互斥锁来说，会快一些，开销也小一些，并且可以根据需要通过构造函数参数设置读/写优先，且locck/unlock语句允许嵌套

#### CAS原子操作
意思就是说，看一看内存`*reg`里的值是不是oldval，如果是的话，则对其赋值newval
``` c++
int compare_and_swap (int* reg, int oldval, int newval)
{
  int old_reg_val = *reg;
  if (old_reg_val == oldval) {
     *reg = newval;
  }
  return old_reg_val;
}
```

#### 实现
我们可以看到，old_reg_val 总是返回，于是，我们可以在 compare_and_swap 操作之后对其进行测试，以查看它是否与 oldval相匹配，因为它可能有所不同，这意味着另一个并发线程已成功地竞争到 compare_and_swap 并成功将 reg 值从 oldval 更改为别的值了。

有三个计数器和一个标志符
- 锁计数
- 等待读
- 等待写
- 当前的写线程

上读锁: 先检测当前线程是否是拥有资源的写线程，如果是的话直接返回，不需要继续上锁；增加读线程等待计数，然后执行循环：原子地加载锁的计数值，如果计数值标记当前是写线程持有资源，就将当前线程所抢到的CPU”时间片”让渡给其他线程,且”当前线程”不参与争抢，直到写线程释放资源；如果是无线程占用或者读线程占用就直接结束循环。然后进行CAS操作：尝试增加当前的锁计数值，如果成功说明成功加上了一把读锁，如果失败，说明有其他线程修改了锁计数值，那么重复之前的循环。
解读锁: 判断当前线程是否是拥有资源的写线程，如果是的话不需要解锁。否则将锁计数值减少，表示解开当前读锁。
上写锁：如果是写写独占状态，不需要加锁。否则先增加写等待计数，然后进入循环：通过CAS操作尝试将锁计数值设为写独占状态，如果不成功就暂时让渡时间片。如果成功，更新读写锁的写线程号。
解写锁: 如果是一个读线程尝试解开写锁，会触发异常。正常情况下，更新读写锁写线程号为无线程，更新锁计数值为无锁。

> 反思

读优先锁对于读线程并发性更好，但也不是没有问题。我们试想一下，如果一直有读线程获取读锁，那么写线程将永远获取不到写锁，这就造成了写线程「饥饿」的现象。

写优先锁可以保证写线程不会饿死，但是如果一直有写线程获取写锁，读线程也会被「饿死」。

既然不管优先读锁还是写锁，对方可能会出现饿死问题，那么我们就不偏袒任何一方，搞个「公平读写锁」。

公平读写锁比较简单的一种方式是：用队列把获取锁的线程排队，不管是写线程还是读线程都按照先进先出的原则加锁即可，这样读线程仍然可以并发，也不会出现「饥饿」的现象。
