1 切换目录到libdalvik   ndk-build  

2 切换到 lib_sendsms    ndk-build  会在/lib/armeabi-v7a/目录下生成libsm.so 将这个文件push到手机里的 /data/local/tmp/目录下 （注：这个是测试环境，正式环境放到/system/lib/目录下，并修改sendsms/hook.c 文件文件里HOOK_SMS_SEND_SO的定义）

3 切换到 sendsmd 目录 ndk-build 生成sendsms 这个是linux可执行文件。放到/data/local/tmp里面。以root权限运行。