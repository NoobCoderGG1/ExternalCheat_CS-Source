#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <Windows.h>
#include<string.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>
#include <fstream>
#include <cmath>
#include <sstream>
#include <math.h>  
#include <vector>
#include <algorithm>

#pragma warning( once : 4996 )
using std::cout;
using std::endl;

struct Memory {
    const WCHAR* processName;
    const WCHAR* moduleName;
    HANDLE processHandle;
    DWORD pid = 0;
    void* ClientModuleAddr;
    void* EngineModuleAddr;


    Memory(const WCHAR* processName) : processName{ processName }
    {
        std::string line; //Текущая строка файла
        std::ifstream file{ "C:\\Users\\Bob\\OneDrive\\Рабочий стол\\OFFSETS.txt" };
        processHandle = GetProcessByName(processName);
        if (!processHandle) {
            cout << "Error, process doesn't found!";
            return;
        }
        ClientModuleAddr = GetModuleFromProcess(pid, L"client.dll");
        EngineModuleAddr = GetModuleFromProcess(pid, L"engine.dll");
        //Работа с текстовым файлом 
        if (file.is_open()) {
            while (std::getline(file, line)) {
                //cout << line << endl;
            }
        }
    }
    HANDLE GetProcessByName(const WCHAR* name)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        if (Process32First(snapshot, &entry) == TRUE)
        {
            while (Process32Next(snapshot, &entry) == TRUE)
            {
                if (wcscmp(entry.szExeFile, name) == 0)
                {
                    pid = entry.th32ProcessID;
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
                }
            }
        }

        CloseHandle(snapshot);
        return 0;
    }
    void* GetModuleFromProcess(DWORD processID, const WCHAR* moduleName) {

        HMODULE hMods[1024];
        DWORD cbNeeded;
        MODULEINFO modinfo;
        // Print the process identifier.
        //printf("\nProcess ID: %u\n", processID);

        // Get a list of all the modules in this process.
        if (EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded))
        {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                WCHAR szModName[MAX_PATH];

                // Get the full path to the module's file.

                if (GetModuleFileNameEx(processHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    // Print the module name and handle value.
                    //_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]); //Вывод полного пути до модуля и его адрес
                    WCHAR* ch = _tcstok(szModName, L"\\");
                    while (ch != NULL) {
                        //_tprintf(TEXT("\t%s (0x%08X)\n"), ch); // <--- Вывод части пути до файла
                        ch = _tcstok(NULL, L"\\");
                        if (ch != NULL && wcscmp(ch, moduleName) == 0) {
                            //_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
                            GetModuleInformation(processHandle, hMods[i], &modinfo, sizeof(MODULEINFO));
                            return (void*)modinfo.lpBaseOfDll;
                        }
                    }
                }
            }
        }
    }
    template<typename TYPE>
    TYPE ReadAddr(DWORD addr) {
        TYPE value; //Значение хранящееся по этому адресу
        LPCVOID address = (LPCVOID)addr;
        ReadProcessMemory(processHandle, address, &value, sizeof(TYPE), 0);  // считываем значение по заданному адресу
        int Error = GetLastError();
        if (Error != 0) cout << "Ошибка в чтении: " << Error << endl;
        return value;  // возвращаем считанное значение
    }

    template<typename TYPE>
    void WriteAddr(DWORD addr, TYPE value) {
        LPVOID address = (LPVOID)addr;
        WriteProcessMemory(processHandle, address, &value, sizeof(TYPE), 0);
        int Error = GetLastError();
        if (Error != 0) cout << "Ошибка в записи: " << Error << endl;
        return;
    }

};