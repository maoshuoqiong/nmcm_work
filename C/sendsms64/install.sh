#! /bin/bash
. ./make.sh
adb push ${CUR_PATH}/${LIB_SENDSMS}/libs/arm64-v8a/libsm.so /data/local/tmp
adb push ${CUR_PATH}/${SENDSMS}/libs/arm64-v8a/sendsms /data/local/tmp
