//this file is part of notepad_aiplug
//Copyright (C)2025
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "DockingFeature/AIPanelDlg.h"
#include "DockingFeature/SettingsDlg.h"

// The plugin data that Notepad++ needs
FuncItem funcItem[nbFunc];

// The data of Notepad++ that you can use in your plugin commands
NppData nppData;

// Module instance handle
static HINSTANCE g_hInstance = NULL;

// AI Panel dialog instance
static AIPanelDlg g_aiPanelDlg;

// Settings dialog instance
static SettingsDlg g_settingsDlg;

// Shortcut for Show AI Panel (Ctrl+Shift+A)
static ShortcutKey g_showPanelShortcut = {true, true, false, 'A'};

HINSTANCE getPluginInstance()
{
    return g_hInstance;
}

void pluginInit(HANDLE hModule)
{
    g_hInstance = (HINSTANCE)hModule;
}

void pluginCleanUp()
{
    g_aiPanelDlg.destroy();
}

void commandMenuInit()
{
    // Initialize AI Panel dialog
    g_aiPanelDlg.init(g_hInstance, nppData._nppHandle);

    // Initialize Settings dialog
    g_settingsDlg.init(g_hInstance, nppData._nppHandle);

    // Set up menu commands
    setCommand(CMD_SHOW_AI_PANEL, TEXT("Show AI Panel"), showAIPanel, &g_showPanelShortcut, false);
    setCommand(CMD_SETTINGS, TEXT("Settings..."), showSettings, NULL, false);
    setCommand(CMD_ABOUT, TEXT("About"), showAbout, NULL, false);
}

void commandMenuCleanUp()
{
    // Clean up shortcut if needed
}

bool setCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

void showAIPanel()
{
    g_aiPanelDlg.setParent(nppData._nppHandle);

    // Register the dockable dialog if not already registered
    static bool isRegistered = false;
    if (!isRegistered)
    {
        tTbData data = {};
        g_aiPanelDlg.create(&data);

        data.uMask = DWS_DF_CONT_RIGHT;  // Default dock on right side
        data.pszModuleName = g_aiPanelDlg.getPluginFileName();
        data.dlgID = CMD_SHOW_AI_PANEL;

        ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
        isRegistered = true;
    }

    // Toggle visibility
    g_aiPanelDlg.display(!g_aiPanelDlg.isVisible());
}

void showSettings()
{
    g_settingsDlg.doDialog();

    // Refresh provider dropdown in AI Panel after settings change
    g_aiPanelDlg.refreshProviderList();
}

void showAbout()
{
    ::MessageBox(nppData._nppHandle,
        TEXT("AI Assistant for Notepad++\n\n")
        TEXT("Version 2.0.1\n\n")
        TEXT("Integrates AI capabilities directly into Notepad++.\n")
        TEXT("Supports ChatGPT, Claude, Ollama and Gemini.\n\n")
        TEXT("Copyright (C) 2025"),
        TEXT("About AI Assistant"),
        MB_OK | MB_ICONINFORMATION);
}
