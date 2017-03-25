/* Plugin includes */
#include "../pinc.h"
#include "script_functions.h"

/* OS-specific includes */
#ifdef WIN32
    #include "mysql/windows/include/mysql.h"
#else
    #include "mysql/unix/include/mysql.h"
#endif

#include "stdio.h"

extern MYSQL g_mysql[MYSQL_CONNECTION_COUNT];
extern MYSQL_RES* g_mysql_res[MYSQL_CONNECTION_COUNT];
extern qboolean g_mysql_reserved[MYSQL_CONNECTION_COUNT];

/* =================================================================
 * Shows script runtime error with crash.
 * For use only inside gsc callbacks.
   ================================================================= */
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
 * Checks if function called after connection.
 * For use only inside gsc callbacks.
   ================================================================= */
static void Scr_MySQL_CheckConnection(int handle)
{
    /* Attempt to call without connection */
    if (g_mysql_reserved[handle] == qfalse)
    {
        Plugin_Scr_Error("'mysql_real_connection' must be called before.");
        return;
    }
}

/* =================================================================
 * Checks if function called after query.
 * For use only inside gsc callbacks.
   ================================================================= */
static void Scr_MySQL_CheckQuery(int handle)
{
    /* Attempt to call without query */
    if (g_mysql_res[handle] == NULL)
    {
        Plugin_Scr_Error("'mysql_query' must be called before.");
    }
}

/* =================================================================
 * Checks if function called after connection and query.
 * For use only inside gsc callbacks.
   ================================================================= */
static void Scr_MySQL_CheckCall(int handle)
{
    Scr_MySQL_CheckConnection(handle);
    Scr_MySQL_CheckQuery(handle);
}

/* =================================================================
 * Checks if handle correct, throws script runtime error otherwise.
 * For use only inside gsc callbacks.
   ================================================================= */
static int Scr_MySQL_GetHandle(int argnum)
{
    int handle = Plugin_Scr_GetInt(argnum);
    if(handle < 0 || handle >= MYSQL_CONNECTION_COUNT)
        Plugin_Scr_ParamError(argnum, "Incorrect connection handle");
    return handle;
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
    int argc = Plugin_Scr_GetNumParam();
    if (argc != 4 && argc != 5)
    {
        Plugin_Scr_Error("Usage: handle = mysql_real_connect(<str host>, "
                         "<str user>, <str passwd>, <str db or "">, "
                         "[int port=3306]);");
        return;
    }

    char* host = Plugin_Scr_GetString(0);
    char* user = Plugin_Scr_GetString(1);
    char* pass = Plugin_Scr_GetString(2);
    char* db = Plugin_Scr_GetString(3);
    int port = 3306;
    if (argc == 5)
    {
        port = Plugin_Scr_GetInt(4);
        if(port < 0 || port > 65535)
        {
            Plugin_Scr_ParamError(4, "Incorrect port: must be any integer "
                                     "from 0 to 65535");
            return;
        }
    }

    if (strcmp(host, "localhost") == 0)
        host = "127.0.0.1";

    if (db[0] == '\0')
        db = 0;

    int handle = 0;
    while(handle < MYSQL_CONNECTION_COUNT)
    {
        if (g_mysql_reserved[handle] == qfalse) /* Will be reserved a bit later */
            break;
        ++handle;

        if(handle == MYSQL_CONNECTION_COUNT - 1) /* Whoops */
        {
            Scr_MySQL_Error("MySQL connect error: max connections exceeded "
                            "(%d)", MYSQL_CONNECTION_COUNT);
            return;
        }
    }
    
    /* Would you like to reconnect if connection is dropped? */
    /* Allows the database to reconnect on a new query etc. */
    qboolean reconnect = qtrue;
    /* Check to see if the mySQL server connection has dropped */
    mysql_options(&g_mysql[handle], MYSQL_OPT_RECONNECT, &reconnect);
    
    MYSQL* result = mysql_real_connect(&g_mysql[handle], host, user, pass,
                                       db, port, NULL, 0);

    /* We don't want to crash the server, so we have a check to return nothing to prevent that */
    if (result == NULL)
    {
        Scr_MySQL_Error("MySQL connect error: (%d) %s", mysql_errno(&g_mysql[handle]),
                         mysql_error(&g_mysql[handle]));
        return;
    }
    g_mysql_reserved[handle] = qtrue;

    Plugin_Scr_AddInt(handle);
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
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_close(<handle>);");
        return;
    }
    int handle = Scr_MySQL_GetHandle(0);

    /* Closes the MySQL Handle Connection and frees its query result */
    Plugin_DPrintf("Closing MySQL connection: %d\n", handle);

    if(g_mysql_res[handle] != NULL)
    {
        mysql_free_result(g_mysql_res[handle]);
        g_mysql_res[handle] = NULL;
    }
    mysql_close(&g_mysql[handle]);
    g_mysql_reserved[handle] = qfalse;
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
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: rows = mysql_affected_rows(<handle>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    Scr_MySQL_CheckCall(handle);

    Plugin_Scr_AddInt(mysql_affected_rows(&g_mysql[handle]));
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
    if (Plugin_Scr_GetNumParam() != 2)
    {
        Plugin_Scr_Error("Usage: mysql_query(<handle>, <string query>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    char* query = Plugin_Scr_GetString(1);

    Scr_MySQL_CheckConnection(handle);

    if (mysql_query(&g_mysql[handle], query) == 0)
    {
        if(g_mysql_res[handle])
        {
            mysql_free_result(g_mysql_res[handle]);
            g_mysql_res[handle] = NULL;
        }

        g_mysql_res[handle] = mysql_store_result(&g_mysql[handle]);

        /* Result may be NULL, with errno == 0. */
        /* For example, try to create database. */
        /* Try fix something weird and call mysql_errno once. */
        unsigned int err = mysql_errno(&g_mysql[handle]);
        if(g_mysql_res[handle] == NULL && err != 0)
        {
            Scr_MySQL_Error("MySQL store error: (%d) %s",
                            err,
                            mysql_error(&g_mysql[handle]));
        }
    }
    else
    {
        Scr_MySQL_Error("MySQL query error: (%d) %s",
                        mysql_errno(&g_mysql[handle]),
                        mysql_error(&g_mysql[handle]));
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
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_num_rows(<handle>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    Scr_MySQL_CheckCall(handle);

    Plugin_Scr_AddInt(mysql_num_rows(g_mysql_res[handle]));
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
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_num_fields(<handle>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    Scr_MySQL_CheckCall(handle);

    Plugin_Scr_AddInt(mysql_num_fields(g_mysql_res[handle]));
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
void Scr_MySQL_Fetch_Row_f()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_fetch_row(<handle>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    Scr_MySQL_CheckCall(handle);

    unsigned int col_count = mysql_num_fields(g_mysql_res[handle]);
    MYSQL_ROW row = mysql_fetch_row(g_mysql_res[handle]);

    if(row != NULL)
    {
        Plugin_Scr_MakeArray();

        int i;
        mysql_field_seek(g_mysql_res[handle], 0);
        for(i = 0; i < col_count; ++i)
        {
            /* A little help here? I don't actually understand data
             * representation. Integer must be integer, string - string,
             * float - float */
            MYSQL_FIELD *field = mysql_fetch_field(g_mysql_res[handle]);
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

/* =================================================================
* Description
    Retrieves all rows from a query .
* Return Value(s)
    A MYSQL_ROW structure from a query. It will return a different
    array depending on the amount of rows. one row and it will return
    a single dimensioned array using the column names as the array
    keys. If it is more than one row then it will return a 2D array
    the first dimension will be a numerical iterator through the rows
    and the second dimension will be the columns names as array keys.
   ================================================================= */
void Scr_MySQL_Fetch_Rows_f()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_fetch_rows(<handle>);");
        return;
    }

    int handle = Scr_MySQL_GetHandle(0);
    Scr_MySQL_CheckCall(handle);

    unsigned int col_count = mysql_num_fields(g_mysql_res[handle]);

    /* Do this no matter what */
    Plugin_Scr_MakeArray();

    if(mysql_num_rows(g_mysql_res[handle]) != 0) /* Rows are exist */
    {
        int i = 0;
        int* keyArrayIndex = calloc(col_count, sizeof(int));
        MYSQL_FIELD* field;
        while((field = mysql_fetch_field(g_mysql_res[handle])) != NULL)
        {
            keyArrayIndex[i] = Plugin_Scr_AllocString(field->name);
            ++i;
        }

        MYSQL_ROW rows;
        while((rows = mysql_fetch_row(g_mysql_res[handle])) != NULL)
        {
            Plugin_Scr_MakeArray();
            for (i = 0; i < col_count; ++i) {
                Plugin_Scr_AddString(rows[i]);
                Plugin_Scr_AddArrayKey(keyArrayIndex[i]);
            }
            Plugin_Scr_AddArray();
        }
        free(keyArrayIndex);
    }
}
