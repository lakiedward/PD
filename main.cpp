#include <Windows.h>
#include <iostream>

void ProcessSubKey(HKEY hKeyServices, LPCTSTR pszSubKeyName);

int wmain() {
    HKEY hKeyServices;
    LONG lResult;

    // Deschiderea cheii HKLM\SYSTEM\CurrentControlSet\Services
    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services"),
        0,
        KEY_READ,
        &hKeyServices
    );

    if (lResult != ERROR_SUCCESS) {
        std::wcerr << L"Erroare la deschiderea cheii Services: " << lResult << std::endl;
        return 1;
    }

    // Enumerarea subcheilor
    DWORD dwIndex = 0;
    TCHAR szSubKeyName[256];
    DWORD dwNameSize = sizeof(szSubKeyName) / sizeof(TCHAR);

    while (RegEnumKeyEx(
        hKeyServices,
        dwIndex,
        szSubKeyName,
        &dwNameSize,
        NULL,
        NULL,
        NULL,
        NULL
    ) == ERROR_SUCCESS) {
        // Procesăm fiecare subcheie
        ProcessSubKey(hKeyServices, szSubKeyName);

        // Resetăm dimensiunea bufferului și incrementăm indexul
        dwIndex++;
        dwNameSize = sizeof(szSubKeyName) / sizeof(TCHAR);
    }

    // Închiderea cheii Services
    RegCloseKey(hKeyServices);
    return 0;
}

void ProcessSubKey(HKEY hKeyServices, LPCTSTR pszSubKeyName) {
    HKEY hSubKey;
    LONG lResult;

    // Deschiderea subcheii
    lResult = RegOpenKeyEx(
        hKeyServices,
        pszSubKeyName,
        0,
        KEY_READ,
        &hSubKey
    );

    if (lResult != ERROR_SUCCESS) {
        std::wcerr << L"Erroare la deschiderea subcheii " << pszSubKeyName << L": " << lResult << std::endl;
        return;
    }

    // Citirea valorii ImagePath
    TCHAR szImagePath[1024];
    DWORD dwImagePathSize = sizeof(szImagePath);
    lResult = RegQueryValueEx(
        hSubKey,
        TEXT("ImagePath"),
        NULL,
        NULL,
        (LPBYTE)szImagePath,
        &dwImagePathSize
    );

    if (lResult == ERROR_SUCCESS) {
        std::wcout << pszSubKeyName << L": " << szImagePath << std::endl;
    }

    // Închiderea subcheii
    RegCloseKey(hSubKey);
}