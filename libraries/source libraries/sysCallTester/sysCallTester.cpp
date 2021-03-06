/***********************************************/
/* sysCallTester.cpp                           */
/* Luca Verderame			       */
/***********************************************/

#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>

#include <jni.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h> //per le operazioni sui file
//#include <pthread.h> // per creare un nuovo thread;
#include <cutils/sockets.h>
//#include <cutils/zygote.h>
#include <sys/prctl.h>
#include <signal.h>
#include <linux/unistd.h>

#define ANDROID_RESERVED_SOCKET_PREFIX "/dev/socket/"


//global variable
static JavaVM *gJavaVM;
static jobject gInterfaceObject, gDataObject;
const char *kInterfacePath = "com/systest/SysTest";

#ifdef __cplusplus
extern "C" {
#endif

static void callback_handler(char *s) {
    int status;
    JNIEnv *env;
    bool isAttached = false;
   
    status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if(status < 0) {
        ALOGE("callback_handler: failed to get JNI environment, "
             "assuming native thread");
        status = gJavaVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            ALOGE("callback_handler: failed to attach "
                 "current thread");
            return;
        }
        isAttached = true;
    }
    /* Construct a Java string */
    jstring js = env->NewStringUTF(s);
    jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
    if(!interfaceClass) {
        ALOGE("callback_handler: failed to get class reference");
        if(isAttached) gJavaVM->DetachCurrentThread();
        return;
    }
    /* Find the callBack method ID */
    jmethodID method = env->GetStaticMethodID(
        interfaceClass, "callBack", "(Ljava/lang/String;)V");
    if(!method) {
        ALOGE("callback_handler: failed to get method ID");
        if(isAttached) gJavaVM->DetachCurrentThread();
        return;
    }
    env->CallStaticVoidMethod(interfaceClass, method, js);
    if(isAttached) gJavaVM->DetachCurrentThread();
}
void *native_thread_start(void *arg) {
    sleep(1);
    callback_handler((char *) "Called from native thread");
    return 0;
}

int testScrittura(){
   int fd;
   int log;
   int code;
   //apertura file
   fd = open("/dev/sysCon",O_RDWR);
   if(fd == -1) {
     ALOGE("open sysCon non riuscita: %s",strerror(errno));
     return -1;
   }
   log = open("/data/data/com.systest/log",O_CREAT|O_RDWR|O_APPEND,00777);
   if(log == -1) {
     ALOGE("open log non riuscita: %s",strerror(errno));
     return -1;
   }
   //variabili
   char buf[1000];
   for(int j=0;j<10;j++){
        //lettura da driver
   	code = read(fd,&buf,1000);
   	if(code == -1) {
     		ALOGE("read non riuscita: %s",strerror(errno));
     		code = 0;
   	}
        //ALOGI("lettura n: %d di %d",j,code);
        //ALOGI("roba letta: %s",buf);
        //parse dati
        char tmp[100] = "";
        int cur = 0;
        for(int m=0;m<code;m++){

           if(buf[m] != '!'){
              tmp[cur] = buf[m]; //è una linea continua
              cur++;
           }
           else{ //finita la linea perchè leggo !
              ALOGI("temp: %s",tmp);
              char *action;
              char *pid;
              char *name;
              //prendo i dati
              action = strtok(tmp,"<");  //<f>
              pid = strtok(NULL,"<");  
              name = strtok(NULL,"<");
              //esecuzione dell'action da parte del programma
              
              //scrittura nel log
              char ll[100] = "";
              strcat(ll,"azione: ");
              strcat(ll,action);
	      strcat(ll," pid: ");
              strcat(ll,pid);
              strcat(ll," nome: ");
              strcat(ll,name);
              strcat(ll,"\n");
              ALOGI("logLine: %s",ll);
              memset(&tmp[0],0,sizeof(tmp));
              cur = 0;
              write(log,ll,strlen(ll));
          }
       }//fine for parsing buffer lettura;
       memset(&buf[0], 0, sizeof(buf));
   } //fine for tentativi
   ALOGI("fine tentativi");
   close(fd);
   close(log); 
   return 0;
}


int test(){
   int tid = gettid();
   ALOGI("processo: %d",tid); 
   getpid();
   int code = 0;
  
   code = prctl(PR_SET_NAME,"<null> terminated string",0,0,0);
   if(code == -1){
     ALOGE("PRCTL non riuscita %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("PRCTL riuscita");

   code = mkdir("/data/data/com.systest/prova",0777);
   if(code == -1){
     ALOGE("MKDIR non riuscita %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("MKDIR riuscita");
   struct stat buf;
   code = lstat64("/data/data/com.systest/",&buf);
   if(code == -1) {
     ALOGE("LSTAT64 non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("LSTAT64 riuscita");

   /////////////operazioni sui file//////////////////////
   int fd = 0;
   fd = open("/data/data/com.systest/file",O_CREAT|O_RDWR,00777);
   //fd = open ("/dev/sysCon",O_RDWR);
   if(fd == -1) {
     ALOGE("open non riuscita: %s",strerror(errno));
     return -1;
   }
   else
     ALOGI("OPEN riuscita");
   const char ba[6] = "prova";
   code = write(fd,ba,sizeof(ba));
   if(code == -1) {
     ALOGE("write non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("WRITE riuscita");

   code = lseek(fd,-sizeof(ba),SEEK_END);
   if(code == -1) {
     ALOGE("lseek non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("LSEEK riuscita");
   char b[30];
   code = read(fd,&b,30);

   if(code == -1) {
     ALOGE("read non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("READ riuscita");

   ALOGI("risultato read: %s",b);
   code = close(fd);
   if(code == -1) {
     ALOGE("close non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("CLOSE riuscita");
   /////////////////////////////////////////////////

   ///////operazioni sui socket/////////////////////
   int f = socket(AF_LOCAL,SOCK_STREAM,0);
   if(f == -1) {
     ALOGE("socket non riuscita: %s",strerror(errno));
     return -1;
   }
   else
     ALOGI("SOCKET riuscita");
   /*
   struct sockaddr_un sa;
   socklen_t salen;
   bzero(&sa, sizeof(sa));
   sa.sun_family = AF_UNIX;
   strcpy(sa.sun_path, "127.0.0.1");
   #if !defined(__FreeBSD__)
	salen = strlen(sa.sun_path) + sizeof(sa.sun_family);
	#else
	salen = SUN_LEN(&sa);
	sa.sun_len = salen;
   #endif
   */
    struct sockaddr_un p_addr;
    socklen_t alen;
    size_t namelen;
    namelen = strlen("zygote") + strlen(ANDROID_RESERVED_SOCKET_PREFIX);
    /* unix_path_max appears to be missing on linux */
    //if (namelen > sizeof(p_addr) - offsetof(struct sockaddr_un, sun_path) - 1) {
    //            ALOGE("errore namelen");
    //}
    strcpy(p_addr.sun_path, ANDROID_RESERVED_SOCKET_PREFIX);
    strcat(p_addr.sun_path, "zygote");
    p_addr.sun_family = AF_LOCAL;
    alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

   struct timeval tv;

   tv.tv_sec = 5;  /* 5 Secs Timeout */
   tv.tv_usec = 0;  // Not init'ing this can cause strange errors

   if(setsockopt(f, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))){
              ALOGE("SetSockOption non riuscito");
              return -1;
   }

   //connect
   code = connect(f, (struct sockaddr *)&p_addr, alen);
   if(code == -1) {
     ALOGE("connect non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("CONNECT riuscita");

   //bind
   code = bind(f,(struct sockaddr *)&p_addr, alen);
   if(code == -1) {
     ALOGE("bind non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("BIND riuscita");
   /*
   //listen
   code = listen(f,2);
   if(code == -1) {
     ALOGE("listen non riuscita: %s",strerror(errno));
     code = 0;
   }
   */
   
   //sendto
   char msg[10] = "prova2";
   code = sendto(f,msg,strlen(msg),0,(struct sockaddr *)&p_addr, alen);
   if(code == -1) {
     ALOGE("sendto non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("SENDTO riuscita");
      
   //codice preliminare per gestire i messaggi
   struct iovec   iov[1];
   struct msghdr  m;
   char   buffer[10];
   memset(&m,   0, sizeof(m));
   memset(iov,    0, sizeof(iov));
   iov[0].iov_base = buffer;
   iov[0].iov_len  = sizeof(buffer);
   m.msg_iov     = iov;
   m.msg_iovlen  = 1;
   
   
   //sendmsg
   code = sendmsg(f,&m,0);
   if(code == -1) {
     ALOGE("sendmsg non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("SENDMSG riuscita");

   //recvfrom
   char recmsg[100];
   code = recvfrom(f,recmsg,strlen(recmsg),0,(struct sockaddr *)&p_addr,&alen);
   if(code == -1) {
     ALOGE("recvfrom non riuscita: %s",strerror(errno)); 
     code = 0;
   }
   else
     ALOGI("RECVFROM riuscita");

   //recvmsg
   code = recvmsg(f,&m,0);
   if(code == -1) {
     ALOGE("recvmsg non riuscita: %s",strerror(errno));
     code = 0;
   }
   else
     ALOGI("RECVMSG riuscita");
  //////////////////////////////////////////////////////////
   code = kill(539,SIGUSR1);
   if(code == -1) {
     ALOGE("kill non riuscita: %s",strerror(errno)); 
     code = 0;
   }
   else
     ALOGI("KILL riuscita");
   exit(0);
   ALOGI("EXIT riuscita");
   //exit_group(0);
}
//fine funz test






JNIEXPORT jint JNICALL
Java_com_systest_SysTest_nativeSysTest(JNIEnv * jEnv, jclass jClass)
{
   ALOGI("sysCallTest nativa in funzione");
   test();
   return 0;  
}
//fine nativeTest



// parte per android

void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
jclass cls = env->FindClass(path);
	if(!cls) {
	ALOGE("initClassHelper: failed to get %s class reference", path);
	return;
	}
jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
	if(!constr) {
	ALOGE("initClassHelper: failed to get %s constructor", path);
	return;
	}
jobject obj = env->NewObject(cls, constr);
	if(!obj) {
	ALOGE("initClassHelper: failed to create a %s object", path);
	return;
	}
(*objptr) = env->NewGlobalRef(obj);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
		JNIEnv *env;
		gJavaVM = vm;
		ALOGI("JNI_OnLoad called");
		if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
			ALOGE("Failed to get the environment using GetEnv()");
			return -1;
		}
		//Class instance caching 12b
		//ALOGI("step 1");
		initClassHelper(env, kInterfacePath, &gInterfaceObject);
		//initClassHelper(env, kDataPath, &gDataObject);
		//ALOGI("step 2");
		//Native function registration 13
		JNINativeMethod methods[] = {
		{
		"nativeSysTest",
		"()I",
		(void *) Java_com_systest_SysTest_nativeSysTest
		},
		};

		if(android::AndroidRuntime::registerNativeMethods(
			env, kInterfacePath, methods, NELEM(methods)) != JNI_OK) {
			ALOGE("Failed to register native methods");
			return -1;
		}
		ALOGI("JNI_OnLoad finished");
		return JNI_VERSION_1_4;
}


#ifdef __cplusplus
}
#endif
