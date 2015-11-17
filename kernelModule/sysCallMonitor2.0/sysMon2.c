/*
Luca Verderame - SysCallMonitor 2012
*/


#include <asm/unistd.h>
#include </home/lucaverdio/seandroid-4.4.4/kernel/goldfish/arch/powerpc/include/asm/syscalls.h>
//#include <linux/autoconf.h>  ///not working here!!
#include </home/lucaverdio/seandroid-4.4.4/kernel/goldfish/include/generated/autoconf.h>
//#include </home/lucaverdio/seandroid-4.4.4/kernel/goldfish/include/config/auto.conf>
#include <linux/in.h>
#include <linux/init_task.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/socket.h>
#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/signal.h>
#include <linux/string.h>


////////////////////////canale netLink per la comunicazione con lo userspace////////////////////////////////////////
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include<linux/mutex.h>
#include <linux/kthread.h>  // for threads

#define NETLINK_USER 31
#define BUF 1024
#define MY_GROUP    49

struct sock *nl_sk = NULL;
struct nlmsghdr *nlsk_mh = NULL;
struct sock* socket = NULL;
struct sk_buff* socket_buff;

int rec_pid = 0;
struct mutex my_lock; /* shared between the threads */

void **sys_call_table; //added

static void nl_receive_callback (struct sk_buff *skb)                                                     
{                                                                                                     
    nlmsg_free(skb);                                                                                  
}    

static int send(char *msg)                                                                  
{                                                                                                     
    struct nlmsghdr *nlsk_mh;                                                                         
    //printk("send : %s",msg);
    socket_buff = nlmsg_new(1024,GFP_KERNEL);    
                                                                                                                                                                                                                                         
    nlsk_mh = nlmsg_put(socket_buff, 0, 0, NLMSG_DONE, strlen(msg), 0);                       
    NETLINK_CB(socket_buff).pid = 0;    // kernel pid                                                   
    NETLINK_CB(socket_buff).dst_group = MY_GROUP;                                                     
    strcpy(nlmsg_data(nlsk_mh), msg);                                                                

    //nlmsg_multicast(socket, socket_buff, 0, MY_GROUP, GFP_KERNEL);
    int flag = nlmsg_multicast(socket, socket_buff, 0, MY_GROUP, GFP_KERNEL); //GFP_ATOMIC                    
    printk("send status: %d \n",flag);
    return 0;                                                                                           
} 


static int hello_init(void)
{
    printk("Entering: %s\n",__FUNCTION__);
    
    //kernel 3.6 and above
    /*
    struct netlink_kernel_cfg cfg = {
                .groups = 1,
                .input = hello_nl_recv_msg,
    };
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    
    */
    //kernel 3.2 and below   user_register not used now
    //nl_sk=netlink_kernel_create(&init_net, NETLINK_USER, 0, user_register, NULL, THIS_MODULE);

    //nl_sk=netlink_kernel_create(&init_net, NETLINK_USER, 0,hello_nl_recv_msg, NULL, THIS_MODULE);

    //socket = netlink_kernel_create(&init_net,NETLINK_USERSOCK,MY_GROUP,nl_receive_callback, NULL, THIS_MODULE);
    socket = netlink_kernel_create(&init_net,NETLINK_USER,0,nl_receive_callback, NULL, THIS_MODULE);
    //socket = netlink_kernel_create(NETLINK_USERSOCK,nl_receive_callback);                                                                                            

    if(!socket)
    {
        printk("Error creating socket.\n");
        return -10;
    }
    printk("Socket kernel creato! \n");
    char *hello = "invio dal kernel";
    send(hello); 
    return 0;
}

static void hello_exit(void){
    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

asmlinkage ssize_t (*orig_read) (int fd, char *buf, size_t count);
asmlinkage ssize_t (*orig_write) (int fd, char *buf, size_t count);
asmlinkage ssize_t (*orig_open) (const char *pathname, int flags);
asmlinkage ssize_t (*orig_close) (int fd);
asmlinkage long    (*orig_connect)(int, struct sockaddr __user *, int);
asmlinkage long (*orig_socket)(int, int, int);
asmlinkage long (*orig_socketpair)(int, int, int, int __user *);
asmlinkage long (*orig_bind)(int, struct sockaddr __user *, int);
asmlinkage long (*orig_accept)(int, struct sockaddr __user *, int __user *);
asmlinkage long (*orig_lseek)(unsigned int fd, off_t offset,unsigned int origin);
asmlinkage long (*orig_lstat64)(char __user *filename,
				struct stat64 __user *statbuf);
asmlinkage long (*orig_mkdir)(const char __user *pathname, int mode);
asmlinkage long (*orig_sendmsg)(int fd, struct msghdr __user *msg, unsigned flags);
asmlinkage long (*orig_recvmsg)(int fd, struct msghdr __user *msg, unsigned flags);
asmlinkage long (*orig_sendto)(int, void __user *, size_t, unsigned,
				struct sockaddr __user *, int);
asmlinkage long (*orig_recvfrom)(int, void __user *, size_t, unsigned,
				struct sockaddr __user *, int __user *);
asmlinkage long (*orig_gettid)(void);
asmlinkage long (*orig_prctl)(int option, unsigned long arg2, unsigned long arg3,
			unsigned long arg4, unsigned long arg5);
asmlinkage long (*orig_exit_group)(int error_code);

// da testare
asmlinkage long (*orig_setuid)(uid_t uid);
asmlinkage long (*orig_setgid)(gid_t gid);
asmlinkage long (*orig_waitid)(int which, pid_t pid, struct siginfo __user *infop, int options, struct rusage __user *ru);
asmlinkage long (*orig_listen)(int, int);
asmlinkage long (*orig_mount)(char __user *dev_name, char __user *dir_name,
				char __user *type, unsigned long flags,
				void __user *data);
asmlinkage long (*orig_umount)(char __user *name, int flags);
asmlinkage long (*orig_rmdir)(const char __user *pathname);
asmlinkage long (*orig_shutdown) (int, int);
asmlinkage long (*orig_chroot)(const char __user *filename);
asmlinkage long (*orig_kill)(int pid, int sig);
asmlinkage long (*orig_tkill)(int pid, int sig);
asmlinkage long (*orig_setuid)(uid_t uid);
asmlinkage long (*orig_exit)(int error_code);
asmlinkage int (*orig_fork)(struct pt_regs *);
asmlinkage int (*orig_execve)(char __user *, char __user *__user *,
			  char __user *__user *, struct pt_regs *);
asmlinkage long (*orig_getpid)(void);
asmlinkage long (*orig_pause)(void);
asmlinkage long (*orig_getuid)(void);
asmlinkage long (*orig_geteuid)(void);
asmlinkage long (*orig_getgid)(void);

  long s1 = 0;
  long s2 = 0;
  long s3 = 0;
  long s4 = 0;


asmlinkage ssize_t
hacked_read (int fd, char *buf, size_t count)
{
 if(s1 != current->tgid){
  //printk(KERN_INFO "SYS_READ:pid %d, %s \n",current->tgid,current->comm);
  char msg[300] = {0};
  s1 = current->tgid; 
   sprintf(msg, "read<%s",current->comm);
   //kthread_run(send,msg,"sender");
 }
 return orig_read(fd,buf,count);	
}

asmlinkage ssize_t
hacked_write (int fd, char *buf, size_t count)
{
 if(s2 != current->tgid){
   //printk(KERN_INFO "SYS_WRITE:pid %d, %s \n",current->tgid,current->comm);
   char msg[300] = {0};
   s2 = current->tgid;
   sprintf(msg, "write<%s",current->comm);
   //kthread_run(send,msg,"sender");
 }
 return orig_write(fd,buf,count);	
}

asmlinkage ssize_t
hacked_open (const char *pathname, int flags)
{
  if(s3 != current->tgid){
    //printk(KERN_INFO "SYS_OPEN:pid %d, %s \n",current->tgid,current->comm);
    s3 = current->tgid; 
   char msg[300] = {0};
   sprintf(msg, "open<%s",current->comm);
   //kthread_run(send,msg,"sender");
 }
 return orig_open(pathname,flags);	
}

asmlinkage ssize_t
hacked_close (int fd)
{
 if(s4 != current->tgid){
   //printk(KERN_INFO "SYS_CLOSE:pid %d, %s \n",current->tgid,current->comm);
   s4 = current->tgid;
 }
   char msg[300] = {0};
   sprintf(msg, "close<%s",current->comm);
   //kthread_run(send,msg,"sender");
 return orig_close(fd);	
}

asmlinkage long
hacked_connect (int a, struct sockaddr __user *b, int c)
{
 //printk(KERN_INFO "SYS_CONNECT:pid %d, %s \n",current->tgid,current->comm);
 return orig_connect(a,b,c);	
}

asmlinkage long
hacked_waitid (int which, pid_t pid, struct siginfo __user *infop, int options, struct rusage __user *ru)
{
 //printk(KERN_INFO "SYS_WAITID:pid %d, %s \n",current->tgid,current->comm);
 return orig_waitid(which,pid,infop,options,ru);	
}

asmlinkage long
hacked_setuid(uid_t uid)
{
 //printk(KERN_INFO "SYS_SETUID:pid %d, %s \n",current->tgid,current->comm);
 return orig_setuid(uid);	
}

asmlinkage long
hacked_setgid (gid_t gid)
{
 //printk(KERN_INFO "SYS_SETGID:pid %d, %s \n",current->tgid,current->comm);
 return orig_setgid(gid);	
}

asmlinkage long
hacked_socket (int a,int b,int c)
{
 //printk(KERN_INFO "SYS_SOCKET:pid %d, %s \n",current->tgid,current->comm);
 return orig_socket(a,b,c);	
}

asmlinkage long 
hacked_socketpair (int a , int b, int c , int __user * d)
{
 //printk(KERN_INFO "SYS_SOCKETPAIR:pid %d, %s \n",current->tgid,current->comm);
 return orig_socketpair(a,b,c,d);	
}

asmlinkage long 
hacked_bind(int a, struct sockaddr __user *b, int c)
{
 //printk(KERN_INFO "SYS_BIND:pid %d, %s \n",current->tgid,current->comm);
 return orig_bind(a,b,c);	
}

asmlinkage long 
hacked_accept (int a, struct sockaddr __user * b, int __user * c)
{
 //printk(KERN_INFO "SYS_ACCEPT:pid %d, %s \n",current->tgid,current->comm);
 return orig_accept(a,b,c);	
}

asmlinkage long 
hacked_listen (int a, int b)
{
 //printk(KERN_INFO "SYS_LISTEN:pid %d, %s \n",current->tgid,current->comm);
 return orig_listen(a,b);	
}

asmlinkage long 
hacked_lseek(unsigned int fd, off_t offset,unsigned int origin)
{
 //printk(KERN_INFO "SYS_LSEEK:pid %d, %s \n",current->tgid,current->comm);
 return orig_lseek(fd,offset,origin);	
}

asmlinkage long 
hacked_mount(char __user *dev_name, char __user *dir_name,
				char __user *type, unsigned long flags,
				void __user *data)
{
 //printk(KERN_INFO "SYS_MOUNT:pid %d, %s \n",current->tgid,current->comm);
 return orig_mount(dev_name,dir_name,type,flags,data);	
}

asmlinkage long 
hacked_umount (char __user *name, int flags)
{
 //printk(KERN_INFO "SYS_UMOUNT:pid %d, %s \n",current->tgid,current->comm);
 return orig_umount(name,flags);	
}

asmlinkage long 
hacked_lstat64 (char __user *filename,
				struct stat64 __user *statbuf)
{
 //printk(KERN_INFO "SYS_LSTAT64:pid %d, %s \n",current->tgid,current->comm);
 return orig_lstat64(filename,statbuf);	
}

asmlinkage long 
hacked_mkdir (const char __user *pathname, int mode)
{
 //printk(KERN_INFO "SYS_MKDIR:pid %d, %s \n",current->tgid,current->comm);
 return orig_mkdir(pathname,mode);	
}

asmlinkage long 
hacked_rmdir (const char __user *pathname)
{
 //printk(KERN_INFO "SYS_RMDIR:pid %d, %s \n",current->tgid,current->comm);
 return orig_rmdir(pathname);	
}

asmlinkage long 
hacked_sendto (int x, void __user * a, size_t b, unsigned c,
				struct sockaddr __user * d, int e)
{
 //printk(KERN_INFO "SYS_SENDTO:pid %d, %s \n",current->tgid,current->comm);
 return orig_sendto(x,a,b,c,d,e);	
}

asmlinkage long 
hacked_recvfrom (int x, void __user * a, size_t b, unsigned c,
				struct sockaddr __user * d, int __user * e)
{
 //printk(KERN_INFO "SYS_RECVFROM:pid %d, %s \n",current->tgid,current->comm);
 return orig_recvfrom(x,a,b,c,d,e);	
}

asmlinkage long 
hacked_shutdown(int a, int b)
{
 //printk(KERN_INFO "SYS_SHUTDOWN:pid %d, %s \n",current->tgid,current->comm);
 return orig_shutdown(a,b);	
}

asmlinkage long 
hacked_sendmsg (int fd, struct msghdr __user *msg, unsigned flags)
{
 //printk(KERN_INFO "SYS_SENDMSG:pid %d, %s \n",current->tgid,current->comm);
 return orig_sendmsg(fd,msg,flags);	
}

asmlinkage long 
hacked_recvmsg (int fd, struct msghdr __user *msg, unsigned flags)
{
 //printk(KERN_INFO "SYS_RECVMSG:pid %d, %s \n",current->tgid,current->comm);
 //char * m = "recvmsg";
 //send(m);
 return orig_recvmsg(fd,msg,flags);	
}

asmlinkage long 
hacked_prctl(int option, unsigned long arg2, unsigned long arg3,
			unsigned long arg4, unsigned long arg5)
{
 //printk(KERN_INFO "SYS_PRCTL:pid %d, %s \n",current->tgid,current->comm);
 return orig_prctl(option,arg2,arg3,arg4,arg5);	
}

asmlinkage long 
hacked_chroot(const char __user *filename)
{
 //printk(KERN_INFO "SYS_CHROOT:pid %d, %s \n",current->tgid,current->comm);
 return orig_chroot(filename);	
}

asmlinkage long 
hacked_kill (int pid, int sig)
{
 //printk(KERN_INFO "SYS_KILL:pid %d, %s \n",current->tgid,current->comm);
 return orig_kill(pid,sig);	
}

asmlinkage long 
hacked_tkill (int pid, int sig)
{
   char msg[300] = {0};
   sprintf(msg, "tkill<%s",current->comm);
   kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_TKILL:pid %d, %s \n",current->tgid,current->comm);
 return orig_tkill(pid,sig);	
}

asmlinkage long
hacked_gettid(void)
{
   char msg[300] = {0};
   sprintf(msg, "gettid<%s",current->comm);
   kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_GETTID:pid %d, %s \n",current->tgid,current->comm);
 return orig_gettid();	
}

asmlinkage long
hacked_exit(int error_code)
{
   char msg[300] = {0};
   sprintf(msg, "EXIT from %d - %s",current->tgid,current->comm);
   //kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_EXIT:pid %d, %s \n",current->tgid,current->comm);
 return orig_exit(error_code);	
}

asmlinkage long
hacked_exit_group(int error_code)
{
   char msg[300] = {0};
   sprintf(msg, "ExitGroup from %d - %s",current->tgid,current->comm);
   kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_EXITGROUP:pid %d, %s \n",current->tgid,current->comm);
 return orig_exit_group(error_code);	
}

asmlinkage long
hacked_fork(struct pt_regs *a)
{
 //printk(KERN_INFO "SYS_FORK:pid %d, %s \n",current->tgid,current->comm);
 return orig_fork(a);	
}

asmlinkage long
hacked_execve(char __user * a, char __user *__user * b,
			  char __user *__user * c, struct pt_regs *d)
{
 //printk(KERN_INFO "SYS_EXECVE:pid %d, %s \n",current->tgid,current->comm);
 return orig_execve(a,b,c,d);	
}

asmlinkage long
hacked_getpid (void)
{
   char msg[300] = {0};
   sprintf(msg, "getpid<%s",current->comm);
   kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_GETPID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_pause (void)
{
 //printk(KERN_INFO "SYS_PAUSE:pid %d, %s \n",current->tgid,current->comm);
 return orig_pause();	
}

asmlinkage long
hacked_getuid (void)
{
 char msg[300] = {0};
 sprintf(msg, "getuid<%s",current->comm);
 kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_GETUID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_geteuid (void)
{
   char msg[300] = {0};
   sprintf(msg, "geteuid<%s",current->comm);
   //kthread_run(send,msg,"sender");
 //printk(KERN_INFO "SYS_GETEUID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_getgid (void)
{
   char msg[300] = {0};
   sprintf(msg, "getgid<%s",current->comm);
   kthread_run(send,msg,"sender");
 return orig_getpid();	
}


static int  __init
root_start (void)
{
 //avvio canale netLink
  hello_init();

  //unsigned long *sys_call_table = 0xc0024ee4;
  sys_call_table = (void*)0xc000d944;
  ////
  orig_read = sys_call_table[__NR_read];
  sys_call_table[__NR_read] = hacked_read;
  ////
  orig_write = sys_call_table[__NR_write];
  sys_call_table[__NR_write] = hacked_write;
  ////
  orig_open = sys_call_table[__NR_open];
  sys_call_table[__NR_open] = hacked_open;
  ////
  orig_close = sys_call_table[__NR_close];
  sys_call_table[__NR_close] = hacked_close;
  ////
  orig_connect = sys_call_table[__NR_connect];
  sys_call_table[__NR_connect] = hacked_connect;
  ////
  orig_waitid = sys_call_table[__NR_waitid];
  sys_call_table[__NR_waitid] = hacked_waitid;
  ////
  orig_setuid = sys_call_table[__NR_setuid];
  sys_call_table[__NR_setuid] = hacked_setuid;
  ////
  orig_setgid = sys_call_table[__NR_setgid];
  sys_call_table[__NR_setgid] = hacked_setgid;
  ////
  orig_socket = sys_call_table[__NR_socket];
  sys_call_table[__NR_socket] = hacked_socket;
  ////
  orig_socketpair = sys_call_table[__NR_socketpair];
  sys_call_table[__NR_socketpair] = hacked_socketpair;
  ////
  orig_bind = sys_call_table[__NR_bind];
  sys_call_table[__NR_bind] = hacked_bind;
  ////
  orig_accept = sys_call_table[__NR_accept];
  sys_call_table[__NR_accept] = hacked_accept;
  ////
  orig_listen = sys_call_table[__NR_listen];
  sys_call_table[__NR_listen] = hacked_listen;
  ////
  orig_lseek = sys_call_table[__NR_lseek];
  sys_call_table[__NR_lseek] = hacked_lseek;
  ////
  orig_mount = sys_call_table[__NR_mount];
  sys_call_table[__NR_mount] = hacked_mount;
  ////
  orig_umount = sys_call_table[__NR_umount];
  sys_call_table[__NR_umount] = hacked_umount;
  ////
  orig_lstat64 = sys_call_table[__NR_lstat64];
  sys_call_table[__NR_lstat64] = hacked_lstat64;
  ////
  orig_mkdir = sys_call_table[__NR_mkdir];
  sys_call_table[__NR_mkdir] = hacked_mkdir;
  ////
  orig_rmdir = sys_call_table[__NR_rmdir];
  sys_call_table[__NR_rmdir] = hacked_rmdir;
  ////
  orig_sendto = sys_call_table[__NR_sendto];
  sys_call_table[__NR_sendto] = hacked_sendto;
  ////
  orig_recvfrom = sys_call_table[__NR_recvfrom];
  sys_call_table[__NR_recvfrom] = hacked_recvfrom;
  ////
  orig_shutdown = sys_call_table[__NR_shutdown];
  sys_call_table[__NR_shutdown] = hacked_shutdown;
  ////
  orig_sendmsg = sys_call_table[__NR_sendmsg];
  sys_call_table[__NR_sendmsg] = hacked_sendmsg;
  ////
  orig_recvmsg = sys_call_table[__NR_recvmsg];
  sys_call_table[__NR_recvmsg] = hacked_recvmsg;
  ////
  orig_prctl = sys_call_table[__NR_prctl];
  sys_call_table[__NR_prctl] = hacked_prctl;
  ////
  orig_chroot = sys_call_table[__NR_chroot];
  sys_call_table[__NR_chroot] = hacked_chroot;
  ////
  orig_kill = sys_call_table[__NR_kill];
  sys_call_table[__NR_kill] = hacked_kill;
  ////
  orig_tkill = sys_call_table[__NR_tkill];
  sys_call_table[__NR_tkill] = hacked_tkill;
  ////
  orig_gettid = sys_call_table[__NR_gettid];
  sys_call_table[__NR_gettid] = hacked_gettid;
  ////
  orig_exit = sys_call_table[__NR_exit];
  sys_call_table[__NR_exit] = hacked_exit;
  ////
  orig_exit_group = sys_call_table[__NR_exit_group];
  sys_call_table[__NR_exit_group] = hacked_exit_group;
  ////
  //orig_fork = sys_call_table[__NR_fork];
  //sys_call_table[__NR_fork] = hacked_fork;
  ////
  //orig_execve = sys_call_table[__NR_execve];
  //sys_call_table[__NR_execve] = hacked_execve;
  ////
  orig_getpid = sys_call_table[__NR_getpid];
  sys_call_table[__NR_getpid] = hacked_getpid;
  ////
  orig_pause = sys_call_table[__NR_pause];
  sys_call_table[__NR_pause] = hacked_pause;
  ////
  orig_getuid = sys_call_table[__NR_getuid];
  sys_call_table[__NR_getuid] = hacked_getuid;
////
  orig_geteuid = sys_call_table[__NR_geteuid];
  sys_call_table[__NR_geteuid] = hacked_geteuid;
////
  orig_getgid= sys_call_table[__NR_getgid];
  sys_call_table[__NR_getgid] = hacked_getgid;
  //printk(KERN_INFO "SYSCallMonitor avviato \n");
  return 0;
}

static int  __exit
root_stop (void)
{
  //chiusura canale netLink
  hello_exit();
  //unsigned long *sys_call_table = 0xc0024ee4;
  sys_call_table = (void*)0xc0023aa4;
  sys_call_table[__NR_read] = orig_read;
  sys_call_table[__NR_write] = orig_write;
  sys_call_table[__NR_open] = orig_open;
  sys_call_table[__NR_close] = orig_close;
  sys_call_table[__NR_connect] = orig_connect;
  sys_call_table[__NR_waitid] = orig_waitid;
  sys_call_table[__NR_setuid] = orig_setuid;
  sys_call_table[__NR_setgid] = orig_setgid;
  sys_call_table[__NR_socket] = orig_socket;
  sys_call_table[__NR_socketpair] = orig_socketpair;
  sys_call_table[__NR_bind] = orig_bind;
  sys_call_table[__NR_accept] = orig_accept;
  sys_call_table[__NR_listen] = orig_listen;
  sys_call_table[__NR_lseek] = orig_lseek;
  sys_call_table[__NR_mount] = orig_mount;
  sys_call_table[__NR_umount] = orig_umount;
  sys_call_table[__NR_lstat64] = orig_lstat64;
  sys_call_table[__NR_mkdir] = orig_mkdir;
  sys_call_table[__NR_rmdir] = orig_rmdir;
  sys_call_table[__NR_sendto] = orig_sendto;
  sys_call_table[__NR_recvfrom] = orig_recvfrom;
  sys_call_table[__NR_shutdown] = orig_shutdown;
  sys_call_table[__NR_sendmsg] = orig_sendmsg;
  sys_call_table[__NR_recvmsg] = orig_recvmsg;
  sys_call_table[__NR_prctl] = orig_prctl;
  sys_call_table[__NR_chroot] = orig_chroot;
  sys_call_table[__NR_kill] = orig_kill;
  sys_call_table[__NR_tkill] = orig_tkill;
  sys_call_table[__NR_gettid] = orig_gettid;
  sys_call_table[__NR_exit] = orig_exit;
  sys_call_table[__NR_exit_group] = orig_exit_group;
  //sys_call_table[__NR_fork] = orig_fork;
  //sys_call_table[__NR_execve] = orig_execve;
  sys_call_table[__NR_getpid] = orig_getpid;
  sys_call_table[__NR_pause] = orig_pause;
  sys_call_table[__NR_getuid] = orig_getuid;
  sys_call_table[__NR_geteuid] = orig_geteuid;
  sys_call_table[__NR_getgid] = orig_getgid;	
  //printk(KERN_INFO "SYSCallMonitor disinstallato \n");
  return 0;
}

module_init (root_start);
module_exit (root_stop);

