# I/O
一切实现的基础
## stdio
### 打开关闭
- fopen()
    - open(linux)
    - openfile(win)
- fclose()
~~~ c
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef false
#include <stdio.h>
FILE *fopen(const char *path,const char *mode);
FILE *fdopen(int fd,const char *mode);
FILE *freopen(const char *pathmconst char *mode,FILE *)
#endif

int main()
{
#ifdef false
    char *ptr = "abc";
    ptr[0] = 'x';如果要修改的话，会报段错误。意图修改一个常量
    printf("%s\n",ptr);
#endif

    FILE *fp;
    fp = fopen("log","r");
    if (fp == NULL){
        fprintf(stderr,"fopen() faild! errno = %d\n",errno);//errno已经被私有化
        perror("fopen");
        printf("%s\n",strerror(errno));
        exit(1);
    }else{
        fputs("ok!\n",stdout);
        fputs("OK\n",fp);
    }
    return 0;
}

~~~
**如果一个系统函数有打开和其对应的关闭操作，那么函数的返回值是放在 堆 上的（malloc 与free），否则有可能在堆上也有可能在静态区**

~~~ bash
/usr/include/asm-generic/errno-base.h
/usr/include/asm-generic/errno.h
~~~

~~~c
#include <errno.h>
perror("fopen()",errno);
#include <string.h>
strerror(errno);
~~~
- 默认最多打开的文件描述符个数
~~~bash
$ ulimit -a

-n: file descriptors                1024
~~~
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    int count = 0;
    FILE *fp = NULL;

    while(1)
    {
        fp = fopen("tmp","r");
        if (fp == NULL)
        {
            perror("fopen()");
            break;
        }
        count++;
    }
    printf("%d\n",count);
    return 0;
}

~~~
#### 新文件权限产生
**mod = 0666 & ~umask**
修改方式 
~~~ bash
umask 022
~~~
### 读写
- fgetc()
- fputc()
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*******
 *这节主要讲fgetc 可以用来统计文件的大小
 *
 * *****/

int main(int argc,char **argv)
{
    FILE *fp = NULL;
    int count = 0;

    if (argc < 2){
        fprintf(stderr,"Usage:\n");
        exit(1);
    }

    fp = fopen(argv[1],"r");
    if (fp == NULL){
        perror("fopen()");
        exit(1);
    }

    while(fgetc(fp) != EOF){
        count++;
    }
    printf("%d\n",count);
    fclose(fp);
    return 0;
}

~~~
- fgets()
- fputs()
cp 的fgets/fputs版本
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SIZE 1024

int main()
{
    FILE *fps,*fpd;
    fps = fopen("./tmp","r");
    if (fps == NULL){
        strerror(errno);
        exit(1);
    }

    fpd = fopen("./copy","w");
    if (fpd == NULL){
        fclose(fps);
        strerror(errno);
        exit(1);
    }

    char buf[SIZE];
    while(fgets(buf,SIZE,fps)){
        fputs(buf,fpd);
    }
    fclose(fps);
    fclose(fpd);
    return 0;
}
~~~
**fgets遇到 size-1 or '\n'停止**
~~~ base
abcd
1-> a b c d '\0'
2-> '\n' '0'
~~~
~~~ c
#define SIZE 5
int main()
{
    FILE *fp = NULL;
    fp = fopen("./tmp","r");
    if (fp == NULL){
        perror("fopen()");
        exit(1);
    }

    char buf[SIZE];

    while(fgets(buf,SIZE,fp)){
        printf("%s\n",buf);
    }
    return 0;
}
~~~
- fread()
- fwrite()
cp的fread/fwrite版本
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SIZE 1024
/***************
 *fread 实现文件复制
 *
 **************/
int main()
{
    FILE *fps,*fpd;
    fps = fopen("./tmp","r");
    if (fps == NULL){
        strerror(errno);
        exit(1);
    }

    fpd = fopen("./copy","w");
    if (fpd == NULL){
        fclose(fps);
        strerror(errno);
        exit(1);
    }

    char buf[SIZE];

    int n = 0;
    while ((n = fread(buf,1,SIZE,fps)) > 0){
        printf("%d\n",n);
        fwrite(buf,1,n,fpd);
    }
    fclose(fps);
    fclose(fpd);
    return 0;
}
~~~
### 打印与输入
- printf()
- scanf()
~~~ c
#include <stdio.h>
/*****************
 *sprintf atoi fprintf
 *sprintf 可以看作是 atoi的反向操作
 * **************/
int main()
{
    char buf[1024];
    int y = 2020;
    int m = 12;
    int d = 24;
    sprintf(buf,"%d/%d/%d",y,m,d);
    printf("%s",buf);
    return 0;
}
~~~
### 文件指针
#### 何谓“文件指针”？
就像读书时眼睛移动一样，文件指针逐行移动
#### 什么时候用？
对一个文件先读后写的时候，比如：
~~~ c
FILE *fp = fopen();
fputc(fp);

fgetc();//无法得到刚刚写入的东西
~~~

~~~ c

int fseek(FILE *stream,long offset,int whence);
//设置文件指针位置 offset是偏移位置
long ftell(FILE *stream);
//返回文件指针位置
void rewind(FILE *stream);
//使得文件指针回到文件开始位置
~~~
- fseek()
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
    const int SIZE = 1024;
    FILE *fp = NULL;
    char buf[SIZE];
    buf[0] = 'A';
    buf[1] = 'A';
    buf[2] = 'A';
    buf[3] = 'A';

    fp = fopen("./tmp","w+");
    if (fp == NULL){
        strerror(errno);
        exit(1);
    }
    
    int i = 0;
    while(i < 10){
        unsigned long n = fwrite(buf,1,4,fp);
        fseek(fp,-n,SEEK_CUR);
        unsigned long len =  fread(buf,1,n,fp);
        printf("%lu\n",len);
        fseek(fp,0,SEEK_END);
        i++;
    }
    
    fseek(fp,1024,SEEK_CUR);
    return 0;
}
~~~
- ftell()
- rewind()
### 刷新缓存
- fflush()
~~~ c
printf("Before while");
fflush();
while(1)
printf("After while";)
~~~
#### 缓冲区
##### 缓冲区的作用a
大多数情况是好事，合并系统调用
##### 行缓冲
换行时刷新 stdout
##### 全缓冲
默认
##### 无缓冲
stderr
### 取到完整的一行
- getline()
~~~ c
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    FILE *fp = NULL;
    fp = fopen("./tmp","r");
    if (fp == NULL){
        strerror(errno);
        exit(1);
    }

    size_t linesize = 0;
    char *line = NULL;

    while(1){
        if (getline(&line,&linesize,fp) < 0){
            break;
        }
        printf("%ld\n",strlen(line));
        printf("%ld\n",linesize);
    }
    return 0;
}
~~~
### 临时文件
- tmpfile()
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
    FILE *tmpfp;
    tmpfp = tmpfile();

    const char* oribuf = "2020/12/25";
    int SIZE = 0;
    while(*(oribuf+SIZE) != 0){
        SIZE++;
    }
    printf("%d\n",SIZE);
    
    fwrite(oribuf,1,SIZE,tmpfp);
    fseek(tmpfp,0,SEEK_SET);
    

    FILE *fp;
    fp = fopen("tmp","w");
    char desbuf[SIZE];
    int n = fread(desbuf,1,SIZE,tmpfp);
    fwrite(desbuf,1,n,fp);

    fclose(tmpfp);
    fclose(fp);
    return 0;
}
~~~
## sysio(系统IO)
**fd**是文件IO中贯穿始终的类型
### 文件描述符的概念
FILE *

整型数，数组下标，文件描述符优先使用当前可用范围内最小的
### 文件IO操作
- open
- close
- read
- write
- lseek
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc,char** argv)
{
    if (argc < 3){
        fprintf(stdout,"Usage...");
        exit(1);
    }

    int sfd = open(argv[1],O_RDONLY);
    if (sfd < 0){
        strerror(errno);
        exit(1);
    }

    int dfd = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(dfd < 0){
        close(sfd);
        strerror(errno);
        exit(1);
    }

    const int SIZE = 1024;
    char buf[SIZE];

    while(1){
        len = read(sfd,buf,SIZE);
        if (len < 0){
            strerror(errno);
            break;
        }
        if (len == 0){
            break;
        }
        //以防写入不足
        while(len > 0){
            int len = 0;//read的返回值
            int ret = 0;//write的返回值
            int pos = 0;//写截至的位置
            ret = write(dfd,buf+pos,len);
            if (ret < 0){
                strerror(errno);
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
    }

    close(dfd);
    close(sfd);

    exit(0);
}

~~~
### 文件IO与标准IO的区别
- 区别 标准IO的吞吐量大 系统IO的响应速度快(是对程序而言，对用户而言stdio的速度“更快”)
- 转换 `fileno` `fdopen`
- 注意 标准IO与系统IO不可以混用

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    for (int i = 0;i < 1025;i++){
        putchar('a');
        write(1,"b",1);
    }

    return 0;
}

~~~

`strace`可以用来查看一个可执行文件的系统调用，使用`strace ./a.out`可以看到先进行1024次系统调用然后缓冲区满了1024合并为一次系统调用
### IO的效率问题


### 文件共享
`truncate` 或者 `ftruncate`

### 原子操作
不可分割的操作
- 作用： 解决竞争和冲突 比如`tmpnam`就操作不原子

### 程序中的重定向
- dup
将传入的文件描述符复制到可使用的(未使用的)最小新文件描述符,在下面的例子中，将标准输出关闭后，文件描述符1空闲(不发生竞争时)，`dup`将会把打开了文件`/tmp/out`的文件描述符复制给文件描述符1 ，之后对文件描述符1 的操作就相当与操作文件`/tmp/out`

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define FNAME "/tmp/out"


int main()
{
    int fd;
    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if (fd < 0) {
        perror("open()");
        exit(1);
    }
    //dup 不原子
    close(1);//关闭标准输出
    dup(fd);
    close(fd);
    
    /*********************/
    printf("Hello world\n");
    return 0;
}

~~~

- dup2
`dup2`可以避免关闭文件描述符后被其他线程抢占
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define FNAME "/tmp/out"


int main()
{
    int fd;
    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if (fd < 0) {
        perror("open()");
        exit(1);
    }
    //dup2 原子
    dup2(fd,1);
	
	if (fd != 1) {//打开的文件描述符如果不是1自己，就可以把他关掉了，有重定向后的 1 可以访问文件，保持尽量少的资源使用
		close(fd);
	}

    /*********************/
    printf("Hello world\n");
    return 0;
}

~~~

### 同步
- sync
设备即将解除挂载时进行全局催促，将buffer cache的数据刷新
- fsync
刷新文件的数据
- fdatasync
刷新文件的数据部分，不修改文件元数据
# 文件系统

<hr/>

### 目标
类似`ls`的实现

## 目录与文件
### 获取文件属性
- stat 通过文件路径获取属性
- fstat 通过文件描述符获取属性
- lstat 面对符号链接文件时，获取的是符号🔗文件`l`的属性,而`stat`获取的是链接对象的属性
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static off_t flen(const char *fname){
    struct stat statres;
    if (stat(fname,&statres) < 0) {
        perror("tata()");
        exit(1);
    }
    return statres.st_size;
}

int main(int argc,char **argv)
{
    if (argc < 2) {
        fprintf(stderr,"Usage...\n");
        exit(1);
    }

    long len = flen(argv[1]);
    printf("st_size = %ld\n",len);

    exit(0);
}

~~~

注意，在unix中文件大小`size`只是一个属性，不一定代表文件真正的大小(与文件系统相关)

~~~ c
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc,char **argv)
{
    if (argc < 2) {
        fprintf(stderr,"Usage...\n");
        exit(1);
    }

    int fd = open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0600);
    if (fd < 0) {
        perror("open()");
        exit(1);
    }

    long err = lseek(fd,5LL*1024LL*1024LL*1024LL-1LL,SEEK_SET);
    if (err == -1) {
        perror("lseek");
        exit(1);
    }

    write(fd,"",1);

    return 0;
}

~~~
### 文件访问权限
- _ _ _ (7种文件类型) _ t(粘住位) _ g+s _ u+s _ _ _ user _ _ _  group _ _ _ other共15位用16位的位图表示
**7种文件类型** `b c d - l s p` `不错的-老色批`
- b 块设备文件
- c 字符设备文件
- d 目录
- - 常规文件
- l 符号链接文件
- s socket文件
- p 匿名管道文件(不可见)
~~~ c

//文件类型
static int ftype(const char* fname) {
    if (stat(fname,&statres) < 0) {
        perror("rstat()");
        exit(1);
    }
    if (S_ISREG(statres.st_mode)) {
        return '-';
    }else if (S_ISDIR(statres.st_mode)) {
        return 'd';
    }else{
        return '?';
    }
~~~
### umask
- 防止产生权限过松的文件
### 文件权限的更改与管理
- chmod (命令)
    - chmod a+x ??x ??x ??x
    - chmod u+x ??x ??? ???
    - chmod g+x ??? ??x ???
    - chmod o+x ??? ??? ??x
- chmod()

~~~ c
#include <sys/stat.h>
int chmod(const char *path,mode_t mode);
int fchmod(int fd,mode_t mode); //修改一个已经成功打开的文件

~~~
### 粘住位
- `t位`
~~~ bash
$ ls -l /
drwxrwxrwt   1 root root 3.6K 2月   8 17:58 tmp
~~~
### 文件系统的实质
**FAT UFS**
文件系统： 文件或数据的存储和管理

### 硬链接 符号连接
- link (命令) 创建`硬链接` 其实就是在`目录项`中添加一条映射
- ln() => ln
- unlink() 删除一个文件的硬连接 但并不是删除文件 只有当一个文件的硬链接数为0 且没有进程占用该文件时一个文件才有被删除的可能（数据可被随意改写）

### 文件读写时间
- utime()

### 目录的创建和销毁
- mkdir ()
- rmdir()

### 更改当前工作路径
- chdir => cd

### 分析目录/读取目录内容
单独调用
- glob **解析模式/通配符**

~~~ c
//glob解析路径
static void Glob(){
    glob_t globres;
    int err = glob(PAT,0,&errfunc,&globres);
    if (err) {
        printf("Error code = %d\n",err);
    }

    for (int i = 0;globres.gl_pathv[i]!= NULL;i++) {
        fprintf(stdout,"%s\n",globres.gl_pathv[i]);
    }
}

~~~

组合调用
- opendir()
- closedir()
- readdir()
- seekdir()
- telldir()

~~~ c
//组合解析路径
static void PathParse(char *Path) {
    DIR *dp;
    struct dirent *cur;

    dp = opendir(Path);
    if (dp == NULL) {
          perror("opendir");
          exit(1);
    }

    while((cur = readdir(dp)) != NULL) {
        fprintf(stdout,"%s ",cur->d_name);
        fprintf(stdout,"type:%d ",cur->d_type);
    }

    closedir(dp);
}

    //getcwd()的使用
    char pwd[1024];
    getcwd(pwd,1024);
    fprintf(stdout,"%s\n",pwd);
    PathParse(pwd);
~~~

- 综合案例 `mydu.c`

~~~ c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <string.h>

#define PATHSIZE 1024

static int path_noloop(const char *path) {
    char *pos;
    pos = strrchr(path,'/');
    if (pos == NULL) {
        exit(1);
    }

    if (strcmp(pos+1,".") == 0||strcmp(pos+1,"..")== 0) {
        return 0;
    }
    return 1;
}

static int64_t mydu(const char *path) {
    static struct stat statres;
    static char nextpath[PATHSIZE];
    glob_t globres;
    int64_t sum = 0;

    //非目录
    if (lstat(path,&statres) < 0) {
        perror("lstat()");
        exit(1);
    }

    if (!S_ISDIR(statres.st_mode)){
        fprintf(stdout,"%ld\t%s\n",statres.st_blocks / 2,path);
        return statres.st_blocks;
    }
    //目录
    //拼接路径
    strncpy(nextpath,path,PATHSIZE);
    strncat(nextpath,"/*",PATHSIZE);
    if (glob(nextpath,0,NULL,&globres) < 0) {
        fprintf(stderr,"glob()");
        exit(1);
    }

    strncpy(nextpath,path,PATHSIZE);
    strncat(nextpath,"/.*",PATHSIZE);
    if (glob(nextpath,GLOB_APPEND,NULL,&globres) < 0) {
        fprintf(stderr,"glob()");
        exit(1);
    }

    sum = statres.st_blocks;
    for (int i = 0;i < globres.gl_pathc;i++){
        if (path_noloop(globres.gl_pathv[i]))
            sum += mydu(globres.gl_pathv[i]);
    }
    
    globfree(&globres);//回收资源
    return sum;
        
}

int main(int argc,char **argv)
{   
    if (argc < 2) {
        fprintf(stderr,"%s\n","Usage...");
        exit(1);
    }
    
    printf("%ld\t%s\n",mydu(argv[1])/2,argv[1]);

    return 0;
}

~~~
## 系统数据文件和信息
1. `/etc/passwd`
    - getpwuid();
    - getpwnam();
2. `/etc/group`
    - getgrgid();
    - getgrgrnam();
3. `/etc/shadow`
    - getspnam();
    - crypt();
    - getpass();

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <shadow.h>
#include <string.h>

int main(int argc,char **argv)
{
    char *input_passwd;//来自命令行的密码原文
    char *crypted_passwd;
    struct spwd *shadowline;
    
    if (argc < 2) {
        fprintf(stderr,"Usage...\n");
        exit(1);
    }

    input_passwd = getpass("PassWoed:");

    shadowline = getspnam(argv[1]);

    crypted_passwd = crypt(input_passwd,shadowline->sp_pwdp);
    
    if (strcmp(shadowline->sp_pwdp,crypted_passwd) == 0)
      puts("ok!");
    else
      puts("failed!");

    return 0;
}

以上代码编译后(编译要加`-lcryp`) ，要使用root用户执行(普通用户没有权限)

~~~

### 时间戳
**time_t => char * => struct_tm**

- time() 从kernel中取出时间戳(以秒为单位)
- gntime() 将时间戳转换为`struct_tm` 格林威治时间
- localtime() 将时间戳转换为`struct_tm` 本地时间
- mktime() jaing struct_tm结构体转换为时间戳，还可以检查是否溢出
- strftime(); 格式化时间字符串

~~~ c
time_t stamp;
time(&stamp);
stamp = time(NULL);

tm = localtime(&stamp);

strftime(buf,BUFSIZE,"%Y-%m-%d",tm);
puts(buf);
~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BUFSIZE 1024

int main()
{

    char fmttime[BUFSIZ];
    int count = 0;

    FILE *fptr = fopen("./log","a+");
    if (fptr == NULL) {
        perror("fopen()");
        exit(1);
    }

    char buf[BUFSIZE];

    while(fgets(buf,BUFSIZE,fptr) != NULL){
        count++;
    }

    char res[BUFSIZE];

    while (1){
        time_t stamp;
        stamp = time(NULL);

        struct tm *struct_tm;
        struct_tm = localtime(&stamp);

        strftime(fmttime,BUFSIZE,"%Y-%m-%d %H:%M:%S",struct_tm);
        fprintf(fptr,"%d %s\n",++count,fmttime);
        fflush(fptr);
        sleep(1);
    }

    fclose(fptr);

    exit(0);
}

~~~

## 进程环境
### main函数
- `int main(int argc,char **argv)`
### 进程的终止
- **正常终止**
     - **从main函数返回** 
     - **调用exit**
     - **调用`_exit`或者`_Exit`**
     - **最后一个线程从其启动例程返回**
     - **最后一个线程调用`pthread_exit`**
#### 钩子函数a
    All functions registered with atexit(3) and on_exit(3) are called,in the reverse order of their registration.
- `atexit()`
~~~ c

~~~

- **异常终止**
    - **调用`abort`**
    - **接到一个信号并终止**
    - **最后一个线程对其取消请求作出响应**

~~~ c

~~~

### 命令行参数的分析

~~~ c

//解析命令行
while(1) {
    c = getopt(argc,argv,"lt-a"); // - 识别非选项的传参
    if (c < 0){
        break;
    }
    
    switch (c){
        case 'l':
            f.filesize = flen(argv[1]);
            strncat(fmtstr,"filesize:%ld ",FMTSTRSIZE-1);
            break;
        case 't':
            f.filetype = ftype(argv[1]);
            strncat(fmtstr,"filetype:%c ",FMTSTRSIZE-1);
            break;
        case 'a':
            PathParse(argv[optind]);
            break;
        default:
            break;
    }
}
~~~

### 环境变量
**本质就是 KEY = VALUE**
- `export`
- getenv()
- setenv()
~~~ c
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

static void getEnv(char *key){
    puts(getenv(key));
}

int main()
{
    
    for (int i = 0;environ[i] != NULL;i++){
        puts(environ[i]);
    }
    getEnv("ZSH");
    return 0;
}
~~~

### C程序的存储空间布局

- pmap (1)
### 库

~~~ c
#ifndef LLIST_H__
#define LLIST_H__
enum{
    F = 1,
    B = 2,
};

//普通节点
struct llist_node_st{
    void *data;
    struct llist_node_st *prev;
    struct llist_node_st *next;
};
//头节点
typedef struct {
    int size;
    struct llist_node_st head;
} LLIST; //LLIST就是一个双向链表的头节点类型，对于链表的操作都是用head来进行的

//传入 每个数据节点的数据类型大小
LLIST *llist_careate(int size);
//传入 一个已经创好的链表的头节点，插入的数据，插入的模式
int llist_insert(LLIST *,const void *data,int mode);
//传入
void *llist_find(LLIST *head,const void* ,int (*func)(const void*,const void*));
//
int llist_delete(LLIST *head,const void* ,int (*func)(const void*,const void*));
//
int llist_fetch(LLIST *head,const void* ,int (*func)(const void*,const void*),void *);
//传入 一个已经创建好的链表头节点
void llist_travel(LLIST* head,void (*func)(const void*));
void llist_destroy(LLIST *);

#endif
~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llist.h"


//传入 每个数据节点的数据类型大小
LLIST *llist_careate(int size){
    LLIST *new;
    new = malloc(sizeof(*new));
    if (new == NULL){
        return NULL;
    }

    new->size = size;
    new->head.data = NULL;
    new->head.prev = &new->head;
    new->head.next = &new->head;
    
    return new;
}
//传入 一个已经创好的链表的头节点，插入的数据，插入的模式
int llist_insert(LLIST *head,const void *data,int mode){
    struct llist_node_st *newnode;
    newnode = malloc(sizeof(*newnode));
    if (newnode == NULL)
      return -1;

    newnode->data = malloc(head->size);
    if (newnode->data == NULL){
        return -2;
    }
    memcpy(newnode->data,data,head->size);

    switch (mode) {
        case F:
            newnode->prev = &head->head;
            newnode->next = head->head.next;
            break;
        case B:
            newnode->prev = head->head.prev;
            newnode->next = &head->head;
            break;
        default:
            return -3;
    }

    newnode->prev->next = newnode;
    newnode->next->prev = newnode;

    return 0;

}
//传入 一个已经创建好的链表头节点,一个辅助遍历函数
void llist_travel(LLIST* head,void (*func)(const void *)){
    struct llist_node_st *cur,*next;
    for (cur = head->head.next;cur != &head->head;cur = next) {
        func(cur->data);
        next = cur->next;
    }
}

//辅助函数
static struct llist_node_st *find_(LLIST *head,const void *key,int (*func)(const void *,const void *)){
    struct llist_node_st *cur;
    for (cur = head->head.next;cur != &head->head;cur = cur->next){
        if (func(key,cur->data) == 0){
            return cur;
        }
    }
    return &head->head;
}

void *llist_find(LLIST *head,const void* key,int (*func)(const void*,const void*)){
    return find_(head,key,func)->data;
##### 静态库
}

//
int llist_delete(LLIST *head,const void* key,int (*func)(const void*,const void*)){
    struct llist_node_st *node;
    node = find_(head,key,func);
    if (node == &head->head){
        return -1;
    }else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        free(node->data);
        free(node);
        return 0;
    }
}
//
int llist_fetch(LLIST *head,const void* key,int (*func)(const void*,const void*),void *data){
    struct llist_node_st *node;
    node = find_(head,key,func);
    if (node == &head->head){
        return -1;
    }else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        data = node->data;
        free(node->data);
        free(node);
        return 0;
    }
}

void llist_destroy(LLIST *head) {
    struct llist_node_st *cur,*next;

    for (cur = head->head.next;cur != &head->head;cur = next) {
        next = cur->next;
        free(cur->data);
        free(cur);
    }
    free(head);
}
~~~

~~~ c
CFLAGS		+=-Wall -g -lstdc++ -D_FILE_OFFSET_BITS=64
CC			=gcc
TARGET		=DoubleLinkList
OBJ			=llist.o
src  		=llist.c

$(TARGET):$(OBJ)
	$(CC) main.c $(OBJ) -o $@

$(OBJ):$(src)
	$(CC) $(src) $(CFLAGS) -c -o $@
	
clean:
	-rm -f $(OBJ)
	-rm -f $(TARGET)
~~~

##### 静态库
- libxx.a xx指代库名
- `ar -cr libxx.a yyy.o`
- 发布到 `/usr/local/include` `/usr/local/lib`
- 编译 `gcc -L/usr/local/lib -o main mian.o -lxx` **`-l`参数必须在最后，有依赖**

~~~ bash
make
ar -cr libllist.a llist.o 
 gcc -L./ -o main main.c -lllist 
~~~
##### 动态库
- `libxx.so` xx是库名
- `gcc -shared -fpic -o libxx.so yyy.c`
- 发布到 `/usr/local/include` `/usr/local/lib` (.h 与 .so)
- /sbin/ldconfig 重读 `/etc/ld.so.conf`
- `gcc -I/usr/local/include -L/usr/local/lib -o ... lxx`

**重名用动态库**
- 非root用户发布
    - sp xx.so ~/lib
    - export LD_LIBRARY_PATH=~/lib

### 函数跳转
    适用场景： 在树结构中查找元素，找到后直接回到第一次调用处(跨函数),不用一层一层返回
- setjmp()
- longjmp()

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

static jmp_buf save;

static void d(){
    printf("%s is called\n",__FUNCTION__);
    longjmp(save,2);
    printf("%s is returned\n",__FUNCTION__);
}


static void c(){
    printf("%s is called\n",__FUNCTION__);
    d();
    printf("%s is returned\n",__FUNCTION__);
}


static void b(){
    printf("%s is called\n",__FUNCTION__);
    c();
    printf("%s is returned\n",__FUNCTION__);
}

static void a(){
    int ret = setjmp(save);
    if  (ret == 0) {
        printf("%s is called\n",__FUNCTION__);
        b();
        printf("%s is returned\n",__FUNCTION__);
    }else {
        printf("code %d return \n",ret);
    }
}

int main()
{
    a();
    return 0;
}

~~~

### 资源的获取与控制
- `getrlimit`
- `setrlimit`

@[TOC](文章目录)
<hr>

# 信号

## 信号的概念
**信号是软件中断，横多比较重要的应用程序都需要处理信号，信号提供了一种处理异步时间的方法**
**信号的响应依赖于中断**
- kill -l
## signal()
- `void (*signal(int signum,void (*func)(int)))(int)`

~~~ c
void test(int sig){};

void (*signal(int sig,test))(int){};
//void (*res)(int) signal(int sig,test){};

~~~


**信号会打断阻塞的系统调用**

## 信号的不可靠
**信号的行为不可靠**

## 可重入函数
**解决信号的不可靠问题 第一次调用没结束 第二次调用开始了 **
**第一次调用没结束 发生第二次调用 但不会出错**
**所有的系统调用都是可重入的 一部分库函数 **
## 信号的响应过程(重要)
**信号从收到到响应有不可避免的延时**
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210213182415635.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQ0ODA3Nzk2,size_16,color_FFFFFF,t_70#pic_center)



## 信号的常用函数
- kill() **不是用来杀死进程的，是用来发送信号的，只不过大多数信号有杀死进程的作用**

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int main()
{
    //int kill(pid_t pid, int sig)
    //pid > 0 发送给指定进程
    //pid == 0 发送给同组所有进程 即 组内广播
    //pid == -1 全局广播 init专用
    //pid < -1 发送给 ppid == |pid|的所有进程
    //当sig == 0时用于检测一个进程是否存在


    exit(0);
}
~~~

- raise() 等价与`kill(getpid(), sig);`
- **alarm()**
- pause 人为制造的阻塞系统调用 等待一个信号来打断它

~~~ c
//5sec_sig.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

//使用 信号 计算当前单核 累加 的速度

static int loop = 1;

static void handler(int sig){
    loop = 0;
}

int main()
{
    int64_t count = 0;

    alarm(5);
    signal(SIGALRM,handler);

    while(loop){
        count++;
    }

    printf("count %ld",count);

    exit(0);
}

~~~

~~~ bash
gcc 5sec_sig.c -S
~~~

~~~ x86asm
main:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	$0, -8(%rbp)
	movl	$5, %edihttps://github.com/impact-eintr/LinuxC
学习记录的笔记与代码，持续更新中
希望各位大佬给个star
	call	alarm@PLT
	leaq	handler(%rip), %rsi
	movl	$14, %edi
	call	signal@PLT
	jmp	.L3
.L4:
	addq	$1, -8(%rbp)
.L3:
	movl	loop(%rip), %eax ;执行循环
	testl	%eax, %eax
	jne	.L4
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, %edi
	call	exit@PLT
	.cfi_endproc
~~~


#### volatile 关键字
**去到这个变量真正的存储空间取数值，而不是根据局部结构判断取值**

~~~ bash
gcc 5sec_sig.c -O1
./a.out         //不会有返回的 进入死循环

~~~

注意注释
~~~ x86asm
main:
.LFB23:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$5, %edi
	call	alarm@PLT
	leaq	handler(%rip), %rsi
	movl	$14, %edi
	call	signal@PLT
	cmpl	$0, loop(%rip)
	je	.L3
.L4:
	jmp	.L4
.L3:
	movl	$0, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, %edi
	call	exit@PLT
	.cfi_endproc
.LFE23:
	.size	main, .-main
	.data
	.align 4
	.type	loop, @object
	.size	loop, 4
loop:
	.long	1                 ;将loop的值固定为1
	.ident	"GCC: (GNU) 10.2.0"
	.section	.note.GNU-stack,"",@progbits
~~~

原因在于这一句
~~~ c
//loop在局部循环中没有改变值 gcc 认为loop的值不会发生改变 永远为 1
while(loop){
    count++;
}
~~~

解决办法就是
~~~ c
static volatile int loop = 1;
~~~


#### 令牌桶(优化后的流控算法)
~~~ c
#ifndef MYTBF_H__
#define MYTBF_H__

#define MYTBF_MAX 1024

typedef void mytbf_t;

mytbf_t *mytbf_init(int cps,int burst);

//获取token
int mytbf_fetchtoken(mytbf_t *,int);
//归还token
int mytbf_returntoken(mytbf_t *,int);

int mytbf_destroy(mytbf_t *);

#endif
~~~

~~~ c
#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "mytbf.h"

struct mytbf_st{
    int csp;
    int burst;
    int token;
    int pos;//任务列表的下标
};

static struct mytbf_st *job[MYTBF_MAX];
static volatile int inited = 0;
static void (*alarm_status)(int);

static int get_free_pos(){
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] == NULL)
          return  i;
    }
    return -1;
}


//信号处理函数
static void handler(int sig){
    alarm(1);
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] != NULL){
            job[i]->token += job[i]->csp;
            if (job[i]->token > job[i]->burst){
                job[i]->token = job[i]->burst;
            }
        }
    }
}

//装载信号处理模块
static void mod_load(){
    alarm_status = signal(SIGALRM,handler);//保存alarm信号处理函数原来的状态
    alarm(1);
}
//卸载信号处理模块 当发生异常退出时 可以将占用的资源释放 将alarm信号取消
static void mod_unload(){
    signal(SIGALRM,alarm_status);
    alarm(0);
    for (int i = 0;i < MYTBF_MAX;i++){
        free(job[i]);
    }
}

mytbf_t *mytbf_init(int cps,int burst){
    struct mytbf_st *tbf;

    if (!inited){
        mod_load();
    }

    //将新的tbf装载到任务组中
    int pos;
    pos = get_free_pos();
    if (pos == -1){
        return NULL;
    }

    tbf = malloc(sizeof(*tbf));
    if (tbf == NULL)
        return NULL;
    tbf->token = 0;
    tbf->csp = cps;
    tbf->burst = burst;
    tbf->pos = pos;
    
    job[pos] = tbf;

    return tbf;
}

//获取token ptr是一个 void * size是用户想要获取的token数
int mytbf_fetchtoken(mytbf_t *ptr,int size){
    struct mytbf_st *tbf = ptr;

    if (size <= 0){
        return -EINVAL;
    }
    
    //有token继续
    while (tbf->token <= 0)
      pause();
    
    int n =tbf->token<size?tbf->token:size;

    tbf->token -= n;
    //用户获取了 n 个token
    return n;
}

//归还token ptr是一个 void *
int mytbf_returntoken(mytbf_t *ptr,int size){
    struct mytbf_st *tbf = ptr;

    if (size <= 0){
        return -EINVAL;
    }
    
    tbf->token += size;
    if (tbf->token > tbf->burst)
        tbf->token = tbf->burst;

    return size;
}

int mytbf_destroy(mytbf_t *ptr){
    struct mytbf_st *tbf = ptr;
    job[tbf->pos] = NULL;
    free(tbf);
    return 0;
}
~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include "mytbf.h"

static const int SIZE = 1024;
static const int CPS = 3;
static const int BURST = 100;//最大令牌数

static volatile int token = 0;//持有令牌数

int main(int argc,char** argv)
{
    if (argc < 2){
        fprintf(stdout,"Usage...");
        exit(1);
    }

    mytbf_t *tbf;

    tbf = mytbf_init(CPS,BURST);
    if (tbf == NULL){
        fprintf(stderr,"tbf init error");
        exit(1);
    }

    //打开文件
    int sfd,dfd = 0;
    do{
        sfd = open(argv[1],O_RDONLY);
        if (sfd < 0){
            if (errno == EINTR)
              continue;
            fprintf(stderr,"%s\n",strerror(errno));
            exit(1);
        }
    }while(sfd < 0);

    char buf[SIZE];
    
    while(1){
        
        int len,ret,pos = 0;
        int size = mytbf_fetchtoken(tbf,SIZE);
        
        int i = 0;
        while(i < 2){
            sleep(1);
            i++;
        }

        if (size < 0){
            fprintf(stderr,"mytbf_fetchtoken()%s\n",strerror(-size));
            exit(1);
        }

        len = read(sfd,buf,size);
        while (len < 0){
            if (errno == EINTR)
              continue;
            strerror(errno);
            break;
        }

        //读取结束
        if (len == 0){
            break;
        }

        //要是读到结尾没用完token
        if (size - len > 0){
            mytbf_returntoken(tbf,size-len);
        }

        //以防写入不足
        while(len > 0){
            ret = write(dfd,buf+pos,len);
            while (ret < 0){
                if (errno == EINTR){
                  continue;
                }
                printf("%s\n",strerror(errno));
                exit(1);
            }

            pos += ret;
            len -= ret;
        }
    }

    close(sfd);
    mytbf_destroy(tbf);

    exit(0);
}

~~~

- setitimer() 更灵活 **而且误差不累积**
~~~ c
//信号处理函数
static void handler(int sig){
    struct itimerval itv;

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL,&itv,NULL) < 0){
        perror("setitimer()");
        exit(1);
    }
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] != NULL){
            job[i]->token += job[i]->csp;
            if (job[i]->token > job[i]->burst){
                job[i]->token = job[i]->burst;
            }
        }
    }
}

//装载信号处理模块
static void mod_load(){
    alarm_status = signal(SIGALRM,handler);//保存alarm信号处理函数原来的状态

    struct itimerval itv;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL,&itv,&old_itv) < 0){
        perror("setitimer()");
        exit(1);
    }
}
//卸载信号处理模块 当发生异常退出时 可以将占用的资源释放 将alarm信号取消
static void mod_unload(){
    signal(SIGALRM,alarm_status);
    setitimer(ITIMER_REAL,&old_itv,NULL);

    for (int i = 0;i < MYTBF_MAX;i++){
        free(job[i]);
    }
}

~~~
- abort
- system()

	- **在有信号参与的程序当中，要阻塞住一个信号，要忽略调两个信号 这样system()才能正常使用**

#### sleep的缺陷

**在某些平台，`sleep()`是使用`alarm`+`pause`封装的，而程序中出现多于1个的`alarm`alarm将失效**
## 信号集
- sigset_t 信号集类型
- sigemptyset() 	将一个信号集置为空集
- sigfillset() 	将一个信号集置为全集
- sigaddset() 	将一个信号加入信号集
- sigdelset()	 将一个信号移除信号集
- sigismember()

### 信号屏蔽字/pending集的处理

**我们无法控制信号何时到来，但可以选择如何响应它**

- sigprocmask(int how,const sigset_t *set,sigset_t *oldset)
- how的取值
	-  SIG_BLOCK make全0
	-  SIG_UNBLOCK mask全1
	-  SIG_SETMASK
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define N 5

void handler(int sig){
    write(1,"S",1);
}


int main()
{
    int i;

    sigset_t sigset,old_sigset,sigset_status;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGINT);

    signal(SIGINT,handler);
    
    //保存进入该模块前的状态
    sigprocmask(SIG_UNBLOCK,&sigset_status,NULL);
    while(1){
         //屏蔽对信号的响应
        sigprocmask(SIG_BLOCK,&sigset,&old_sigset);
        for (i = 0;i < N;i++){
            write(1,"*",1);
            sleep(1);
        }
        write(1,"\n",1);
        //恢复对信号的响应
        sigprocmask(SIG_SETMASK,&old_sigset,NULL);
    }
    //恢复进入该模块前的状态
    sigprocmask(SIG_SETMASK,&sigset_status,NULL);

    exit(0);
}
~~~
ctrl+\ 退出

## 扩展
- sigsuspend() 信号驱动
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define N 5

static void handler(int sig){
    write(1,"S",1);
}

int main()
{
    int i;

    sigset_t sigset,old_sigset,sigset_status;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGINT);

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,NULL);

    //保存进入该模块前的状态
    sigprocmask(SIG_UNBLOCK,&sigset_status,NULL);
    sigprocmask(SIG_BLOCK,&sigset,&old_sigset);
    while(1){
        for (i = 0;i < N;i++){
            write(1,"*",1);
            sleep(1);
        }
        write(1,"\n",1);
        sigsuspend(&old_sigset);
    }
    //恢复进入该模块前的状态
    sigprocmask(SIG_SETMASK,&sigset_status,NULL);

    exit(0);
}
~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define N 5

void handler(int sig){
    write(1,"S",1);
}


int main()
{
    int i;

    sigset_t sigset,old_sigset,sigset_status;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGINT);

    signal(SIGINT,handler);
    
    //保存进入该模块前的状态
    sigprocmask(SIG_UNBLOCK,&sigset_status,NULL);
    sigprocmask(SIG_BLOCK,&sigset,&old_sigset);
    while(1){
        for (i = 0;i < N;i++){
            write(1,"*",1);
            sleep(1);
        }
        write(1,"\n",1);
        sigsuspend(&old_sigset);
    }
    //恢复进入该模块前的状态
    sigprocmask(SIG_SETMASK,&sigset_status,NULL);

    exit(0);
}
~~~

- sigaction() 用来替换signal() 还可以指定信号的来源以选择是否响应
~~~ c
//信号处理函数
static void handler(int sig,siginfo_t *infop,void *unused){
    struct itimerval itv;

    if (infop->si_code != SI_KERNEL){
        return ;
    }

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL,&itv,NULL) < 0){
        perror("setitimer()");
        exit(1);
    }
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] != NULL){
            job[i]->token += job[i]->csp;
            if (job[i]->token > job[i]->burst){
                job[i]->token = job[i]->burst;
            }
        }
    }
}

//装载信号处理模块
static void mod_load(){
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGALRM,&sa,&old_sa) < 0){
        perror("sigaction()");
        exit(1);
    }


    struct itimerval itv;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL,&itv,&old_itv) < 0){
        perror("setitimer()");
        exit(1);
    }
}
//卸载信号处理模块 当发生异常退出时 可以将占用的资源释放 将alarm信号取消
static void mod_unload(){
   //signal(SIGALRM,alarm_status);
   sigaction(SIGALRM,&old_sa,NULL);
   
   //恢复时钟信号状态
    struct itimerval itv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL,&itv,&old_itv) < 0){
        perror("setitimer()");
        exit(1);
    }

    for (int i = 0;i < MYTBF_MAX;i++){
        free(job[i]);
    }
}
~~~

## 实时信号
不会丢失 有顺序
# 进程


### PID
类型 pid_t
- ps axf 查看进程树
- ps axm
- ps ax -L
- ps -ef
### 进程的产生
- fork() 
        - 注意理解关键字 duplicating 意味着拷贝 克隆 一模一样
        - fork 后父子进程的区别 ： fork 的返回值不一样 pid不同 ppid也不同 未决信号与文🔓 不继承资源利用量清0
        - init进程 是所以进程的祖先进程 pid == 1
        - 调度器的调度策略来决定哪个进程先运行
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    printf("%d start !\n",getpid());
    fflush(NULL);//记得刷新 否则begin放到缓冲区 父子进程的缓冲区里各有一句begin

    pid_t pid = fork();

    if (pid == 0){
        printf("child %d\n",getpid());
    }else{
        printf("parent %d\n",getpid());
    }
    getchar();
    printf("pid %d end\n",getpid());
    return 0;
}

~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define LEFT 2
#define RIGHT 200

int main()
{
    pid_t pid = 0;
    int i,j,mark;

    for (i = LEFT;i <= RIGHT;i++){
        pid = fork();
        if (pid == 0){
            mark = 1;
            for (j = 2;j < i/2;j++){
                if (i%j == 0){
                    mark = 0;
                    break;
                }
            }
            if (mark) {
                printf("%d is a primer\n",i);
            }
            exit(0);
        }
    }
    getchar();

    exit(0);
}

~~~

- vfork()

### 进程的消亡以及释放资源
- wait()
- waitpid()
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define N 3
#define LEFT 100000002
#define RIGHT 100000200

//交叉算法计算 池类算法涉及到竞争
int main()
{
    printf("[%d] start !\n",getpid());
    fflush(NULL);//记得刷新 否则begin放到缓冲区 父子进程的缓冲区里各有一句begin
    pid_t pid = 0;
    int i,j,mark;

    for (int n = 0;n < N;n++){
        pid = fork();
        if (pid < 0){
            perror("fork");
            for (int k = 0;k < n;k++){
                wait(NULL);
            }
            exit(1);
        }

        if (pid == 0){
            for (i = LEFT+n;i <= RIGHT;i+=N){
                mark = 1;
                for (j = 2;j <= i/2;j++){
                    if (i%j == 0){
                        mark = 0;
                        break;
                    }
                }
                if (mark) {
                    printf("%d is a primer\n",i);
                }
            }
            printf("[%d] exit\n",n);
            exit(0);
        }

    }

    int st,n;
    for (n =0 ;n < N;n++){
        wait(&st);
        printf("%d end\n",st);
    }

    exit(0);
}

~~~

### exec函数族
**exec 替换 当前进程映像**
- `extern char **environ`
- execl
- execlp
- execle
- execv
- execvpa
###### 一个小shell
~~~ c
//lhq yyds
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <glob.h>
#include <string.h>

#define BUFSIZE 1024
#define DELIMS " \t\n"

extern char **environ;

static int cd(char *path){
    int ret = chdir(path);
    if (ret == -1){
        perror("chdir");
    }
    return ret;
}

static void readrc(char *name){
    FILE *fp;
    fp = fopen(name,"r+");
    //处理文件内容
    fclose(fp);
}

static void prompt()
{
    char pwd[BUFSIZE];
    char name[BUFSIZE];
    getcwd(pwd,BUFSIZE);
    getlogin_r(name,BUFSIZE);
    printf("%s %s $ ",name,pwd);
}

static int parse(char *linebuf,glob_t *globres){
    char *tok;
    int flag = 0;
    

    while (1){
        tok = strsep(&linebuf,DELIMS);
        if (tok == NULL){
            break;
            return -1;
        }else if(strcmp(tok,"cd") == 0){
            char *path = strsep(&linebuf,DELIMS);
            return cd(path);
        }else if(tok[0] == '\0'){
            continue;
        }

        glob(tok,GLOB_NOCHECK|GLOB_APPEND*flag,NULL,globres);//第一次不能append glob_argv中是随机值 GLOB_NOCHECK | (GLOB_APPEND*flag)==0 第一次不append
        flag = 1;
    }
    return 1;
}


//之后记得 将 ctrl+c 转为 stdout:\n 将ctrl+d 转为 退出+再见标语
int main()
{
    printf("This is YSHELL\n");

    pid_t pid;
    char *linebuf = NULL;
    size_t lienbuf_size = 0;
    glob_t globres;//解析命令行

    //读取配置文件
    char *yshrc = "/home/yixingwei/.yshrc";//填一个绝对路径
    readrc(yshrc);

    while(1){
        prompt();


        //获取命令
        getline(&linebuf,&lienbuf_size,stdin);
        //解析命令
        int ret = parse(linebuf,&globres);
        
        if (ret == -1){
            
        }else if (ret == 0){//内部命令
            
        }else if (ret == 1){//外部命令
            fflush(NULL);
            pid = fork();
            if (pid < 0){
                perror("fork()");
                exit(1);
            }else if(pid == 0){
                execvp(globres.gl_pathv[0],globres.gl_pathv);
                perror("execl()");
                exit(1);
            }
        }
        waitpid(pid,NULL,0);
    } 
    
    exit(0);
}

~~~

### 用户权限以及组权限
- u+s 当其他用户调用该可执行文件时，会切换成当前可执行文件的user的身份来执行
- g+s

- uid/gid
    - r real
    - e effective

函数
- getuid 返回 real
- geteuid 返回 effective
~~~ c
// mysudo
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main(int argc,char **argv)
{
    if (argc < 3){
        fprintf(stderr,"Useage");
        exit(1);
    }

    pid_t pid;

    pid = fork();
    if (pid == 0){
        setuid(atoi(argv[1]));
        execvp(argv[2],argv+2);
        perror("execvp()");
        exit(1);

    }else {
        wait(NULL);
    }

    exit(0);
}

~~~

~~~ bash
$ su
# chown root ./mysudo
# chmod u+s ./mysudo

$ ./mysudo 0 /etc/shadow
~~~
- getgid
- getegid
- setuid 设置effective
- setgid 设置effective
- setreuid 交换 r e //是原子的交换
- setregid 交换 r e

### system()
可以理解成 fork+exec+wait 封装

### 进程会计
- acct()

### 进程时间
- times()

### 守护进程
- sesion 标识是`sid`
- 终端
- setsid()
- getpgrp()
- getpgid()
- setpgid()


### 系统日志
- syslogd 服务
- openlog
- syslog
- closelog

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#define FNAME "/tmp/out"

static int deamonize(){
    int fd;
    pid_t pid;
    pid = fork();

    if (pid < 0){
        return -1;
    }

    if (pid > 0){
        exit(0);
    }

    fd = open("/dev/null",O_RDWR);//输出都忽略
    if (fd < 0){
        return -1;
    }
    if (pid == 0){
        printf("test");
        fflush(NULL);
        dup2(fd,0);
        dup2(fd,1);
        dup2(fd,2);
        if (fd > 2){
            close(fd);
        }
        setsid();//脱离终端
        //umask();
        chdir("/");
    }
    return 0;
}

int main()
{
    FILE* fp;

    //开启日志服务
    openlog("print i",LOG_PID,LOG_DAEMON);

    if (deamonize()){
        syslog(LOG_ERR,"init failed!");
    }else{
        syslog(LOG_INFO,"successded!");
    }

    fp = fopen(FNAME,"w+");
    if (fp == NULL){
        syslog(LOG_ERR,"write file failed!");
        exit(1);
    }

    syslog(LOG_INFO,"%s opened",FNAME);

    for(int i = 0; ;i++){
        fprintf(fp,"%d\n",i);
        fflush(NULL);
        syslog(LOG_DEBUG,"%d 写入",i);
        sleep(1);
    }

    closelog();
    fclose(fp);
    exit(0);
}

~~~

~~~ bash
 journalctl _PID=XXX

~~~

@[TOC](文章目录)
# 线程

## 线程的概念
会话是用来承载进程组的，里面可以有一个或多个进程，一个线程中可以有一个或多个线程
**线程的本质就是一个正在运行的函数 ，线程没有主次之分(main函数 也只是一个main线程)，多个线程之间共享内存，**

 **`posix`线程是一套标准，而不是实现，我们主要讨论这套标准**
- 线程标识 `pthead_t` 类型不确定
- pthread_equal()
- pthread_self()

## 线程的一生
### 线程的创建
- pthread_create()

~~~ c#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

static void *func(void *p){
    puts("thread is working");
    return NULL;
}

int main()
{
    printf("Begin\n");
    pthread_t ptid;

    int err = pthread_create(&ptid,NULL,func,NULL);
    if (err){
        fprintf(stderr,"pthread_create err %s\n",strerror(err));
        exit(1);
    }
    sleep(1);
    printf("End\n");
    exit(0);
}


~~~
**线程的调度取决于调度器的测略**

### 线程的终止
1. **线程从启动例程返回，返回值就是线程的退出码**
2. **线程可以被同一进程的其他线程取消**
3. **线程调用`pthread_exit()`函数**

- pthread_exit();

~~~ c
static void *func(void *p){
    puts("thread is working");
    pthread_exit(NULL);
}

int main()
{
    printf("Begin\n");
    pthread_t ptid;

    int err = pthread_create(&ptid,NULL,func,NULL);
    if (err){
        fprintf(stderr,"pthread_create err %s\n",strerror(err));
        exit(1);
    }
    sleep(1);
    printf("End\n");
    exit(0);
}


~~~

- pthread_join() 先当于进程的 wait()

~~~ c
static void *func(void *p){
    puts("thread is working");
    pthread_exit(NULL);
}

int main()
{
    printf("Begin\n");
    pthread_t ptid;

    int err = pthread_create(&ptid,NULL,func,NULL);
    if (err){
        fprintf(stderr,"pthread_create err %s\n",strerror(err));
        exit(1);
    }
    
    pthread_join(ptid,NULL);
    printf("End\n");
    exit(0);
}
~~~

### 线程/栈的清理
- pthread_cleanup_push()
- pthread_cleanup_pop()

**类似钩子函数，程序只要正常终止，钩子函数就会被逆序调用，push 与 pop 可以指定操作**

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static void cleanup(void *p){
    puts(p);
}

static void *func(void *p){
    puts("Thread is working");

    pthread_cleanup_push(cleanup,"1");//这个函数是宏
    pthread_cleanup_push(cleanup,"2");
    pthread_cleanup_push(cleanup,"3");
    
    //pthread_exit(NULL);
    //下面的内容执行不到但是不会报错 会按照全为 1 处理

    pthread_cleanup_pop(1);//语法结构一定要对等
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(1);

    puts("push over");

    pthread_exit(NULL);
}

int main()
{
    pthread_t ptid;
    printf("Start !\n");

    int err = pthread_create(&ptid,NULL,func,NULL);//ptid 属性 处理函数 传参
    if (err){
        fprintf(stderr,"pthread_create() : %s\n",strerror(err));
        exit(1);
    }

    pthread_join(ptid,NULL);

    printf("End !\n");
    exit(0);
}

~~~

### 线程的取消选项
- 多线程任务 有时需要取消部分任务(线程)
- pthread_cancel()
- 取消有2种状态 
    - 不允许
    - 允许
        - 异步cancel
        - **推迟cancel(默认) 推迟到cancel点再响应**
        - **cancel点 ： POSIX定义的cancel点 都是可能引发阻塞的系统调用**
~~~ c
//示意

static void cleanup(void *){
    close(*p);
}

//cancel点
fd1 = open();
if (fd1 < 0){
    perror();
    exit(1);//只要调用exit 不管是 ezit(1) 还是 exit(0) 都是正常终止 都会执行线程清理函数
}
//在这收到一个取消请求 但因为下面的代码没有阻塞的系统调用 所以不会响应
pthread_cleanup_push(cleanup,&fd);
pthread)cleanup_pop(1);


//cancel点 下面的open是阻塞的系统调用 对上面的取消请求做出响应

fd2 = open();
if (fd2 < 0){
    perror();
    exit(1);
}
~~~

- pthread_setcancelstate() 设置是否允许取消
- pthread_testcancel() 什么都不做 本身就是一个取消点

- 进程异常终止的条件之一
    - **最后一个线程对其取消请求作出响应**

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>

#define N 5
#define LEFT 30000000
#define RIGHT 30000200


static void *handler(void *p){
    int n = *(int *)p;
    int i,j,mark;
    for (i = LEFT+n;i <= RIGHT;i+=N){
        mark = 1;
        for (j = 2;j <= i/2;j++){
            if (i%j == 0){
                mark = 0;
                break;
            }
        }
        if (mark) {
            printf("%d is a primer [%d]\n",i,n);
        }
    }
    pthread_exit(p);

}

//交叉算法计算 池类算法涉及到竞争
int main()
{
    pthread_t Ptid[N];
    void *ptr = NULL;

    for (int n = 0;n < N;n++){
        int *num = malloc(sizeof(int));
        *num = n;
        int err = pthread_create(Ptid+n,NULL,handler,num);
        if (err){
            fprintf(stderr,"%s\n",strerror(err));
            exit(1);
        }
    }

    int n;
    for (n =0 ;n < N;n++){
        pthread_join(Ptid[n],ptr);
        free(ptr);
    }

    exit(0);
}

~~~

## 线程同步
#### 互斥量

**锁住的是一段代码而不是一个变量**
- pthread_mutex_t
- pthread_mutex_init()
- pthread_mutex_destory()
- pthread_mutex_lock()
- pthread_mutex_trylock()
- pthread_mutex_unlock()
- pthread_once() **动态模块的单词初始化函数**

互斥量像是`bool`，非黒即白，没有共享性

~~~ c
//互斥量
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *handler(void *p){
    FILE *fp = fopen(FNAME,"r+");
    char buf[BUFSIZE];

    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }

    //进入临界区
    pthread_mutex_lock(&mutex);
    fgets(buf,BUFSIZE,fp);
    fseek(fp,0,SEEK_SET);
    sleep(1);
    fprintf(fp,"%d\n",atoi(buf)+1);
    fclose(fp);
    pthread_mutex_unlock(&mutex);
    //离开临界区

    pthread_exit(NULL);
}

int main()
{
    pthread_t Ptid[THRNUM];

    for (int i = 0;i < THRNUM;i++){
        int err = pthread_create(Ptid+i,NULL,handler,NULL);
        if (err){
           fprintf(stderr,"%s\n",strerror(err));
           exit(1);
        }
    }

    for (int i = 0;i < THRNUM;i++){
        pthread_join(Ptid[i],NULL);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}

~~~

~~~ c
//循环打印 abcd
#define N 4
#define LEFT 30000000
#define RIGHT 30000200

static pthread_mutex_t mutex_arr[N];


static void *handler(void *p){
    int n = *(int *)p;
    int ch = '1'+n;

    while(1){
        pthread_mutex_lock(mutex_arr+n);
        write(1,&ch,1);
        pthread_mutex_unlock(mutex_arr+(n+1)%N);
    }
    pthread_exit(p);
}

int main()
{
    pthread_t Ptid[N];
    void *ptr = NULL;

    for (int n = 0;n < N;n++){
        int *num = malloc(sizeof(int));
        *num = n;

        pthread_mutex_init(mutex_arr+n,NULL);
        pthread_mutex_lock(mutex_arr+n);

        int err = pthread_create(Ptid+n,NULL,handler,num);
        if (err){
            fprintf(stderr,"%s\n",strerror(err));
            exit(1);
        }
    }

    pthread_mutex_unlock(mutex_arr+0);
    alarm(3) ;   

    int n;
    for (n =0 ;n < N;n++){
        pthread_join(Ptid[n],ptr);
        free(ptr);
    }

    exit(0);
}

~~~

~~~ c
//池类算法算质数
#define THRNUM 3
#define LEFT 30000000
#define RIGHT 30000200

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
static int num = 0;

static void *handler(void *p){
    int task,mark;

    while(1){
        pthread_mutex_lock(&mutex);
        while(num == 0){
            pthread_mutex_unlock(&mutex);
            sched_yield();
            pthread_mutex_lock(&mutex);
        }
    
        if (num == -1){
            pthread_mutex_unlock(&mutex);
            break;
        }

        task = num;
        num = 0;//成功领取任务
        pthread_mutex_unlock(&mutex);

        mark = 1;
        for (int j = 2;j <= task/2;j++){
            if (task%j == 0){
                mark = 0;
                break;
            }
        }
        if (mark) {
            printf("[%d] %d is a priamer\n",(int)p,task);
        }
    }

    pthread_exit(NULL);
}

//池类算法
int main()
{
    pthread_t Ptid[THRNUM];

    for (int n = 0;n < THRNUM;n++){
        int err = pthread_create(Ptid+n,NULL,handler,(void *)n);
        if (err){
            fprintf(stderr,"%s\n",strerror(err));
            exit(1);
        }
    }

    for (int i = LEFT;i <= RIGHT;i++){
        pthread_mutex_lock(&mutex);
        
        //任务没有被领取
        while(num != 0){
            pthread_mutex_unlock(&mutex);
            sched_yield();
            pthread_mutex_lock(&mutex);
        }
        //任务已经成功下发
        num = i;
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_lock(&mutex);
    //任务没有被领取
    while(num != 0){
        pthread_mutex_unlock(&mutex);
        sched_yield();
        pthread_mutex_lock(&mutex);
    }
    //任务已经成功下发
    num = -1;
    pthread_mutex_unlock(&mutex);

    int n;
    for (n =0 ;n < THRNUM;n++){
        pthread_join(Ptid[n],NULL);
    }

    pthread_mutex_destroy(&mutex);

    exit(0);
}

~~~
**头文件 与 测试主函数 请见信号篇的 令牌桶**
~~~ c
//令牌桶
struct mytbf_st{
    int csp;
    int burst;
    int token;
    int pos;//任务列表的下标
    pthread_mutex_t mut;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t ptid;
static pthread_once_t pth_once = PTHREAD_ONCE_INIT;

static struct mytbf_st *job[MYTBF_MAX];
static volatile int inited = 0;

static int get_free_pos_unlocked(){
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] == NULL)
          return  i;
    }
    return -1;
}

//线程处理函数
static void *handler(void *p){
    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    while(1){
        pthread_mutex_lock(&mutex);
        for (int i = 0;i < MYTBF_MAX;i++){
            if (job[i] != NULL){
                pthread_mutex_lock(&job[i]->mut);
                job[i]->token += job[i]->csp;
                if (job[i]->token > job[i]->burst){
                    job[i]->token = job[i]->burst;
                }
                pthread_mutex_unlock(&job[i]->mut);
            }
        }
        pthread_mutex_unlock(&mutex);
        nanosleep(&ts,NULL);

    }
    pthread_exit(NULL);
}

//卸载线程处理模块
static void mod_unload(){
    pthread_cancel(ptid);
    pthread_join(ptid,NULL);
    for (int i = 0;i < MYTBF_MAX;i++){
        if (job[i] != NULL){
            mytbf_destroy(job[i]);
        }
        free(job[i]);
    }

    pthread_mutex_destroy(&mutex);
}

//装载线程处理模块
static void mod_load(){

    int err = pthread_create(&ptid,NULL,handler,NULL);
    if (err){
        fprintf(stderr,"%s\n",strerror(err));
    }

    atexit(mod_unload);
}

mytbf_t *mytbf_init(int cps,int burst){
    struct mytbf_st *tbf;

    pthread_once(&pth_once,mod_load);

    tbf = malloc(sizeof(*tbf));
    if (tbf == NULL){
        return NULL;
    }
    tbf->token = 0;
    tbf->csp = cps;
    tbf->burst = burst;
    pthread_mutex_init(&tbf->mut,NULL);

    pthread_mutex_lock(&mutex);
    //将新的tbf装载到任务组中
    int pos = get_free_pos_unlocked();
    if (pos == -1){
        free(tbf);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    tbf->pos = pos;
    job[pos] = tbf;
    
    pthread_mutex_unlock(&mutex);

    return tbf;
}

//获取token ptr是一个 void * size是用户想要获取的token数
int mytbf_fetchtoken(mytbf_t *ptr,int size){
    struct mytbf_st *tbf = ptr;

    if (size <= 0){
        return -EINVAL;
    }
    
    //有token继续
    pthread_mutex_lock(&tbf->mut);
    while (tbf->token <= 0){
        pthread_mutex_unlock(&tbf->mut);
        sched_yield();
        pthread_mutex_lock(&tbf->mut);
    }

    int n =tbf->token<size?tbf->token:size;
    tbf->token -= n;

    pthread_mutex_unlock(&tbf->mut);
    //用户获取了 n 个token
    return n;
}

//归还token ptr是一个 void *
int mytbf_returntoken(mytbf_t *ptr,int size){
    struct mytbf_st *tbf = ptr;

    if (size <= 0){
        return -EINVAL;
    }
    pthread_mutex_lock(&tbf->mut);
    tbf->token += size;
    if (tbf->token > tbf->burst)
        tbf->token = tbf->burst;
    pthread_mutex_unlock(&tbf->mut);

    return size;
}

int mytbf_destroy(mytbf_t *ptr){
    struct mytbf_st *tbf = ptr;
    pthread_mutex_lock(&mutex);
    job[tbf->pos] = NULL;
    pthread_mutex_unlock(&mutex);

    pthread_mutex_destroy(&tbf->mut);
    free(tbf);
    return 0;
}
~~~

#### 条件变量

- pthread_cond_t
- pthread_cond_init()
- pthread_cond_destory()
- pthread_cond_wait() **等通知 + 抢锁**
- pthread_cond_broadcast() 广播给所有线程
- pthread_cond_signal() 通知任意一个线程

条件变量可以解决 互斥量进行盲等的问题 即实现了通知法，**通知互斥量什么时候上锁**

~~~ c
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;;
static int num = 0;

static void *handler(void *p){
    int task,mark;

    while(1){
        pthread_mutex_lock(&mutex);
        while(num == 0){
            pthread_cond_wait(&cond,&mutex);
        }
    
        if (num == -1){
            pthread_mutex_unlock(&mutex);
            break;
        }

        task = num;
        num = 0;//成功领取任务
        //通知所有线程任务被领取走了
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        mark = 1;
        for (int j = 2;j <= task/2;j++){
            if (task%j == 0){
                mark = 0;
                break;
            }
        }
        if (mark) {
            printf("[%d] %d is a priamer\n",*(int *)p,task);
        }
    }

    pthread_exit(NULL);
}

//池类算法
int main()
{
    pthread_t Ptid[THRNUM];

    for (int n = 0;n < THRNUM;n++){
        int *num = malloc(sizeof(int));
        *num = n;
        int err = pthread_create(Ptid+n,NULL,handler,num);
        if (err){
            fprintf(stderr,"%s\n",strerror(err));
            exit(1);
        }
    }

    for (int i = LEFT;i <= RIGHT;i++){
        pthread_mutex_lock(&mutex);
        
        //任务没有被领取
        while(num != 0){
            pthread_cond_wait(&cond,&mutex);
        }
        //任务已经成功下发
        num = i;
        //叫醒任意一个线程执行任务
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_lock(&mutex);
    //任务没有被领取
    while(num != 0){
        pthread_cond_wait(&cond,&mutex);
    }
    //任务已经成功下发
    num = -1;
    //广播给所有下游线程 任务结束
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    int n;
    for (n =0 ;n < THRNUM;n++){
        pthread_join(Ptid[n],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    exit(0);
}

~~~

#### 信号量

**通过互斥量与条件变量的配合我们可以实现信号量 信号量像是一个激活函数 当这个变量超过阈值时 将会触发条件变量给互斥量上锁**

~~~ c
#include "mysem.h"

struct mysem_st{
    int vaclue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

mysem_t *mysem_init(int initval){
    struct mysem_st *sem;
    sem = malloc(sizeof(*sem));

    if (sem == NULL){
        return NULL;
    }
    
    sem->vaclue = initval;
    pthread_mutex_init(&sem->mutex,NULL);
    pthread_cond_init(&sem->cond,NULL);

    return sem;
}

int mysem_add(mysem_t *ptr,int n){

    struct mysem_st *sem = ptr;

    pthread_mutex_lock(&sem->mutex);
    sem->vaclue += n;
    pthread_cond_broadcast(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);

    return n;
}

int mysem_sub(mysem_t *ptr,int n){
    struct mysem_st *sem = ptr;

    pthread_mutex_lock(&sem->mutex);

    while(sem->vaclue < n){
        pthread_cond_wait(&sem->cond,&sem->mutex);
    }
    sem->vaclue -= n;
    pthread_mutex_unlock(&sem->mutex);
    
    return n;
}

int mysem_destory(mysem_t *ptr){
    struct mysem_st *sem = ptr;

    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
    free(sem);

    return 0;
}

~~~

~~~ c

#define THRNUM 20
#define N 5
#define LEFT 30000000
#define RIGHT 30000200

static mysem_t *sem;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;;
static int num = 0;

static void *handler(void *p){
    int task,mark= 0;

    pthread_mutex_lock(&mutex);
    while(num == 0){
        pthread_cond_wait(&cond,&mutex);
    }
    
    if (num == -1){
        pthread_mutex_unlock(&mutex);
        mysem_add(sem,1);
        pthread_exit(NULL);
    }

    task = num;
    num = 0;//成功领取任务
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    mark = 1;
    for (int j = 2;j <= task/2;j++){
        if (task%j == 0){
            mark = 0;
            //归还计算资源
            sleep(1);
            mysem_add(sem,1);
            pthread_exit(NULL);
        }
    }
    if (mark) {
        printf("[%d] %d is a priamer\n",*(int *)p,task);
    }

    sleep(1);

    //归还计算资源
    mysem_add(sem,1);

    pthread_exit(NULL);
}

//池类算法
int main()
{
    pthread_t Ptid[THRNUM];
    sem = mysem_init(N);//初始化计算资源

    for (int i = LEFT;i <= RIGHT;i++){
        mysem_sub(sem,1);//消耗一个计算资源
        int *ptid = malloc(sizeof(int));
        *ptid = i-LEFT;
        int err = pthread_create(Ptid+(i-LEFT)%THRNUM,NULL,handler,ptid);
        if (err){
            fprintf(stderr,"%s\n",strerror(err));
            exit(1);
        }

        pthread_mutex_lock(&mutex);
        
        //任务没有被领取
        while(num != 0){
            pthread_cond_wait(&cond,&mutex);
        }
        //任务已经成功下发
        num = i;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_lock(&mutex);
    //任务没有被领取
    while(num != 0){
        pthread_cond_wait(&cond,&mutex);
    }
    //任务已经成功下发
    num = -1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    int n;
    for (n =0 ;n < THRNUM;n++){
        pthread_join(Ptid[n],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    exit(0);
}

~~~

#### 读写锁

~~~ c

~~~

## 线程属性
- pthread_attr_init()
- pthread_attr_destory()
- pthread_attr_setstacksize()
其他请见 `man pthread_attr_init` 的 `see also`

### 线程同步的属性
#### 互斥量属性
- pthread_mutexattr_init()
- pthread_mutexattr_destory()
- **clone** 进程 线程 不分家a
    - 跨进程设置锁
    - pthread_mutexattr_getshared()
    - pthread_mutexattr_setshared()
- pthread_mutexattr_gettype()
- pthread_mutexattr_settype()

#### 条件变量属性
- pthread_condattr_init()
- pthread_condattr_destory()

### 重入(reentry)
#### 多线程中的IO
- getchar_unlocked

### 线程与信号
- pthread_sigmask()
- sigwait()
- pthread_kill()

## openmp

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main()
{
#pragma omp parallel sections
{
#pragma omp section
    printf("Hello %d\n",omp_get_thread_num());
#pragma omp section
    printf("World %d\n",omp_get_thread_num());
}
    exit(0);
}

~~~
# 高级IO
## 非阻塞IO
不死等，检测到没有回应，一会儿再检测



### 有限状态机编程
**可以解决复杂流程问题**
- 简单流程 自然流程是结构化的
- 复杂流程 自然流程不是结构化的

### 数据中继

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define BUFSIZE 1024
#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"

enum {
    STATE_R = 1,
    STATE_W,
    STATE_Ex,
    STATE_T
};

struct fsm_st{
    int state;
    int sfd;//源
    int dfd;//目标
    char buf[BUFSIZE];
    int len;
    int pos;
    //出错原因
    char *errMess;
};

static void fsm_driver(struct fsm_st* fsm){
    int ret;

    switch(fsm->state){
        case STATE_R:
            fsm->len = read(fsm->sfd,fsm->buf,BUFSIZE);
            if (fsm->len == 0){
                fsm->state = STATE_T;//正常结束
            }else if (fsm->len < 0){
                if (errno == EAGAIN)
                  fsm->state = STATE_R;//假错 保留状态
                else{
                    fsm->state = STATE_Ex;//真正出错
                    fsm->errMess = "读取失败";
                }

            }else{
                fsm->pos = 0;
                fsm->state = STATE_W;//成功读入 转换状态
            }

            break;
        case STATE_W:
            ret = write(fsm->dfd,fsm->buf+fsm->pos,fsm->len);
            if (ret < 0){
                if (errno == EAGAIN){
                    fsm->state = STATE_W;
                }else{
                    fsm->errMess = "写入失败";
                    fsm->state = STATE_Ex;
                }
            }else{
                //坚持写够
                fsm->pos += ret;
                fsm->len -= ret;
                if (fsm->len == 0){
                    fsm->state = STATE_R;
                }else{
                    fsm->state = STATE_W;
                }
            }

            break;
        case STATE_Ex:
            perror(fsm->errMess);
            fsm->state = STATE_T;
            break;
        case STATE_T:
            /*do sth*/
            break;
        default:
            abort();
            break;
    }

}

static void relay(int fd1,int fd2){
    int old_fd1,old_fd2;

    struct fsm_st fsm12,fsm21;//读左写右 读右写左

    old_fd1 = fcntl(fd1,F_GETFL);
    fcntl(fd1,F_SETFL,old_fd1|O_NONBLOCK);

    old_fd2 = fcntl(fd2,F_GETFL);
    fcntl(fd2,F_SETFL,old_fd2|O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while(fsm12.state != STATE_T || fsm21.state != STATE_T){//状态机没有停转
        fsm_driver(&fsm12);
        fsm_driver(&fsm21);

    }
    

    //恢复原来的文件描述符状态
    fcntl(fd1,F_SETFL,old_fd1);
    fcntl(fd2,F_SETFL,old_fd2);

}

int main()
{
    int fd1,fd2;
    //假设用户使用阻塞的方式打开设备
    fd1 = open(TTY1,O_RDWR);
    if (fd1 < 0){
        perror("open()");
        exit(1);
    }
    write(fd1,"TTY1\n",5);

    fd2 = open(TTY2,O_RDWR|O_NONBLOCK);
    if (fd2 < 0){
        perror("open()");
        close(fd1);
        exit(1);
    }
    write(fd2,"TTY2\n",5);

    relay(fd1,fd2);

    close(fd1);
    close(fd2);
    exit(0);
}

~~~

## IO多路转接
**解决`IO密集型任务`中盲等的问题，监视文件描述符的行为，当当前文件描述符发生了我们感兴趣的行为时，才去做后续操作**
可以实现安全的休眠(替代sleep)

- select() 兼容性好 但设计有缺陷 **以事件为单位组织文件描述符**
- `nfds`的类型问题
- 参数没有`const`修饰 也就是函数会修改 fdset 任务和结果放在一起
- 监视的事件泰国单一 读 写 异常(异常的种类非常多)

~~~ c

static void relay(int fd1,int fd2){
    int old_fd1,old_fd2;

    struct fsm_st fsm12,fsm21;//读左写右 读右写左

    fd_set rset,wset;

    old_fd1 = fcntl(fd1,F_GETFL);
    fcntl(fd1,F_SETFL,old_fd1|O_NONBLOCK);

    old_fd2 = fcntl(fd2,F_GETFL);
    fcntl(fd2,F_SETFL,old_fd2|O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while(fsm12.state != STATE_T || fsm21.state != STATE_T){//状态机没有停转
        //布置监视任务
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        if (fsm12.state == STATE_R)
          FD_SET(fsm12.sfd,&rset);
        if (fsm12.state == STATE_W)
          FD_SET(fsm12.sfd,&wset);
        if (fsm21.state == STATE_R)
          FD_SET(fsm21.sfd,&rset);
        if (fsm21.state == STATE_W)
          FD_SET(fsm21.sfd,&wset);

        //监视
        struct  timeval ts;
        ts.tv_sec = 0;
        ts.tv_usec= 2;
        int maxfd = fd1>fd2?fd1:fd2;
        if (fsm12.state < STATE_AUTO ||fsm21.state < STATE_AUTO){
            if (select(maxfd+1,&rset,&wset,NULL,&ts) < 0){
                if (errno == EINTR)
                  continue;
                perror("select()");
                exit(1);
            }
        }

        //查看监视结果
        if (FD_ISSET(fd1,&rset) || FD_ISSET(fd2,&wset) || fsm12.state > STATE_AUTO){
            fsm_driver(&fsm12);
        }
        if (FD_ISSET(fd2,&rset) || FD_ISSET(fd1,&wset) || fsm21.state > STATE_AUTO){
            fsm_driver(&fsm21);
        }
    }

    //恢复原来的文件描述符状态
    fcntl(fd1,F_SETFL,old_fd1);
    fcntl(fd2,F_SETFL,old_fd2);

}
~~~

- poll()   
	- **wait for some event on a file descriptor （以文件描述符为单位组织事件）**
    - int poll(struct pollfd *fds,nfds,int timeout);

~~~ c
#include <poll.h>
static void relay(int fd1,int fd2){
    int old_fd1,old_fd2;

    struct fsm_st fsm12,fsm21;//读左写右 读右写左
    
    struct pollfd pfd[2];

    old_fd1 = fcntl(fd1,F_GETFL);
    fcntl(fd1,F_SETFL,old_fd1|O_NONBLOCK);

    old_fd2 = fcntl(fd2,F_GETFL);
    fcntl(fd2,F_SETFL,old_fd2|O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    pfd[0].fd = fd1;
    pfd[1].fd = fd2;

    while(fsm12.state != STATE_T || fsm21.state != STATE_T){//状态机没有停转
        //布置监视任务
        pfd[0].events = 0;//事件清零
        if (fsm12.state == STATE_R)//1可读
          pfd[0].events |= POLLIN;
        if (fsm21.state == STATE_W)//1可写
          pfd[0].events |= POLLOUT;

        pfd[1].events = 0;//事件清零
        if (fsm21.state == STATE_R)//2可读
          pfd[1].events |= POLLIN;
        if (fsm12.state == STATE_W)//2可写
          pfd[1].events |= POLLOUT;

        //监视
        if (fsm12.state < STATE_AUTO ||fsm21.state < STATE_AUTO){
            while (poll(pfd,2,-1) < 0){
                if (errno == EINTR)
                  continue;
                perror("poll()");
                exit(1);
            }
        }

        //查看监视结果
        if (pfd[0].revents&POLLIN || pfd[1].revents&POLLOUT || fsm12.state > STATE_AUTO){
            fsm_driver(&fsm12);
        }
        if (pfd[1].revents&POLLIN || pfd[0].revents&POLLOUT || fsm21.state > STATE_AUTO){
            fsm_driver(&fsm21);
        }
    }

    //恢复原来的文件描述符状态
    fcntl(fd1,F_SETFL,old_fd1);
    fcntl(fd2,F_SETFL,old_fd2);

}
~~~

- epoll     linux的poll封装方案 不可移植
    - epoll_create()
    - epoll_ctl(int epfd,int op,int fd,struct epoll_event *event) 指定epool实例 指定操作动作  指定操作的文件描述符 指定事件
	~~~ c
	typedef union epoll_data {
	    void        *ptr;
	    int          fd;
	    uint32_t     u32;
	    uint64_t     u64;
	} epoll_data_t;
	
	struct epoll_event {
	    uint32_t     events;      /* Epoll events */
	    epoll_data_t data;        /* User data variable */
	};
	
	~~~
	- epoll不仅可以使用文件描述符 也可以记录相关的信息
    - epoll_wait()

~~~ c
#include <sys/epoll.h>
static void relay(int fd1,int fd2){
    int old_fd1,old_fd2;

    struct fsm_st fsm12,fsm21;//读左写右 读右写左
    
    struct epoll_event ev;

    old_fd1 = fcntl(fd1,F_GETFL);
    fcntl(fd1,F_SETFL,old_fd1|O_NONBLOCK);

    old_fd2 = fcntl(fd2,F_GETFL);
    fcntl(fd2,F_SETFL,old_fd2|O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    int epfd = epoll_create(2);
    if (epfd < 0){
        perror("epoll_create()");
        exit(1);
    }
    ev.data.fd = fd1;
    ev.events = 0;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd1,&ev);
    ev.data.fd = fd2;
    ev.events = 0;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd2,&ev);

    while(fsm12.state != STATE_T || fsm21.state != STATE_T){//状态机没有停转
        ev.data.fd = fd1;
        ev.events = 0;
        //布置监视任务
        if (fsm12.state == STATE_R)//1可读
          ev.events |= EPOLLIN;
        if (fsm21.state == STATE_W)//1可写
          ev.events |= EPOLLOUT;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd1,&ev);

        ev.data.fd = fd2;
        ev.events = 0;
        if (fsm21.state == STATE_R)//2可读
          ev.events |= EPOLLIN;
        if (fsm12.state == STATE_W)//2可写
          ev.events |= EPOLLOUT;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd2,&ev);

        //监视
        if (fsm12.state < STATE_AUTO ||fsm21.state < STATE_AUTO){
            while (epoll_wait(epfd,&ev,1,-1) < 0){
                if (errno == EINTR)
                  continue;
                perror("poll()");
                exit(1);
            }
        }

        //查看监视结果
        if (ev.data.fd == fd1 && ev.events&POLLIN ||
                ev.data.fd == fd2 && ev.events&POLLOUT ||
                    fsm12.state > STATE_AUTO){
            fsm_driver(&fsm12);
        }
        if (ev.data.fd == fd2 && ev.events&POLLIN ||
                ev.data.fd == fd1 && ev.events&POLLOUT ||
                    fsm21.state > STATE_AUTO){
            fsm_driver(&fsm21);
        }
    }

    //恢复原来的文件描述符状态
    fcntl(fd1,F_SETFL,old_fd1);
    fcntl(fd2,F_SETFL,old_fd2);

    close(epfd)

}


~~~
## 其他读写函数
- readv()
- writev()

## 存储映射IO
把内存中的内容 或者 某一个文件的内容 映射到当前进程空间中来
- mmap(void *addr,size_t length,int prot,int flags,int fd,odd_t offset);
    - 匿名映射可以实现malloc的功能
- munmap(void *addr,size_t length)

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc,char **argv)
{
    int fd;
    struct stat statres;
    char *str = NULL;
    int count = 0;

    if (argc < 2){
        fprintf(stderr,"Usage...\n");
        exit(1);
    }

    
    fd = open(argv[1],O_RDONLY);
    if (fd < 0){
        perror("open()");
        exit(1);
    }

    if (fstat(fd,&statres)< 0){
        perror("fstat()");
        exit(1);
    }

    str = mmap(str,statres.st_size,PROT_READ,MAP_SHARED,fd,0);
    if (str == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    close(fd);//映射结束
    
    for (int i = 0;i < statres.st_size;i++){
        if (str[i] == 'a'){
            count++;
        }
    }
    printf("count %d\n",count);

    munmap(str,statres.st_size);

    exit(0);
}

~~~

## 文件锁
- fcntl()
- lockf()
- flock()

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

#define THRNUM 20
#define FNAME "/tmp/out"
#define BUFSIZE 1024

static void handler(){
    FILE *fp = fopen(FNAME,"r+");
    char buf[BUFSIZE];

    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }
    int fd = fileno(fp);
    //进入临界区
    lockf(fd,F_LOCK,0);

    fgets(buf,BUFSIZE,fp);
    fseek(fp,0,SEEK_SET);
    sleep(1);
    fprintf(fp,"%d\n",atoi(buf)+1);
    fflush(fp);

    //离开临界区
    lockf(fd,F_ULOCK,0);

    fclose(fp);
}

int main()
{
    pid_t pid;

    for (int i = 0;i < THRNUM;i++){
        pid = fork() ;
        if (pid < 0){
            perror("fork()");
            exit(1);
        }
        if (pid == 0){
            handler();
            exit(0);
        }
    }

    for (int i = 0;i < THRNUM;i++){
        wait(NULL);
    }

    return 0;
}

~~~
# 单机IPC(进程间通信) 
IPC Inter-Process Communication
## 管道
- **由linux内核创建与维护**
- **单工 一端读 一端写**
- **自同步机制 流速快的 等 流速慢的**
#### 匿名管道
**用于有血缘关系的进程间通信**
- pipe()

形同 `cat ./test.mp3 | map123 -`

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <wait.h>
#include <errno.h>

#define BUFSIZE 1024

int main()
{
    int pd[2];
    pid_t pid;
    char buf[BUFSIZE];

    if (pipe(pd) < 0){
        perror("pipe()");
        exit(1);
    }

    fprintf(stdout,"hello,welcome to MusicPlayer\n");

    pid = fork();
    if (pid < 0){
        perror("fork()");
        exit(1);
    }
    
    if(pid == 0){
        close(pd[1]); //关闭写端

        dup2(pd[0],0);//将将读端重定向到stdin mpg123 - "-" 表示从stdin读取
        close(pd[0]);

        int fd = open("/dev/null",O_RDWR);//屏蔽其他标准输出
        dup2(fd,1);
        dup2(fd,2);

        execl("/bin/mpg123","mpg123","-",NULL);
        perror("execvp()");
        exit(1);
    }else{
        close(pd[0]);//关闭读端
        int fd = open("./test.mp3",O_RDONLY);
        int len;

        while(1){
            len = read(fd,buf,BUFSIZE);
            if (len < 0){
                if (errno == EINTR)
                  continue;
                close(pd[1]);
                exit(1);
            }
            if (len == 0){
                break;
            }
            write(pd[1],buf,len);
        }
        close(pd[1]);
        wait(NULL);
    }

    exit(0);
}

~~~

#### 命名管道
- mkfifo 
~~~ bash
makefifo musicpipe
cat ./test.mp3 > musicpipe
~~~

~~~ bash
#同一个目录
mpg123 musicpipe
~~~

## XSI(SysV)
- `ipcs`命令
- ftok() **使得通信双方拿到同一个机制**
- 函数的命名方式类似
    - xxxget 获取
    - xxxop 操作
    - xxxctl 控制
### 消息队列 Message Queues
- msgget()
- msgop()
- msgctl()

~~~ c
#ifndef PROTO_H__
#define PROTO_H__

#define KEYPATH "/etc/services"
#define KEYPROJ 'g'
#define NAMESIZE 1024

struct msg_st{
    long mtype;
    char name[NAMESIZE];
    int math;
    int chinese;
};


#endif

~~~

~~~ c
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include<sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#include "proto.h"


int main(){
    
    key_t key;
    int msgid;
    struct msg_st sbuf;

    key = ftok(KEYPATH,KEYPROJ);
    if (key < 0){
        perror("ftik()");
        exit(1);
    }
    
    msgid = msgget(key,IPC_CREAT|0666);
    if (msgid < 0){
        perror("msgget()");
        exit(1);
    }
    
    strcpy(sbuf.name,"EINTR");
    sbuf.chinese = 100;
    sbuf.math = 100;
    
    if (msgsnd(msgid,&sbuf,sizeof(sbuf)-sizeof(long),0) < 0){
        perror("msgrcv()");
        exit(1);
    }

    fprintf(stdout,"OK");

    //销毁当前实例
    msgctl(key,IPC_RMID,0);

    exit(0);
}
~~~

~~~ c
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>

#include "proto.h"


int main(){
    key_t key;
    int msgid;
    struct msg_st rbuf;

    key = ftok(KEYPATH,KEYPROJ);
    if (key < 0){
        perror("ftik()");
        exit(1);
    }
    
    msgid = msgget(key,IPC_CREAT|0666);
    if (msgid < 0){
        perror("msgget()");
        exit(1);
    }
    
    while(1){
        if (msgrcv(msgid,&rbuf,sizeof(rbuf)-sizeof(long),0,0) < 0){
            perror("msgrcv()");
            exit(1);
        }

        fprintf(stdout,"name : %s ",rbuf.name);
        fprintf(stdout,"chinese : %d ",rbuf.chinese);
        fprintf(stdout,"math : %d\n",rbuf.math);
        fflush(NULL);
    }
    //销毁当前实例
    msgctl(key,IPC_RMID,0);

    exit(0);
}
~~~
        
### 信号量数组 Semaphore Arrays
- semget()
- semop()
- semctl()
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>

#define THRNUM 20
#define FNAME "/tmp/out"
#define BUFSIZE 1024

static int semid;

//取资源量
static void P(){
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = -1;//取一个资源
    op.sem_flg = 0;//特殊要求

    while(semop(semid,&op,1) < 0){
        if (errno == EINTR||errno == EAGAIN){
            continue;
        }else{
            perror("semop()");
            exit(1);
        }
    }

}

//还资源量
static void V(){
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = 1;//取一个资源
    op.sem_flg = 0;//特殊要求

    while(semop(semid,&op,1) < 0){
        if (errno == EINTR||errno == EAGAIN){
            continue;
        }else{
            perror("semop()");
            exit(1);
        }
    }

}

static void handler(){
    FILE *fp = fopen(FNAME,"r+");
    char buf[BUFSIZE];


    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }
    int fd = fileno(fp);

    //进入临界区
    P();

    fgets(buf,BUFSIZE,fp);
    fseek(fp,0,SEEK_SET);
    sleep(1);
    fprintf(fp,"%d\n",atoi(buf)+1);
    fflush(fp);

    //离开临界区
    V();

    fclose(fp);
}

int main()
{
    pid_t pid;

    semid = semget(IPC_PRIVATE,1,0666);//父子关系的进程通信可以使用匿名IPC
    if (semid < 0){
        perror("semget()");
        exit(1);
    }
    //初始化
    if (semctl(semid,0,SETVAL,1)){//相当于互斥量
        perror("semctl()");
        exit(1);

    }    

    for (int i = 0;i < THRNUM;i++){
        pid = fork() ;
        if (pid < 0){
            perror("fork()");
            exit(1);
        }
        if (pid == 0){
            handler();
            exit(0);
        }
    }

    for (int i = 0;i < THRNUM;i++){
        wait(NULL);
    }
    semctl(semid,0,IPC_RMID);

    return 0;
}

~~~
### 共享内存 Shared Memory
- shmget()
- shmop()
- shmctl()

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>

#define BUFSIZE 1024
#define SHMSIZE 4*1024*1024

static int shmid;

int main()
{
    pid_t pid;
    char *ptr;

    shmid = shmget(IPC_PRIVATE,SHMSIZE,0666);
    if (shmid < 0){
        perror("shmget()");
        exit(1);
    }

    pid = fork() ;
    if (pid < 0){
        perror("fork()");
        exit(1);
    }
    if (pid == 0){
        ptr = shmat(shmid,NULL,0);
        if (ptr == (void *)-1){
            perror("shmat()");
            exit(1);
        }
        strcpy(ptr,"hello");
        shmdt(ptr);
        exit(0);
    }else{
        wait(NULL);
        ptr = shmat(shmid,NULL,0);
        if (ptr == (void *)-1){
            perror("shmat()");
            exit(1);
        }
        puts(ptr);
        shmdt(ptr);
        shmctl(shmid,IPC_RMID,NULL);
        exit(0);

    }
}

~~~
# 跨主机IPC(进程间通信)

## 网络套接字(socket)

## 跨主机的传输要注意的问题
### 字节序
- 大端 **低地址放高字节**
- 小端 **高地址放低字节（x86）**
~~~ c
0x00 00 00 05

//大端
05 00 00 00
//小端
00 00 00 05
~~~
- 主机字节序 **host**
- 网络字节序 **network**
- _ to _ 长度()
    - htons()
    - htonl()
    - ntohs()
    - ntohl()
### 对齐
~~~ c
struct{
    int i;
    char ch;
    float f;
};//12个字节
~~~
结构体内存对齐
- 解决方案 **指定宏 告诉编译器不对齐**
### 类型长度
- int的长度未定义
- char有无符号未定义
解决： `int32_t` `uint32_t` `int64_t` `int8_t(有符号的char)` `uint8_t(无符号的char)`

### socket
**一个中间层，连接网络协议与文件操作**
socket就是插座，与兴在计算机中两个从小通过socket建立起一个通道，数据在通道中传输
socket把复杂的TCP/IP协议族隐藏了起来，对于程序元来说只要用好socket相关的函数接可以完成网络通信

socket提供了`stream` `datagram` 两种通信机制，即流socket和数据包socket

流socket基于TCP协议，是一个有序、可靠、双向字节刘的通道，传输数据不会丢失、不会重复、顺序也不会错乱

数据报socket基于UDP协议，不需要建立和尉迟连接，可能会丢失或错乱。UDP不是一个可靠的协议，对数据的长度有限制，但是效率较高
~~~ c
int socket(int domain,int type,int protocol)
          //SOCK_STREAM 有序 可靠 双工 基于(单字节)字节流
          //SOCK_DGRAM 分组的 不可靠的 无连接
          //SOCK_SEQPACKET (有序分组式) 有序可靠 报式传输
~~~

#### 常用函数
绑定地址和端口
- int bind(int socket, const struct sockaddr \*address, socklen_t address_len)
sockaddr -> struct socketaddr_in
~~~ c
 struct sockaddr_in {
               sa_family_t    sin_family; /* address family: AF_INET */
               in_port_t      sin_port;   /* port in network byte order */
               struct in_addr sin_addr;   /* internet address */
           };

~~~

- recvfrom()
- sendto()

- inet_pton() 点分式转二进制数

## UDP
### 步骤
- 被动端
    - 取得SOCKET
    - 给SOCKET取得地址
    - 收/发消息
    - 关闭SOCKET
- 主动端
    - 取得SOCKET
    - 给SOCKET取得地址(可以省略 本地通信地址由操作系统分配)
    - 发/收消息
    - 关闭SOCKET

- socket()
- bind()
- sendto()
- rcvfrom()
- inet_pton()
- inet_ntop()

~~~ c
#ifndef PROTO_H__
#define PROTO_H__

#include <stdint.h>

#define NAMEMAX 512-8-8//(UDP推荐长度-UDP报头长度-结构体的长度)

struct msg_st{
    uint32_t math;
    uint32_t chinese;
    char name[0];
}__attribute__((packed));//不对齐

#endif

~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>

#include "proto.h"

#define IPSIZE 1024
#define SERVERPORT "2333"

static void debug(char *fmt,...){
    va_list ap;
    va_start(ap,fmt);

    printf("DEBUG ");
    printf(fmt,va_arg(ap,int));

    va_end(ap);
}

int main()
{
    int sfd;
    struct sockaddr_in laddr;//local addr
    struct sockaddr_in raddr;//remote addr
    struct msg_st *rbuf;
    char ip[IPSIZE];

    int pkglen = sizeof(struct msg_st)+NAMEMAX;
    debug("%d\n",pkglen);
    rbuf = malloc(pkglen);
    if (rbuf == NULL){
        perror("malloc()");
        exit(1);
    }

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }

    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    socklen_t raddr_len = sizeof(raddr);
    while(1){
        recvfrom(sfd,rbuf,pkglen,0,(void *)&raddr,&raddr_len);//报式套接字每次通信都需要知道对方是谁
        inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);
        printf("%s %d\n",ip,ntohs(raddr.sin_port));
        printf("%s %d %d\n",rbuf->name,ntohl(rbuf->math),ntohl(rbuf->chinese));
        fflush(NULL);
    }

    close(sfd);
    
    exit(0);
}

~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "proto.h"

#define SERVERPORT "2333"

int main()
{
    int sfd;
    struct msg_st *sbuf;
    struct sockaddr_in raddr;//remote addr

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    int pkglen = sizeof(struct msg_st)+strlen("Mike")+1;// 注意给'/0'留位置
    sbuf = malloc(pkglen);
    if (sbuf == NULL){
        perror("malloc()");
        exit(1);
    }
    
    char *name = "Mike";
    strcpy(sbuf->name,name);
    sbuf->math = htonl(rand()%100);//主机字节序转网络字节序
    sbuf->chinese = htonl(rand()%100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET,"127.0.0.1",&raddr.sin_addr);

    if(sendto(sfd,sbuf,pkglen,0,(void *)&raddr,sizeof(raddr)) < 0){
        perror("sendto()");
        exit(1);
    }

    puts("OK");

    close(sfd);

    exit(0);
}
~~~

### 多点通讯
#### 广播(全网广播，子网广播)
- getsockopt()
- setsockopt()

~~~ c
//client.c
int main()
{
    int sfd;
    struct msg_st *sbuf;
    struct sockaddr_in raddr;//remote addr

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    if(sfd < 0){
        perror("socket()");
        exit(1);
    }
    //设置socket的属性
    int val = 1;
    if (setsockopt(sfd,SOL_SOCKET,SO_BROADCAST,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }//打开广播属性
    

    int pkglen = sizeof(struct msg_st)+strlen("Mike")+1;// 注意给'/0'留位置
    sbuf = malloc(pkglen);
    if (sbuf == NULL){
        perror("malloc()");
        exit(1);
    }
    
    char *name = "Mike";
    strcpy(sbuf->name,name);
    sbuf->math = htonl(rand()%100);//主机字节序转网络字节序
    sbuf->chinese = htonl(rand()%100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET,"255.255.255.255",&raddr.sin_addr);

    if(sendto(sfd,sbuf,pkglen,0,(void *)&raddr,sizeof(raddr)) < 0){
        perror("sendto()");
        exit(1);
    }

    puts("OK");

    close(sfd);

    exit(0);
}
~~~

~~~ c
//server.c
int main()
{
    int sfd;
    struct sockaddr_in laddr;//local addr
    struct sockaddr_in raddr;//remote addr
    struct msg_st *rbuf;
    char ip[IPSIZE];

    int pkglen = sizeof(struct msg_st)+NAMEMAX;
    debug("%d\n",pkglen);
    rbuf = malloc(pkglen);
    if (rbuf == NULL){
        perror("malloc()");
        exit(1);
    }

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }

    //设置socket属性
    int val = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_BROADCAST,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    socklen_t raddr_len = sizeof(raddr);
    while(1){
        recvfrom(sfd,rbuf,pkglen,0,(void *)&raddr,&raddr_len);//报式套接字每次通信都需要知道对方是谁
        inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);
        printf("%s %d\n",ip,ntohs(raddr.sin_port));
        printf("%s %d %d\n",rbuf->name,ntohl(rbuf->math),ntohl(rbuf->chinese));
        fflush(NULL);
    }

    close(sfd);
    
    exit(0);
}

~~~

#### 多播/组播
相较广播更灵活
`224.0.0.1` **这个地址表示所有支持多播的节点默认都存在于这个组中且无法离开**

~~~ c
//client.c
int main()
{
    int sfd;
    struct msg_st *sbuf;
    struct sockaddr_in raddr;//remote addr
    struct ip_mreqn mreqn;

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    if(sfd < 0){
        perror("socket()");
        exit(1);
    }
    //设置socket的属性
    inet_pton(AF_INET,"0.0.0.0",&mreqn.imr_address);
    inet_pton(AF_INET,MULTICASTADDR,&mreqn.imr_multiaddr);
    mreqn.imr_ifindex = if_nametoindex("wlp7s0");

    if (setsockopt(sfd,IPPROTO_IP,IP_MULTICAST_IF,&mreqn,sizeof(mreqn)) < 0){
        perror("setsockopt()");
        exit(1);
    }//打开广播属性
    

    int pkglen = sizeof(struct msg_st)+strlen("Mike")+1;// 注意给'/0'留位置
    sbuf = malloc(pkglen);
    if (sbuf == NULL){
        perror("malloc()");
        exit(1);
    }
    
    char *name = "Mike";
    strcpy(sbuf->name,name);
    sbuf->math = htonl(rand()%100);//主机字节序转网络字节序
    sbuf->chinese = htonl(rand()%100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET,MULTICASTADDR,&raddr.sin_addr);

    if(sendto(sfd,sbuf,pkglen,0,(void *)&raddr,sizeof(raddr)) < 0){
        perror("sendto()");
        exit(1);
    }

    puts("OK");

    close(sfd);

    exit(0);
}
~~~

~~~ c
//server.c
int main()
{
    int sfd;
    struct sockaddr_in laddr;//local addr
    struct sockaddr_in raddr;//remote addr
    struct msg_st *rbuf;
    char ip[IPSIZE];

    int pkglen = sizeof(struct msg_st)+NAMEMAX;
    rbuf = malloc(pkglen);
    if (rbuf == NULL){
        perror("malloc()");
        exit(1);
    }

    sfd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }

    //设置socket属性
    struct ip_mreqn mreqn;
    inet_pton(AF_INET,"0.0.0.0",&mreqn.imr_address);
    //224.0.0.1 这个地址表示所有支持多播的节点默认都存在于这个组中且无法离开
    inet_pton(AF_INET,MULTICASTADDR,&mreqn.imr_multiaddr);
    mreqn.imr_ifindex = if_nametoindex("wlp7s0");
    if (setsockopt(sfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreqn,sizeof(mreqn)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    socklen_t raddr_len = sizeof(raddr);
    while(1){
        recvfrom(sfd,rbuf,pkglen,0,(void *)&raddr,&raddr_len);//报式套接字每次通信都需要知道对方是谁
        inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);
        printf("%s %d\n",ip,ntohs(raddr.sin_port));
        printf("%s %d %d\n",rbuf->name,ntohl(rbuf->math),ntohl(rbuf->chinese));
        fflush(NULL);
    }

    close(sfd);
    
    exit(0);
}

~~~

#### UDP需要特别注意的问题
**丢包是由阻塞造成的(网络被路由器或其他网络节点按照某种算法移除),而不是ttl**
- TTL time to live  数据包跳转的路由数


比如我们要实现一个ftp文件服务器的程序.

~~~ c
#ifndef PROTO_H__
#define PROTO_H__

#define PATHSIZE 1024
#define DATASIZE 1024

enum{
    MSG_PATH = 1,
    MSG_DATA,
    MSG_EOT//end_of_transfer
};

typedef struct msg_path_st{
    long mtype;
    char path[PATHSIZE];
} msg_path_t;


typedef struct msg_data_st{
    long mtype;
    char data[DATASIZE];
    int datalen;
} msg_data_t;

typedef struct msg_eot_st{
    long mtype;
} msg_eot_t;

union msg_s2c_un{
    long mtype;
    msg_data_t datamsg;
    msg_eot_t eofmsg;
};

#endif

~~~

Client&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Server
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;------文件路径--->
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;文件
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data1--------
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data2--------    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data3--------    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<------EOT--------- 
文件


由于网络的不可靠，如果出现丢包将会使得文件传输不完整，如何确保对方一定能收到完整内容呢？
**停等式流控**

Client&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Server
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;--文件路径-->
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;文件
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data1--------等待
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-------ACK1------>继续发送
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data2--------等待    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-------ACK2------>继续发送
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<-----data3--------等待    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-------ACK3------>继续发送
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<------EOT---------等待    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;--------ACK4----->停止发送
文件

中途如果有ACK包Server没有成功收到那么将会再次发送数据包，来回通信延迟（Round-trip delay time），在通信（Communication）、电脑网络（Computer network）领域中，意指：在双方通信中，发讯方的信号（Signal）传播（Propagation）到收讯方的时间（意即：传播延迟（Propagation delay）），加上收讯方回传消息到发讯方的时间（如果没有造成双向传播速率差异的因素，此时间与发讯方将信号传播到收讯方的时间一样久）


## TCP

关于TCP更多细节请看
[TCP 的那些事儿（上）](https://coolshell.cn/articles/11564.html)
[TCP 的那些事儿（下）](https://coolshell.cn/articles/11609.html)
### 步骤
Client
1. 获取SOCKET
2. 给SOCKET取得地址
3. 发送连接
4. 收/发消息
5. 关闭
Server
1. 获取SOCKET
2. 给SOCKET取得地址
3. 将SOCKET置为监听模式
4. 接受连接
5. 收/发消息
6. 关闭

##### 普通多进程版
~~~ c
#ifndef PROTO_H__
#define PROTO_H__

#include <stdint.h>

#define NAMEMAX 512-8-8//(UDP推荐长度-UDP报头长度-结构体的长度)
#define FMT_STAMP "%lld\n"
#define SERVERPORT "2333"


#endif

~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "proto.h"

#define IPSIZE 1024
#define BUFSIZE 1024
#define SERVERPORT "2333"

static void server_job(int newsd){
    char buf[BUFSIZE];
    int pkglen = 0;

    pkglen = sprintf(buf,FMT_STAMP,(long long)time(NULL));

    if (send(newsd,buf,pkglen,0) < 0){
        perror("send()");
        exit(1);
    }
}

int main()
{
    int sfd;
    struct sockaddr_in laddr;//local addr
    struct sockaddr_in raddr;//remote addr
    char ip[IPSIZE];

    sfd = socket(AF_INET,SOCK_STREAM,0/*IPPROTO_TCP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }
    
    int val = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(sfd,1024) < 0){//全连接数量
        perror("listen()");
        exit(1);
    }


    socklen_t raddr_len = sizeof(raddr);
    pid_t pid;

    while(1){
        int newsd;
        newsd = accept(sfd,(void *)&raddr,&raddr_len);//接收客户端连接
        if (newsd < 0){
            perror("accept()");
            exit(1);
        }
        
        pid = fork();
        if (pid < 0){
            perror("fork()");
            exit(1);
        }
        if (pid == 0){
            close(sfd);
            inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);
            printf("client %s %d\n",ip,ntohs(raddr.sin_port));
            server_job(newsd);
            close(newsd);
            exit(0);
        }
        close(newsd);//父子进程必须都将打开的来自client的socket关闭，否则socket不会返回client
    }

    close(sfd);
    
    exit(0);
}

~~~

~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "proto.h"

#define BUFSIZE 1024

int main()
{
    int sfd;
    struct sockaddr_in raddr;//remote addr

    sfd = socket(AF_INET,SOCK_STREAM,0/*IPPROTO_TCP*/);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET,"127.0.0.1",&raddr.sin_addr);

    if(connect(sfd,(void *)&raddr,sizeof(raddr)) < 0){
        perror("connect()");
        exit(1);
    }

    FILE *fp;
    fp = fdopen(sfd,"r+");
    if (fp == NULL){
        perror("fopen()");
        exit(1);
    }

    long long stamp;
    if (fscanf(fp,FMT_STAMP,&stamp) < 1){
        fprintf(stderr,"Bad format\n");
    }else{
        fprintf(stdout,FMT_STAMP,stamp);
    }

    close(sfd);

    exit(0);
}
~~~

##### 动态进程池版
~~~ c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <errno.h>

#include "proto.h"

#define SIG_NOTIFY SIGUSR2

#define MINSPACESERVER 5
#define MAXSPACESERVER 10
#define MAXCLINETS     20

#define IPSIZE 1024
#define BUFSIZE 1024

enum {
    STATE_IDLE=0,//idle 空闲的
    STATE_BUSY
};

struct server_st{
    pid_t pid;
    int state;
};

static struct server_st *serverpool;
static int idle_count = 0,busy_count = 0;

//socket相关全局变量
static int sfd;

static void handle(int sig){
    return;
}

static void server_job(int pos){
    int newsd;
    int ppid;

    struct sockaddr_in raddr;//remote addr
    char ip[IPSIZE];

    socklen_t raddr_len = sizeof(raddr);

    ppid = getppid();//父进程

    while(1){
        serverpool[pos].state = STATE_IDLE;
        kill(ppid,SIG_NOTIFY);

        newsd = accept(sfd,(void *)&raddr,&raddr_len);//接收客户端连接
        if (newsd < 0){
            if (errno == EINTR || errno == EAGAIN)
              continue;
            else{
                perror("accept()");
                exit(1);
            }
        }

        serverpool[pos].state = STATE_BUSY;
        kill(ppid,SIG_NOTIFY);
        inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);

        char buf[BUFSIZE];
        int pkglen = 0;

        pkglen = sprintf(buf,FMT_STAMP,(long long)time(NULL));

        if (send(newsd,buf,pkglen,0) < 0){
            perror("send()");
            exit(1);
        }
        close(newsd);
        sleep(5);
    }
}

static int add_one_server(){
    int slot;
    pid_t pid;

    if (idle_count + busy_count >= MAXCLINETS){
        return -1;
    }

    for(slot = 0;slot < MAXCLINETS;slot++){
        if (serverpool[slot].pid == -1){
            break;
        }
    }
    serverpool[slot].state = STATE_IDLE;
    pid = fork();
    if (pid < 0){
        perror("fork");
        exit(1);
    }
    if (pid == 0){
        server_job(slot);
        exit(0);
    }else{
        serverpool[slot].pid = pid;
        idle_count++;
    }
    return 0;
}

static int del_one_server(){
    int slot;
    if (idle_count == 0){
        return -1;
    }

    for(slot = 0;slot < MAXCLINETS;slot++){
        if (serverpool[slot].pid != -1 && serverpool[slot].state == STATE_IDLE){
            kill(serverpool[slot].pid,SIGTERM);
            serverpool[slot].pid = -1;
            idle_count--;
            break;
        }
    }
    return 0;
}

static void scan_pool(){
    int idle = 0,busy = 0;
    for (int i = 0;i < MAXCLINETS;i++){
        if (serverpool[i].pid == -1){
            continue;
        }
        if (kill(serverpool[i].pid,0)){//kill pid 0检测一个进程是否存在
            serverpool[i].pid =-1;
            continue;
        }
        //统计进程池的状态
        if(serverpool[i].state == STATE_IDLE)
          idle++;
        else if(serverpool[i].state == STATE_BUSY)
          busy++;
        else{
            fprintf(stderr,"未知状态!\n");
            abort();
        }
    }
    idle_count = idle;
    busy_count = busy;
}

int main()
{
    struct sigaction sa,osa;
    
    sa.sa_handler = SIG_IGN;//忽略父进程的资源回收信号
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;//让子进程结束后自行消亡,不会变成僵尸状态
    sigaction(SIGCLD,&sa,&osa);
    
    sa.sa_handler = handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIG_NOTIFY,&sa,&osa);

    //屏蔽信号
    sigset_t sigset,oldsigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIG_NOTIFY);
    sigprocmask(SIG_BLOCK,&sigset,&oldsigset);

    serverpool = mmap(NULL,sizeof(struct server_st)*MAXCLINETS,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if (serverpool == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    //初始化进程池
    for (int i = 0;i < MAXCLINETS;i++){
        serverpool[i].pid = -1;
    }

    sfd = socket(AF_INET,SOCK_STREAM,0/*IPPROTO_TCP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }
    
    int val = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    struct sockaddr_in laddr;//local addr
    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(sfd,1024) < 0){//全连接数量
        perror("listen()");
        exit(1);
    }

    for(int i = 0;i < MINSPACESERVER;i++){
        add_one_server();
    }

    while(1){
        sigsuspend(&oldsigset);

        scan_pool();//扫描进程池

        //管理进程池
        if (idle_count > MAXSPACESERVER){
            for (int i = 0;i < (idle_count-MAXSPACESERVER);i++){
                del_one_server();
            }
        }else if (idle_count < MINSPACESERVER){
            printf("削减server\n");
            for (int i = 0;i < (MINSPACESERVER-idle_count);i++){
                add_one_server();
            }
        }
        
        //print the pool
        for(int i = 0;i < MAXCLINETS;i++){
            if (serverpool[i].pid == -1){
                printf(" ");
            }else if (serverpool[i].state == STATE_IDLE){
                printf(".");
            }else if(serverpool[i].state == STATE_BUSY){
                printf("X");
            }
            fflush(NULL);
        }
        printf("\n");
    }

    close(sfd);
    sigprocmask(SIG_SETMASK,&oldsigset,NULL);
    
    exit(0);
}

~~~

请配合之前写的客户端程序使用
server端
~~~ bash
./server
~~~
建议开启3个以上会话启动客户端来模拟高仿问量
~~~ bash
while true;do ./client ;sleep 1;done
~~~
