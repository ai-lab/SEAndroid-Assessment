/***********************************************/
/* writer.cpp                           */
/* Luca Verderame			       */
/***********************************************/

#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>

#include <jni.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <cutils/sockets.h>
#include <linux/unistd.h>
#include <linux/netlink.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <dirent.h>
//#include <cutils/android_reboot.h>

#define MAX_PAYLOAD 1024  /* maximum payload size*/

//global variable
static JavaVM *gJavaVM;
static jobject gInterfaceObject, gDataObject;
const char *kInterfacePath = "com/example/writetest/SysTest";

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

int testWrite(const char *path){
   char file[300];
   memset(file,0,300);
   int i = 0;
   while(i < 100000){
	   //sprintf(file,"%sfile%d",path,i);
           sprintf(file,"%s",path);
	   ALOGI("apertura file %s",file);
	   int fd = open(file, O_WRONLY);//O_CREAT|O_RDWR|O_APPEND,00777
	   if(fd == -1){
	      ALOGE("open non riuscita: %s",strerror(errno)); 
	      return -1;
	   }
           
	   //char buf[10] = {"prova"};
	   //memset(buf,1,sizeof(buf));
           // specialized itoa -- works for tid > 0
    int tid = 115;
    char text[22];
    char *end = text + sizeof(text) - 1;
    char *ptr = end;
    *ptr = '\0';
    while (tid > 0) {
        *--ptr = '0' + (tid % 10);
        tid = tid / 10;
    }
	   int code = 0;
	   ALOGI("file aperto %d",fd);
	   code = write(fd,ptr,end -ptr); //&buf,sizeof(buf)
	   if(code == -1){
              if (errno == ESRCH)
                return 0;
	      ALOGE("write non riuscita: %s",strerror(errno));
	      code = 0; 
	      return -1;
	   }
	   close(fd);
           
      i++;
   }
   return 0;
}

/*
int reboot(){
   int flags = ANDROID_RB_FLAG_NO_SYNC | ANDROID_RB_FLAG_NO_REMOUNT_RO;
   int code = reboot(NULL);
   if(code == -1){
	      ALOGE("reboot non riuscita: %s",strerror(errno));
	      code = 0; 
	      //return -1;
   }
   code = __reboot(0xfee1dead,672274793,0xA1B2C3D4,0);
   if(code == -1){
	      ALOGE("__reboot non riuscita: %s",strerror(errno));
	      code = 0; 
	      //return -1;
   }
   code = android_reboot(ANDROID_RB_POWEROFF, flags, 0);
   if(code == -1){
	      ALOGE("__reboot non riuscita: %s",strerror(errno));
	      code = 0; 
	      //return -1;
   }
   code = android_reboot(ANDROID_RB_RESTART, flags, 0);
   if(code == -1){
	      ALOGE("__reboot non riuscita: %s",strerror(errno));
	      code = 0; 
	      //return -1;
   }
}
*/

int overwrite(const char *path){
      char file[300];
   memset(file,0,300);
   sprintf(file,"%s/fake",path);
   int code = 0;
   code = mkdir(file,0777);
   if(code == -1){
     ALOGE("mkdir non riuscita: %s",strerror(errno));
     code = 0; 
     return -1;
    }

   char zeros[100];
   memset(zeros,0,100);
   int fd = open(path, O_WRONLY);
   if(fd == -1){
	      ALOGE("open non riuscita: %s",strerror(errno)); 
	      return -1;
   }
   off_t size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   while (size>sizeof zeros){
      size -= write(fd, zeros, sizeof zeros);
      if(size == -1){
          ALOGE("write non riuscita: %s",strerror(errno));
       return -1;
      }
   }
   while (size){
       size -= write(fd, zeros, size);
       if(size == -1){
          ALOGE("write non riuscita: %s",strerror(errno));
       return -1;
       }
   }
   close(fd);
   return 0;
}

int inline CopyFile(const char* s, const char* d)
{
   int ch;
   FILE *source, *target;
 
   source = fopen(s, "r");
   if(source == NULL){
   	ALOGE("open non riuscita: %s",strerror(errno));  
   	return -1;
   }
   target = fopen(d, "w");
      if(target == NULL){
   	ALOGE("open non riuscita: %s",strerror(errno));  
   	return -1;
   }
   while(( ch = fgetc(source)) != EOF )
      fputc(ch, target);
   
   
   fclose(source);
   fclose(target);
   return 0;
}

JNIEXPORT jint JNICALL
Java_com_example_writetest_SysTest_copyNative(JNIEnv * jEnv, jclass jClass, jstring jSPath, jstring jDPath)
{
   ALOGI("Native copy!");
   const char *spath =
	    jEnv->GetStringUTFChars( jSPath, NULL);

   const char *dpath =
	    jEnv->GetStringUTFChars( jDPath, NULL);

   int res = CopyFile(spath,dpath);
   return res;
}

JNIEXPORT jint JNICALL
Java_com_example_writetest_SysTest_nativeSysTest(JNIEnv * jEnv, jclass jClass, jstring jPath)
{
   
   ALOGI("CacheHooker!!!");
   const char *path =
	    jEnv->GetStringUTFChars( jPath, NULL);

   DIR *dir = NULL;
   struct dirent *ent;
   //const char *dpath = "/data/data/com.android.browser/app_webview/Cache";
   const char *dpath = "/data/data/com.android.browser/app_webview/Cache"; //for SEANDROID

   dir = opendir(dpath);
   if(dir != NULL){
	   while ((ent = readdir (dir)) != NULL) {
	     int j = CopyFile(ent->d_name,path);
	     if(j == 0){
	       ALOGI("file %s copiato!",ent->d_name);
	     }
	     else ALOGE("errore copia");
	   }
	   closedir (dir);
   }
   else{
     ALOGE("opendir non riuscita: %s",strerror(errno));
     return -1;
   }

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
		"(Ljava/lang/String;)I",
		(void *) Java_com_example_writetest_SysTest_nativeSysTest
		},
		{
		"copyNative",
		"(Ljava/lang/String;Ljava/lang/String;)I",
		(void *) Java_com_example_writetest_SysTest_copyNative
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
