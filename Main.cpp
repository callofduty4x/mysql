#include "MySQLPlugin.hpp"

PCL void OnInfoRequest(pluginInfo_t *info)
{
    CMySQLPlugin::OnInfoRequest(info);
}

PCL int OnInit()
{
    InitPlugin();
    return GetPlugin()->OnInit();
}

PCL void OnTerminate()
{
    FreePlugin();
}

PCL void OnExitLevel()
{
    GetPlugin()->Clear();
}
