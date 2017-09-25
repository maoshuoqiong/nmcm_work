/*
* Create by Marshark. For sqlite3 APIs
* not thread safe
* Link with -ldl
*/

#ifndef SYM_SQLITE_H_
#define SYM_SQLITE_H_

#include <android/log.h>
#include <dlfcn.h>

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


typedef int Sqlite_open(const char* filename, void** ppDb);
typedef int Sqlite_close(void* db);
typedef int Sqlite_exec(void*db, const char*sql, int(*callback)(void*,int, char**,char**),void* argforcallback, char **errmsg);

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
extern Sqlite_open * sym_open; 
extern Sqlite_close* sym_close;
extern Sqlite_exec * sym_exec;

/*
*  Function: initialization sym_open,sym_close, sym_exec functions
*  Note:  Must use this function first.
* 
*  parm:
*  return:  create_sqlite return 0 if success. Otherwise, nonzero is return.
*/
extern int create_sqlite();

/*
* Free libs and symbols
*/
extern void destroy_sqlite();
#endif

