//
// Created by maoshuoqiong on 17-9-29.
//

#include <jni.h>
#include "log.h"

static const char* TAG = "SENDMESSAGE";
static const char* SENT_SMS_ACTION = "SENT_SMS_ACTION";
static const char* DELIVERED_SMS_ACTION = "DELIVERED_SMS_ACTION";
static const char* JAR_PATH = "/data/local/tmp/test.jar";

static int init = 0;


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

static jobject create_receiver(JNIEnv *env)
{
    jclass receiver_class = (*env)->FindClass(env, "com/example/maoshuoqiong/sendmessage/PendingBroadCastReceiver");
    jmethodID create_receiver = (*env)->GetMethodID(env, receiver_class, "<init>","()V");
    jobject receiver = (*env)->NewObject(env, receiver_class, create_receiver);
    LOGD("receiver :%p", receiver);

    return receiver;
}

static jobject create_receiver_dex(JNIEnv *env, jobject context)
{
    jstring jarpath = (*env)->NewStringUTF(env, JAR_PATH); /* /data/local/tmp/test.jar */
    jstring classname = (*env)->NewStringUTF(env, "com.nmcm.sms.sendreceiverlibraray.SendResultReceiver");

    jclass fileclass = (*env)->FindClass(env, "java/io/File");
    jmethodID getabsolutepath = (*env)->GetMethodID(env,fileclass, "getAbsolutePath", "()Ljava/lang/String;");

    jclass applicationclass = (*env)->FindClass(env, "android/content/ContextWrapper");
    jmethodID getcachedir   = (*env)->GetMethodID(env, applicationclass, "getCacheDir","()Ljava/io/File;");
    jobject cachedir = (*env)->CallObjectMethod(env, context, getcachedir);
    jstring cachepath = (*env)->CallObjectMethod(env, cachedir, getabsolutepath);

//    jstring cachepath = (*env)->NewStringUTF(env, "/data/local/tmp");

    jclass classloaderclass = (*env)->FindClass(env,"java/lang/ClassLoader");
    jmethodID getclassloader = (*env)->GetStaticMethodID(env,classloaderclass,"getSystemClassLoader","()Ljava/lang/ClassLoader;");
    jobject classloader = (*env)->CallStaticObjectMethod(env, classloaderclass,getclassloader);
    LOGD("classloader: %p", classloader);

    jclass dexloaderclass = (*env)->FindClass(env, "dalvik/system/DexClassLoader");
    jmethodID loadclass = (*env)->GetMethodID(env,dexloaderclass, "loadClass","(Ljava/lang/String;)Ljava/lang/Class;");
    jmethodID createdxclassloader = (*env)->GetMethodID(env, dexloaderclass, "<init>",
                                                 "(Ljava/lang/String;Ljava/lang/String;"
                                                         "Ljava/lang/String;Ljava/lang/ClassLoader;)V");

    jobject dexclassloader = (*env)->NewObject(env, dexloaderclass, createdxclassloader, jarpath, cachepath, NULL, classloader );
    LOGD("dexclassloader: %p", dexclassloader);
    if(dexclassloader == NULL)
        return NULL;

    jclass receiverclass = (jclass)(*env)->CallObjectMethod(env, dexclassloader, loadclass, classname);
	if((*env)->ExceptionCheck(env))
    {
        LOGE("findclass exception");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return NULL;
    }

    jmethodID createreceiver = (*env)->GetMethodID(env, receiverclass, "<init>","()V");
    jobject receiver = (*env)->NewObject(env, receiverclass, createreceiver);

    LOGD("receiver_dex :%p", receiver);
    return receiver;
}

static void register_receiver(JNIEnv* env, jobject context, const char* action)
{
    jstring action_str = (*env)->NewStringUTF(env,action);

//    jobject receiver = create_receiver(env);

    jobject receiver = create_receiver_dex(env, context);
    if (receiver == NULL)
        return;

    jclass intent_filter_class = (*env)->FindClass(env, "android/content/IntentFilter");
    jmethodID create_intent_filter = (*env)->GetMethodID(env, intent_filter_class, "<init>","(Ljava/lang/String;)V");
    jobject intent_filter = (*env)->NewObject(env, intent_filter_class, create_intent_filter, action_str);
    LOGD("intent_filter :%p", intent_filter);

    jclass context_class = (*env)->FindClass(env,"android/content/Context");
    jmethodID register_receiver = (*env)->GetMethodID(env,context_class,"registerReceiver",
                                                      "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;");

    (*env)->CallObjectMethod(env,context, register_receiver, receiver, intent_filter);
}

static jobject get_pending_intent(JNIEnv * env, jobject context, const char* action)
{
    jobject ret = NULL;
    jstring action_str = (*env)->NewStringUTF(env,action);

    LOGD("get_intent");
    /* Intetn */
    jclass intent_class = (*env)->FindClass(env, "android/content/Intent");
    if(intent_class == NULL)
    {
        LOGE("intent_class find error");
        return ret;
    }
    jmethodID create_intent = (*env)->GetMethodID(env, intent_class, "<init>","(Ljava/lang/String;)V");
    if(create_intent == NULL)
    {
        LOGE(" get create_intent error");
        return ret;
    }
    LOGD("ready to new intent");
    jobject intent = (*env)->NewObject(env,intent_class, create_intent, action_str);
    if(intent == NULL)
    {
        LOGE(" intent create error");
        return ret;
    }

    LOGD("get_pending_intent");
    /* pending intent */
    jclass pending_intent_class = (*env)->FindClass(env,"android/app/PendingIntent");
    if(pending_intent_class == NULL)
    {
        LOGE("pending_intent find error");
        return ret;
    }
    jmethodID getbroadcast = (*env)->GetStaticMethodID(env,pending_intent_class,
                               "getBroadcast","(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;");

    if(getbroadcast == NULL)
    {
        LOGE("getbroadcast error");
        return ret;
    }

    jfieldID flag_field = (*env)->GetStaticFieldID(env, pending_intent_class, "FLAG_UPDATE_CURRENT", "I");
    jint flag = (*env)->GetStaticIntField(env, pending_intent_class, flag_field);
    LOGD("flag: %d", flag);

    LOGD("ready to getbroadcast");
    ret = (*env)->CallStaticObjectMethod(env, pending_intent_class, getbroadcast,context,0,intent,flag);
    return ret;
}

static void send_message(JNIEnv * env, jstring phone, jstring message)
{
    jclass manager_class = (*env)->FindClass(env,"android/telephony/SmsManager");
    if(manager_class == NULL)
    {
        LOGE("SmsManager find error");
        return;
    }

    jmethodID getsmsmanager_method =
            (*env)->GetStaticMethodID(env, manager_class,"getDefault","()Landroid/telephony/SmsManager;");

    if(getsmsmanager_method == NULL)
    {
        LOGE("get getsmsmanager_method fail");
        return;
    }

    jmethodID sendmsg_method =
            (*env)->GetMethodID(env, manager_class, "sendTextMessage",
                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/app/PendingIntent;Landroid/app/PendingIntent;)V");

    if(sendmsg_method == NULL)
    {
        LOGE("getsendmsg error");
        return;
    }

    jobject sms_massager = (*env)->CallStaticObjectMethod(env,manager_class, getsmsmanager_method);


    jobject send_intent = NULL;
    jobject delivered_intent = NULL;
    jobject context = getcontext(env);
    send_intent = get_pending_intent(env, context, SENT_SMS_ACTION);
    delivered_intent = get_pending_intent(env, context, DELIVERED_SMS_ACTION);
    LOGD("send sms, send_intent: %p, delivered_intent: %p", send_intent, delivered_intent);

    (*env)->CallVoidMethod(env,sms_massager,sendmsg_method,phone,0,message,send_intent,delivered_intent);


    LOGD("send end");
}

void send_text_message(JNIEnv * env, const char* num, const char* text)
  {
      LOGD("num: %s, text: %s, env: %p, *env: %p\n", num, text, env, *env);
	  jstring jnum  = (*env)->NewStringUTF(env, num);
	  jstring jtext = (*env)->NewStringUTF(env, text);

      send_message(env, jnum, jtext);
  }

int init_register(JNIEnv* env)
{
    if(!init)
    {
        LOGD("regist receiver, env: %p, *env: %p", env, *env);
        jobject context = getcontext(env);
        register_receiver(env, context, SENT_SMS_ACTION);
        register_receiver(env, context, DELIVERED_SMS_ACTION);

        init = 1;
    }
	return 0;
}
