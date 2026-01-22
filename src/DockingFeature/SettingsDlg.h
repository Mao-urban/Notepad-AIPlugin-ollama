//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include "StaticDialog.h"
#include "Utils/Config.h"
#include <vector>

class SettingsDlg : public StaticDialog
{
public:
    SettingsDlg() : StaticDialog() {}

    void init(HINSTANCE hInst, HWND parent);
    void doDialog();

protected:
    virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Control handles
    HWND _hProviderList = nullptr;
    HWND _hNameEdit = nullptr;
    HWND _hKeyEdit = nullptr;
    HWND _hUrlEdit = nullptr;
    HWND _hDefaultProviderCombo = nullptr;
    HWND _hEditModeCombo = nullptr;

    // Local copy of providers for editing
    std::vector<AIProviderConfig> _providers;
    int _selectedIndex = -1;

    // Initialize dialog controls
    void initControls();

    // Populate provider list
    void populateProviderList();

    // Update edit fields from selected provider
    void updateEditFields();

    // Get values from edit fields
    void getEditFieldValues(AIProviderConfig& provider);

    // Add new provider from fields
    void addProvider();

    // Update selected provider from fields
    void updateSelectedProvider();

    // Remove selected provider
    void removeSelectedProvider();

    // Save all settings to Config
    void saveSettings();

    // Owner-draw listbox handling
    void drawProviderItem(LPDRAWITEMSTRUCT lpDrawItem);
    void measureProviderItem(LPMEASUREITEMSTRUCT lpMeasureItem);
};

#endif // SETTINGSDLG_H
