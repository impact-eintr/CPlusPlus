先介绍两种高性能服务器模型Reactor、Proactor，注意这是**网络模型**

### Reactor模型： 

1 向事件分发器注册事件回调 
2 事件发生 
3 事件分发器调用之前注册的函数 
4 在回调函数中读取数据，对数据进行后续处理 

> Reactor模型实例：libevent，Redis、ACE

### Proactor模型： 

1 向事件分发器注册事件回调 
2 事件发生 
3 操作系统读取数据，并放入应用缓冲区，然后通知事件分发器 
4 事件分发器调用之前注册的函数 
5 在回调函数中对数据进行后续处理 

> Preactor模型实例：ASIO

 

### reactor和proactor的主要区别：

#### 主动和被动

以主动写为例： 
Reactor将handle放到select()，等待可写就绪，然后调用write()写入数据；写完处理后续逻辑； 
Proactor调用aio_write后立刻返回，由内核负责写操作，写完后调用相应的回调函数处理后续逻辑；

可以看出，Reactor被动的等待指示事件的到来并做出反应；它有一个等待的过程，做什么都要先放入到监听事件集合中等待handler可用时再进行操作； 
Proactor直接调用异步读写操作，调用完后立刻返回；

#### 实现

Reactor实现了一个被动的事件分离和分发模型，服务等待请求事件的到来，再通过不受间断的同步处理事件，从而做出反应；

Proactor实现了一个主动的事件分离和分发模型；这种设计允许多个任务并发的执行，从而提高吞吐量；并可执行耗时长的任务（各个任务间互不影响）

#### 优点

Reactor实现相对简单，对于耗时短的处理场景处理高效； 
操作系统可以在多个事件源上等待，并且避免了多线程编程相关的性能开销和编程复杂性； 
事件的串行化对应用是透明的，可以顺序的同步执行而不需要加锁； 
事务分离：将与应用无关的多路分解和分配机制和与应用相关的回调函数分离开来，

Proactor性能更高，能够处理耗时长的并发场景；

#### 缺点

Reactor处理耗时长的操作会造成事件分发的阻塞，影响到后续事件的处理；

Proactor实现逻辑复杂；依赖操作系统对异步的支持，目前实现了纯异步操作的操作系统少，实现优秀的如windows IOCP，但由于其windows系统用于服务器的局限性，目前应用范围较小；而Unix/Linux系统对纯异步的支持有限，应用事件驱动的主流还是通过select/epoll来实现；

#### 适用场景

Reactor：同时接收多个服务请求，并且依次同步的处理它们的事件驱动程序； 
Proactor：异步接收和同时处理多个服务请求的事件驱动程序；

 





### 再说Actor模型： 

Actor模型被称为高并发事务的终极解决方案，
主角是Actor，类似一种worker，Actor彼此之间直接发送消息，不需要经过什么中介，消息是异步发送和处理的。Actor模式中，“一切皆是Actor”，所有逻辑或者模块均别看做Actor，通过不同Actor之间的消息传递实现模块之间的通信和交互。Actor模型描述了一组为了避免并发编程的常见问题的公理:

1.所有Actor状态是Actor本地的，外部无法访问。
2.Actor必须只有通过消息传递进行通信。
3.一个Actor可以响应消息:推出新Actor,改变其内部状态,或将消息发送到一个或多个其他参与者。
4.Actor可能会堵塞自己,但Actor不应该堵塞它运行的线程。

> actor模型实例：skynet，Erlang 

Actor模型是一个概念模型，用于处理并发计算。它定义了一系列系统组件应该如何动作和交互的通用规则，最著名的使用这套规则的编程语言是Erlang。

一个Actor指的是一个最基本的计算单元。它能接收一个消息并且基于其执行计算。

这个理念很像面向对象语言，一个对象接收一条消息（方法调用），然后根据接收的消息做事（调用了哪个方法）。

Actors一大重要特征在于actors之间相互隔离，它们并不互相共享内存。这点区别于上述的对象。也就是说，一个actor能维持一个私有的状态，并且这个状态不可能被另一个actor所改变。

 

### Actor优点、缺点分析

   传统的并发编程方式大都使用“锁”的机制，相信大多数都是”悲观锁“，这里几乎可以断定会出现两个明显的问题：

  1.随着项目体量增大，业务愈加复杂，不可避免大量使用“锁”，然而大家都知道“锁”的机制其实很耗性能的，所以大量使用锁的机制肯定会造成效率不高

  2.即使大量依赖“锁”解决了系统中资源竞争的情况，但是由于没有一个规范的编程模式，最后系统的稳定性肯定会出问题，最根本的原因是没把系统的任务调度抽象出来，由于任务调度和业务逻辑的耦合在一起，很难做一个很高层的抽象，保证任务调度有序。

  3.难以维护等等弊端

  上面是传统通过“锁”的机制实现并发编程的缺点，然而Actor为什么一定层度上可以解决这些问题呢？个人认为其最根本的原因是Actor模式下提供了一种可靠的任务调度系统，也就是在原生的线程或者协程级别上做了更高层次的封装。这会给编程模式带来巨大的好处：

  1.由于抽象了任务调度系统，那么就可以使系统的线程调度可控，易于统一处理，稳定性和可维护性好

  2.作为开发者我们只需要关心每个Actor的逻辑就可以了，避免“锁”的“滥用”

  3.Actor也提供了很多基本准则，避免了很多并发编程中的问题 

  ……

那么Actor没有缺点吗？那也不是，比如当所有逻辑都跑在Actor中时，很难掌控Actor的粒度，稍有不慎就可能造成系统中Actor个数爆炸的情况，Actor当出现必须共享数据或者状态时就很难避免使用“锁”，但似乎由于上面的“Actor可能会堵塞自己,但Actor不应该堵塞它运行的线程”准则冲突，哈哈，这个时候也许可以选择使用redis做数据共享

 

### Actor技术推荐

  Actor技术现在还是蛮成熟的，最著名的原生支持Actor并发模式的Erlang，还有内置Actor核心库的Scala语言。

有些语言如java，C#并没有原生对Actor模式的支持，但是有很多的开源库弥补了这个缺失。



### 通俗描述：

其实无论是使用数据库锁 还是多线程，这里有一个共同思路，就是将数据喂给线程，就如同计算机是一套加工流水线，数据作为原材料投入这个流水线的开始，流水线出来后就是成品，这套模式的前提是数据是被动的，自身不复杂，没有自身业务逻辑要求。适合大数据处理或互联网网站应用等等。

但是如果数据自身要求有严格的一致性，也就是事务机制，数据就不能被动被加工，要让数据自己有行为能力保护实现自己的一致性，就像孩子小的时候可以任由爸妈怎么照顾关心都可以，但是如果孩子长大有自己的思想和要求，他就可能不喜欢被爸妈照顾，他要求自己通过行动实现自己的要求。

数据也是如此。

只有我们改变思路，让数据自己有行为维护自己的一致性，才能真正安全实现真正的事务。

我们可以看到

Actor模型=数据+行为+消息。

Actor模型内部的状态由自己的行为维护，外部线程不能直接调用对象的行为，必须通过消息才能激发行为，这样就保证Actor内部数据只有被自己修改。