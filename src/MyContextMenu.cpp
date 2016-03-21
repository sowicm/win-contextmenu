// Copyright (c) 2012, Sowicm
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither the name of the Sowicm nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "MyContextMenu.h"
#include <Shlwapi.h>
#include <tchar.h>

#define USE_PIC 0

extern long g_cDllRef;

MyContextMenu::MyContextMenu()
    : m_cRef(1)
    , m_pszMenuText(_T("&Display File Name"))
    , m_pszVerb("cppdisplay")
    , m_pwszVerb(L"cppdisplay")
    , m_pszVerbCanonicalName("CppDisplayFileName")
    , m_pwszVerbCanonicalName(L"CppDisplayFileName")
    , m_pszVerbHelpText("Display File Name")
    , m_pwszVerbHelpText(L"Display File Name")
{
    InterlockedIncrement(&g_cDllRef);

#if USE_PIC
    // Load the bitmap for the menu item. 
    // If you want the menu item bitmap to be transparent, the color depth of 
    // the bitmap must not be greater than 8bpp.
    m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
        IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
#endif
}

MyContextMenu::~MyContextMenu()
{
#if USE_PIC
    if (m_hMenuBmp)
    {
        DeleteObject(m_hMenuBmp);
        m_hMenuBmp = NULL;
    }
#endif
    InterlockedDecrement(&g_cDllRef);
}

IFACEMETHODIMP MyContextMenu::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(MyContextMenu, IContextMenu),
        QITABENT(MyContextMenu, IShellExtInit),
        {0}
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) MyContextMenu::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) MyContextMenu::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP MyContextMenu::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    m_folderDroppedIn[0] = '\0';
    m_files[0]           = '\0';
    if (NULL != pDataObj)
    {
    	FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    	STGMEDIUM stg;// = {TYMED_HGLOBAL};

    	if (SUCCEEDED(pDataObj->GetData(&fmt, &stg)))
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
            if (NULL != hDrop)
            {
                UINT numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
                //m_files.resize(uNumFiles);
                for (UINT i = 0; i < numFiles; ++i)
                    if (DragQueryFile(hDrop, i, m_files, MAX_PATH))
                    {
                        //if (PathIsDirectory(m_files[i]))
                        //{
                        //  m_files.clear();
                        //  break;
                        //}
                    }
                    else
                    {
                        m_files[0] = '\0';
                    }
                GlobalUnlock(stg.hGlobal);
            }
            ReleaseStgMedium(&stg);
        }
    }
	SHGetPathFromIDList(pidlFolder, m_folderDroppedIn);
	return S_OK;
}

STDMETHODIMP MyContextMenu::QueryContextMenu(HMENU hMenu, UINT index, UINT idFirstCmd, UINT idLastCmd, UINT uFlags)
{
    if (uFlags & CMF_DEFAULTONLY)
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
    UINT id = idFirstCmd;
/*
    MENUITEMINFO mii = {sizeof(mii)};
    mii.fMask =
#if USE_PIC
        MIIM_BITMAP |
#endif
        MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mii.wID = id++;
    mii.fType = MFT_STRING;
    mii.dwTypeData = m_pszMenuText;
    mii.fState = MFS_ENABLED;
    mii.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
    if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
        return HRESULT_FROM_WIN32(GetLastError());

    // Add a separator.
    MENUITEMINFO sep = {sizeof(sep)};
    sep.fMask = MIIM_TYPE;
    sep.fType = MFT_SEPARATOR;
    if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
        return HRESULT_FROM_WIN32(GetLastError());
*/
    if (m_folderDroppedIn[0] && m_files[0])
    {
        InsertMenu(hMenu, index, MF_BYPOSITION, id++, _T("Make Junction Here"));
    }
    else
    {
        HMENU hSubmenu = CreatePopupMenu();
        int i = 0;
        if (m_folderDroppedIn[0])
        {
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("命令提示符"));
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("命令提示符(管理员)"));
            if (wcscmp(m_folderDroppedIn, L"C:\\Users\\Sowicm\\Desktop") == 0)
                InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("刷新环境变量"));
        }
        else
        {
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Open with notepad"));
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Open with notepad2"));
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Open with EMACS(cygwin)"));
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Open with EMACS(win)"));
            InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Open with Sublime Text 2"));
            if (GetFileAttributes(m_files) & FILE_ATTRIBUTE_DIRECTORY)
            {
                InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Set to webroot"));
            }
            else
            {
                InsertMenu(hSubmenu, i++, MF_BYPOSITION, id++, _T("Run in terminal"));
            }
        }

        InsertMenu(hMenu, index, MF_BYPOSITION | MF_POPUP, (UINT_PTR) hSubmenu, _T("颤抖吧！凡人！"));
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, id - idFirstCmd);
}

STDMETHODIMP MyContextMenu::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    return E_INVALIDARG;
#if 0
    switch (idCmd)
    {
    case 0:
        if (uFlags & GCS_HELPTEXT)
        {
            if (uFlags & GCS_UNICODE)
            {
                StrCpyNW((LPWSTR)pszName, L"Make a junction here", cchMax);
            }
            else
            {
                StrCpyNA(pszName, "Make a junction here", cchMax);
            }
        }
        return S_OK;
    }
    return E_INVALIDARG;
#endif
}

STDMETHODIMP MyContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    BOOL Unicode = FALSE;
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if (pici->fMask & CMIC_MASK_UNICODE)
            Unicode = TRUE;
    }

    if (!Unicode && HIWORD(pici->lpVerb))
    {
        if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
            OnCommand(pici->hwnd, 0);
        else
            return E_FAIL;
    }
    else if (Unicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
    {
        if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
            OnCommand(pici->hwnd, 0);
        else
            return E_FAIL;
    }
    else
    {
#if 1
        OnCommand(pici->hwnd, LOWORD(pici->lpVerb));
#else
        switch (LOWORD(pici->lpVerb))
        {
        case 0:
            OnCommand(pici->hwnd, 0);
        case 1:
            OnCommand(pici->hwnd, 1);
        default:
            return E_FAIL;
        }
#endif
    }

    return S_OK;
}
