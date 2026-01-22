//this file is part of notepad_aiplug
//Copyright (C)2025

#include "DiffViewDlg.h"
#include <windows.h>

INT_PTR CALLBACK DiffViewDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Set dialog text
            HWND hOriginal = ::GetDlgItem(_hSelf, IDC_DIFF_ORIGINAL);
            HWND hModified = ::GetDlgItem(_hSelf, IDC_DIFF_MODIFIED);

            if (hOriginal)
            {
                ::SetWindowTextA(hOriginal, _originalText.c_str());
            }
            if (hModified)
            {
                ::SetWindowTextA(hModified, _modifiedText.c_str());
            }

            // Center dialog on screen
            RECT rc;
            ::GetWindowRect(_hSelf, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
            ::SetWindowPos(_hSelf, NULL,
                (screenWidth - width) / 2,
                (screenHeight - height) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_DIFF_APPLY_BTN:
                case IDOK:
                    _applied = true;
                    ::EndDialog(_hSelf, IDOK);
                    return TRUE;

                case IDC_DIFF_CANCEL_BTN:
                case IDCANCEL:
                    _applied = false;
                    ::EndDialog(_hSelf, IDCANCEL);
                    return TRUE;
            }
            break;
        }

        case WM_SIZE:
        {
            resizeControls();
            return TRUE;
        }

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 400;
            mmi->ptMinTrackSize.y = 300;
            return TRUE;
        }
    }

    return FALSE;
}

bool DiffViewDlg::showDiff(const std::string& original, const std::string& modified)
{
    _originalText = original;
    _modifiedText = modified;
    _applied = false;

    INT_PTR result = ::DialogBoxParam(
        _hInst,
        MAKEINTRESOURCE(IDD_DIFF_VIEW),
        _hParent,
        dlgProc,
        reinterpret_cast<LPARAM>(this));

    return (result == IDOK);
}

void DiffViewDlg::resizeControls()
{
    if (!_hSelf)
        return;

    RECT rc;
    ::GetClientRect(_hSelf, &rc);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    int margin = 7;
    int labelHeight = 15;
    int buttonHeight = 25;
    int buttonWidth = 85;

    // Calculate panel width (split in half minus margins)
    int panelWidth = (width - 3 * margin) / 2;
    int panelHeight = height - labelHeight - buttonHeight - 4 * margin;

    // Original panel
    HWND hOriginalLabel = ::GetDlgItem(_hSelf, IDC_STATIC);
    HWND hOriginal = ::GetDlgItem(_hSelf, IDC_DIFF_ORIGINAL);

    // Modified label (find by position or use a different approach)
    HWND hModified = ::GetDlgItem(_hSelf, IDC_DIFF_MODIFIED);

    if (hOriginal)
    {
        ::MoveWindow(hOriginal, margin, margin + labelHeight, panelWidth, panelHeight, TRUE);
    }

    if (hModified)
    {
        ::MoveWindow(hModified, margin + panelWidth + margin, margin + labelHeight, panelWidth, panelHeight, TRUE);
    }

    // Buttons at bottom
    int buttonY = height - buttonHeight - margin;
    HWND hApplyBtn = ::GetDlgItem(_hSelf, IDC_DIFF_APPLY_BTN);
    HWND hCancelBtn = ::GetDlgItem(_hSelf, IDC_DIFF_CANCEL_BTN);

    if (hApplyBtn)
    {
        ::MoveWindow(hApplyBtn, width - 2 * buttonWidth - 2 * margin, buttonY, buttonWidth, buttonHeight, TRUE);
    }

    if (hCancelBtn)
    {
        ::MoveWindow(hCancelBtn, width - buttonWidth - margin, buttonY, buttonWidth, buttonHeight, TRUE);
    }
}
