VMACHINE=libdalvik
LIB_SENDSMS=lib_sendsms
SENDSMS=sendsms
CUR_PATH=${PWD}
#APP_ABI=armeabi-v7a
APP_ABI=arm64-v8a


install:all
	adb push ${CUR_PATH}/${LIB_SENDSMS}/libs/${APP_ABI}/libsm.so /data/local/tmp
	adb push ${CUR_PATH}/${SENDSMS}/libs/${APP_ABI}/sendsms /data/local/tmp

all:
	cd ${SENDSMS} && make all
	#cd ${VMACHINE} && make all
	cd ${LIB_SENDSMS} && make all
	@echo "****************"


clean:
	cd ${SENDSMS} && make clean
	cd ${LIB_SENDSMS} && make clean
	cd ${VMACHINE} && make clean
