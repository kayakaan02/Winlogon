
#include <Windows.h>
#include <wtsapi32.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>
#include "TCHAR.h"
#include <sstream>
#include "Wtsapi32.h"
#include <iostream>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <TlHelp32.h>
#include <filesystem>
#include <direct.h>
#include <iostream>
#pragma comment(lib, "Wtsapi32.lib")

//must use this to return last error as stdout
std::string  GetLastErrorAsString()
{
    //get the error message id
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //why not ask win32 to give it as string
    //tell win32 to create the buffer that holds the message for us because we don't yet know how long the message string will be
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //get that message as std::string.
    std::string message(messageBuffer, size);

    //free the dude
    LocalFree(messageBuffer);

    return message;
}

//I can't find the process widiout widening its names string version
std::wstring widen(const std::string& str)
{
    //uncommented code here, just use
    std::wostringstream wstm;
    const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc());
    for (size_t i = 0; i < str.size(); ++i)
        wstm << ctfacet.widen(str[i]);
    return wstm.str();
}

//return the process id, old code as widen() so uncommented
DWORD getProcess(std::string cstr)
{
    //widen and format the process' name because _tcsicmp is stupid(hes smarter than me)
    const std::wstring widestring = widen(cstr);
    const wchar_t* wcstr = widestring.c_str();

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    DWORD sessionID = WTSGetActiveConsoleSessionId();

    DWORD retval = 0;

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_tcsicmp(entry.szExeFile, wcstr) == 0)
            {
                retval = entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return retval;
}

bool isSessionLocked()
{
    WTSINFOEXW* pInfo = NULL;
    WTS_INFO_CLASS wtsic = WTSSessionInfoEx;
    LPTSTR ppBuffer = NULL;
    DWORD dwBytesReturned = 0;
    LONG sessionFlags = WTS_SESSIONSTATE_UNKNOWN; // until we know otherwise. Prevents a false positive since WTS_SESSIONSTATE_LOCK == 0

    DWORD dwSessionID = WTSGetActiveConsoleSessionId();

    if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, dwSessionID, wtsic, &ppBuffer, &dwBytesReturned))
    {
        if (dwBytesReturned > 0)
        {
            pInfo = (WTSINFOEXW*)ppBuffer;
            if (pInfo->Level == 1)
            {
                sessionFlags = pInfo->Data.WTSInfoExLevel1.SessionFlags;
            }
        }
        WTSFreeMemory(ppBuffer);
        ppBuffer = NULL;
    }

    return (sessionFlags == WTS_SESSIONSTATE_LOCK);
}

void sendEnter()
{
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_RETURN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_RETURN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

//install this codes deploy as a service otherwise we don't have enough permission to "steal" security token
int main()
{
    HANDLE hToken;
    HANDLE hProcess;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, getProcess("winlogon.exe"));

    if (hProcess == NULL)
    {
        std::cout << "where logon?" << GetLastErrorAsString() << std::endl;
        return 1;
    }

    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
    {
        std::cout << "doit get the process token, " << GetLastErrorAsString() << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hCurrentProcessToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hCurrentProcessToken))
    {
        std::cout << "you cant even get your own token, how did you get winlogon.exe's??, " << GetLastErrorAsString() << std::endl;
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return 1;
    }

    if (!ImpersonateLoggedOnUser(hToken))
    {
        std::cout << "guess who's isnt a master in disguise, " << GetLastErrorAsString() << std::endl;
        CloseHandle(hCurrentProcessToken);
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return 1;
    }

    LPTSTR lpCommandLine = NULL;
    LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
    LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
    BOOL bInheritHandles = FALSE;
    DWORD dwCreationFlags = CREATE_NO_WINDOW;
    LPVOID lpEnvironment = NULL;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    //we have to start the process in the login screen without a window, in line 169 if you want open a window idc
    std::string dg = "WinSta0\\WinLogOn";
    si.lpDesktop = const_cast<char*>(dg.c_str());
    //hide window even we didn't create a window
    si.wShowWindow = SW_HIDE;

    for (size_t i = 0; i < 6; i++) {
        //start process using CreateProcessAsUserA
        //YOU CAN CHANGE THE DIRECTORY AND EXECUTABLES NAME HERE
        if (!CreateProcessAsUserA(hToken, "C:\\sendin\\sendPassword.exe", NULL, NULL, NULL,
            bInheritHandles, dwCreationFlags, lpEnvironment, "C:\\sendin\\", &si, &pi))
        {
            std::cout << "CreateProcessAsUserA failed with error " << GetLastErrorAsString;
            return 1;
        }
        else {
            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            std::cout << "\nexitcode " << exitCode << std::endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        Sleep(1000);

        //in case of sending the wrong password
        if (isSessionLocked()) {
            sendEnter();
        }
        else {
            break;
        }

        Sleep(1000);
    }

    //close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hCurrentProcessToken);
    CloseHandle(hToken);
    CloseHandle(hProcess);

    return 0;
}