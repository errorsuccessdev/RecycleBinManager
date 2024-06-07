#define _CRT_SECURE_NO_WARNINGS

#pragma once
#include <Windows.h>
#include <KnownFolders.h>
#include <PathCch.h>
#include <shlobj_core.h>
#include <stdio.h>
#include <assert.h>

// Settings.ini file parameters

#define PROGRAM_VENDOR							L"ERROR_SUCCESS Software"
#define PROGRAM_NAME							L"Recycle Bin Manager"	
#define INI_FILENAME							L"Settings.ini"
#define INI_SECTION_NAME						L"Settings"
#define INI_KEY_SHOW_DELETE_DIALOG				L"ShowDeleteDialog"
#define INI_DEFAULT_VALUE_SHOW_DELETE_DIALOG	TRUE
#define INI_COMMENT								"; ShowDeleteDialog controls \
if a confirmation dialog appears when emptying the recycle bin.\r\n\
; Set to 1 to be prompted before the recycle bin is emptied.\r\n\
; Set to 0 to skip the dialog (files will be PERMANENTLY DELETED when empty \
button is clicked).\r\n\r\n"

// Functions

wchar_t* checkForIni(void);
char* convertWideToUtf8(wchar_t* wString);
void createAppDataDirIfNonexistent(void);
BOOL createIni(wchar_t* iniPath);
BOOL createIniIfNonexistent(void);
wchar_t* getAppDataIniPath(void);
wchar_t* getLocalAppDataDirectory(void);
wchar_t* getProgramDirIniPath(void); 
BOOL getIniSetting(wchar_t* key);
BOOL saveSettingToIni(wchar_t* iniPath, wchar_t* key, BOOL value);
void testIni(void);