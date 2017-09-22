#! /bin/bash
#1 切换目录到libdalvik   ndk-build  
#
#2 切换到 lib_sendsms    ndk-build  会在/lib/armeabi-v7a/目录下生成libsm.so 将这个文件push到手机里的 /data/local/tmp/目录下 （注：这个是测试环境，正式环境放到/system/lib/目录下，并修改sendsms/hook.c 文件文件里HOOK_SMS_SEND_SO的定义）

#3 切换到 sendsmd 目录 ndk-build 生成sendsms 这个是linux可执行文件。放到/data/local/tmp里面。以root权限运行。

#

CUR_PATH=${PWD}
VMACHINE=libdalvik
LIB_SENDSMS=lib_sendsms
LIB_SENDSMS_PATH=${LIB_SENDSMS}/libs/arm64-v8a/libsm.so 

SENDSMS=sendsms
SENDSMS_PATH=${SENDSMS}/libs/arm64-v8a/sendsms
SENDSMS_PATH_EX=${SENDSMS}/obj/local/arm64-v8a/sendsms

CC=/home/maoshuoqiong/Tools/android-sdk-linux/ndk-bundle/ndk-build

if [ -e ${LIB_SENDSMS_PATH} ];then echo "rm ${LIB_SENDSMS_PATH}"; rm ${LIB_SENDSMS_PATH}; fi 
if [ -e ${LIB_SENDSMS}/obj ];then echo "rm ${LIB_SENDSMS}/obj"; rm -r ${LIB_SENDSMS}/obj; fi 
if [ -e ${SENDSMS_PATH} ];then echo "rm ${SENDSMS_PATH}"; rm ${SENDSMS_PATH}; fi 
if [ -e ${SENDSMS_PATH_EX} ];then echo "rm ${SENDSMS_PATH_EX}"; rm ${SENDSMS_PATH_EX}; fi 
if [ -e ${SENDSMS}/obj ];then echo "rm ${SENDSMS}/obj"; rm -r ${SENDSMS}/obj; fi 
if [ -e ${VMACHINE}/obj ];then echo "rm ${VMACHINE}/obj"; rm -r ${VMACHINE}/obj; fi 
echo -e "****************\n"

cd ${SENDSMS}
make
echo -e "****************\n"

cd ${CUR_PATH}/${VMACHINE}
make
echo -e "****************\n"

cd ${CUR_PATH}/${LIB_SENDSMS}
make
echo -e "****************\n"


#clean:
#	@if [ -e ${LIB_SENDSMS_PATH} ];then echo "rm ${LIB_SENDSMS_PATH}"; rm ${LIB_SENDSMS_PATH}; fi 
#	@if [ -e ${SENDSMS_PATH} ];then echo "rm ${SENDSMS_PATH}"; rm ${SENDSMS_PATH}; fi 
#	@if [ -e ${SENDSMS_PATH_EX} ];then echo "rm ${SENDSMS_PATH_EX}"; rm ${SENDSMS_PATH_EX}; fi 
