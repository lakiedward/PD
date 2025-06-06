#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <usbiodef.h>   // GUID_DEVINTERFACE_USB_DEVICE
#include <iostream>
#include <regex>        // pentru extragerea VID & PID

#pragma comment(lib, "setupapi.lib")

int main() {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        &GUID_DEVINTERFACE_USB_DEVICE,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        std::cerr << "Eroare: nu s-a putut obține lista de dispozitive USB." << std::endl;
        return 1;
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DWORD index = 0;

    std::wcout << L"Dispozitive USB conectate:\n\n";

    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &GUID_DEVINTERFACE_USB_DEVICE, index, &deviceInterfaceData)) {
        DWORD requiredSize = 0;

        // Obține dimensiunea necesară
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            std::cerr << "Eroare la obținerea dimensiunii detaliilor dispozitivului." << std::endl;
            break;
        }

        // Alocă buffer
        PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
        if (!deviceDetailData) {
            std::cerr << "Eroare la alocarea memoriei." << std::endl;
            break;
        }
        deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceDetailData, requiredSize, NULL, NULL)) {
            SP_DEVINFO_DATA devInfoData;
            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            std::wcout << L"[Dispozitiv " << (index + 1) << L"]\n";

            if (SetupDiEnumDeviceInfo(deviceInfoSet, index, &devInfoData)) {
                TCHAR deviceName[1024];
                if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &devInfoData, SPDRP_DEVICEDESC, NULL,
                    (PBYTE)deviceName, sizeof(deviceName), NULL)) {
                    std::wcout << L"    Nume: " << deviceName << std::endl;
                }
            }

            std::wcout << L"    Path: " << deviceDetailData->DevicePath << std::endl;

            // Extrage Vendor ID și Product ID din DevicePath
            std::wregex vidPidRegex(L"vid_([0-9a-fA-F]{4})&pid_([0-9a-fA-F]{4})");
            std::wsmatch matches;

            std::wstring devicePathStr = deviceDetailData->DevicePath;
            if (std::regex_search(devicePathStr, matches, vidPidRegex))
            {
                std::wcout << L"    Vendor ID: " << matches[1]
                    << L", Product ID: " << matches[2] << std::endl;
            }

            std::wcout << std::endl;
        }
        else {
            std::cerr << "Eroare la obținerea detaliilor dispozitivului." << std::endl;
        }

        free(deviceDetailData);
        index++;
    }

    if (index == 0) {
        std::cout << "Niciun dispozitiv USB nu a fost găsit." << std::endl;
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    std::cout << "\nApăsați Enter pentru a ieși...";
    std::cin.get();
    return 0;
}
