#include <windows.h>
#include <iostream>
#include <string>

int main() {
    HMODULE hDll = LoadLibraryA("RobotStrategyDll.dll");

    if (!hDll) {
        std::cout << "Failed to load DLL! Error: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "DLL loaded at: " << hDll << std::endl;
    std::cout << "Listing exported functions:" << std::endl;
    std::cout << "================================" << std::endl;

    // 获取 DLL 的基地址
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hDll;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hDll + dosHeader->e_lfanew);
    IMAGE_DATA_DIRECTORY* exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (exportDir->Size > 0) {
        IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hDll + exportDir->VirtualAddress);

        DWORD* names = (DWORD*)((BYTE*)hDll + exports->AddressOfNames);
        WORD* ordinals = (WORD*)((BYTE*)hDll + exports->AddressOfNameOrdinals);
        DWORD* functions = (DWORD*)((BYTE*)hDll + exports->AddressOfFunctions);

        for (DWORD i = 0; i < exports->NumberOfNames; i++) {
            const char* name = (const char*)((BYTE*)hDll + names[i]);
            DWORD funcAddr = functions[ordinals[i]];

            std::cout << "  " << name << " at 0x" << std::hex << funcAddr << std::dec << std::endl;
        }
    }

    FreeLibrary(hDll);
    return 0;
}
