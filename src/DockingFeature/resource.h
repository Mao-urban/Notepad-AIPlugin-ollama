//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef RESOURCE_H
#define RESOURCE_H

#ifndef IDC_STATIC
#define IDC_STATIC  -1
#endif

// AI Panel Dialog
#define IDD_AI_PANEL                3000
#define IDC_PROVIDER_COMBO          3001
#define IDC_CHAT_HISTORY            3002
#define IDC_PROMPT_EDIT             3003
#define IDC_SEND_BTN                3004
#define IDC_CLEAR_BTN               3005
#define IDC_INCLUDE_CONTEXT_CHK     3006
#define IDC_APPLY_BTN               3007

// Settings Dialog
#define IDD_SETTINGS                3100
#define IDC_OPENAI_KEY_EDIT         3101
#define IDC_CLAUDE_KEY_EDIT         3102
#define IDC_GEMINI_KEY_EDIT         3103
#define IDC_DEFAULT_PROVIDER_COMBO  3104
#define IDC_EDIT_MODE_COMBO         3105
#define IDC_OPENAI_MODEL_COMBO      3106
#define IDC_CLAUDE_MODEL_COMBO      3107
#define IDC_GEMINI_MODEL_COMBO      3108

// Settings Dialog - New Provider List Controls
#define IDC_PROVIDER_LIST           3301
#define IDC_PROVIDER_NAME_EDIT      3302
#define IDC_PROVIDER_TYPE_COMBO     3303
#define IDC_PROVIDER_KEY_EDIT       3304
#define IDC_PROVIDER_URL_EDIT       3305
#define IDC_ADD_PROVIDER_BTN        3306
#define IDC_REMOVE_PROVIDER_BTN     3307
#define IDC_UPDATE_PROVIDER_BTN     3308

// Diff View Dialog
#define IDD_DIFF_VIEW               3200
#define IDC_DIFF_ORIGINAL           3201
#define IDC_DIFF_MODIFIED           3202
#define IDC_DIFF_APPLY_BTN          3203
#define IDC_DIFF_CANCEL_BTN         3204

#endif // RESOURCE_H
