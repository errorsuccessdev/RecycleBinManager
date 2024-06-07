/*
* Use an ini file for persisting confirmation dialog setting
*
* Copyright(C) 2024 ERROR_SUCCESS Software
*
* This program is free software : you can redistribute it and /or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#pragma once
#include "ini.h"
#include "logger.h"

/// @brief checks for the existence of the ini file in the program's 
/// directory or localappdata
/// @param none
/// @return the path to the found ini file, or NULL if no ini is found
wchar_t* checkForIni(void)
{
	wchar_t* programDirPath = getProgramDirIniPath();
	wchar_t* appDataPath = getAppDataIniPath();
	if (GetFileAttributesW(programDirPath) != INVALID_FILE_ATTRIBUTES)
	{
		return programDirPath;
	}
	else if (GetFileAttributesW(appDataPath) != INVALID_FILE_ATTRIBUTES)
	{
		return appDataPath;
	}
	return NULL;
}

/// @brief converts a wide string to a UTF-8 string
/// @param wideString a wide string
/// @return a UTF-8 string
char* convertWideToUtf8(wchar_t* wideString)
{
	int stringSize = WideCharToMultiByte(CP_UTF8,
										 0,
										 wideString,
										 -1,
										 NULL,
										 0,
										 NULL,
										 NULL);
	char* string = HeapAlloc(GetProcessHeap(),
							 HEAP_ZERO_MEMORY,
							 stringSize);
	WideCharToMultiByte(CP_UTF8,
						0,
						wideString,
						-1,
						string,
						stringSize,
						NULL,
						NULL);
	return string;
}

/// @brief creates appdata directories used by the program
/// if they do not already exist
/// @param none
void createAppDataDirIfNonexistent(void)
{
	wchar_t* localAppData = getLocalAppDataDirectory();
	wchar_t pathBuilder[MAX_PATH] = { 0 };

	// Add vendor directory
	_snwprintf(pathBuilder,
			   ARRAYSIZE(pathBuilder),
			   L"%s\\%s",
			   localAppData,
			   PROGRAM_VENDOR);
	pathBuilder[MAX_PATH - 1] = 0;
	if (GetFileAttributesW(pathBuilder) == INVALID_FILE_ATTRIBUTES)
	{
		int createResult = CreateDirectoryW(pathBuilder,
											NULL);
		assert(createResult != 0);
	}

	// Add program name directory
	_snwprintf(pathBuilder,
			   ARRAYSIZE(pathBuilder),
			   L"%s\\%s",
			   pathBuilder,
			   PROGRAM_NAME);
	pathBuilder[MAX_PATH - 1] = 0;
	if (GetFileAttributesW(pathBuilder) == INVALID_FILE_ATTRIBUTES)
	{
		int createResult = CreateDirectoryW(pathBuilder,
											NULL);
		assert(createResult != 0);
	}
}

/// @brief creates the ini file at the specified path
/// @param progDirIniPath the full path to the ini file (including the filename)
/// @return TRUE if the ini file was created, FALSE if not
BOOL createIni(wchar_t* iniPath)
{
	// Open a handle at the specified path
	HANDLE hIni = CreateFileW(iniPath,
							  GENERIC_READ | GENERIC_WRITE,
							  0,
							  NULL,
							  OPEN_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);

	// If opening the file failed, log an error and return false
	if (hIni == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		LOG(L"Error creating file: %d\n",
			error);
		if (error == ERROR_ACCESS_DENIED)
		{
			LOG(L"Access was denied!\n");
		}
		else if (error == ERROR_PATH_NOT_FOUND)
		{
			LOG(L"Path not found: %s\n",
				iniPath);
		}
		return FALSE;
	}

	// Add a comment explaining the show delete dialog setting
	DWORD commentLength = (DWORD) strlen(INI_COMMENT);
	DWORD bytesWritten = 0;
	BOOL result = WriteFile(hIni,
							INI_COMMENT,
							commentLength,
							&bytesWritten,
							NULL);
	CloseHandle(hIni);
	if (result == FALSE)
	{
		LOG(L"There was a problem writing the comment to the file!\n");
		return FALSE;
	}
	result = saveSettingToIni(iniPath,
							  INI_KEY_SHOW_DELETE_DIALOG,
							  INI_DEFAULT_VALUE_SHOW_DELETE_DIALOG);
	assert(result);
	testIni();
	return result;
}

/// @brief creates the ini file if it does not already exist in the 
/// program's directory if possible, or the appdata directory if not
/// @param none
/// @return TRUE if the ini file was created, FALSE if not
BOOL createIniIfNonexistent(void)
{
	// Check if we already have an ini file
	if (checkForIni())
	{
		return TRUE;
	}

	// If we don't, create one
	wchar_t* programDirIniPath = getProgramDirIniPath();
	wchar_t* appDataIniPath = getAppDataIniPath();

	// Attempt to create ini in program's directory
	if (createIni(programDirIniPath))
	{
		LOG(L"Using ini file %s\n",
			programDirIniPath);
		return TRUE;
	}

	// Attempt to create ini file in local appdata as a fallback
	createAppDataDirIfNonexistent();
	if (createIni(appDataIniPath))
	{
		LOG(L"Using ini file %s\n",
			appDataIniPath);
		return TRUE;
	}
	LOG(L"No ini files can be created!\n");
	return FALSE;
}

/// @brief gets the path to the ini file in appdata
/// @param none
/// @return the path to the ini file, including the filename
wchar_t* getAppDataIniPath(void)
{
	static BOOL pathIsTooLong = FALSE;
	static wchar_t appDataIniPath[MAX_PATH + 1] = { 0 };
	if (pathIsTooLong)
	{
		return NULL;
	}
	else if (appDataIniPath[0] == 0)
	{
		wchar_t* appDataDir = getLocalAppDataDirectory();

		// Check if resulting path is too long. We need 3 slashes.
		size_t pathLength = wcslen(appDataDir) +
			wcslen(PROGRAM_VENDOR) +
			wcslen(PROGRAM_NAME) +
			wcslen(INI_FILENAME) +
			3;
		if (pathLength > MAX_PATH)
		{
			pathIsTooLong = TRUE;
		}

		// Otherwise, build the path
		else
		{
			_snwprintf(appDataIniPath,
					   ARRAYSIZE(appDataIniPath),
					   L"%s\\%s\\%s\\%s",
					   appDataDir,
					   PROGRAM_VENDOR,
					   PROGRAM_NAME,
					   INI_FILENAME);
		}
	}
	return appDataIniPath;
}

/// @brief gets the local appdata directory using its known folder ID
/// @param none
/// @return the local appdata directory
wchar_t* getLocalAppDataDirectory(void)
{
	static wchar_t localAppData[MAX_PATH + 1] = { 0 };
	if (localAppData[0] == 0)
	{
		wchar_t* localAppDataKnownFolder;
		HRESULT result = SHGetKnownFolderPath(&FOLDERID_LocalAppData,
											  0,
											  NULL,
											  &localAppDataKnownFolder);
		if (result == S_OK)
		{
			LOG(L"Local AppData is: %s\n",
				localAppDataKnownFolder);
			wcscpy(localAppData,
				   localAppDataKnownFolder);
		}
		else
		{
			LOG(L"Failed to get local appdata folder!\n");
		}
		CoTaskMemFree(localAppDataKnownFolder);
	}
	return localAppData;
}

/// @brief gets the path to the ini file in the program's directory
/// @param none
/// @return the path to the ini file, including the filename
wchar_t* getProgramDirIniPath(void)
{
	static BOOL pathIsTooLong = FALSE;
	static wchar_t progDirIniPath[MAX_PATH + 1] = { 0 };

	// If we know the path is too long, return NULL
	if (pathIsTooLong)
	{
		return NULL;
	}

	else if (progDirIniPath[0] == 0)
	{
		// Otherwise, try building the file path
		GetModuleFileNameW(NULL,
						   progDirIniPath,
						   MAX_PATH);
		PathCchRemoveFileSpec(progDirIniPath,
							  MAX_PATH);

		// We need to add a character for the \ we're going to add the path
		size_t pathLength = wcslen(progDirIniPath) + wcslen(INI_FILENAME) + 1;

		// Check if the resulting path will be too long
		if (pathLength > MAX_PATH)
		{
			LOG(L"%s %s %s\n",
				L"The file path",
				progDirIniPath,
				L"is too long.");
			pathIsTooLong = TRUE;
			return NULL;
		}

		// If not, create the path and store it in appDataIniPath
		_snwprintf(progDirIniPath,
				   ARRAYSIZE(progDirIniPath),
				   L"%s\\%s",
				   progDirIniPath,
				   INI_FILENAME);
		LOG(L"%s %s\n",
			L"Settings.ini path:",
			progDirIniPath);
	}
	return progDirIniPath;
}

/// @brief get a setting in the ini file
/// @param key the key name of the setting to retreive
/// @return the setting, evaluated as a BOOL variable, defaulted to FALSE
/// @note currently, this only returns true if the setting is explicitly
/// equal to 1. This is to reduce the possibility of the setting being set 
/// to true accidentally.
BOOL getIniSetting(wchar_t* key)
{
	wchar_t* iniPath = checkForIni();
	assert(iniPath != NULL);
	if (iniPath == NULL)
	{
		return FALSE; // Defualt to false if there is a problem with the ini file
	}
	int fileSetting = GetPrivateProfileIntW(INI_SECTION_NAME,
											key,
											INT_MAX,
											iniPath);
	assert(fileSetting != INT_MAX);
	LOG(L"Show delete dialog in file set to %d\n",
		fileSetting);
	// We actually care if this is 1
	return ((fileSetting == 1) ? TRUE : FALSE);
}

/// @brief saves a setting to the ini file
/// @param iniPath the path to the ini file
/// @param key the key of the setting
/// @param value the value of the setting
/// @return TRUE if saving the setting succeeds, FALSE othwerise
BOOL saveSettingToIni(wchar_t* iniPath,
					  wchar_t* key,
					  BOOL value)
{
	// Add the setting to the ini file
	wchar_t* valueAsString = (value) ? L"1" : L"0";
	BOOL result = WritePrivateProfileStringW(INI_SECTION_NAME,
											 key,
											 valueAsString,
											 iniPath);
	return result;
}

/// @brief compares the actual and expected contents of a newly created
/// ini file in a debug build, returns immediately in a release build
/// @param none
void testIni(void)
{
#ifndef NDEBUG

	// Verify ini file exists
	wchar_t* iniPath = checkForIni();
	assert(iniPath != NULL); // There was not a problem with the path
	DWORD attributes = GetFileAttributesW(iniPath);
	assert(attributes != INVALID_FILE_ATTRIBUTES); // File exists

	// Verify contents
	HANDLE hFile = CreateFileW(iniPath,
							   GENERIC_READ,
							   0,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	assert(hFile != INVALID_HANDLE_VALUE); // We have a valid handle to the file

	char buffer[1024] = { 0 };
	BOOL result = ReadFile(hFile,
						   buffer,
						   1024,
						   NULL,
						   NULL);
	CloseHandle(hFile);
	assert(result); // Read operation succeeded
	char* utf8IniSection = convertWideToUtf8(INI_SECTION_NAME);
	char* utf8ShowDeleteDialog = convertWideToUtf8(INI_KEY_SHOW_DELETE_DIALOG);
	char expectedContents[1024] = { 0 };
	snprintf(expectedContents,
			 sizeof(expectedContents),
			 "%s[%s]\r\n%s=%d\r\n",
			 INI_COMMENT,
			 utf8IniSection,
			 utf8ShowDeleteDialog,
			 INI_DEFAULT_VALUE_SHOW_DELETE_DIALOG);
	HeapFree(GetProcessHeap(),
			 0,
			 utf8IniSection);
	HeapFree(GetProcessHeap(),
			 0,
			 utf8ShowDeleteDialog);

	// Verify the expected file contents are the same as the actual contents
	assert(strcmp(buffer, expectedContents) == 0);

	// Verify show delete dialog setting is readable and has correct initial value
	int showDeleteDialogSetting = GetPrivateProfileIntW(INI_SECTION_NAME,
														INI_KEY_SHOW_DELETE_DIALOG,
														INT_MAX,
														iniPath);
	assert(showDeleteDialogSetting == INI_DEFAULT_VALUE_SHOW_DELETE_DIALOG);
#endif
}
