#include "sym_sqlite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>


#define LOGD(fmt,args...) \
	__android_log_print( ANDROID_LOG_DEBUG, "HOOK", fmt, ##args)

#define LOGE(fmt,args...) \
	__android_log_print( ANDROID_LOG_ERROR, "HOOK", fmt, ##args)

static int callback(void* param1, int argc, char**argv, char**azColName)
{
	int i;
	for(i=0; i<argc; i++)
		LOGD("%s = %s", azColName[i], argv[i]? argv[i]:"NULL");
	return 0;
}

int main(void)
{
	LOGD("Begin...");
	if(create_sqlite()!=0)
		printf("create_sqlite error"),exit(-1);

	int ret = 0;
	
	int sym_ret = SQLITE_OK;
	void *db = NULL;
	if( (sym_ret = sym_open("/data/local/tmp/msq.db", &db)) != SQLITE_OK)
	{
		LOGE("open /data/local/tmp/msq.db error: %d", sym_ret);
		ret = -1;
		goto exit;
	}	

	char *zErrMsg = NULL;
	const char* sql = "create table if not exists person( ID INT PRIMARY KEY not null, "
			"NAME TEXT NOT NULL, "
			"AGE  INT  NOT NULL);";

	if( (sym_ret = sym_exec(db, sql, callback, db, &zErrMsg)) != SQLITE_OK)
	{
		LOGE("sql error: %s", zErrMsg);
		goto exit1;
	}

	char szSql[256] = {0x00};
	for(int i = 0; i< 10; i++)
	{
		memset(szSql, 0x00, sizeof(szSql));
		sprintf(szSql, "insert into person values(%d, 'Maomao%d', 18);", i+1, i);
		
		if( (sym_ret = sym_exec(db, szSql, callback, db, &zErrMsg)) != SQLITE_OK)
		{
			LOGE("sql error: %s", zErrMsg);
			goto exit1;
		}
	}

	memset(szSql, 0x00, sizeof(szSql));
	sprintf(szSql, "select * from person;");
	if( (sym_ret = sym_exec(db, szSql, callback, db, &zErrMsg)) != SQLITE_OK)
	{
		LOGE("sql error: %s", zErrMsg);
		goto exit1;
	}

exit1:
	LOGD("sym_close");
	sym_close(db);

exit:
	LOGD("destroy_sqlite");
	destroy_sqlite();
	return ret;
}

