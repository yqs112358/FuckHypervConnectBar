// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include <detours.h>

static BOOL (WINAPI* Real_ShowWindow)(HWND hWnd, int nCmdShow) = ShowWindow;

wchar_t wndTitleBuf[MAX_PATH];
wchar_t wndClassNameBuf[MAX_PATH];

BOOL WINAPI hookedShowWindow(HWND hWnd, int nCmdShow)
{
    if (GetClassNameW(hWnd, wndClassNameBuf, sizeof(wndClassNameBuf) / sizeof(wchar_t)) == 0
        || GetWindowTextW(hWnd, wndTitleBuf, sizeof(wndTitleBuf) / sizeof(wchar_t)) == 0)
    {
        return Real_ShowWindow(hWnd, nCmdShow);
    }

    if (wcscmp(wndClassNameBuf, L"BBarWindowClass") == 0 && wcscmp(wndTitleBuf, L"BBar") == 0)
    {
        // Hooked
        // Hide it forever!
        if (nCmdShow != SW_HIDE)
            return TRUE;
    }
    return Real_ShowWindow(hWnd, nCmdShow);
}

void Hook()
{
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)Real_ShowWindow, hookedShowWindow);
    DetourTransactionCommit();
}

void UnHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)Real_ShowWindow, hookedShowWindow);
    DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL, L"The connection bar of vmconnect.exe will be hidden forever.\n"
            "If you need to go out of virtual machine, press \"Ctrl+Alt+←\".", L"vmconnect.exe", MB_OK);
        Hook();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        UnHook();
        break;
    }
    return TRUE;
}

