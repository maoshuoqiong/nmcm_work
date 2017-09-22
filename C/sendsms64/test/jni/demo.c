#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <dlfcn.h>
#include <jni.h>
#include <android/log.h>

#define LOGE(fmt, args...) \
	__android_log_print( ANDROID_LOG_ERROR, "HOOK", fmt, ##args)

int main(void)
{
	LOGE("Begin....");
    void* dvm_hand = dlopen("libart.so", RTLD_NOW); //libdvm.so  libart.so
    LOGE("dvm_hand = %p\n", dvm_hand);

    /* JNI_GetCreatedJavaVMs_Method* p = (JNI_GetCreatedJavaVMs_Method*)mydlsym(d->dvm_hand,"JNI_GetCreatedJavaVMs"); */
	JNI_CreateJavaVM_Method* p = (JNI_CreateJavaVM_Method*)mydlsym(dvm_hand,"JNI_CreateJavaVM");
	if(!p)
	{
		LOGE("Get JNI_CreateJavaVM_Method error");
		return -1;
	}
    JavaVM * jvm=NULL;
    JNIEnv* env = NULL;  
	
	JavaVMInitArgs vm_args; /* JDK/JRE 6 VM initialization arguments */
	JavaVMOption options[1];
    options[0].optionString = "-Djava.class.path=/usr/lib/java";
    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = 1;
    /* load and initialize a Java VM, return a JNI interface
     * pointer in env */
    p(&jvm, &env, &vm_args);
	
    LOGE("JNIEnv= %p\n", env);
    /* invoke the Main.test method using the JNI */

	dlclose(dvm_hand);

	return 0;
}

