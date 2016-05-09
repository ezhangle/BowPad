// This file is part of BowPad.
//
// Copyright (C) 2014-2016 - Stefan Kueng
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// See <http://www.gnu.org/licenses/> for a copy of the full license text
//

#include "stdafx.h"
#include "CmdMisc.h"
#include "BowPad.h"
#include "AppUtils.h"
#include "Theme.h"
#include "SysInfo.h"

CCmdToggleTheme::CCmdToggleTheme(void * obj)
    : ICommand(obj)
    , text(0)
    , back(0)
    , high(0)
{
    CTheme::Instance().GetRibbonColors(text, back, high);
    int dark = (int)CIniSettings::Instance().GetInt64(L"View", L"darktheme", 0);
    if (dark)
    {
        CTheme::Instance().SetDarkTheme(dark != 0);
        CTheme::Instance().SetRibbonColorsHSB(UI_HSB(0, 0, 255), UI_HSB(160, 0, 0), UI_HSB(160, 44, 0));
    }
    InvalidateUICommand(UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
}

CCmdToggleTheme::~CCmdToggleTheme(void)
{
}

bool CCmdToggleTheme::Execute()
{
    if (!HasActiveDocument())
        return false;
    CTheme::Instance().SetDarkTheme(!CTheme::Instance().IsDarkTheme());
    CDocument doc = GetActiveDocument();
    SetupLexerForLang(doc.m_language);
    if (CTheme::Instance().IsDarkTheme())
    {
        CTheme::Instance().SetRibbonColorsHSB(UI_HSB(0, 0, 255), UI_HSB(160, 0, 0), UI_HSB(160, 44, 0));
    }
    else
        CTheme::Instance().SetRibbonColorsHSB(text, back, high);

    CIniSettings::Instance().SetInt64(L"View", L"darktheme", CTheme::Instance().IsDarkTheme() ? 1 : 0);

    ScintillaCall(SCI_CLEARDOCUMENTSTYLE);
    ScintillaCall(SCI_COLOURISE, 0, -1);
    TabActivateAt(GetActiveTabIndex());

    InvalidateUICommand(UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
    return true;
}

HRESULT CCmdToggleTheme::IUICommandHandlerUpdateProperty(REFPROPERTYKEY key, const PROPVARIANT* /*ppropvarCurrentValue*/, PROPVARIANT* ppropvarNewValue)
{
    if (UI_PKEY_BooleanValue == key)
    {
        return UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, CTheme::Instance().IsDarkTheme(), ppropvarNewValue);
    }
    return E_NOTIMPL;
}

bool CCmdConfigShortcuts::Execute()
{
    std::wstring userFile = CAppUtils::GetDataPath() + L"\\shortcuts.ini";
    if (!PathFileExists(userFile.c_str()))
    {
        HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_SHORTCUTS), L"config");
        if (hResource)
        {
            HGLOBAL hResourceLoaded = LoadResource(NULL, hResource);
            if (hResourceLoaded)
            {
                const char* lpResLock = (const char *) LockResource(hResourceLoaded);
                if (lpResLock)
                {
                    const char* lpStart = strstr(lpResLock, "#--");
                    if (lpStart)
                    {
                        const char* lpEnd = strstr(lpStart + 3, "#--");
                        if (lpEnd)
                        {
                            HANDLE hFile = CreateFile(userFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hFile != INVALID_HANDLE_VALUE)
                            {
                                DWORD dwWritten = 0;
                                WriteFile(hFile, lpStart, (DWORD)(lpEnd-lpStart), &dwWritten, NULL);
                                CloseHandle(hFile);
                            }
                        }
                    }
                }
            }
        }
    }
    return OpenFile(userFile.c_str(), 0) >= 0;
}


CCmdAutoBraces::CCmdAutoBraces(void * obj) : ICommand(obj)
{
    InvalidateUICommand(UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
}

CCmdAutoBraces::~CCmdAutoBraces(void)
{}

bool CCmdAutoBraces::Execute()
{
    CIniSettings::Instance().SetInt64(L"View", L"autobrace", CIniSettings::Instance().GetInt64(L"View", L"autobrace", 1) ? 0 : 1);
    InvalidateUICommand(UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
    return true;
}

HRESULT CCmdAutoBraces::IUICommandHandlerUpdateProperty(REFPROPERTYKEY key, const PROPVARIANT* /*ppropvarCurrentValue*/, PROPVARIANT* ppropvarNewValue)
{
    if (UI_PKEY_BooleanValue == key)
    {
        return UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, (BOOL)CIniSettings::Instance().GetInt64(L"View", L"autobrace", 1), ppropvarNewValue);
    }
    return E_NOTIMPL;
}

CCmdViewFileTree::CCmdViewFileTree(void * obj) : ICommand(obj)
{
    InvalidateUICommand(UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
}

CCmdViewFileTree::~CCmdViewFileTree(void)
{}

bool CCmdViewFileTree::Execute()
{
    ShowFileTree(!IsFileTreeShown());
    return true;
}

HRESULT CCmdViewFileTree::IUICommandHandlerUpdateProperty(REFPROPERTYKEY key, const PROPVARIANT* /*ppropvarCurrentValue*/, PROPVARIANT* ppropvarNewValue)
{
    if (UI_PKEY_BooleanValue == key)
    {
        return UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, IsFileTreeShown(), ppropvarNewValue);
    }
    if (UI_PKEY_Enabled == key)
    {
        return UIInitPropertyFromBoolean(UI_PKEY_Enabled, !GetFileTreePath().empty(), ppropvarNewValue);
    }
    return E_NOTIMPL;
}
