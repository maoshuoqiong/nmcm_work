/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include "Common.h"
#include "dexstuff.h"
#include "log.h"

static void* mydlsym(void *hand, const char *name)
{
	void* ret = dlsym(hand, name);
    LOGE("%s = 0x%x\n", name, ret);
	return ret;
}

/*
extern "C" jint JNI_GetCreatedJavaVMs(JavaVM** vms, jsize, jsize* vm_count) 
{
	Runtime* runtime = Runtime::Current();
	if (runtime == nullptr) {
		*vm_count = 0;
	} else {
		*vm_count = 1;
		vms[0] = runtime->GetJavaVM();
	}
	return JNI_OK;
}
*/

typedef jint JNI_GetCreatedJavaVMs_Method(JavaVM** , jsize, jsize* );

void dexstuff_resolv_dvm(struct dexstuff_t *d)
{
	LOGE("dexstuff_resolv_dvm  00");
#ifdef ART
    d->dvm_hand = dlopen("libart.so", RTLD_NOW); //libdvm.so  libart.so
#else 
    d->dvm_hand = dlopen("libdvm.so", RTLD_NOW); //libdvm.so  libart.so
#endif
    LOGE("dvm_hand = 0x%x\n", d->dvm_hand);

	JNI_GetCreatedJavaVMs_Method* p = (JNI_GetCreatedJavaVMs_Method*)mydlsym(d->dvm_hand,"JNI_GetCreatedJavaVMs");
	JavaVM * vms=NULL;
	jsize vm_count = 0;
	jsize count=1;
	p(&vms,count, &vm_count);
    LOGE("JavaVm = 0x%x,count=[%d]\n", vms,vm_count);

	JNIEnv* env = NULL;  
    if((*vms)->GetEnv(vms,(void**)&env , JNI_VERSION_1_6) != JNI_OK)  
        LOGE("GetEnv error");
    LOGE("JNIEnv= 0x%x\n", env);
	d->g_env = env;
	
}

int dexstuff_loaddex(struct dexstuff_t *d, char *path)
{
	jvalue pResult;
	jint result;
	
    LOGD("dexstuff_loaddex, path = 0x%x\n", path);
	void *jpath = d->dvmStringFromCStr_fnPtr(path, strlen(path), ALLOC_DEFAULT);
	u4 args[2] = { (u4)jpath, (u4)NULL };
	
	d->dvm_dalvik_system_DexFile[0].fnPtr(args, &pResult);
	result = (jint) pResult.l;
    LOGD("cookie = 0x%x\n", pResult.l);

	return result;
}

void* dexstuff_defineclass(struct dexstuff_t *d, char *name, int cookie)
{
	u4 *nameObj = (u4*) name;
	jvalue pResult;
	
    LOGD("dexstuff_defineclass: %s using %x\n", name, cookie);
	
	void* cl = d->dvmGetSystemClassLoader_fnPtr();
	Method *m = d->dvmGetCurrentJNIMethod_fnPtr();
    LOGD("sys classloader = 0x%x\n", cl);
    LOGD("cur m classloader = 0x%x\n", m->clazz->classLoader);
	
	void *jname = d->dvmStringFromCStr_fnPtr(name, strlen(name), ALLOC_DEFAULT);
	//log("called string...\n")
	
	u4 args[3] = { (u4)jname, (u4) m->clazz->classLoader, (u4) cookie };
	d->dvm_dalvik_system_DexFile[3].fnPtr( args , &pResult );

	jobject *ret = pResult.l;
    LOGD("class = 0x%x\n", ret);
	return ret;
}

void* getSelf(struct dexstuff_t *d)
{
	return d->dvmThreadSelf_fnPtr();
}

void dalvik_dump_class(struct dexstuff_t *dex, char *clname)
{
	if (strlen(clname) > 0) {
		void *target_cls = dex->dvmFindLoadedClass_fnPtr(clname);
		if (target_cls)
			dex->dvmDumpClass_fnPtr(target_cls, (void*)1);
	}
	else {
		dex->dvmDumpAllClasses_fnPtr(0);
	}
}


