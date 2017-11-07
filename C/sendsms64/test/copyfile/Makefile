install:all
	adb push libs/arm64-v8a/copyfile /data/local/tmp
	#adb push libs/armeabi-v7a/inotify /data/local/tmp
all:
	ndk-build
clean:
	rm -r obj
	rm -r libs
	#ndk-build clean
