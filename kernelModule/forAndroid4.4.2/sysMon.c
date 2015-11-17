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
#include <linux/kthread.h>  // for threads

////////////////////////char device per la comunicazione con lo userspace////////////////////////////////////////
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include<linux/module.h>
#include<linux/init.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>

#include <linux/slab.h>

#define PARROT_MSG_FIFO_SIZE 8192 
#define PARROT_MSG_FIFO_MAX  137
#define MSG_SIZE 60 //300

MODULE_LICENSE("GPL");

//char my_data[80]="hi from kernel"; /* our device */

int readerID = 0;


int my_open(struct inode *inode,struct file *filep);
int my_release(struct inode *inode,struct file *filep);
ssize_t my_read(struct file* filp, char __user *buffer, size_t length, loff_t* offset);


/* A mutex will ensure that only one process accesses our device */
static DEFINE_MUTEX(parrot_device_mutex);
/* This table keeps track of each message length in the FIFO */
static unsigned int parrot_msg_len[PARROT_MSG_FIFO_MAX];
/* Read and write index for the table above */
static int parrot_msg_idx_rd, parrot_msg_idx_wr;


/* Use a Kernel FIFO for read operations */
static DECLARE_KFIFO(parrot_msg_fifo, char, PARROT_MSG_FIFO_SIZE);

struct mutex my_mutex; /* shared between the threads */
	
struct file_operations my_fops={
	open: my_open,
	read: my_read,
	//write: my_write,
	release:my_release,
};

int my_open(struct inode *inode,struct file *filep)
{
	/*MOD_INC_USE_COUNT;*/ /* increments usage count of module */
	return 0;
}

int my_release(struct inode *inode,struct file *filep)
{
	/*MOD_DEC_USE_COUNT;*/ /* decrements usage count of module */
	return 0;
}
ssize_t my_read(struct file* filp, char __user *buffer, size_t length, loff_t* offset)
{
	 int retval;
	 unsigned int copied;
	 if(readerID == 0)
              readerID = current->tgid;  //me lo salvo per dopo
	 if (kfifo_is_empty(&parrot_msg_fifo)) {
	  printk("no message in fifo\n");
	  return 0;
	 }
	 
	//retval = kfifo_to_user(&parrot_msg_fifo, buffer, MSG_SIZE, &copied);
	retval = kfifo_to_user(&parrot_msg_fifo, buffer, parrot_msg_len[parrot_msg_idx_rd], &copied);
 	/* Ignore short reads (but warn about them) */
 	if (parrot_msg_len[parrot_msg_idx_rd] != copied) {
  		printk("short read detected\n");
 	}
 	/* loop into the message length table */
 	parrot_msg_idx_rd = (parrot_msg_idx_rd+1)%PARROT_MSG_FIFO_MAX;

	mutex_unlock(&parrot_device_mutex); 	 
	return retval ? retval : copied;
}


/*ssize_t my_write(struct file *filep,const char *buff,size_t count,loff_t *offp )
{
	// function to copy user space buffer to kernel space
	if ( copy_from_user(my_data,buff,count) != 0 )
		printk( "Userspace -> kernel copy failed!\n" );
	return 0;
}*/

static int r_init(void);
static void r_cleanup(void);
static struct class *class_foo;
static struct device *dev_foo;

#define foo "foo"


static int r_init(void)
{
    int result=0;
    int major = 222;


    /* register_chrdev */
    result=register_chrdev(major,foo,&my_fops);
    if (result < 0)
    {
    printk(KERN_INFO "Registering device failed\n");
    return result;
    }
    
    class_foo = class_create(THIS_MODULE, foo);

    dev_foo = device_create(class_foo, NULL, MKDEV(major, 0), NULL, foo);

    /* This device uses a Kernel FIFO for its read operation */
    INIT_KFIFO(parrot_msg_fifo);
    mutex_init(&my_mutex); /* called only ONCE */

    //char a[] = "prova kernel";
    //kernel_write(a,sizeof(a));

    printk("Guten tag, SysMon driver initialized\n");
    return 0;



}
static void r_cleanup(void)
{
	printk("<1>bye\n");
	unregister_chrdev(222,"my_device");
	return ;
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



int kernel_write(const char* buff){

 unsigned int copied;
 size_t count = MSG_SIZE;

 if (!mutex_trylock(&my_mutex)) {
  	printk("another thred is accessing the device\n");
  	return -EBUSY;
 }
 
 if (kfifo_avail(&parrot_msg_fifo) < MSG_SIZE) {
  printk("not enough space left on fifo\n");
  mutex_unlock(&my_mutex);
  return -ENOSPC;
 }

 if ((parrot_msg_idx_wr+1)%PARROT_MSG_FIFO_MAX == parrot_msg_idx_rd) {
  /* We've looped into our message length table */
  printk("message length table is full\n");
  mutex_unlock(&my_mutex);
  return -ENOSPC;
 }
 
 /* The buffer is already in kernel space, so no need for ..._from_user() */
 copied = kfifo_in(&parrot_msg_fifo, buff, count);
 parrot_msg_len[parrot_msg_idx_wr] = copied;
 if (copied != count) {
  printk("short write detected\n");
 }
 parrot_msg_idx_wr = (parrot_msg_idx_wr+1)%PARROT_MSG_FIFO_MAX;

 mutex_unlock(&my_mutex);
 printk("write complete\n");
 return copied;

}



int writeUtil(char* type,char * c,char *param){
  
  char *msg = NULL;
  //int lenght = (strlen(c)<=MSG_SIZE) ? strlen(c) : MSG_SIZE;
  msg = kmalloc((sizeof(char) * (MSG_SIZE)), GFP_KERNEL);
  if(param != NULL)
  	sprintf(msg, "%s&%s&%s",type,current->comm,param);
  else
	sprintf(msg, "%s&%s",type,current->comm);
  kthread_run(kernel_write,msg,"sender");
  //kernel_write(msg,300);
  kfree(msg); 
  return 0;
  //debug
  //char a[] = "prova kernel";
  //kernel_write(a,sizeof(a));
}




asmlinkage ssize_t
hacked_read (int fd, char *buf, size_t count)
{
 if(s1 != current->tgid){
  //printk(KERN_INFO "SYS_READ:pid %d, %s \n",current->tgid,current->comm);
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d",fd,count);
  if(readerID != current->tgid) writeUtil("read",current->comm,param);
  s1 = current->tgid;
  kfree(param);
  */
  if(readerID != current->tgid) writeUtil("read",current->comm,NULL); 
 }
 return orig_read(fd,buf,count);	
}

asmlinkage ssize_t
hacked_write (int fd, char *buf, size_t count)
{
 if(s2 != current->tgid){
   s2 = current->tgid;
   //printk(KERN_INFO "SYS_WRITE:pid %d, %s \n",current->tgid,current->comm);
   /*char *param = NULL;
   param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
   sprintf(param, "%d:%s:%d",fd,buf,count);
   if(readerID != current->tgid) writeUtil("write",current->comm,param);
   */
   if(readerID != current->tgid) writeUtil("write",current->comm,NULL);
 }
 return orig_write(fd,buf,count);	
}

asmlinkage ssize_t
hacked_open (const char *pathname, int flags)
{
  if(s3 != current->tgid){
    //printk(KERN_INFO "SYS_OPEN:pid %d, %s \n",current->tgid,current->comm);
    s3 = current->tgid; 
   /*char *param = NULL;
   param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
   sprintf(param, "%s:%d",pathname,flags);
   if(readerID != current->tgid) writeUtil("open",current->comm,param); 
   */
   if(readerID != current->tgid) writeUtil("open",current->comm,NULL); 
 }
 return orig_open(pathname,flags);	
}

asmlinkage ssize_t
hacked_close (int fd)
{
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d",fd);
  if(readerID != current->tgid) writeUtil("close",current->comm,param);*/
  if(readerID != current->tgid) writeUtil("close",current->comm,NULL);
 return orig_close(fd);	
}


asmlinkage long 
hacked_umount (char __user *name, int flags)
{
 //printk(KERN_INFO "SYS_UMOUNT:pid %d, %s \n",current->tgid,current->comm);
  /*char *p;
  long len;
  int ret;
  len = strlen_user(name);
  p = kmalloc(len, GFP_KERNEL);
  if (p){
	  ret = strncpy_from_user(p, name, len);
	  if(ret > 0 ){
                char *param;
                param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
                if(param){
		  	sprintf(param, "%s:%d",p,flags);
		  	if(readerID != current->tgid) writeUtil("umount",current->comm,param);
		        kfree(param);
                }
	  }
    kfree(p);
  }
  */
  if(readerID != current->tgid) writeUtil("umount",current->comm,NULL);
 return orig_umount(name,flags);	
}

asmlinkage long 
hacked_lstat64 (char __user *filename,
				struct stat64 __user *statbuf)
{
 //printk(KERN_INFO "SYS_LSTAT64:pid %d, %s \n",current->tgid,current->comm);
  /*char *p;
  long len;
  int ret;
  len = strlen_user(filename);
  p = kmalloc(len, GFP_KERNEL);
  if (p){
	  ret = strncpy_from_user(p, filename, len);
	  if(ret > 0 ){
                char *param;
                param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
                if(param){
		  	sprintf(param, "%s",p);
		  	if(readerID != current->tgid) writeUtil("lstat64",current->comm,param);
		        kfree(param);
                }
	  }
    kfree(p);
  }
*/
   if(readerID != current->tgid) writeUtil("lstat64",current->comm,NULL);
 return orig_lstat64(filename,statbuf);	
}




asmlinkage long 
hacked_shutdown(int a, int b)
{
  //printk(KERN_INFO "SYS_SHUTDOWN:pid %d, %s \n",current->tgid,current->comm);
/*  char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d",a,b);
  if(readerID != current->tgid) writeUtil("shutdown",current->comm,param);
  kfree(param);
*/
 if(readerID != current->tgid) writeUtil("shutdown",current->comm,NULL);
 return orig_shutdown(a,b);	
}

asmlinkage long 
hacked_prctl(int option, unsigned long arg2, unsigned long arg3,
			unsigned long arg4, unsigned long arg5)
{
 /* char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%lu:%lu:%lu:%lu",option,arg2,arg3,arg4,arg5);
  if(readerID != current->tgid) writeUtil("prctl",current->comm,param);
  kfree(param);
 //printk(KERN_INFO "SYS_PRCTL:pid %d, %s \n",current->tgid,current->comm);
*/
  if(readerID != current->tgid) writeUtil("prctl",current->comm,NULL);
 return orig_prctl(option,arg2,arg3,arg4,arg5);	
}

asmlinkage long 
hacked_mkdir (const char __user *pathname, int mode)
{
 //printk(KERN_INFO "SYS_MKDIR:pid %d, %s \n",current->tgid,current->comm);
  /*char *p;
  long len;
  int ret;
  len = strlen_user(pathname);
  p = kmalloc(len, GFP_KERNEL);
  if (p){
	  ret = strncpy_from_user(p, pathname, len);
	  if(ret > 0 ){
                char *param;
                param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
                if(param){
		  	sprintf(param, "%s:%d",p,mode);
		  	if(readerID != current->tgid) writeUtil("mkdir",current->comm,param);
		        kfree(param);
                }
	  }
    kfree(p);
  }
 */
   if(readerID != current->tgid) writeUtil("mkdir",current->comm,NULL);
  return orig_mkdir(pathname,mode);	
}

asmlinkage long 
hacked_rmdir (const char __user *pathname)
{
  //printk(KERN_INFO "SYS_RMDIR:pid %d, %s \n",current->tgid,current->comm);
  /*char *p;
  long len;
  int ret;
  len = strlen_user(pathname);
  p = kmalloc(len, GFP_KERNEL);
  if (p){
	  ret = strncpy_from_user(p, pathname, len);
	  if(ret > 0 ){
                char *param;
                param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
                if(param){
		  	sprintf(param, "%s",p);
		  	if(readerID != current->tgid) writeUtil("rmdir",current->comm,param);
		        kfree(param);
                }
	  }
    kfree(p);
  }
*/
  if(readerID != current->tgid) writeUtil("rmdir",current->comm,NULL);
 return orig_rmdir(pathname);	
}

asmlinkage long 
hacked_chroot(const char __user *filename)
{
  
  /*char *p;
  long len;
  int ret;
  len = strlen_user(filename);
  p = kmalloc(len, GFP_KERNEL);
  if (p){
	  ret = strncpy_from_user(p, filename, len);
	  if(ret > 0 ){
                char *param;
                param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
                if(param){
		  	sprintf(param, "%s",p);
		  	if(readerID != current->tgid) writeUtil("chroot",current->comm,param);
		        kfree(param);
                }
	  }
    kfree(p);
  }
  */
  if(readerID != current->tgid) writeUtil("chroot",current->comm,NULL);
 //printk(KERN_INFO "SYS_CHROOT:pid %d, %s \n",current->tgid,current->comm);
 return orig_chroot(filename);	
}

asmlinkage long 
hacked_kill (int pid, int sig)
{
 //printk(KERN_INFO "SYS_KILL:pid %d, %s \n",current->tgid,current->comm);
 /* char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d",pid,sig);
  if(readerID != current->tgid) writeUtil("kill",current->comm,param);
  kfree(param);
 */
  if(readerID != current->tgid) writeUtil("kill",current->comm,NULL);
 return orig_kill(pid,sig);	
}

asmlinkage long 
hacked_tkill (int pid, int sig)
{
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d",pid,sig);
  if(readerID != current->tgid) writeUtil("tkill",current->comm,param);
  kfree(param); 
 //printk(KERN_INFO "SYS_TKILL:pid %d, %s \n",current->tgid,current->comm);
 */
  if(readerID != current->tgid) writeUtil("tkill",current->comm,NULL);
 return orig_tkill(pid,sig);	
}

asmlinkage long
hacked_gettid(void)
{
 /* if(readerID != current->tgid) writeUtil("gettid",current->comm,NULL); 
 //printk(KERN_INFO "SYS_GETTID:pid %d, %s \n",current->tgid,current->comm);
 */
  if(readerID != current->tgid) writeUtil("gettid",current->comm,NULL);
 return orig_gettid();	
}

asmlinkage long
hacked_exit(int error_code)
{
 /* char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d",error_code);
  if(readerID != current->tgid) writeUtil("exit",current->comm,param);
  kfree(param); 
  */
  if(readerID != current->tgid) writeUtil("exit",current->comm,NULL); 
 //printk(KERN_INFO "SYS_EXIT:pid %d, %s \n",current->tgid,current->comm);
 return orig_exit(error_code);	
}

asmlinkage long
hacked_exit_group(int error_code)
{
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d",error_code);
  if(readerID != current->tgid) writeUtil("exit_group",current->comm,param);
  kfree(param); 
 */
  if(readerID != current->tgid) writeUtil("exit_group",current->comm,NULL);
 return orig_exit_group(error_code);	
}

asmlinkage long
hacked_getpid (void)
{
 if(readerID != current->tgid) writeUtil("getpid",current->comm,NULL);
 //printk(KERN_INFO "SYS_GETPID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_pause (void)
{
 //printk(KERN_INFO "SYS_PAUSE:pid %d, %s \n",current->tgid,current->comm);
 if(readerID != current->tgid) writeUtil("pause",current->comm,NULL);
 return orig_pause();	
}

asmlinkage long
hacked_getuid (void)
{
 if(readerID != current->tgid) writeUtil("getuid",current->comm,NULL);
 //printk(KERN_INFO "SYS_GETUID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_geteuid (void)
{
 if(readerID != current->tgid) writeUtil("geteuid",current->comm,NULL);
 //printk(KERN_INFO "SYS_GETEUID:pid %d, %s \n",current->tgid,current->comm);
 return orig_getpid();	
}

asmlinkage long
hacked_getgid (void)
{
 if(readerID != current->tgid) writeUtil("getgid",current->comm,NULL);
 return orig_getpid();	
}

asmlinkage long
hacked_setuid(uid_t uid)
{
 //printk(KERN_INFO "SYS_SETUID:pid %d, %s \n",current->tgid,current->comm);
 /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d",uid);
  if(readerID != current->tgid) writeUtil("setuid",current->comm,param);
  kfree(param);
*/
  if(readerID != current->tgid) writeUtil("setuid",current->comm,NULL);
 return orig_setuid(uid);	
}

asmlinkage long
hacked_setgid (gid_t gid)
{
 //printk(KERN_INFO "SYS_SETGID:pid %d, %s \n",current->tgid,current->comm);
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d",gid);
  if(readerID != current->tgid) writeUtil("setgid",current->comm,param);
  kfree(param);
 */
  if(readerID != current->tgid) writeUtil("setgid",current->comm,NULL);
 return orig_setgid(gid);	
}


///TODO

asmlinkage long
hacked_connect (int a, struct sockaddr __user *b, int c)
{
 //printk(KERN_INFO "SYS_CONNECT:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("connect",current->comm,NULL);
 return orig_connect(a,b,c);	
}

asmlinkage long
hacked_waitid (int which, pid_t pid, struct siginfo __user *infop, int options, struct rusage __user *ru)
{
 //printk(KERN_INFO "SYS_WAITID:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("waitid",current->comm,NULL);
 return orig_waitid(which,pid,infop,options,ru);	
}

asmlinkage long
hacked_socket (int a,int b,int c)
{
 //printk(KERN_INFO "SYS_SOCKET:pid %d, %s \n",current->tgid,current->comm);
 /* char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d:%d",a,b,c);
  if(readerID != current->tgid) writeUtil("socket",current->comm,param);
  kfree(param);
 */
  if(readerID != current->tgid) writeUtil("socket",current->comm,NULL);
 return orig_socket(a,b,c);	
}

asmlinkage long 
hacked_socketpair (int a , int b, int c , int __user * d)
{
 //printk(KERN_INFO "SYS_SOCKETPAIR:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("socketpair",current->comm,NULL);
 return orig_socketpair(a,b,c,d);	
}

asmlinkage long 
hacked_bind(int a, struct sockaddr __user *b, int c)
{
 //printk(KERN_INFO "SYS_BIND:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("bind",current->comm,NULL);
 return orig_bind(a,b,c);	
}

asmlinkage long 
hacked_accept (int a, struct sockaddr __user * b, int __user * c)
{
 //printk(KERN_INFO "SYS_ACCEPT:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("accept",current->comm,NULL);
 return orig_accept(a,b,c);	
}

asmlinkage long 
hacked_listen (int a, int b)
{
 //printk(KERN_INFO "SYS_LISTEN:pid %d, %s \n",current->tgid,current->comm);
  /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%d",a,b);
  if(readerID != current->tgid) writeUtil("listen",current->comm,param);
  kfree(param);
 */
  if(readerID != current->tgid) writeUtil("listen",current->comm,NULL);
 return orig_listen(a,b);	
}

asmlinkage long 
hacked_lseek(unsigned int fd, off_t offset,unsigned int origin)
{
 //printk(KERN_INFO "SYS_LSEEK:pid %d, %s \n",current->tgid,current->comm);
 /*char *param = NULL;
  param = kmalloc((sizeof(char) * (MSG_SIZE-100)), GFP_KERNEL);
  sprintf(param, "%d:%ld:%d",fd,(long int) offset,origin);      //NON FUNZIONA OFFSET
  if(readerID != current->tgid) writeUtil("lseek",current->comm,param);
  kfree(param);*/
  if(readerID != current->tgid) writeUtil("lseek",current->comm,NULL);
 return orig_lseek(fd,offset,origin);	
}

asmlinkage long 
hacked_mount(char __user *dev_name, char __user *dir_name,
				char __user *type, unsigned long flags,
				void __user *data)
{
 //printk(KERN_INFO "SYS_MOUNT:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("mount",current->comm,NULL);
 return orig_mount(dev_name,dir_name,type,flags,data);	
}

asmlinkage long
hacked_fork(struct pt_regs *a)
{
 //printk(KERN_INFO "SYS_FORK:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("fork",current->comm,NULL);
 return orig_fork(a);	
}

asmlinkage long
hacked_execve(char __user * a, char __user *__user * b,
			  char __user *__user * c, struct pt_regs *d)
{
 //printk(KERN_INFO "SYS_EXECVE:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("execve",current->comm,NULL);
 return orig_execve(a,b,c,d);	
}

asmlinkage long 
hacked_sendto (int x, void __user * a, size_t b, unsigned c,
				struct sockaddr __user * d, int e)
{
 //printk(KERN_INFO "SYS_SENDTO:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("sendto",current->comm,NULL);
 return orig_sendto(x,a,b,c,d,e);	
}

asmlinkage long 
hacked_recvfrom (int x, void __user * a, size_t b, unsigned c,
				struct sockaddr __user * d, int __user * e)
{
 //printk(KERN_INFO "SYS_RECVFROM:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("recvfrom",current->comm,NULL);
 return orig_recvfrom(x,a,b,c,d,e);	
}

asmlinkage long 
hacked_sendmsg (int fd, struct msghdr __user *msg, unsigned flags)
{
 //printk(KERN_INFO "SYS_SENDMSG:pid %d, %s \n",current->tgid,current->comm);
  if(readerID != current->tgid) writeUtil("sendmsg",current->comm,NULL);
 return orig_sendmsg(fd,msg,flags);	
}

asmlinkage long 
hacked_recvmsg (int fd, struct msghdr __user *msg, unsigned flags)
{
 //printk(KERN_INFO "SYS_RECVMSG:pid %d, %s \n",current->tgid,current->comm);
 //char * m = "recvmsg";
 //send(m);
  if(readerID != current->tgid) writeUtil("recvmsg",current->comm,NULL);
 return orig_recvmsg(fd,msg,flags);	
}


void **sys_call_table; //added

static int  __init
root_start (void)
{
  //avvio canale netLink
  //hello_init();
  
  r_init();

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
  //hello_exit();

  r_cleanup();
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

