//this file is part of notepad_aiplug
//Copyright (C)2025

#include "PluginDefinition.h"
#include "Utils/Config.h"
#include <shlobj.h>

extern FuncItem funcItem[nbFunc];
extern NppData nppData;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/)
{
    try {
        switch (reasonForCall)
        {
            case DLL_PROCESS_ATTACH:
                pluginInit(hModule);
                break;

            case DLL_PROCESS_DETACH:
                pluginCleanUp();
                break;

            case DLL_THREAD_ATTACH:
                break;

            case DLL_THREAD_DETACH:
                break;
        }
    }
    catch (...) { return FALSE; }

    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
    nppData = notpadPlusData;

    // Initialize config with Notepad++ config directory
    TCHAR configDir[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);
    Config::getInstance().init(configDir);

    commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
    return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
    *nbF = nbFunc;
    return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    switch (notifyCode->nmhdr.code)
    {
        case NPPN_READY:
        {
            // Notepad++ is fully loaded - can auto-show panel here if desired
            break;
        }

        case NPPN_SHUTDOWN:
        {
            // Save config before shutdown
            Config::getInstance().save();
            commandMenuCleanUp();
            break;
        }

        default:
            return;
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
