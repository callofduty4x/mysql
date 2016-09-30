/* Std lib includes */
#include "stdio.h"

/* Plugin includes */
#include "../pinc.h"
#include "script_functions.h"

/* OS-specific includes */
#ifdef WIN32
	#include "mysql/windows/include/mysql.h"
#else
	#include "mysql/unix/include/mysql.h"
#endif

/* Plugin definitions */
#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 1

#define PLUGIN_NAME "CoD4X MySQL Plugin"
#define PLUGIN_DESCR PLUGIN_NAME" allows you to query information from " \
                    "mysql database. MySQL version: %lu"
#define PLUGIN_SHORT PLUGIN_NAME" by Sharpienero, MichaelHillcox, T-Max"

/* Globals */
MYSQL g_mysql[MYSQL_CONNECTION_COUNT];
MYSQL_RES* g_mysql_res[MYSQL_CONNECTION_COUNT];
qboolean g_mysql_reserved[MYSQL_CONNECTION_COUNT];

PCL void OnInfoRequest(pluginInfo_t *info)
{
	char description[1024] = {'\0'};

	sprintf(description, PLUGIN_DESCR, mysql_get_client_version());
	description[sizeof(description) - 1] = '\0';

	info->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
	info->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;

	info->pluginVersion.major = PLUGIN_VERSION_MAJOR;
	info->pluginVersion.minor = PLUGIN_VERSION_MINOR;

	memcpy(info->fullName, PLUGIN_NAME, sizeof(PLUGIN_NAME));
	memcpy(info->shortDescription, PLUGIN_SHORT, sizeof(PLUGIN_SHORT));
	memcpy(info->longDescription, description, sizeof(description));
}

PCL int OnInit()
{
	//g_mysql_port = Plugin_Cvar_RegisterInt("mysql_port", 3306, 0, 65535, CVAR_ARCHIVE, "Port for connection");

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

	// TODO: @michaelhillcox add more mysql functions for better support
	int i = 0;
	while(i < MYSQL_CONNECTION_COUNT)
	{
		if (mysql_init(&g_mysql[i]) == NULL)
		{
			Plugin_PrintError("MySQL plugin initialization[%d] failed: "
			                  "(%d) %s", i, mysql_errno(&g_mysql[i]),
			                  mysql_error(&g_mysql[i]));
		}
		g_mysql_res[i] = NULL;
		++i;
	}

	return 0;
}

PCL void OnTerminate()
{
	int i = 0;
	while(i < MYSQL_CONNECTION_COUNT)
	{
		if(g_mysql_res[i])
			mysql_free_result(g_mysql_res[i]);
		mysql_close(&g_mysql[i]);
		++i;
	}
}
