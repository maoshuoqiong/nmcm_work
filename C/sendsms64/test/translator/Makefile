TRACER=translator
CUR_PATH=${PWD}
#APP_ABI=armeabi-v7a
APP_ABI=arm64-v8a


install:all
	adb push ${CUR_PATH}/${TRACER}/libs/${APP_ABI}/${TRACER} /data/local/tmp

all:
	cd ${TRACER} && make all
	@echo "****************"


clean:
	cd ${TRACER} && make clean
