//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef AIPANELDLG_H
#define AIPANELDLG_H

#include "DockingDlgInterface.h"
#include "resource.h"
#include <string>
#include <vector>

// Forward declarations
class AIProvider;

// Chat message structure
struct ChatMessage
{
    bool isUser;
    std::wstring content;
    std::wstring timestamp;
    std::wstring providerName;  // Provider name for AI responses
};

class AIPanelDlg : public DockingDlgInterface
{
public:
    AIPanelDlg() : DockingDlgInterface(IDD_AI_PANEL) {}

    virtual void display(bool toShow = true) const override {
        DockingDlgInterface::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, IDC_PROMPT_EDIT));
    }

    void setParent(HWND parent2set) {
        _hParent = parent2set;
    }

    bool isVisible() const {
        return ::IsWindowVisible(_hSelf) ? true : false;
    }

    // Get current document text
    std::string getCurrentDocumentText();

    // Get selected text
    std::string getSelectedText();

    // Get current file path
    std::wstring getCurrentFilePath();

    // Get current language name
    std::wstring getCurrentLanguage();

    // Apply text to document
    void applyToDocument(const std::string& text, bool replaceSelection);

    // Refresh provider list (called after settings change)
    void refreshProviderList();

protected:
    virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    // UI control handles
    HWND _hProviderCombo = nullptr;
    HWND _hChatHistory = nullptr;
    HWND _hPromptEdit = nullptr;
    HWND _hSendBtn = nullptr;
    HWND _hClearBtn = nullptr;
    HWND _hIncludeContextChk = nullptr;
    HWND _hApplyBtn = nullptr;

    // Chat history
    std::vector<ChatMessage> _chatHistory;

    // Last AI response (for apply function)
    std::string _lastResponse;

    // Initialize controls
    void initControls();

    // Send prompt to AI
    void sendPrompt();

    // Clear chat history
    void clearHistory();

    // Update chat display
    void updateChatDisplay();

    // Add message to history
    void addMessage(bool isUser, const std::wstring& content, const std::wstring& providerName = L"");

    // Apply last response to document
    void applyLastResponse();

    // Resize controls
    void resizeControls();

    // Get current provider index
    int getCurrentProviderIndex();
};

#endif // AIPANELDLG_H
