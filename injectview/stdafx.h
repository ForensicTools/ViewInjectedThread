// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <TlHelp32.h>
#include <iomanip>
#include <psapi.h>



// TODO: reference additional headers your program requires here

typedef NTSTATUS(WINAPI *NtQueryThreadPointer)(HANDLE, LONG, PVOID, ULONG, PULONG);

#define ThreadQuerySetWin32StartAddress 9

#define Padding 30

bool AllProcesses = false;