/***********************************************/
/* sysCallTester.cpp                           */
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
#include <errno.h>

#define MAX_PAYLOAD 1024  /* maximum payload size*/


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

void replay(char *msg){
   char *p = NULL;
   p = strtok(msg,"<");
   if(p != NULL){
      //blocco base
      if(strcmp(p,"getpid")){
        getpid();
        p = strtok(NULL,"<");
        ALOGI("getpid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"getuid")){
        getuid();
        p = strtok(NULL,"<");
        ALOGI("getuid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"geteuid")){
        geteuid();
        p = strtok(NULL,"<");
        ALOGI("geteuid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"getgid")){
        getgid();
        p = strtok(NULL,"<");
        ALOGI("getgid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"gettid")){
        gettid();
        p = strtok(NULL,"<");
        ALOGI("gettid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"")){
        p = strtok(NULL,"<");
        ALOGI("getpid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"")){
        p = strtok(NULL,"<");
        ALOGI("getpid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"")){
        p = strtok(NULL,"<");
        ALOGI("getpid from %s",p);
        return;
      }
      //blocco base
      if(strcmp(p,"")){
        p = strtok(NULL,"<");
        ALOGI("getpid from %s",p);
        return;
      }
  }//fine if p != null
}

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define MSG 60

JNIEXPORT jint JNICALL
Java_com_systest_SysTest_nativeSysTest(JNIEnv * jEnv, jclass jClass)
{
   ALOGI("test comunicazione NetLink in funzione");
   int fd=0,ret=0;
   FILE *stat = NULL;
   int times = 0;	
	
	fd=open("/dev/foo",O_RDONLY);
	stat=fopen("/mnt/sdcard/log.txt","w+");
        
        if(fd < 0){                   
		ALOGE("fd :%d\n",fd);
                return -1;
        }
        if(stat == NULL){
		ALOGE("stat is null!\n");
                return -1;
        }
	char buff[MSG+1]="";
        int hit = 0;
        while(times < 150000 && hit < 100000){
                ret = read(fd,buff,MSG); 
		buff[ret]='\0';
                if(ret>=0){
                	//replay(buff);
			if(!(strstr(buff,"logcat") != NULL || strstr(buff,"adbd") != NULL)){
				if(strstr(buff,"AsyncTask") == NULL){ //evito quelli di debug che non sono interessanti
					ALOGI("%s",buff);
                        		fprintf(stat,"%s \n",buff);
 					hit +=1;
                                }
                        }
                }
                times+=1;
	}

	close(fd);
        fclose(stat);
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
