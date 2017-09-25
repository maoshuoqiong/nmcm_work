#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOGE(fmt,args...) \
	__android_log_print( ANDROID_LOG_ERROR, "HOOKTEST", fmt, ##args)

#ifdef __aarch64__
#define LIBSQLITE   "/system/lib64/libsqlite.so"
#else
#define LIBSQLITE   "/system/lib/libsqlite.so"
#endif



#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */

/*
SQLITE_API int sqlite3_open(
  const char *filename,   //Database filename (UTF-8) 
  sqlite3 **ppDb          // OUT: SQLite db handle 
);

SQLITE_API int sqlite3_close(sqlite3*);
SQLITE_API int sqlite3_close_v2(sqlite3*);

SQLITE_API int sqlite3_exec(
  sqlite3*,                                  // An open database 
  const char *sql,                           // SQL to be evaluated 
  int (*callback)(void*,int,char**,char**),  // Callback function 
  void *,                                    // 1st argument to callback 
  char **errmsg                              // Error msg written here 
);


*/

typedef int Sqlite3_open(const char* filename, void** ppDb);
typedef int Sqlite3_close(void* db);
typedef int Sqlite3_exec(void*db, const char*sql, int(*callback)(void*,int, char**,char**),void* argforcallback, char **errmsg);

static int callback(void *notused, int argc, char**argv, char**azColName)
{
	int i;
	for(i=0; i<argc; i++)
		LOGE("%s = %s", azColName[i], argv[i]? argv[i] : "NULL");

	return 0;
}

int main(void)
{
	LOGE("Begining+++++++");
	void* tmp_handle = dlopen(LIBSQLITE, RTLD_NOW|RTLD_GLOBAL);
	if(tmp_handle == NULL)
		LOGE("dlopen error: %s",dlerror()),exit(-1);
	LOGE("dlopen %s success", LIBSQLITE);

	Sqlite3_open* sym_open = NULL;
	if( (sym_open = (Sqlite3_open*)dlsym(tmp_handle, "sqlite3_open"))==NULL )
	{
		LOGE("dlsym [sqlite3_open] error: %s", dlerror());
		dlclose(tmp_handle);
		return -1;	
	}

	Sqlite3_close* sym_close = NULL;
	if( (sym_close = (Sqlite3_close*)dlsym(tmp_handle, "sqlite3_close"))==NULL )
	{
		LOGE("dlsym [sqlite3_close] error: %s", dlerror());
		dlclose(tmp_handle);
		return -1;	
	}	

	Sqlite3_exec* sym_exec = NULL;
	if( (sym_exec = (Sqlite3_exec*)dlsym(tmp_handle, "sqlite3_exec"))==NULL )
	{
		LOGE("dlsym [sqlite3_exec] error: %s", dlerror());
		dlclose(tmp_handle);
		return -1;	
	}	

	int ret = SQLITE_OK;
	void * db = NULL;
	if( (ret = sym_open("/data/local/tmp/msq.db", &db)) != SQLITE_OK )
	{
		LOGE("open /data/local/tmp/msq.db error: %d", ret);
		dlclose(tmp_handle);
		return -1;
	}

	if(db == NULL)
	{
		LOGE("db is null");
		dlclose(tmp_handle);
		exit(-1);
	}

	char *zErrMsg = NULL;

	const char* sql = "create table if not exists person( ID INT PRIMARY KEY not null, "
			"NAME TEXT NOT NULL, "
			"AGE  INT  NOT NULL );";

	if( (ret = sym_exec(db, sql, callback, db, &zErrMsg)) != SQLITE_OK)
	{
		LOGE("sql error: %s",zErrMsg);
		sym_close(db);
		dlclose(tmp_handle);
		return -1;
	}

	char szSql[256] = {0x00};


	for(int i = 0; i<10;i++)
	{
		memset(szSql, 0x00, sizeof(szSql));
		sprintf(szSql , "insert into person values(%d,'Maomao%d',18);",i+1,i);

		if( (ret = sym_exec(db, szSql, callback, db, &zErrMsg)) != SQLITE_OK)
		{
			LOGE("sql error: %s",zErrMsg);
			sym_close(db);
			dlclose(tmp_handle);
			return -1;
		}
	}

	memset(szSql, 0x00, sizeof(szSql));
	sprintf(szSql , "select * from person;");
	if( (ret = sym_exec(db, szSql, callback, db, &zErrMsg)) != SQLITE_OK)
	{
		LOGE("sql error: %s",zErrMsg);
		sym_close(db);
		dlclose(tmp_handle);
		return -1;
	}

	sym_close(db);
	dlclose(tmp_handle);
	return 0;

}

