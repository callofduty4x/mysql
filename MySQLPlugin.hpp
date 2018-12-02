#pragma once
#include "../pinc.h"

//extern "C"
//{
    #ifdef WIN32
    #include "mysql/windows/include/mysql.h"
    #else
    #include "mysql/unix/include/mysql.h"
    #endif
//}

#define MYSQL_CONNECTION_COUNT 4

class CMySQLPlugin
{
public:
    CMySQLPlugin();
    ~CMySQLPlugin();

    // No copy class.
    CMySQLPlugin(const CMySQLPlugin& Other_) = delete;
    void operator = (const CMySQLPlugin& Other_) = delete;

    int OnInit();
    static void OnInfoRequest(pluginInfo_t* Info_);
    void Clear();

    /* MySQL-documented */
    void OnScript_Real_Connect();
    void OnScript_Close();
    void OnScript_Affected_Rows();
    void OnScript_Query();
    void OnScript_Num_Rows();
    void OnScript_Num_Fields();
    void OnScript_Fetch_Row();
    /* MySQL-custom */
    void OnScript_Fetch_Rows();

    static CMySQLPlugin* g_MySQLPlugin;

private:
    // Maintenance methods.
    static const char* const getName();
    static const char* const getDescription();
    static const char* const getShortDescription();
    static unsigned int getMajorVersion();
    static unsigned int getMinorVersion();

    // Script methods.
    int getHandleIndexForScriptArg(const int ArgNum_) const;
    void checkConnection(const int HandleIndex_) const;
    void checkQuery(const int HandleIndex_) const;
    void pluginError(const char* const Format_, ...) const;

    
    // Members.
    MYSQL m_MySQL[MYSQL_CONNECTION_COUNT];
    MYSQL_RES* m_MySQLResults[MYSQL_CONNECTION_COUNT];
    bool m_MySQLInUse[MYSQL_CONNECTION_COUNT];
    unsigned int m_MYSQLErrNo[MYSQL_CONNECTION_COUNT];
};

void InitPlugin();
CMySQLPlugin* const GetPlugin();
void FreePlugin();
