// injectview.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void PrintUsage() {
	std::wcout << L"Usage: " << L"./injectview" << L"[-ph]" << std::endl;
}

BOOL IsInAddressSpace(DWORD Pid, DWORD ThreadAddress) {
	HANDLE ProcessHandle;
	HANDLE ModuleSnapshot;
	DWORD cbneeded;
	MODULEENTRY32 me32 = {0};
	me32.dwSize = sizeof(MODULEENTRY32);

	ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Pid);

	if (ProcessHandle == NULL) {
		std::wcout << L"OpenProcess Exited with Error " << GetLastError() << L" at PID: " << Pid << std::endl;
			return true;
	}

	ModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid);
	
	if (ModuleSnapshot == NULL) {
		std::wcout << L"ModuleSnapshot Failed " << GetLastError() << L" at PID: " << Pid << std::endl;
		return true;
	}

	if (!Module32First(ModuleSnapshot, &me32))
	{
		CloseHandle(ModuleSnapshot);
		return(FALSE);
	}

	//  Now walk the module list of the process, 
	//  and display information about each module 
	do
	{

		DWORD max = (DWORD)me32.modBaseAddr + me32.modBaseSize;

		if (ThreadAddress >= (DWORD)me32.modBaseAddr && ThreadAddress <= max) {
			return true;
		}
		else {
			continue;
		}

	} while (Module32Next(ModuleSnapshot, &me32));
	

	return false;
}


/*
FindInjectedThread

Loops through the threads of the process in question and exaimines the security
attributes of them

*/
void FindInjectedThread(DWORD Pid) {
	HANDLE ThreadSnap = NULL;
	THREADENTRY32 te32 = { 0 };
	te32.dwSize = sizeof(THREADENTRY32);
	HANDLE ThreadHandle;
	DWORD StartAddress;
	DWORD InjectedThreadCount;

	//Get address of NtQueryInformationThread so we can use it
	NtQueryThreadPointer NtQueryInformationThread = (NtQueryThreadPointer)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread");

	//create a snapshot of the threads
	ThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (!Thread32First(ThreadSnap, &te32))
	{
		std::wcout << L"Error getting thread snapshot " << GetLastError() << std::endl;
		CloseHandle(ThreadSnap);     
		return;
	}

	//if we are getting all processes, just set the pid we are looking for to the current process
	if (AllProcesses == true) {
		Pid = te32.th32OwnerProcessID;
	}

	//loop through them and perform actions on them
	do {
		
		if (te32.th32OwnerProcessID == Pid) {
			
			std::wcout << L"[*] Checking Thread ID: " << te32.th32ThreadID << std::endl;

			//get a handle to the thread
			ThreadHandle = OpenThread(THREAD_ALL_ACCESS, NULL, te32.th32ThreadID);
			
			StartAddress = 0;
			//get start address information on thread via NtQueryInformationThread
			DWORD NtStatus = NtQueryInformationThread(ThreadHandle, ThreadQuerySetWin32StartAddress, &StartAddress, sizeof(DWORD_PTR), NULL);

			if (!IsInAddressSpace(Pid, StartAddress)) {
				std::wcout << L"[!] Thread ID: " << te32.th32ThreadID << " has likely been injected!" << std::endl;
				InjectedThreadCount = InjectedThreadCount + 1;
			}
		}

	} while (Thread32Next(ThreadSnap, &te32));

	std::wcout << L"[+] Finished. Potential Malicous Thread Count: " << InjectedThreadCount << std::endl;

	CloseHandle(ThreadSnap);

}

/*
Main function

Takes command line arguments in as the process to be inspected.
*/
int wmain(int argc, wchar_t **argv)
{
	DWORD Pid = 0;

	if (argc == 1 || argc > 3) {
		std::wcout << L"Usage: " << argv[0] << L" PID" << std::endl;
		return 0;
	}
	
	if (wcscmp(argv[1], L"--all") == 0) {
		AllProcesses = true;
		FindInjectedThread(Pid);
	}
	else if(wcscmp(argv[1],L"--pid") == 0 || wcscmp(argv[1],L"-p") == 0){
		Pid = wcstod(argv[1], _T('\0'));
		FindInjectedThread(Pid);
	}
	else {
		std::wcout << L"Invalid arguments" << std::endl;
		std::wcout << L"Usage: " << L"./injectview" << L"[-ph]" << std::endl;
	}

	return 0;

}

