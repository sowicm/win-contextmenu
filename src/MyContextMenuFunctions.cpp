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
#include <tchar.h>

#if 0
namespace{
#include <ntdef.h>
/*
 *  MKLINK.C - mklink internal command.
 */

/* There is no API for creating junctions, so we must do it the hard way */
static BOOL CreateJunction(LPCTSTR LinkName, LPCTSTR TargetName)
{
    /* Structure for reparse point daya when ReparseTag is one of
     * the tags defined by Microsoft. Copied from MinGW winnt.h */
    typedef struct _REPARSE_DATA_BUFFER {
        DWORD  ReparseTag;
        WORD   ReparseDataLength;
        WORD   Reserved;
        union {
            struct {
                WORD   SubstituteNameOffset;
                WORD   SubstituteNameLength;
                WORD   PrintNameOffset;
                WORD   PrintNameLength;
                ULONG  Flags;
                WCHAR PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct {
                WORD   SubstituteNameOffset;
                WORD   SubstituteNameLength;
                WORD   PrintNameOffset;
                WORD   PrintNameLength;
                WCHAR PathBuffer[1];
            } MountPointReparseBuffer;
            struct {
                BYTE   DataBuffer[1];
            } GenericReparseBuffer;
        } DUMMYUNIONNAME;
    } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

    HMODULE hNTDLL = GetModuleHandle(_T("NTDLL"));
    BOOLEAN (WINAPI *RtlDosPathNameToNtPathName_U)(PCWSTR, PUNICODE_STRING, PCWSTR *, CURDIR *)
        = (BOOLEAN (WINAPI *)(PCWSTR, PUNICODE_STRING, PCWSTR *, CURDIR *))GetProcAddress(hNTDLL, "RtlDosPathNameToNtPathName_U");
    VOID (WINAPI *RtlFreeUnicodeString)(PUNICODE_STRING)
        = (VOID (WINAPI *)(PUNICODE_STRING))GetProcAddress(hNTDLL, "RtlFreeUnicodeString");

    TCHAR TargetFullPath[MAX_PATH];
#ifdef UNICODE
#define TargetFullPathW TargetFullPath
#else
    WCHAR TargetFullPathW[MAX_PATH];
#endif
    UNICODE_STRING TargetNTPath;
    HANDLE hJunction;

    /* The data for this kind of reparse point has two strings:
     * The first ("SubstituteName") is the full target path in NT format,
     * the second ("PrintName") is the full target path in Win32 format.
     * Both of these must be wide-character strings. */
    if (!RtlDosPathNameToNtPathName_U ||
        !RtlFreeUnicodeString ||
        !GetFullPathName(TargetName, MAX_PATH, TargetFullPath, NULL) ||
#ifndef UNICODE
        !MultiByteToWideChar(CP_ACP, 0, TargetFullPath, -1, TargetFullPathW, -1) ||
#endif
        !RtlDosPathNameToNtPathName_U(TargetFullPathW, &TargetNTPath, NULL, NULL))
    {
        return FALSE;
    }

    /* We have both the names we need, so time to create the junction.
     * Start with an empty directory */
    if (!CreateDirectory(LinkName, NULL))
    {
        RtlFreeUnicodeString(&TargetNTPath);
        return FALSE;
    }

    /* Open the directory we just created */
    hJunction = CreateFile(LinkName, GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hJunction != INVALID_HANDLE_VALUE)
    {
        /* Allocate a buffer large enough to hold both strings, including trailing NULs */
        SIZE_T TargetLen = wcslen(TargetFullPathW) * sizeof(WCHAR);
        DWORD DataSize = (DWORD)(FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer)
                          + TargetNTPath.Length + sizeof(WCHAR)
                          + TargetLen           + sizeof(WCHAR));
        PREPARSE_DATA_BUFFER Data = _alloca(DataSize);

        /* Fill it out and use it to turn the directory into a reparse point */
        Data->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
        Data->ReparseDataLength = (WORD)(DataSize - FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer));
        Data->Reserved = 0;
        Data->MountPointReparseBuffer.SubstituteNameOffset = 0;
        Data->MountPointReparseBuffer.SubstituteNameLength = TargetNTPath.Length;
        wcscpy(Data->MountPointReparseBuffer.PathBuffer,
               TargetNTPath.Buffer);
        Data->MountPointReparseBuffer.PrintNameOffset = TargetNTPath.Length + sizeof(WCHAR);
        Data->MountPointReparseBuffer.PrintNameLength = (USHORT)TargetLen;
        wcscpy((WCHAR *)((BYTE *)Data->MountPointReparseBuffer.PathBuffer
                         + Data->MountPointReparseBuffer.PrintNameOffset),
               TargetFullPathW);
        if (DeviceIoControl(hJunction, FSCTL_SET_REPARSE_POINT,
                            Data, DataSize, NULL, 0, &DataSize, NULL))
        {
            /* Success */
            CloseHandle(hJunction);
            RtlFreeUnicodeString(&TargetNTPath);
            return TRUE;
        }
        CloseHandle(hJunction);
    }
    RemoveDirectory(LinkName);
    RtlFreeUnicodeString(&TargetNTPath);
    return FALSE;
}

INT
cmd_mklink(LPTSTR param)
{
    HMODULE hKernel32 = GetModuleHandle(_T("KERNEL32"));
    DWORD Flags = 0;
    enum { SYMBOLIC, HARD, JUNCTION } LinkType = SYMBOLIC;
    INT NumFiles = 0;
    LPTSTR Name[2];
    INT argc, i;
    LPTSTR *arg;

    if (!_tcsncmp(param, _T("/?"), 2))
    {
        ConOutResPuts(STRING_MKLINK_HELP);
        return 0;
    }

    arg = split(param, &argc, FALSE, FALSE);
    for (i = 0; i < argc; i++)
    {
        if (arg[i][0] == _T('/'))
        {
            if (!_tcsicmp(arg[i], _T("/D")))
                Flags |= 1; /* SYMBOLIC_LINK_FLAG_DIRECTORY */
            else if (!_tcsicmp(arg[i], _T("/H")))
                LinkType = HARD;
            else if (!_tcsicmp(arg[i], _T("/J")))
                LinkType = JUNCTION;
            else
            {
                error_invalid_switch(arg[i][1]);
                freep(arg);
                return 1;
            }
        }
        else
        {
            if (NumFiles == 2)
            {
                error_too_many_parameters(arg[i]);
                freep(arg);
                return 1;
            }
            Name[NumFiles++] = arg[i];
        }
    }
    freep(arg);

    if (NumFiles != 2)
    {
        error_req_param_missing();
        return 1;
    }

    nErrorLevel = 0;

    if (LinkType == SYMBOLIC)
    {
        /* CreateSymbolicLink doesn't exist in old versions of Windows,
         * so load dynamically */
        BOOL (WINAPI *CreateSymbolicLink)(LPCTSTR, LPCTSTR, DWORD)
#ifdef UNICODE
            = (BOOL (WINAPI *)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hKernel32, "CreateSymbolicLinkW");
#else
            = (BOOL (WINAPI *)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hKernel32, "CreateSymbolicLinkA");
#endif
        if (CreateSymbolicLink && CreateSymbolicLink(Name[0], Name[1], Flags))
        {
            ConOutResPrintf(STRING_MKLINK_CREATED_SYMBOLIC, Name[0], Name[1]);
            return 0;
        }
    }
    else if (LinkType == HARD)
    {
        /* CreateHardLink doesn't exist in old versions of Windows,
         * so load dynamically */
        BOOL (WINAPI *CreateHardLink)(LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES)
#ifdef UNICODE
            = (BOOL (WINAPI *)(LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES))GetProcAddress(hKernel32, "CreateHardLinkW");
#else
            = (BOOL (WINAPI *)(LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES))GetProcAddress(hKernel32, "CreateHardLinkA");
#endif
        if (CreateHardLink && CreateHardLink(Name[0], Name[1], NULL))
        {
            ConOutResPrintf(STRING_MKLINK_CREATED_HARD, Name[0], Name[1]);
            return 0;
        }
    }
    else
    {
        if (CreateJunction(Name[0], Name[1]))
        {
            ConOutResPrintf(STRING_MKLINK_CREATED_JUNCTION, Name[0], Name[1]);
            return 0;
        }
    }

    ErrorMessage(GetLastError(), _T("MKLINK"));
    return 1;
}
} // namespace
#endif

void MyContextMenu::OnCommand(HWND hWnd, int id)
{
    if (m_folderDroppedIn[0] && m_files[0])
    {
        switch (id)
        {
        case 0:
        {
            // Make Junction Here
            wchar_t cmd[MAX_PATH * 3];
            wsprintf(cmd, L"mklink /j \"%s\\%s\" \"%s\"", m_folderDroppedIn, wcsrchr(m_files, '\\') + 1, m_files);
            _wsystem(cmd);
            //MessageBox(hWnd, link, L"MyContextMenu", MB_ICONINFORMATION);
            // SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
            // sei.lpFile       = L"mklink.exe";
            // sei.lpVerb       = L"open";
            // sei.lpParameters = parameters;
            // ShellExecuteEx(&sei);
            break;
        }
        }
    }
    else
    {
        if (m_folderDroppedIn[0])
        {
            switch (id)
            {
            case 0:
            {
                // 命令提示符
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"C:\\Windows\\System32\\cmd.exe";
                sei.lpVerb       = L"open";
                sei.lpDirectory  = m_folderDroppedIn;
                ShellExecuteEx(&sei);
                break;
            }
            case 1:
            {
                // 命令提示符(管理员)
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                /*SECURITY_ATTRIBUTES sa;
                sa.nLength = sizeof(sa);
                sa.bInheritHandle = TRUE;
                sa.lpSecurityDescriptor = NULL;*/
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"C:\\Windows\\System32\\cmd.exe";
                sei.lpVerb       = L"runas";
                wchar_t param[MAX_PATH * 2 + 32] = L"/k cd \"";
                wcscat( wcscat(param, m_folderDroppedIn), L"\"" );
                sei.lpParameters = param;
                sei.lpDirectory  = m_folderDroppedIn;
                ShellExecuteEx(&sei);
                break;
            }
            case 2:
            {
                // 刷新环境变量
                /*
                  您可以通过编辑以下注册表项修改用户环境变量：
                  HKEY_CURRENT_USER /
                  Environment
                  可以通过编辑以下注册表项来修改系统环境变量：
                  HKEY_LOCAL_MACHINE /
                  SYSTEM /
                  CurrentControlSet /
                  Control /
                  Session Manager /
                  Environment
                  注意必须作为 REG_EXPAND_SZ 注册表值在注册表中存储要扩展
                  （例如对于使用 %system%）
                  时需要的所有环境变量。不将从注册表读取时展开类型 REG_SZ 的任
                  何值。
                  请注意 RegEdit.exe 没有一种添加 REG_EXPAND_SZ 的方法。 使
                  用 RegEdt32.exe 手动编辑这些值时。
                */
                DWORD_PTR msgResult;
                if (0 == SendMessageTimeout(
                        HWND_BROADCAST,
                        WM_SETTINGCHANGE,
                        0,
                        LPARAM(L"Environment"),
                        SMTO_ABORTIFHUNG,
                        5000,
                        &msgResult))
                    MessageBox(NULL,
                               _T("刷新失败"),
                               _T("MyContextMenu"),
                               MB_OK);
                break;
            }
            }
        }
        else
        {
            switch (id)
            {
            case 0:
            {
                // Open with notepad
                wchar_t param[MAX_PATH * 2 + 32] = L"\"";
                wcscat( wcscat(param, m_files), L"\"" );
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"notepad";
                sei.lpVerb       = L"open";
                sei.lpParameters = param;
                ShellExecuteEx(&sei);
                break;
            }
            case 1:
            {
                // Open with notepad2
                wchar_t param[MAX_PATH * 2 + 32] = L"\"";
                wcscat( wcscat(param, m_files), L"\"" );
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"C:\\Users\\Sowicm\\_MyApps\\notepad2_x64\\Notepad2.exe";
                sei.lpVerb       = L"open";
                sei.lpParameters = param;
                ShellExecuteEx(&sei);
                break;
            }
            case 2:
            {
                if (GetFileAttributes(m_files) & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Set to webroot
                    TCHAR cmd[MAX_PATH * 2 + 32];
                    wsprintf(cmd, L"mklink /j C:\\Users\\Sowicm\\localhost \"%s\"", m_files);
                    _wsystem(L"rmdir C:\\Users\\Sowicm\\localhost");
                    _wsystem(cmd);
                }
                else
                {
                    // Open with EMACS(cygwin)
                    // emacsclient -a '' -c
                    // -c (new frame)
                    wchar_t param[MAX_PATH * 2 + 32] = L"-a '' -c \"/cygdrive/";
                    int l = wcslen(param);
                    param[l++] = m_files[0];
                    auto p = m_files + 2;
                    while (p && *p)
                    {
                        param[l++] = (*p == '\\' ? '/' : *p);
                        ++p;
                    }
                    param[l++] = '"';
                    param[l] = '\0';
                    SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                    sei.nShow        = SW_HIDE; // do not show command window
                    sei.lpFile       = L"C:\\Users\\Sowicm\\_MyApps\\cygwin\\bin\\emacsclient-w32.exe";
                    sei.lpVerb       = L"open";
                    sei.lpParameters = param;
                    ShellExecuteEx(&sei);
                }
                break;
            }
            case 3:
            {
                // Open with EMACS(win)
                wchar_t param[MAX_PATH * 2 + 32] = L"\"";
                wcscat( wcscat(param, m_files), L"\"" );
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"C:\\Users\\Sowicm\\_MyApps\\emacs\\bin\\runemacs.exe";
                sei.lpVerb       = L"open";
                sei.lpParameters = param;
                ShellExecuteEx(&sei);
                break;
            }
            case 4:
            {
                // Open with Sublime Text 2
                wchar_t param[MAX_PATH * 2 + 32] = L"\"";
                wcscat( wcscat(param, m_files), L"\"" );
                SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
                sei.nShow        = SW_NORMAL;
                sei.lpFile       = L"C:\\Users\\Sowicm\\_MyApps\\Sublime Text 2\\sublime_text.exe";
                sei.lpVerb       = L"open";
                sei.lpParameters = param;
                ShellExecuteEx(&sei);
                break;
            }
            case 5:
            {
                // Run in terminal
                TCHAR cmd[MAX_PATH + 32];
                wsprintf(cmd, L"call \"%s\"\r\npause", m_files);
                _wsystem(cmd);
                break;
            }
            }
        }
    }
}
