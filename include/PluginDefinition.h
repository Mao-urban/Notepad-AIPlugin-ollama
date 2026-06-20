//this file is part of notepad_aiplug
//Copyright (C)2025
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

#include "PluginInterface.h"

// Plugin name
const TCHAR NPP_PLUGIN_NAME[] = TEXT("AI Assistant");

// Number of plugin commands
const int nbFunc = 3;

// Menu command indices
#define CMD_SHOW_AI_PANEL    0
#define CMD_SETTINGS         1
#define CMD_ABOUT            2

// Plugin initialization
void pluginInit(HANDLE hModule);
void pluginCleanUp();

// Command menu
void commandMenuInit();
void commandMenuCleanUp();

// Set command helper
bool setCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);

// Plugin command functions
void showAIPanel();
void showSettings();
void showAbout();

// Get plugin module handle
HINSTANCE getPluginInstance();

#endif //PLUGINDEFINITION_H
