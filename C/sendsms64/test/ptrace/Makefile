LIBRARY=hook
TRACER=tracer
CUR_PATH=${PWD}
#APP_ABI=armeabi-v7a
APP_ABI=arm64-v8a


install:all
	adb push ${CUR_PATH}/${LIBRARY}/libs/${APP_ABI}/lib${LIBRARY}.so /data/local/tmp
	adb push ${CUR_PATH}/${TRACER}/libs/${APP_ABI}/${TRACER} /data/local/tmp

all:
	cd ${TRACER} && make all
	cd ${LIBRARY} && make all
	@echo "****************"


clean:
	cd ${TRACER} && make clean
	cd ${LIBRARY} && make clean
