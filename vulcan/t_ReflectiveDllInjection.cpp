//===============================================================================================//
// Copyright (c) 2012, Stephen Fewer of Harmony Security (www.harmonysecurity.com)
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted 
// provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice, this list of 
// conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above copyright notice, this list of 
// conditions and the following disclaimer in the documentation and/or other materials provided 
// with the distribution.
// 
//     * Neither the name of Harmony Security nor the names of its contributors may be used to
// endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//===============================================================================================//

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <ntsecapi.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "LoadLibraryR.h"

#pragma comment(lib,"Advapi32.lib")

#define BREAK_WITH_ERROR(e) { wprintf(TEXT("[-] %hs. Error=%d"), e, GetLastError()); break; }

extern "C" HANDLE __stdcall LoadRemoteLibraryR(HANDLE hProcess, LPVOID lpBuffer, DWORD dwLength, LPVOID lpParameter);

DWORD demoReflectiveDllInjection(PCWSTR cpDllFile, DWORD dwProcessId)
{
	HANDLE hFile = NULL;
	HANDLE hModule = NULL;
	HANDLE hProcess = NULL;
	LPVOID lpBuffer = NULL;
	DWORD dwLength = 0;
	DWORD dwBytesRead = 0;

	do
	{
		hFile = CreateFileW(cpDllFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) BREAK_WITH_ERROR("[-] Failed to open the DLL file!");

		dwLength = GetFileSize(hFile, NULL);
		if (dwLength == INVALID_FILE_SIZE || dwLength == 0) BREAK_WITH_ERROR("[-] Failed to get the DLL file size!");

#ifdef _DEBUG
		wprintf(TEXT("[+] File Size: %d\n"), dwLength);
#endif

		lpBuffer = HeapAlloc(GetProcessHeap(), 0, dwLength);
		if (!lpBuffer) BREAK_WITH_ERROR("[-] Failed to get the DLL file size!");

		if (ReadFile(hFile, lpBuffer, dwLength, &dwBytesRead, NULL) == FALSE) BREAK_WITH_ERROR("[-] Failed to alloc a buffer!");

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, dwProcessId);
		if (!hProcess) BREAK_WITH_ERROR("[-] Failed to open the target process!");

		hModule = LoadRemoteLibraryR(hProcess, lpBuffer, dwLength, NULL);
		if (!hModule) BREAK_WITH_ERROR("[-] Failed to inject the DLL!");

		wprintf(TEXT("[+] Injected '%s' into process ID %d!"), cpDllFile, dwProcessId);

		WaitForSingleObject(hModule, -1);

	} while (0);

	if (lpBuffer) HeapFree(GetProcessHeap(), 0, lpBuffer);

	if (hProcess) CloseHandle(hProcess);

	return 0;
}