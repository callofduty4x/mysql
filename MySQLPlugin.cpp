#include <cstring>
#include <cstdio>

#include "MySQLPlugin.hpp"
#include "ScriptFunctions.hpp"

CMySQLPlugin* CMySQLPlugin::g_MySQLPlugin = NULL;

void InitPlugin()
{
    CMySQLPlugin::g_MySQLPlugin = new CMySQLPlugin();
}

CMySQLPlugin *const GetPlugin()
{
    return CMySQLPlugin::g_MySQLPlugin;
}

void FreePlugin()
{
    if (CMySQLPlugin::g_MySQLPlugin)
        delete CMySQLPlugin::g_MySQLPlugin;
}
/////////////////////////////////////////////////////////////////////////////////
CMySQLPlugin::CMySQLPlugin()
{
    for (int i = 0; i < MYSQL_CONNECTION_COUNT; ++i)
    {
        memset(&m_MySQL[i], 0, sizeof(MYSQL));
        m_MySQLResults[i] = NULL;
        m_MySQLInUse[i] = false;
    }
}
/////////////////////////////////////////////////////////////////////////////////
CMySQLPlugin::~CMySQLPlugin()
{
    Clear();
}
/////////////////////////////////////////////////////////////////////////////////
int CMySQLPlugin::OnInit()
{
    /* MySQL-documented */
    Plugin_ScrAddFunction("mysql_real_connect", Scr_MySQL_Real_Connect_f);
    Plugin_ScrAddFunction("mysql_close", Scr_MySQL_Close_f);
    Plugin_ScrAddFunction("mysql_affected_rows", Scr_MySQL_Affected_Rows_f);
    Plugin_ScrAddFunction("mysql_query", Scr_MySQL_Query_f);
    Plugin_ScrAddFunction("mysql_num_rows", Scr_MySQL_Num_Rows_f);
    Plugin_ScrAddFunction("mysql_num_fields", Scr_MySQL_Num_Fields_f);
    Plugin_ScrAddFunction("mysql_fetch_row", Scr_MySQL_Fetch_Row_f);
    /* MySQL-custom */
    Plugin_ScrAddFunction("mysql_fetch_rows", Scr_MySQL_Fetch_Rows_f);

    return 0;
}
/////////////////////////////////////////////////////////////////////////////////
void CMySQLPlugin::Clear()
{
    for (int i = 0; i < MYSQL_CONNECTION_COUNT; ++i)
    {
        if (!m_MySQLInUse[i])
            continue;
        // Free query result.
        if (m_MySQLResults[i])
            mysql_free_result(m_MySQLResults[i]);

        // Close opened connection.
        mysql_close(&m_MySQL[i]);
        // Clean up data.
        m_MySQLResults[i] = NULL;
        m_MySQLInUse[i] = false;
    }
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Real_Connect()
{

    int argc = Plugin_Scr_GetNumParam();
    if (argc != 4 && argc != 5)
    {
        Plugin_Scr_Error("Usage: handle = mysql_real_connect(<str host>, "
                         "<str user>, <str passwd>, <str db or "
                         ">, "
                         "[int port=3306]);");
        return;
    }

    const char *host = Plugin_Scr_GetString(0);
    char *user = Plugin_Scr_GetString(1);
    char *pass = Plugin_Scr_GetString(2);
    char *db = Plugin_Scr_GetString(3);
    int port = 3306;
    if (argc == 5)
    {
        port = Plugin_Scr_GetInt(4);
        if (port < 0 || port > 65535)
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

    int i = 0; // todo: for
    for (i = 0; i < MYSQL_CONNECTION_COUNT; ++i)
    {
        if (m_MySQLInUse[i] == false) /* Will be reserved a bit later */
            break;
    }
    if (i == MYSQL_CONNECTION_COUNT - 1) /* Whoops */
    {
        pluginError("MySQL connect error: max connections exceeded "
                    "(%d)",
                    MYSQL_CONNECTION_COUNT);
        return;
    }

    MYSQL *pMysql = &m_MySQL[i];
    // Init connections.
    if (mysql_init(&m_MySQL[i]) == NULL)
    {
        Plugin_PrintError("MySQL init[%d] failed: (%d) %s", i,
            mysql_errno(&m_MySQL[i]), mysql_error(&m_MySQL[i]));
        return;
    }
    /* Would you like to reconnect if connection is dropped? */
    /* Allows the database to reconnect on a new query etc. */
    qboolean reconnect = qtrue;
    /* Check to see if the mySQL server connection has dropped */
    mysql_options(pMysql, MYSQL_OPT_RECONNECT, &reconnect);

    MYSQL *result = mysql_real_connect(pMysql, host, user, pass,
                                       db, port, NULL, 0);

    /* We don't want to crash the server, so we have a check to return nothing to prevent that */
    if (result == NULL)
    {
        pluginError("MySQL connect error: (%d) %s", mysql_errno(pMysql),
                    mysql_error(pMysql));
        return;
    }
    m_MySQLInUse[i] = true;

    Plugin_Scr_AddInt(i);
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Close()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_close(<handle>);");
        return;
    }
    unsigned int idx = getHandleIndexForScriptArg(0);

    /* Closes the MySQL Handle Connection and frees its query result */
    Plugin_DPrintf("Closing MySQL connection: %d\n", idx);

    if (m_MySQLResults[idx] != NULL)
    {
        mysql_free_result(m_MySQLResults[idx]);
        m_MySQLResults[idx] = NULL;
    }
    mysql_close(&m_MySQL[idx]);
    m_MySQLInUse[idx] = false;
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Affected_Rows()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: rows = mysql_affected_rows(<handle>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    checkConnection(idx);
    checkQuery(idx);

    Plugin_Scr_AddInt(mysql_affected_rows(&m_MySQL[idx]));
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Query()
{
    if (Plugin_Scr_GetNumParam() != 2)
    {
        Plugin_Scr_Error("Usage: mysql_query(<handle>, <string query>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    char *query = Plugin_Scr_GetString(1);

    checkConnection(idx);

    if (mysql_query(&m_MySQL[idx], query) == 0)
    {
        if (m_MySQLResults[idx])
        {
            mysql_free_result(m_MySQLResults[idx]);
            m_MySQLResults[idx] = NULL;
        }

        m_MySQLResults[idx] = mysql_store_result(&m_MySQL[idx]);

        /* Result may be NULL, with errno == 0. */
        /* For example, try to create database. */
        /* Try fix something weird and call mysql_errno once. */
        unsigned int err = mysql_errno(&m_MySQL[idx]);
        if (m_MySQLResults[idx] == NULL && err != 0)
        {
            pluginError("MySQL store error: (%d) %s",
                        err,
                        mysql_error(&m_MySQL[idx]));
        }
    }
    else
    {
        pluginError("MySQL query error: (%d) %s",
                    mysql_errno(&m_MySQL[idx]),
                    mysql_error(&m_MySQL[idx]));
    }
}
/////////////////////////////////////////////////////////////////////////////////
/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-num-rows.html
* Description
    Returns the number of rows in the result set.
* Return Value(s)
    The number of rows in the result set.
   ================================================================= */
void CMySQLPlugin::OnScript_Num_Rows()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_num_rows(<handle>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    checkConnection(idx);
    checkQuery(idx);

    Plugin_Scr_AddInt(mysql_num_rows(m_MySQLResults[idx]));
}
/////////////////////////////////////////////////////////////////////////////////
/* =================================================================
* URL
    http://dev.mysql.com/doc/refman/5.7/en/mysql-num-fields.html
* Description
    Returns the number of columns in a result set.
* Return Value(s)
    An unsigned integer representing the number of columns in a result set.
   ================================================================= */
void CMySQLPlugin::OnScript_Num_Fields()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_num_fields(<handle>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    checkConnection(idx);
    checkQuery(idx);

    Plugin_Scr_AddInt(mysql_num_fields(m_MySQLResults[idx]));
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Fetch_Row()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_fetch_row(<handle>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    checkConnection(idx);
    checkQuery(idx);

    unsigned int col_count = mysql_num_fields(m_MySQLResults[idx]);
    MYSQL_ROW row = mysql_fetch_row(m_MySQLResults[idx]);

    if (row != NULL)
    {
        Plugin_Scr_MakeArray();

        mysql_field_seek(m_MySQLResults[idx], 0);
        for (unsigned int i = 0; i < col_count; ++i)
        {
            /* A little help here? I don't actually understand data
             * representation. Integer must be integer, string - string,
             * float - float */
            MYSQL_FIELD *field = mysql_fetch_field(m_MySQLResults[idx]);
            if (field == NULL)
            {
                pluginError("Houston, we got a problem: unnamed column!");
                return;
            }
            Plugin_Scr_AddString(row[i]);
            Plugin_Scr_AddArrayKey(Plugin_Scr_AllocString(field->name));
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////
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
void CMySQLPlugin::OnScript_Fetch_Rows()
{
    if (Plugin_Scr_GetNumParam() != 1)
    {
        Plugin_Scr_Error("Usage: mysql_fetch_rows(<handle>);");
        return;
    }

    int idx = getHandleIndexForScriptArg(0);
    checkConnection(idx);
    checkQuery(idx);
    MYSQL_RES *mysqlResult = m_MySQLResults[idx];

    /* Do this no matter what */
    Plugin_Scr_MakeArray();

    if (mysql_num_rows(mysqlResult) == 0) /* Rows are exist */
        return;

    unsigned int i = 0;
    unsigned int col_count = mysql_num_fields(mysqlResult);
    int *keyArrayIndex = reinterpret_cast<int *>(Plugin_Malloc(col_count * sizeof(int)));
    MYSQL_FIELD *field;
    while ((field = mysql_fetch_field(mysqlResult)) != NULL)
    {
        keyArrayIndex[i] = Plugin_Scr_AllocString(field->name);
        ++i;
    }

    MYSQL_ROW rows;
    while ((rows = mysql_fetch_row(mysqlResult)) != NULL)
    {
        Plugin_Scr_MakeArray();
        for (unsigned int i = 0; i < col_count; ++i)
        {
            Plugin_Scr_AddString(rows[i]);
            Plugin_Scr_AddArrayKey(keyArrayIndex[i]);
        }
        Plugin_Scr_AddArray();
    }
    Plugin_Free(keyArrayIndex);
}
/////////////////////////////////////////////////////////////////////////////////
void CMySQLPlugin::OnInfoRequest(pluginInfo_t *Info_)
{
    Info_->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
    Info_->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;

    Info_->pluginVersion.major = getMajorVersion();
    Info_->pluginVersion.minor = getMinorVersion();

    const char *const name = getName();
    strncpy(Info_->fullName, name, strlen(name));

    const char *const shortDescription = getShortDescription();
    strncpy(Info_->shortDescription, shortDescription, strlen(shortDescription));

    char longDescription[1024] = {'\0'};
    sprintf(longDescription, getDescription(), mysql_get_client_version());
    longDescription[sizeof(longDescription) - 1] = '\0';

    strncpy(Info_->longDescription, longDescription, sizeof(longDescription));
}
/////////////////////////////////////////////////////////////////////////////////
const char *const CMySQLPlugin::getName()
{
    return "CoD4X MySQL Plugin";
}
/////////////////////////////////////////////////////////////////////////////////
const char *const CMySQLPlugin::getDescription()
{
    return "CoD4X MySQL Plugin allows you to query information from "
           "mysql database. MySQL version: %lu";
}
/////////////////////////////////////////////////////////////////////////////////
const char *const CMySQLPlugin::getShortDescription()
{
    return "CoD4X MySQL Plugin by Sharpienero, MichaelHillcox, T-Max";
}
/////////////////////////////////////////////////////////////////////////////////
unsigned int CMySQLPlugin::getMajorVersion()
{
    return 2;
}
/////////////////////////////////////////////////////////////////////////////////
unsigned int CMySQLPlugin::getMinorVersion()
{
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////
int CMySQLPlugin::getHandleIndexForScriptArg(const int ArgNum_) const
{
    int idx = Plugin_Scr_GetInt(ArgNum_);
    if (idx < 0 || idx >= MYSQL_CONNECTION_COUNT)
        Plugin_Scr_ParamError(ArgNum_, "Incorrect connection handle");
    return idx;
}
/////////////////////////////////////////////////////////////////////////////////
void CMySQLPlugin::checkConnection(const int HandleIndex_) const
{
    if (HandleIndex_ < 0 || HandleIndex_ >= MYSQL_CONNECTION_COUNT)
    {
        Plugin_Scr_Error("Incorrect connection handle");
        return;
    }

    // Attempt to call without connection
    if (m_MySQLInUse[HandleIndex_] == false)
        Plugin_Scr_Error("'mysql_real_connection' must be called before.");
}
/////////////////////////////////////////////////////////////////////////////////
void CMySQLPlugin::checkQuery(const int HandleIndex_) const
{
    if (HandleIndex_ < 0 || HandleIndex_ >= MYSQL_CONNECTION_COUNT)
    {
        Plugin_Scr_Error("Incorrect connection handle");
        return;
    }

    // Attempt to call without query
    if (m_MySQLResults[HandleIndex_] == NULL)
        Plugin_Scr_Error("'mysql_query' must be called before.");
}
/////////////////////////////////////////////////////////////////////////////////
void CMySQLPlugin::pluginError(const char *const Format_, ...) const
{
    char buffer[1024] = {'\0'};
    va_list va;
    va_start(va, Format_);
    _vsnprintf(buffer, sizeof(buffer), Format_, va);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(va);

    Plugin_Scr_Error(buffer);
}
