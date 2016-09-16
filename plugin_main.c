#include "../pinc.h"

#include "mysql/include/mysql.h"
#include "script_functions.h"

#include "stdio.h"


#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 0

#define PLUGIN_NAME "CoD4X MySQL Plugin"
#define PLUGIN_DESCR PLUGIN_NAME" allows you to query information from " \
                    "mysql database. MySQL version: %s"
#define PLUGIN_SHORT PLUGIN_NAME" by T-Max, Sharpienero, MichaelHillcox"

MYSQL mysql;
MYSQL_RES* mysql_res;

cvar_t *g_mysql_port;

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
	// We only need port
	g_mysql_port = Plugin_Cvar_RegisterInt("mysql_port", 3306, 0, 65536, CVAR_ARCHIVE, "Port for connection");

	Plugin_ScrAddFunction("mysql_real_connect", Scr_MySQL_Real_Connect_f);
	Plugin_ScrAddFunction("mysql_close", Scr_MySQL_Close_f);
	Plugin_ScrAddFunction("mysql_affected_rows", Scr_MySQL_Affected_Rows_f);
	Plugin_ScrAddFunction("mysql_query", Scr_MySQL_Query_f);
	Plugin_ScrAddFunction("mysql_num_rows", Scr_MySQL_Num_Rows_f);
	Plugin_ScrAddFunction("mysql_num_fields", Scr_MySQL_Num_Fields_f);
	Plugin_ScrAddFunction("mysql_fetch_rows", Scr_MySQL_Fetch_Rows_f);
	// TODO: @michaelhillcox add more mysql functions for better support

	if (mysql_init(&mysql) == NULL)
	{
		Plugin_PrintError("MySQL plugin initialization failed: (%d) %s\n",
		                   mysql_errno(&mysql), mysql_error(&mysql));
	}
}

PCL void OnTerminate()
{
	if(mysql_res)
		mysql_free_result(mysql_res);
	mysql_close(&mysql);
}
