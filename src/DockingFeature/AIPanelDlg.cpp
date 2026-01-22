//this file is part of notepad_aiplug
//Copyright (C)2025

#include "AIPanelDlg.h"
#include "PluginDefinition.h"
#include "AIProviders/AIProvider.h"
#include "AIProviders/ProviderFactory.h"
#include "Utils/Config.h"
#include "Scintilla.h"
#include "Notepad_plus_msgs.h"
#include <ctime>
#include <sstream>
#include <thread>
#include <tuple>

extern NppData nppData;

INT_PTR CALLBACK AIPanelDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            initControls();
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_SEND_BTN:
                    sendPrompt();
                    return TRUE;

                case IDC_CLEAR_BTN:
                    clearHistory();
                    return TRUE;

                case IDC_APPLY_BTN:
                    applyLastResponse();
                    return TRUE;
            }
            break;
        }

        case WM_SIZE:
        {
            resizeControls();
            return TRUE;
        }

        case WM_SHOWWINDOW:
        {
            if (wParam == TRUE)  // Window is being shown
            {
                refreshProviderList();
            }
            return TRUE;
        }

        // Handle AI response received (lParam = tuple<responseW, response, providerName>)
        case WM_USER + 100:
        {
            auto* data = reinterpret_cast<std::tuple<std::wstring, std::string, std::wstring>*>(lParam);
            if (data)
            {
                addMessage(false, std::get<0>(*data), std::get<2>(*data));
                _lastResponse = std::get<1>(*data);
                ::EnableWindow(_hApplyBtn, TRUE);
                delete data;
            }
            return TRUE;
        }

        // Handle provider creation failed (lParam = providerName)
        case WM_USER + 101:
        {
            auto* providerName = reinterpret_cast<std::wstring*>(lParam);
            std::wstring name = providerName ? *providerName : L"AI";
            addMessage(false, L"Error: Failed to create provider. Check your API key.", name);
            delete providerName;
            return TRUE;
        }

        // Handle exception error (lParam = pair<errorMsg, providerName>)
        case WM_USER + 102:
        {
            auto* data = reinterpret_cast<std::pair<std::wstring, std::wstring>*>(lParam);
            if (data)
            {
                addMessage(false, L"Error: " + data->first, data->second);
                delete data;
            }
            return TRUE;
        }

        default:
            return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
    }
    return FALSE;
}

void AIPanelDlg::initControls()
{
    // Get control handles
    _hProviderCombo = ::GetDlgItem(_hSelf, IDC_PROVIDER_COMBO);
    _hChatHistory = ::GetDlgItem(_hSelf, IDC_CHAT_HISTORY);
    _hPromptEdit = ::GetDlgItem(_hSelf, IDC_PROMPT_EDIT);
    _hSendBtn = ::GetDlgItem(_hSelf, IDC_SEND_BTN);
    _hClearBtn = ::GetDlgItem(_hSelf, IDC_CLEAR_BTN);
    _hIncludeContextChk = ::GetDlgItem(_hSelf, IDC_INCLUDE_CONTEXT_CHK);
    _hApplyBtn = ::GetDlgItem(_hSelf, IDC_APPLY_BTN);

    // Populate provider combo from Config
    refreshProviderList();

    // Set include context checkbox to checked by default
    if (_hIncludeContextChk)
    {
        ::SendMessage(_hIncludeContextChk, BM_SETCHECK, BST_CHECKED, 0);
    }

    // Disable apply button initially
    if (_hApplyBtn)
    {
        ::EnableWindow(_hApplyBtn, FALSE);
    }
}

void AIPanelDlg::refreshProviderList()
{
    // If dialog not created yet, controls will be populated in initControls()
    if (!_hSelf || !_hProviderCombo)
        return;

    // Save current selection
    int currentSel = (int)::SendMessage(_hProviderCombo, CB_GETCURSEL, 0, 0);

    // Clear combo
    ::SendMessage(_hProviderCombo, CB_RESETCONTENT, 0, 0);

    // Add providers from Config
    const auto& providers = Config::getInstance().getProviders();
    for (size_t i = 0; i < providers.size(); i++)
    {
        // Convert name to wide string
        int len = MultiByteToWideChar(CP_UTF8, 0, providers[i].name.c_str(), -1, nullptr, 0);
        std::wstring wname(len - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, providers[i].name.c_str(), -1, &wname[0], len);

        // Add indicator if no API key
        if (providers[i].apiKey.empty())
        {
            wname += L" [No Key]";
        }

        ::SendMessage(_hProviderCombo, CB_ADDSTRING, 0, (LPARAM)wname.c_str());
    }

    // Restore selection or use default
    if (currentSel >= 0 && currentSel < (int)providers.size())
    {
        ::SendMessage(_hProviderCombo, CB_SETCURSEL, currentSel, 0);
    }
    else
    {
        int defaultProvider = Config::getInstance().getDefaultProvider();
        ::SendMessage(_hProviderCombo, CB_SETCURSEL, defaultProvider, 0);
    }
}

void AIPanelDlg::sendPrompt()
{
    // Get prompt text
    int len = ::GetWindowTextLength(_hPromptEdit);
    if (len == 0)
        return;

    // Get current provider and check for API key
    int providerIndex = getCurrentProviderIndex();
    const auto& providers = Config::getInstance().getProviders();

    if (providerIndex < 0 || providerIndex >= (int)providers.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a valid provider."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    if (providers[providerIndex].apiKey.empty())
    {
        ::MessageBox(_hSelf, TEXT("No API key configured for this provider.\n\nPlease go to Settings to add your API key."),
            TEXT("No API Key"), MB_OK | MB_ICONWARNING);
        return;
    }

    std::vector<TCHAR> buffer(len + 1);
    ::GetWindowText(_hPromptEdit, buffer.data(), len + 1);
    std::wstring prompt(buffer.data());

    // Add user message to history
    addMessage(true, prompt);

    // Check if we should include context
    bool includeContext = (::SendMessage(_hIncludeContextChk, BM_GETCHECK, 0, 0) == BST_CHECKED);

    // Build context string
    std::string context;
    if (includeContext)
    {
        std::string selectedText = getSelectedText();
        if (!selectedText.empty())
        {
            context = "Selected text:\n```\n" + selectedText + "\n```\n\n";
        }
        else
        {
            // Get current file info and content
            std::wstring filePath = getCurrentFilePath();
            std::wstring language = getCurrentLanguage();
            std::string fileContent = getCurrentDocumentText();

            if (!filePath.empty())
            {
                // Convert wide strings to narrow for context
                int pathLen = WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string pathStr(pathLen - 1, 0);
                WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, &pathStr[0], pathLen, nullptr, nullptr);

                int langLen = WideCharToMultiByte(CP_UTF8, 0, language.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string langStr(langLen - 1, 0);
                WideCharToMultiByte(CP_UTF8, 0, language.c_str(), -1, &langStr[0], langLen, nullptr, nullptr);

                context = "File: " + pathStr + "\nLanguage: " + langStr + "\n\nFile content:\n```\n" + fileContent + "\n```\n\n";
            }
        }
    }

    // Convert prompt to UTF-8
    int promptLen = WideCharToMultiByte(CP_UTF8, 0, prompt.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string promptUtf8(promptLen - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, prompt.c_str(), -1, &promptUtf8[0], promptLen, nullptr, nullptr);

    // Clear prompt input
    ::SetWindowText(_hPromptEdit, TEXT(""));

    // Get provider name for display
    std::wstring providerNameW;
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, providers[providerIndex].name.c_str(), -1, nullptr, 0);
        providerNameW.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, providers[providerIndex].name.c_str(), -1, &providerNameW[0], len);
    }

    // Show loading indicator
    addMessage(false, L"Thinking...", providerNameW);

    // Create provider and send request in background thread
    std::thread([this, providerIndex, promptUtf8, context, providerNameW]() {
        try
        {
            auto provider = ProviderFactory::createProvider(providerIndex);
            if (provider)
            {
                std::string response = provider->sendPrompt(promptUtf8, context);

                // Convert response to wide string
                int responseLen = MultiByteToWideChar(CP_UTF8, 0, response.c_str(), -1, nullptr, 0);
                std::wstring responseW(responseLen - 1, 0);
                MultiByteToWideChar(CP_UTF8, 0, response.c_str(), -1, &responseW[0], responseLen);

                // Update UI on main thread (pass provider name)
                ::PostMessage(_hSelf, WM_USER + 100, 0,
                    (LPARAM)new std::tuple<std::wstring, std::string, std::wstring>(responseW, response, providerNameW));
            }
            else
            {
                ::PostMessage(_hSelf, WM_USER + 101, 0, (LPARAM)new std::wstring(providerNameW));
            }
        }
        catch (const std::exception& e)
        {
            int len = MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0);
            std::wstring errorW(len - 1, 0);
            MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, &errorW[0], len);
            ::PostMessage(_hSelf, WM_USER + 102, 0,
                (LPARAM)new std::pair<std::wstring, std::wstring>(errorW, providerNameW));
        }
    }).detach();
}

void AIPanelDlg::clearHistory()
{
    _chatHistory.clear();
    _lastResponse.clear();
    updateChatDisplay();
    ::EnableWindow(_hApplyBtn, FALSE);
}

void AIPanelDlg::updateChatDisplay()
{
    if (!_hChatHistory)
        return;

    std::wstringstream ss;
    for (const auto& msg : _chatHistory)
    {
        if (msg.isUser)
        {
            ss << L"You: ";
        }
        else if (!msg.providerName.empty())
        {
            ss << msg.providerName << L": ";
        }
        else
        {
            ss << L"AI: ";
        }
        ss << msg.content;
        ss << L"\r\n\r\n";
    }

    ::SetWindowText(_hChatHistory, ss.str().c_str());

    // Scroll to bottom
    int lineCount = (int)::SendMessage(_hChatHistory, EM_GETLINECOUNT, 0, 0);
    ::SendMessage(_hChatHistory, EM_LINESCROLL, 0, lineCount);
}

void AIPanelDlg::addMessage(bool isUser, const std::wstring& content, const std::wstring& providerName)
{
    ChatMessage msg;
    msg.isUser = isUser;
    msg.content = content;
    msg.providerName = providerName;

    // Get timestamp
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    wchar_t timebuf[32];
    wcsftime(timebuf, 32, L"%H:%M:%S", timeinfo);
    msg.timestamp = timebuf;

    // Remove "Thinking..." message if present
    if (!_chatHistory.empty() && !_chatHistory.back().isUser && _chatHistory.back().content == L"Thinking...")
    {
        _chatHistory.pop_back();
    }

    _chatHistory.push_back(msg);
    updateChatDisplay();
}

void AIPanelDlg::applyLastResponse()
{
    if (_lastResponse.empty())
        return;

    // Check if there's selected text
    std::string selectedText = getSelectedText();

    // Apply to document
    applyToDocument(_lastResponse, !selectedText.empty());
}

void AIPanelDlg::resizeControls()
{
    if (!_hSelf)
        return;

    RECT rc;
    ::GetClientRect(_hSelf, &rc);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    int margin = 5;
    int buttonHeight = 25;
    int comboHeight = 22;
    int promptHeight = 60;
    int checkHeight = 20;

    // Provider combo at top
    if (_hProviderCombo)
    {
        ::MoveWindow(_hProviderCombo, margin, margin, width - 2 * margin, comboHeight, TRUE);
    }

    // Include context checkbox
    int currentY = margin + comboHeight + margin;
    if (_hIncludeContextChk)
    {
        ::MoveWindow(_hIncludeContextChk, margin, currentY, width - 2 * margin, checkHeight, TRUE);
    }

    // Chat history (main area)
    currentY += checkHeight + margin;
    int chatHeight = height - currentY - promptHeight - buttonHeight - 4 * margin;
    if (_hChatHistory)
    {
        ::MoveWindow(_hChatHistory, margin, currentY, width - 2 * margin, chatHeight, TRUE);
    }

    // Prompt edit
    currentY += chatHeight + margin;
    if (_hPromptEdit)
    {
        ::MoveWindow(_hPromptEdit, margin, currentY, width - 2 * margin, promptHeight, TRUE);
    }

    // Buttons at bottom
    currentY += promptHeight + margin;
    int btnWidth = (width - 4 * margin) / 3;
    if (_hSendBtn)
    {
        ::MoveWindow(_hSendBtn, margin, currentY, btnWidth, buttonHeight, TRUE);
    }
    if (_hClearBtn)
    {
        ::MoveWindow(_hClearBtn, margin + btnWidth + margin, currentY, btnWidth, buttonHeight, TRUE);
    }
    if (_hApplyBtn)
    {
        ::MoveWindow(_hApplyBtn, margin + 2 * (btnWidth + margin), currentY, btnWidth, buttonHeight, TRUE);
    }
}

int AIPanelDlg::getCurrentProviderIndex()
{
    if (_hProviderCombo)
    {
        return (int)::SendMessage(_hProviderCombo, CB_GETCURSEL, 0, 0);
    }
    return 0;
}

std::string AIPanelDlg::getCurrentDocumentText()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return "";

    HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    int len = (int)::SendMessage(curScintilla, SCI_GETLENGTH, 0, 0);
    if (len == 0)
        return "";

    std::vector<char> buffer(len + 1);
    ::SendMessage(curScintilla, SCI_GETTEXT, len + 1, (LPARAM)buffer.data());

    return std::string(buffer.data());
}

std::string AIPanelDlg::getSelectedText()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return "";

    HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    int selStart = (int)::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
    int selEnd = (int)::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);

    if (selStart == selEnd)
        return "";

    int len = selEnd - selStart;
    std::vector<char> buffer(len + 1);
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)buffer.data());

    return std::string(buffer.data());
}

std::wstring AIPanelDlg::getCurrentFilePath()
{
    TCHAR path[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)path);
    return std::wstring(path);
}

std::wstring AIPanelDlg::getCurrentLanguage()
{
    int langType = 0;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&langType);

    TCHAR langName[64] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETLANGUAGENAME, langType, (LPARAM)langName);

    return std::wstring(langName);
}

void AIPanelDlg::applyToDocument(const std::string& text, bool replaceSelection)
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    if (replaceSelection)
    {
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)text.c_str());
    }
    else
    {
        ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)text.c_str());
    }
}
