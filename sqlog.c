#include "sqlog.h"

int execute_log_ddl(sqlog *);
int execute_internal_log_ddl(sqlog *);
int execute_write_log_dml(sqlog *, char * log_text);
int execute_read_log_dml(sqlog *, int entries);

int create_log(char * log_db_name, sqlog ** log, int create_new, int internal_logging)
{
	(*log) = (sqlog *)malloc(sizeof(sqlog));
	(*log)->dbname = log_db_name;
	(*log)->internal_log_enabled = internal_logging;
	
	int retval;
	if(internal_logging)
	{
		//set up internal log at log_db_name directory

		char *log_dir = strdup(log_db_name);
		log_dir = dirname(log_dir);
		char internal_log_name[255];

		strcat(internal_log_name, log_dir);
		strcat(internal_log_name, "/internal_sqlog.db");

		(*log)->internal_dbname = internal_log_name;

		if(sqlite3_open_v2((*log)->internal_dbname, &(*log)->internal_sql, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
		{
			fprintf(stderr, "Unable to start internal sqlog logging\n");
			(*log)->internal_log_enabled = 0;
		}

		if(sqlite3_table_column_metadata((*log)->internal_sql, "", "internal_sqlog", NULL,NULL,NULL,NULL,NULL,NULL) != SQLITE_OK)
		{
			if(execute_internal_log_ddl((*log)) != SQLITE_OK)
			{
				fprintf(stderr, "Call to execute_internal_log_ddl failed. No internal logging will take place.\n");
				(*log)->internal_log_enabled = 0;
			}
		}

		(*log)->internal_log_enabled = 1;
	}

	if(create_new)
	{
		int sql_open = sqlite3_open_v2((*log)->dbname, &(*log)->sql, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(sql_open != SQLITE_OK)
		{
			if((*log)->internal_log_enabled)
			{

			}
			fprintf(stderr, "sqlite_open_v2 return %d\n", sql_open);
			return -1;
		}
		//check db for correct table for logging
		int log_table_check = sqlite3_table_column_metadata((*log)->sql, "", "sqlog", NULL, NULL, NULL, NULL, NULL, NULL);
		if(log_table_check != SQLITE_OK)
		{
			//log table does not exist, this is probably a new log file. create it

			retval = execute_log_ddl((*log));
			fprintf(stdout, "Log created: %d\n", retval);
		}

	}
	else
	{
		retval = sqlite3_open((*log)->dbname, &(*log)->sql);
	}

}

//TODO: open log is probably not needed with create_new parameter on create_log
int open_log(sqlog *log, char * database)
{
	int sql_open = sqlite3_open(database, &log->sql);

	return sql_open;
}

int read_log(sqlog *log, int entries)
{
	return execute_read_log_dml(log, entries);
}

int write_log(sqlog *log, char * log_text)
{
	return execute_write_log_dml(log, log_text);
}

int write_log_internal(sqlog * log, char * log_text, int log_type)
{


}

int clear_log(sqlog *log, int archive)
{
	if(!archive)
	{
		sqlite3_exec(log->sql, "TRUNCATE TABLE sqlog;", NULL,NULL,NULL);
		return 0;
	}

	//alter table name and re-run log ddl to get the log restarted
	
	sqlite3_int64 archive_time = (sqlite3_int64)time(NULL);
	char str_archive_time[20];
	sprintf(str_archive_time, "%lld", archive_time);

	char log_rename[27] = "sqlog_";
	strcat(log_rename, str_archive_time);

	char rename_sql[255] = "ALTER TABLE 'sqlog' RENAME TO '";
	strcat(rename_sql, log_rename);
	strcat(rename_sql, "'");

	int rename = sqlite3_exec(log->sql, rename_sql, NULL,NULL,NULL);

	int new_log = execute_log_ddl(log);

	return 0;
}

int close_log(sqlog * log)
{
	return sqlite3_close(log->sql);
}

int execute_log_ddl(sqlog * log)
{
	const char * log_ddl = "CREATE TABLE \"sqlog\" (\"logkey\" INTEGER NOT NULL UNIQUE,\"logtime\" INTEGER NOT NULL,\"logtext\" TEXT NOT NULL,PRIMARY KEY(\"logkey\"));";
	return sqlite3_exec(log->sql, log_ddl, NULL,NULL,NULL);
}

int execute_internal_log_ddl(sqlog * log)
{
	const char * log_ddl = "CREATE TABLE \"internal_sqlog\" (\"logkey\" INTEGER NOT NULL UNIQUE, \"logtype\" TEXT NOT NULL, \"logtime\" INTEGER NOT NULL,\"logtext\" TEXT NOT NULL,PRIMARY KEY(\"logkey\"));";
	return sqlite3_exec(log->internal_sql, log_ddl, NULL,NULL,NULL);
}


int execute_write_log_dml(sqlog *log, char * log_text)
{
	//check if statement exists on log struct, if not prepare statement
	if(log->lstmt != NULL)
	{
		//reset log statement
		sqlite3_reset(log->lstmt);
	}
	else
	{
		const char * log_dml = "INSERT INTO sqlog (logtime, logtext) VALUES (@unixtime, @logtext);";
		//prepare new statment, set log->lstmt
		int prepared = sqlite3_prepare_v2(log->sql, log_dml, strlen(log_dml), &log->lstmt, NULL);

		if(prepared != SQLITE_OK)
		{
			fprintf(stderr, "sqlite_prepare_v2 returned %d\n", prepared);
			return -1;
		}
	}
	
	//bind parameters
	

	int int_bind = sqlite3_bind_int64(log->lstmt, sqlite3_bind_parameter_index(log->lstmt, "@unixtime"), time(NULL));

	if(int_bind != SQLITE_OK){
		fprintf(stderr, "sqlite3_bind_text unable to bind time. returned %d\n", int_bind);
		return -1;
	}

	int text_bind = sqlite3_bind_text(log->lstmt, sqlite3_bind_parameter_index(log->lstmt, "@logtext"), log_text, -1, NULL);
	if(text_bind != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_bind_text unable to bind log_text. returned %d\n", text_bind);
		return -1;
	}
	
	//bind unix time to logtime
	
	//execute log insert
	
	sqlite3_exec(log->sql, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if(sqlite3_step(log->lstmt) != SQLITE_DONE)
	{
		//unable to log text, rollback.
		sqlite3_exec(log->sql, "ROLLBACK;", NULL, NULL, NULL);
		return -1;
	}
	
	sqlite3_exec(log->sql, "COMMIT;", NULL, NULL, NULL);
	return SQLITE_OK;

}

int execute_read_log_dml(sqlog * log, int entries)
{
	return 0;
}
