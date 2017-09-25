install:all
	#adb push libs/arm64-v8a/work /data/local/tmp
	#adb push libs/arm64-v8a/hook /data/local/tmp
	#adb push libs/arm64-v8a/sqlitetest /data/local/tmp
	adb push libs/armeabi-v7a/sym_sqlite_test /data/local/tmp
all:
	ndk-build
clean:
	rm -r obj
	rm -r libs
