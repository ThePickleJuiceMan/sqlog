#ifndef SQLITE3_HEADER
#define SQLITE3_HEADER

#include "./sqlite3/sqlite3.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#endif

typedef struct sqlog
{
	sqlite3 *sql;
	char * dbname;
	
	sqlite3_stmt * lstmt; //this will be used to continuously execute logging without re-preparing sql each time.

	//int fifo; currently unused named pipe file descriptor
} sqlog;

//Public functions for sqlog.c
int create_log(char *, sqlog **, int);
int open_log(sqlog *, char *);
int read_log(sqlog *, int);
int write_log(sqlog *, char *);
int clear_log(sqlog *, int);

//Internal functions for sqlog.c
int save_log_metadata(sqlog *);

