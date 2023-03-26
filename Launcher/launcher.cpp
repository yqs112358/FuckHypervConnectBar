#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>
#include <filesystem>
#pragma comment (lib, "Psapi.lib")
using namespace std;


#define INJECTED_DLL_PATH "ApiHooker.dll"


BOOL InjectDllToProcess(DWORD processId, PCSTR dllName)
{
    BOOL bOk = FALSE;
    HANDLE hProcess = NULL, hThread = NULL;
    PCSTR pszLibFileRemote = NULL;

    do
    {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
            PROCESS_VM_WRITE, FALSE, processId);
        if (hProcess == NULL)
            break;

        pszLibFileRemote = 
            (PCSTR) VirtualAllocEx(hProcess, NULL, strlen(dllName) * sizeof(CHAR) + 1, MEM_COMMIT, PAGE_READWRITE);
        if (pszLibFileRemote == NULL) 
            break;
        if (!WriteProcessMemory(hProcess, (LPVOID)pszLibFileRemote, (PVOID)dllName, strlen(dllName) * sizeof(CHAR) + 1, NULL))
        {
            break;
        }

        PTHREAD_START_ROUTINE pfnThreadRtn = 
            (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");
        if (pfnThreadRtn == NULL) 
            break;
        hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (LPVOID)pszLibFileRemote, 0, NULL);
        if (hThread == NULL)
            break;

        if (hThread != NULL)
            CloseHandle(hThread);
        if (hProcess != NULL)
            CloseHandle(hProcess);
        bOk = TRUE;
    } 
    while (FALSE);
    return bOk;
}

bool EndsWith(const char* str, const char* suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

DWORD GetProcessId(LPCSTR lpProcessName)
{
    DWORD dwProcessID[0x500] = { 0 };
    DWORD dwNeeded = 0;
    BOOL bEnumRes = EnumProcesses(dwProcessID, sizeof(dwProcessID), &dwNeeded);
    UINT uCount = dwNeeded / sizeof(DWORD);
    for (UINT i = 0; i < uCount; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID[i]);
        if (NULL != hProcess)
        {
            CHAR szProcessName[MAX_PATH] = { 0 };
            DWORD dwNameLen = MAX_PATH;
            BOOL bRet = QueryFullProcessImageNameA(hProcess, 0, szProcessName, &dwNameLen);
            if (EndsWith(szProcessName, lpProcessName))
            {
                return dwProcessID[i];
            }
        }
    }
    return 0;
}

int main()
{
    DWORD vmconnectPid = GetProcessId("vmconnect.exe");
    if (vmconnectPid <= 0)
    {
        cout << "[ERROR] vmconnect.exe no found." << endl;
        system("pause");
        exit(1);
    }
    cout << "[INFO] Found vmconnect.exe, pid: " << vmconnectPid << endl;

    std::error_code ec;
    auto dllPath = std::filesystem::canonical(std::filesystem::path(INJECTED_DLL_PATH), ec);
    if(ec)
    {
        cout << "[ERROR] " << INJECTED_DLL_PATH << " no found." << endl;
        system("pause");
        exit(1);
    }
    dllPath.make_preferred();
    string dllPathStr = dllPath.generic_u8string();
    cout << "[INFO] Inject DLL \"" << dllPathStr << "\" into vmconnect.exe..." << endl;

    if (InjectDllToProcess(vmconnectPid, dllPathStr.c_str()))
        cout << endl << "[INFO] Success." << endl << endl;
    else
        cout << endl << "[ERROR] Failed. Error code: " << GetLastError() << endl << endl;
	system("pause");
	return 0;
}