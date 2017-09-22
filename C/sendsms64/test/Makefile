install:all
	adb push libs/arm64-v8a/work /data/local/tmp
	adb push libs/arm64-v8a/hook /data/local/tmp
all:
	ndk-build
clean:
	rm -r obj
	rm -r libs
