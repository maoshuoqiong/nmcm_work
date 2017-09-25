#include "sym_sqlite.h"

#define LOGE(fmt,args...) \
	__android_log_print( ANDROID_LOG_ERROR, "HOOK", fmt, ##args)

#define LOGD(fmt,args...) \
	__android_log_print( ANDROID_LOG_DEBUG, "HOOK", fmt, ##args)

#define LOGI(fmt,args...) \
	__android_log_print( ANDROID_LOG_INFO, "HOOK", fmt, ##args)


#ifdef __aarch64__
#define LIBSQLITE   "/system/lib64/libsqlite.so"
#else
#define LIBSQLITE   "/system/lib/libsqlite.so"
#endif

static void* lib_handle = NULL;
Sqlite_open * sym_open  = NULL; 
Sqlite_close* sym_close = NULL;
Sqlite_exec * sym_exec  = NULL;
Sqlite_free * sym_free  = NULL;

int create_sqlite()
{
	LOGD("create_sqlite");
	if(lib_handle != NULL)
	{ /* not thread safe */
		dlclose(lib_handle);
		LOGD("rebuild sqlite");
	}

	lib_handle = dlopen(LIBSQLITE, RTLD_NOW|RTLD_GLOBAL);
	if(lib_handle == NULL)
	{
		LOGE("open sqlite error: %s", dlerror());
		return -1;
	}
	LOGD("open sqlite success");

	if( (sym_open = (Sqlite_open*)dlsym(lib_handle, "sqlite3_open")) == NULL)
	{
		LOGE("sqlite_open error: %s", dlerror());
		dlclose(lib_handle);
		lib_handle = NULL;
		return -1;	
	}
	LOGD("get sqlite_open success");

	if( (sym_close = (Sqlite_close*)dlsym(lib_handle, "sqlite3_close")) == NULL)
	{
		LOGE("sqlite_close error: %s", dlerror());
		dlclose(lib_handle);
		lib_handle = NULL;
		return -1;
	}
	LOGD("get sqlite_close success");

	if( (sym_exec = (Sqlite_exec*)dlsym(lib_handle, "sqlite3_exec")) == NULL)
	{
		LOGE("sqlite_exec error: %s", dlerror());
		dlclose(lib_handle);
		lib_handle = NULL;
		return -1;
	}
	LOGD("get sqlite_exec success");

	if( (sym_free = (Sqlite_free*)dlsym(lib_handle, "sqlite3_free")) == NULL)
	{
		LOGE("sqlite_free error: %s", dlerror());
		dlclose(lib_handle);
		lib_handle = NULL;
		return -1;
	}
	LOGD("get sqlite_free success");

	return 0;

}

void destroy_sqlite()
{
	LOGD("destroy_sqlite");
	if(lib_handle)
		dlclose(lib_handle);

	lib_handle = NULL;
	sym_open   = NULL;
	sym_close  = NULL;
	sym_exec   = NULL;
	sym_free   = NULL;
}

