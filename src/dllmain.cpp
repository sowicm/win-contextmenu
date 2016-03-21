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

#include <Windows.h>
#include <Guiddef.h>
#include <tchar.h>
#include "SReg.h"
#include "ClassFactory.h"
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")


// {5DAF5968-3D2A-43B0-B57B-96D685224361}
#define CLSID_STRING L"{5DAF5968-3D2A-43B0-B57B-96D685224361}"
const CLSID CLSID_MyContextMenuExt =
{0x5DAF5968, 0x3D2A, 0x43B0, {0xB5, 0x7B, 0x96, 0xD6, 0x85, 0x22, 0x43, 0x61}};

HINSTANCE g_hInst;
long      g_cDllRef = 0;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, void** /*lpReserved*/)
{
    g_hInst = hInstance;
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_MyContextMenuExt, rclsid))
    {
        hr = E_OUTOFMEMORY;
        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}

STDAPI DllRegisterServer(void)
{
    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    SReg reg;
    if (!reg.create(HKEY_CLASSES_ROOT, L"CLSID\\" CLSID_STRING, KEY_SET_VALUE) ||
        !reg.setStringValue(0, L"Sowicm Shell Extension") ||
        !reg.create(HKEY_CLASSES_ROOT, L"CLSID\\" CLSID_STRING L"\\InprocServer32", KEY_SET_VALUE) ||
        !reg.setStringValue(0, szModule) ||
        !reg.setStringValue(L"ThreadingModel", L"Apartment") ||
        !reg.create(HKEY_CLASSES_ROOT, L"*\\ShellEx\\ContextMenuHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.create(HKEY_CLASSES_ROOT, L"Directory\\ShellEx\\ContextMenuHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.create(HKEY_CLASSES_ROOT, L"Directory\\ShellEx\\DragDropHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.create(HKEY_CLASSES_ROOT, L"Directory\\Background\\ShellEx\\ContextMenuHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.create(HKEY_CLASSES_ROOT, L"Drive\\ShellEx\\ContextMenuHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.create(HKEY_CLASSES_ROOT, L"Drive\\ShellEx\\DragDropHandlers\\MyContextMenu", KEY_SET_VALUE) ||
        !reg.setStringValue(0, CLSID_STRING) ||
        !reg.open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", KEY_SET_VALUE) ||
        !reg.setStringValue(CLSID_STRING, L"Sowicm Shell Extension"))
        return E_ACCESSDENIED;
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    SReg reg;
    if (!reg.deleteTree(HKEY_CLASSES_ROOT, L"CLSID\\" CLSID_STRING) ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"*\\ShellEx\\ContextMenuHandlers\\MyContextMenu") ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"Directory\\ShellEx\\ContextMenuHandlers\\MyContextMenu") ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"Directory\\ShellEx\\DragDropHandlers\\MyContextMenu") ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"Directory\\Background\\ShellEx\\ContextMenuHandlers\\MyContextMenu") ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"Drive\\ShellEx\\ContextMenuHandlers\\MyContextMenu") ||
        !reg.deleteTree(HKEY_CLASSES_ROOT, L"Drive\\ShellEx\\DragDropHandlers\\MyContextMenu") ||
        !reg.open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", KEY_SET_VALUE) ||
        !reg.deleteValue(CLSID_STRING))
        return E_ACCESSDENIED;
    return S_OK;
}
