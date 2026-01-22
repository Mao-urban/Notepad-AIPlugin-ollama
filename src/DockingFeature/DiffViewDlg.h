//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef DIFFVIEWDLG_H
#define DIFFVIEWDLG_H

#include "StaticDialog.h"
#include "resource.h"
#include <string>

class DiffViewDlg : public StaticDialog
{
public:
    DiffViewDlg() : StaticDialog() {}

    void init(HINSTANCE hInst, HWND parent) {
        StaticDialog::init(hInst, parent);
    }

    // Show the diff dialog with original and modified text
    // Returns true if user clicked Apply, false if Cancel
    bool showDiff(const std::string& original, const std::string& modified);

    // Get the modified text (in case user wants to edit)
    std::string getModifiedText() const { return _modifiedText; }

protected:
    virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    std::string _originalText;
    std::string _modifiedText;
    bool _applied = false;

    void resizeControls();
};

#endif // DIFFVIEWDLG_H
