#include "ScriptFunctions.hpp"
#include "MySQLPlugin.hpp"

void Scr_MySQL_Real_Connect_f()
{
    GetPlugin()->OnScript_Real_Connect();
}

void Scr_MySQL_Close_f()
{
    GetPlugin()->OnScript_Close();
}

void Scr_MySQL_Affected_Rows_f()
{
    GetPlugin()->OnScript_Affected_Rows();
}

void Scr_MySQL_Query_f()
{
    GetPlugin()->OnScript_Query();
}

void Scr_MySQL_Num_Rows_f()
{
    GetPlugin()->OnScript_Num_Rows();
}

void Scr_MySQL_Num_Fields_f()
{
    GetPlugin()->OnScript_Num_Fields();
}

void Scr_MySQL_Fetch_Row_f()
{
    GetPlugin()->OnScript_Fetch_Row();
}

void Scr_MySQL_Fetch_Rows_f()
{
    GetPlugin()->OnScript_Fetch_Rows();
}
