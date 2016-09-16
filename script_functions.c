/* I don't want, but I have to! T-Max */
#include "../pinc.h"
#include "mysql/include/mysql.h"
#include "stdio.h"

extern MYSQL mysql;
extern MYSQL_RES* mysql_res;
extern cvar_t *g_mysql_host;
extern cvar_t *g_mysql_port;
extern cvar_t *g_mysql_user;
extern cvar_t *g_mysql_password;
extern cvar_t *g_mysql_database;

static void Scr_MySQL_Error(const char* fmt, ...)
{
	char buffer[1024] = {'\0'};
	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(va);

	Plugin_Scr_Error(buffer);
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-real-connect.html
* Description
    mysql_real_connect() attempts to establish a connection to a
    MySQL database engine running on host. mysql_real_connect()
    must complete successfully before you can execute any other
    API functions that require a valid MYSQL connection handle structure.
* Return Value(s)
    A MYSQL* connection handle if the connection was successful, NULL
    if the connection was unsuccessful. For a successful connection,
    the return value is the same as the value of the first parameter.
   ================================================================= */
void Scr_MySQL_Real_Connect_f()
{
	// I want this. You may have more than one db to query against
	char* host = Plugin_Scr_GetString(0);
	char* user = Plugin_Scr_GetString(1);
	char* pass = Plugin_Scr_GetString(2);
	char* db = Plugin_Scr_GetString(3);
	int port = Plugin_Scr_GetInt(4);

	if (Plugin_Scr_GetNumParam() != 3)
	{
		Plugin_Scr_Error("Usage: mysql_real_connect(host, user, pass, port*);\n*port not required");
		return;
	}

#ifndef WIN32
	/* On *Unix based systems, using "localhost" instead of 127.0.0.1
	 * causes a unix socket error, we replace it */
	if (strcmp(host, "localhost") == 0)
	{
		Plugin_Cvar_SetString(host, "127.0.0.1");
	}
#endif

	// IF port is defined the use port ELSEIF cvar port is defined ELSE use default
	int newPort;
	if( port != NULL )
		newPort = g_mysql_port->integer;
	else if ( g_mysql_port->integer != NULL )
		newPort = port;
	else
		newPort = 3306;


	MYSQL* result = mysql_real_connect(&mysql, host,
									   user,
	                                   pass,
	                                   db,
									   newPort, NULL, 0);

	/* We don't want to crash the server, so we have a check to return nothing to prevent that */
	if (result == NULL)
	{
		Scr_MySQL_Error("MySQL connect error: (%d) %s", mysql_errno(&mysql),
		                 mysql_error(&mysql));
		Plugin_Scr_AddUndefined(); // make sure we return undefined so GSC does not shit it's self.
		return;
	}

	/* Would you like to reconnect if connection is dropped? */
	qboolean reconnect = qtrue; // allows the database to reconnect on a new query etc.

	/* Check to see if the mySQL server connection has dropped */
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, &reconnect);
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-close.html
* Description
    Closes a previously opened connection. mysql_close()
    also deallocates the connection handle pointed to by
    mysql if the handle was allocated automatically by
    mysql_init() or mysql_connect().
* Return Value(s)
    None.
   ================================================================= */
void Scr_MySQL_Close_f()
{
	if (Plugin_Scr_GetNumParam() > 0)
	{
		Plugin_Scr_Error("Usage: mysql_close();");
		return;
	}

	/* Closes the MySQL Handle Connection */
	Com_DPrintf("Closing CID: %d\n", mysql); // t
	mysql_close(&mysql);
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-affected-rows.html
* Description
    mysql_affected_rows() may be called immediately after executing
    a statement with mysql_query() or mysql_real_query(). It
    returns the number of rows changed, deleted, or inserted by
    the last statement if it was an UPDATE, DELETE, or INSERT.
    For SELECT statements, mysql_affected_rows() works like
    mysql_num_rows().
* Return Value(s)
    An integer greater than zero indicates the number of rows
    affected or retrieved. Zero indicates that no records
    were updated for an UPDATE statement, no rows matched the
    WHERE clause in the query or that no query has yet been
    executed. -1 indicates that the query returned an error
    or that, for a SELECT query, mysql_affected_rows()
    was called prior to calling mysql_store_result().
   ================================================================= */
void Scr_MySQL_Affected_Rows_f()
{
	if (Plugin_Scr_GetNumParam() > 0)
	{
		Plugin_Scr_Error("Usage: mysql_affected_rows();");
		return;
	}

	/* Attempt to call without query */
	if (mysql_res == NULL)
	{
		Plugin_Scr_Error("'mysql_query' must be called before.");
		return;
	}

	Plugin_Scr_AddInt(mysql_affected_rows(&mysql));
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-query.html
* Description
    Executes the SQL statement pointed to by the null-terminated
    string stmt_str. Normally, the string must consist of a single
    SQL statement without a terminating semicolon (;) or \g.
    If multiple-statement execution has been enabled, the string
    can contain several statements separated by semicolons.
* Return Value(s)
    Zero for success. Nonzero if an error occurred.
   ================================================================= */

/* What about SQL-injections? Should prevent? Or they already handled? */
/* Combine result output here? */
void Scr_MySQL_Query_f()
{
	if (Plugin_Scr_GetNumParam() != 1)
	{
		Plugin_Scr_Error("Usage: mysql_query(<string query>);");
		return;
	}

	char* query = Plugin_Scr_GetString(0);

	if (mysql_query(&mysql, query) == 0)
	{
		if(mysql_res)
		{
			mysql_free_result(mysql_res);
			mysql_res = NULL;
		}

		mysql_res = mysql_store_result(&mysql);

		if(mysql_res == NULL)
		{
			Scr_MySQL_Error("MySQL store error: (%d) %s", mysql_errno(&mysql),
			                 mysql_error(&mysql));
		}
	}
	else
	{
		Scr_MySQL_Error("MySQL query error: (%d) %s", mysql_errno(&mysql),
		                 mysql_error(&mysql));
	}
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-num-rows.html
* Description
    Returns the number of rows in the result set.
* Return Value(s)
    The number of rows in the result set.
   ================================================================= */
void Scr_MySQL_Num_Rows_f()
{
	if (Plugin_Scr_GetNumParam() > 0)
	{
		Plugin_Scr_Error("Usage: mysql_num_rows();");
		return;
	}

	/* Attempt to call without query */
	if (mysql_res == NULL)
	{
		Plugin_Scr_Error("'mysql_query' must be called before.");
		return;
	}

	Plugin_Scr_AddInt(mysql_num_rows(mysql_res));
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-num-fields.html
* Description
    Returns the number of columns in a result set.
* Return Value(s)
    An unsigned integer representing the number of columns in a result set.
   ================================================================= */
void Scr_MySQL_Num_Fields_f()
{
	if (Plugin_Scr_GetNumParam() > 0)
	{
		Plugin_Scr_Error("Usage: mysql_num_fields();");
		return;
	}

	/* Attempt to call without query */
	if (mysql_res == NULL)
	{
		Plugin_Scr_Error("'mysql_query' must be called before.");
		return;
	}

	Plugin_Scr_AddInt(mysql_num_fields(mysql_res));
}

/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-fetch-row.html
* Description
    Retrieves the next row of a result set.
* Return Value(s)
    A MYSQL_ROW structure for the next row. NULL if there are no more rows to retrieve or if an error occurred.
   ================================================================= */
/* Todo: must be tweaked. I think, key for arrays will be great. */
/* Todo: double check that. I didn't worked with mysql before */
void Scr_MySQL_Fetch_Rows_f()
{
	if (Plugin_Scr_GetNumParam() > 0)
	{
		Plugin_Scr_Error("Usage: mysql_fetch_row();");
		return;
	}

	/* Attempt to call without query */
	if (mysql_res == NULL)
	{
		Plugin_Scr_Error("'mysql_query' must be called before.");
		return;
	}

	unsigned int col_count = mysql_num_fields(mysql_res);
	MYSQL_ROW row = mysql_fetch_row(mysql_res);

	if(row != NULL)
	{
		Plugin_Scr_MakeArray();

		int i;
		for(i = 0; i < col_count; ++i)
		{
			/* A little help here? I don't actually understand data representation
			   Integer must be integer, string - string, float - float */
			MYSQL_FIELD *field = mysql_fetch_field(mysql_res);
			if(field == NULL)
			{
				Scr_MySQL_Error("Houston, we got a problem: unnamed column!");
				return;
			}
			Plugin_Scr_AddString(row[i]);
			Plugin_Scr_AddArrayKey(Plugin_Scr_AllocString(field->name));
		}
	}
}
