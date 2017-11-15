#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

#include "hook.h"

static const char* TAG = "HOOKTEST";

#define LOGD(fmt, args...)  \
	__android_log_print( ANDROID_LOG_DEBUG, TAG, fmt, ##args)

#define LOGE(fmt, args...)  \
	__android_log_print( ANDROID_LOG_ERROR, TAG, fmt, ##args)

#define LIBART_PATH "/system/lib64/libart.so"
#define LIBDVM_PATH "/system/lib/libdvm.so"
#define LIBART "libart.so"
#define LIBDVM "libdvm.so"

static const char* JAR_PATH = "/data/data/com.example.maoshuoqiong.sendmessage/cache/handler.jar";

typedef jint JNI_GetCreatedJavaVMs_Method(JavaVM** , jsize, jsize* );

static JNIEnv* getEnv()
{

	JNIEnv* env = NULL;
	const char* szLib = LIBDVM;
	if(access(LIBART_PATH, F_OK) == 0)
		szLib = LIBART;

	void* hand = NULL;
	if(( hand = dlopen(szLib, RTLD_NOW) ) == NULL)
	{
		LOGE("Libraray %s open fail: %s", szLib, dlerror());
		return NULL;	
	}	

	JNI_GetCreatedJavaVMs_Method *p = (JNI_GetCreatedJavaVMs_Method*)dlsym(hand, "JNI_GetCreatedJavaVMs");
	if(p == NULL)
	{
		LOGE("find JNI_GetCreatedJavaVMs error: %s", dlerror());
		goto exit1;
	}

	JavaVM * vms = NULL;
	jsize vm_count;
	jsize count = 1;
	p(&vms, count, &vm_count);
	LOGD("JavaVm = %p, count = [%d]\n", vms, vm_count);

	if((*vms)->GetEnv(vms,(void**)&env, JNI_VERSION_1_4) != JNI_OK)
		LOGE("GetEnv error");
	LOGD("JNIEnv = %p", env);

exit1:
	if(dlclose(hand)!= 0)
		LOGE("dlclose error: %s", dlerror());
exit2:
	return env;
} 

static jobject getcontext(JNIEnv * env)
{
    LOGD("getcontext");
    /* 获取Activity Thread的实例对象 */
    jclass activityThread = (*env)->FindClass(env,"android/app/ActivityThread");
    jmethodID currentActivityThread = (*env)->GetStaticMethodID(env,activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = (*env)->CallStaticObjectMethod(env,activityThread, currentActivityThread);
    if(at == NULL) {
        LOGE("getcurrent thread error");
        return NULL;
    }

    LOGD("Get application");
    /* 获取Application，也就是全局的Context */
    jmethodID getApplication = (*env)->GetMethodID(env,activityThread, "getApplication", "()Landroid/app/Application;");
    jobject context = (*env)->CallObjectMethod(env,at, getApplication);

    LOGD("context: %p", context);
    return context;
}


jobject obtainMessage(JNIEnv * env)
{
	jobject context = getcontext(env);
	jstring jarpath = (*env)->NewStringUTF(env, JAR_PATH); /* /data/local/tmp/test.jar */
    jstring classname = (*env)->NewStringUTF(env, "com.nmcm.sms.sendreceiverlibraray.SendResultHandle");

    jclass fileclass = (*env)->FindClass(env, "java/io/File");
    jmethodID getabsolutepath = (*env)->GetMethodID(env,fileclass, "getAbsolutePath", "()Ljava/lang/String;");

    jclass applicationclass = (*env)->FindClass(env, "android/content/ContextWrapper");
    jmethodID getcachedir   = (*env)->GetMethodID(env, applicationclass, "getCacheDir","()Ljava/io/File;");
    jobject cachedir = (*env)->CallObjectMethod(env, context, getcachedir);
    jstring cachepath = (*env)->CallObjectMethod(env, cachedir, getabsolutepath);
	const char* tmp= (*env)->GetStringUTFChars(env, cachepath, 0);
	LOGD("cachepath [%s]", tmp);
	(*env)->ReleaseStringUTFChars(env, cachepath, tmp);

//    jstring cachepath = (*env)->NewStringUTF(env, "/data/local/tmp");

    jclass classloaderclass = (*env)->FindClass(env,"java/lang/ClassLoader");
    jmethodID getclassloader = (*env)->GetStaticMethodID(env,classloaderclass,"getSystemClassLoader","()Ljava/lang/ClassLoader;");
    jobject classloader = (*env)->CallStaticObjectMethod(env, classloaderclass,getclassloader);
    LOGD("classloader: %p", classloader);

    jclass dexloaderclass = (*env)->FindClass(env, "dalvik/system/DexClassLoader");
	if(dexloaderclass == NULL)
		return NULL;

    jmethodID loadclass = (*env)->GetMethodID(env,dexloaderclass, "loadClass","(Ljava/lang/String;)Ljava/lang/Class;");
	if(loadclass == NULL)
		return NULL;

    jmethodID createdxclassloader = (*env)->GetMethodID(env, dexloaderclass, "<init>",
                                                        "(Ljava/lang/String;Ljava/lang/String;"
                                                                "Ljava/lang/String;Ljava/lang/ClassLoader;)V");

	if(createdxclassloader == NULL)
		return NULL;

	LOGD("create dexclassloader");

    jobject dexclassloader = (*env)->NewObject(env, dexloaderclass, createdxclassloader, jarpath, cachepath, NULL, classloader );
    if((*env)->ExceptionCheck(env))
    {
        LOGE("create dexclassloader exception");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return NULL;
    }

    if(dexclassloader == NULL)
        return NULL;
	else
    	LOGD("dexclassloader: %p", dexclassloader);
		

    jclass handleclass = (jclass)(*env)->CallObjectMethod(env, dexclassloader, loadclass, classname);
    if((*env)->ExceptionCheck(env))
    {
        LOGE("findclass exception");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return NULL;
    }

	LOGD("create handler");
    jmethodID createHandler = (*env)->GetMethodID(env, handleclass, "<init>","()V");
    jobject handle = (*env)->NewObject(env, handleclass, createHandler);
    LOGD("handle_dex :%p", handle);

    jmethodID obtain = (*env)->GetMethodID(env, handleclass, "obtainMessage","(I)Landroid/os/Message;");
    jobject msg = (*env)->CallObjectMethod(env, handle, obtain,2);

    return msg;
	
}

int hook_entry()
{
	LOGD("Hook success, pid[%d]", getpid());
	JNIEnv* env = getEnv();
	if(env == NULL)
	{
		LOGE("get java env failed.");
		return -1;
	}

	jobject msg = obtainMessage(env);	
	LOGE("msg: %p", msg);
		
	return 0;
}

