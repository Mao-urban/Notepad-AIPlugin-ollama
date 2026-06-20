//this file is part of notepad_aiplug
//Copyright (C)2025

#include "SettingsDlg.h"
#include "resource.h"
#include <windowsx.h>

void SettingsDlg::init(HINSTANCE hInst, HWND parent)
{
    Window::init(hInst, parent);
}

void SettingsDlg::doDialog()
{
    // Copy current providers to local list before showing dialog
    _providers = Config::getInstance().getProviders();
    _selectedIndex = -1;

    // Show as modal dialog using DialogBox
    ::DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_SETTINGS), _hParent,
        dlgProc, reinterpret_cast<LPARAM>(this));
}

void SettingsDlg::initControls()
{
    // Get control handles
    _hProviderList = ::GetDlgItem(_hSelf, IDC_PROVIDER_LIST);
    _hNameEdit = ::GetDlgItem(_hSelf, IDC_PROVIDER_NAME_EDIT);
    _hKeyEdit = ::GetDlgItem(_hSelf, IDC_PROVIDER_KEY_EDIT);
    _hUrlEdit = ::GetDlgItem(_hSelf, IDC_PROVIDER_URL_EDIT);
    _hDefaultProviderCombo = ::GetDlgItem(_hSelf, IDC_DEFAULT_PROVIDER_COMBO);
    _hEditModeCombo = ::GetDlgItem(_hSelf, IDC_EDIT_MODE_COMBO);

    // Set text limits for edit controls
    ::SendMessage(_hNameEdit, EM_SETLIMITTEXT, 255, 0);
    ::SendMessage(_hKeyEdit, EM_SETLIMITTEXT, 255, 0);
    ::SendMessage(_hUrlEdit, EM_SETLIMITTEXT, 512, 0);  // URLs can be longer

    // Populate Edit Mode combo
    ::SendMessage(_hEditModeCombo, CB_RESETCONTENT, 0, 0);
    ::SendMessage(_hEditModeCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Diff First"));
    ::SendMessage(_hEditModeCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Direct Edit"));
    ::SendMessage(_hEditModeCombo, CB_SETCURSEL,
        static_cast<int>(Config::getInstance().getEditMode()), 0);

    // Clear edit fields initially
    ::SetWindowText(_hNameEdit, TEXT(""));
    ::SetWindowText(_hKeyEdit, TEXT(""));
    ::SetWindowText(_hUrlEdit, TEXT(""));
}

void SettingsDlg::populateProviderList()
{
    // Clear listbox
    ::SendMessage(_hProviderList, LB_RESETCONTENT, 0, 0);

    // Clear default provider combo
    ::SendMessage(_hDefaultProviderCombo, CB_RESETCONTENT, 0, 0);

    // Add providers to listbox
    for (size_t i = 0; i < _providers.size(); i++)
    {
        // Convert name to wide string
        int len = MultiByteToWideChar(CP_UTF8, 0, _providers[i].name.c_str(), -1, nullptr, 0);
        std::wstring wname(len - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, _providers[i].name.c_str(), -1, &wname[0], len);

        // Add to listbox (store index as item data)
        int idx = (int)::SendMessage(_hProviderList, LB_ADDSTRING, 0, (LPARAM)wname.c_str());
        ::SendMessage(_hProviderList, LB_SETITEMDATA, idx, (LPARAM)i);

        // Add to default provider combo
        ::SendMessage(_hDefaultProviderCombo, CB_ADDSTRING, 0, (LPARAM)wname.c_str());
    }

    // Set current default provider selection
    int defaultProvider = Config::getInstance().getDefaultProvider();
    if (defaultProvider >= 0 && defaultProvider < (int)_providers.size())
    {
        ::SendMessage(_hDefaultProviderCombo, CB_SETCURSEL, defaultProvider, 0);
    }
}

void SettingsDlg::updateEditFields()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_providers.size())
    {
        // Clear fields
        ::SetWindowText(_hNameEdit, TEXT(""));
        ::SetWindowText(_hKeyEdit, TEXT(""));
        ::SetWindowText(_hUrlEdit, TEXT(""));
        return;
    }

    const AIProviderConfig& provider = _providers[_selectedIndex];

    // Set name
    int len = MultiByteToWideChar(CP_UTF8, 0, provider.name.c_str(), -1, nullptr, 0);
    std::wstring wname(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, provider.name.c_str(), -1, &wname[0], len);
    ::SetWindowText(_hNameEdit, wname.c_str());

    // Set API key
    len = MultiByteToWideChar(CP_UTF8, 0, provider.apiKey.c_str(), -1, nullptr, 0);
    std::wstring wkey(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, provider.apiKey.c_str(), -1, &wkey[0], len);
    ::SetWindowText(_hKeyEdit, wkey.c_str());

    // Set custom URL (show default URL if empty for built-in providers)
    std::string urlToShow = provider.customEndpoint;
    if (urlToShow.empty() && provider.isBuiltIn)
    {
        // Show default URL based on API type
        switch (provider.apiType)
        {
            case APIType::OpenAI:
                urlToShow = "https://api.openai.com/v1/chat/completions";
                break;
            case APIType::Claude:
                urlToShow = "https://api.anthropic.com/v1/messages";
                break;
            case APIType::Gemini:
                urlToShow = "https://generativelanguage.googleapis.com/v1/models/{model}:generateContent";
                break;
			case APIType::Ollama:
				urlToShow = "http://127.0.0.1:11434/v1/chat/completions";
			break;	
        }
    }
    len = MultiByteToWideChar(CP_UTF8, 0, urlToShow.c_str(), -1, nullptr, 0);
    std::wstring wurl(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, urlToShow.c_str(), -1, &wurl[0], len);
    ::SetWindowText(_hUrlEdit, wurl.c_str());

    // Disable name edit for built-in providers
    ::EnableWindow(_hNameEdit, !provider.isBuiltIn);
}

void SettingsDlg::getEditFieldValues(AIProviderConfig& provider)
{
    // Get name
    wchar_t buffer[512] = {0};
    ::GetWindowText(_hNameEdit, buffer, 256);
    int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    provider.name.resize(len - 1);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &provider.name[0], len, nullptr, nullptr);

    // Default to OpenAI API type for new providers
    //provider.apiType = APIType::OpenAI;

    // Get API key
    memset(buffer, 0, sizeof(buffer));
    ::GetWindowText(_hKeyEdit, buffer, 256);
    len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    provider.apiKey.resize(len - 1);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &provider.apiKey[0], len, nullptr, nullptr);

    // Get custom URL (use larger buffer for URLs)
    memset(buffer, 0, sizeof(buffer));
    ::GetWindowText(_hUrlEdit, buffer, 512);
    len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    provider.customEndpoint.resize(len - 1);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &provider.customEndpoint[0], len, nullptr, nullptr);
	
	//Default to openAi type for new 
	if (provider.name == "Ollama" ||
		provider.customEndpoint.find("11434") != std::string::npos)
	{
		provider.apiType = APIType::Ollama;
	}
	else
	{
		provider.apiType = APIType::OpenAI;
	}

    // Clear custom endpoint if it matches the default URL (so default is used)
    if (provider.customEndpoint == "https://api.openai.com/v1/chat/completions" ||
        provider.customEndpoint == "https://api.anthropic.com/v1/messages" ||
        provider.customEndpoint == "https://generativelanguage.googleapis.com/v1/models/{model}:generateContent" ||
		provider.customEndpoint == "http://127.0.0.1:11434/v1/chat/completions" )
    {
        provider.customEndpoint.clear();
    }
	
}

void SettingsDlg::addProvider()
{
    AIProviderConfig newProvider;
    getEditFieldValues(newProvider);

    if (newProvider.name.empty())
    {
        ::MessageBox(_hSelf, TEXT("Please enter a provider name."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    newProvider.isBuiltIn = false;
    _providers.push_back(newProvider);
    populateProviderList();

    // Select the new provider
    _selectedIndex = (int)_providers.size() - 1;
    ::SendMessage(_hProviderList, LB_SETCURSEL, _selectedIndex, 0);
    updateEditFields();
}

void SettingsDlg::updateSelectedProvider()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_providers.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a provider first."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    AIProviderConfig updated;
    getEditFieldValues(updated);

    if (updated.name.empty())
    {
        ::MessageBox(_hSelf, TEXT("Please enter a provider name."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    // Preserve isBuiltIn flag and apiType
    updated.isBuiltIn = _providers[_selectedIndex].isBuiltIn;
    updated.apiType = _providers[_selectedIndex].apiType;

    // For built-in providers, preserve the name
    if (updated.isBuiltIn)
    {
        updated.name = _providers[_selectedIndex].name;
    }

    _providers[_selectedIndex] = updated;
    populateProviderList();

    // Reselect
    ::SendMessage(_hProviderList, LB_SETCURSEL, _selectedIndex, 0);
}

void SettingsDlg::removeSelectedProvider()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_providers.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a provider first."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    if (_providers[_selectedIndex].isBuiltIn)
    {
        ::MessageBox(_hSelf, TEXT("Cannot remove built-in providers."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    _providers.erase(_providers.begin() + _selectedIndex);
    _selectedIndex = -1;
    populateProviderList();
    updateEditFields();
}

void SettingsDlg::saveSettings()
{
    Config& config = Config::getInstance();

    // Check if there are unsaved changes in edit fields
    wchar_t nameBuffer[256] = {0};
    ::GetWindowText(_hNameEdit, nameBuffer, 256);

    if (wcslen(nameBuffer) > 0)
    {
        // Convert name to UTF-8 for comparison
        int len = WideCharToMultiByte(CP_UTF8, 0, nameBuffer, -1, nullptr, 0, nullptr, nullptr);
        std::string nameUtf8(len - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, nameBuffer, -1, &nameUtf8[0], len, nullptr, nullptr);

        // Check if this name already exists in providers
        bool exists = false;
        int existingIndex = -1;
        for (size_t i = 0; i < _providers.size(); i++)
        {
            if (_providers[i].name == nameUtf8)
            {
                exists = true;
                existingIndex = (int)i;
                break;
            }
        }

        if (exists)
        {
            // Update the existing provider with current field values
            _selectedIndex = existingIndex;
            updateSelectedProvider();
        }
        else
        {
            // Name doesn't exist, auto-add as new provider
            addProvider();
        }
    }

    // Update providers in Config
    config.getProviders() = _providers;

    // Update default provider
    int defaultProvider = (int)::SendMessage(_hDefaultProviderCombo, CB_GETCURSEL, 0, 0);
    config.setDefaultProvider(defaultProvider);

    // Update edit mode
    int editMode = (int)::SendMessage(_hEditModeCombo, CB_GETCURSEL, 0, 0);
    config.setEditMode(static_cast<EditMode>(editMode));

    // Save to file
    config.save();
}

void SettingsDlg::drawProviderItem(LPDRAWITEMSTRUCT lpDrawItem)
{
    if (lpDrawItem->itemID == (UINT)-1)
        return;

    // Get provider index from item data
    int providerIdx = (int)lpDrawItem->itemData;
    if (providerIdx < 0 || providerIdx >= (int)_providers.size())
        return;

    const AIProviderConfig& provider = _providers[providerIdx];

    // Set colors
    COLORREF textColor;
    COLORREF bkColor;

    if (lpDrawItem->itemState & ODS_SELECTED)
    {
        textColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
        bkColor = ::GetSysColor(COLOR_HIGHLIGHT);
    }
    else
    {
        bkColor = ::GetSysColor(COLOR_WINDOW);
        // Yellow text if no API key
        if (provider.apiKey.empty())
        {
            textColor = RGB(180, 140, 0); // Dark yellow/orange
        }
        else
        {
            textColor = ::GetSysColor(COLOR_WINDOWTEXT);
        }
    }

    // Fill background
    HBRUSH hBrush = ::CreateSolidBrush(bkColor);
    ::FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBrush);
    ::DeleteObject(hBrush);

    // Set text color
    ::SetTextColor(lpDrawItem->hDC, textColor);
    ::SetBkMode(lpDrawItem->hDC, TRANSPARENT);

    // Get item text
    wchar_t text[256] = {0};
    ::SendMessage(_hProviderList, LB_GETTEXT, lpDrawItem->itemID, (LPARAM)text);

    // Add status suffix
    std::wstring displayText = text;
    if (provider.apiKey.empty())
    {
        displayText += L" [No API Key]";
    }
    else if (provider.isBuiltIn)
    {
        displayText += L" [Built-in]";
    }

    // Draw text with padding
    RECT textRect = lpDrawItem->rcItem;
    textRect.left += 4;
    ::DrawText(lpDrawItem->hDC, displayText.c_str(), -1, &textRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Draw focus rect if focused
    if (lpDrawItem->itemState & ODS_FOCUS)
    {
        ::DrawFocusRect(lpDrawItem->hDC, &lpDrawItem->rcItem);
    }
}

void SettingsDlg::measureProviderItem(LPMEASUREITEMSTRUCT lpMeasureItem)
{
    // Set item height
    lpMeasureItem->itemHeight = 20;
}

INT_PTR CALLBACK SettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            initControls();
            populateProviderList();
            goToCenter();
            return TRUE;

        case WM_MEASUREITEM:
            if (wParam == IDC_PROVIDER_LIST)
            {
                measureProviderItem((LPMEASUREITEMSTRUCT)lParam);
                return TRUE;
            }
            break;

        case WM_DRAWITEM:
            if (wParam == IDC_PROVIDER_LIST)
            {
                drawProviderItem((LPDRAWITEMSTRUCT)lParam);
                return TRUE;
            }
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_PROVIDER_LIST:
                    if (HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        _selectedIndex = (int)::SendMessage(_hProviderList, LB_GETCURSEL, 0, 0);
                        updateEditFields();
                    }
                    return TRUE;

                case IDC_ADD_PROVIDER_BTN:
                    addProvider();
                    return TRUE;

                case IDC_UPDATE_PROVIDER_BTN:
                    updateSelectedProvider();
                    return TRUE;

                case IDC_REMOVE_PROVIDER_BTN:
                    removeSelectedProvider();
                    return TRUE;

                case IDOK:
                    saveSettings();
                    ::EndDialog(_hSelf, IDOK);
                    return TRUE;

                case IDCANCEL:
                    ::EndDialog(_hSelf, IDCANCEL);
                    return TRUE;
            }
            break;
        }

        case WM_CLOSE:
            ::EndDialog(_hSelf, IDCANCEL);
            return TRUE;
    }

    return FALSE;
}
