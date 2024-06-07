/*
* RBMLauncher - Helper program for launching Recycle Bin Manager from
* a shortcut. 
* 
* If a shortcut is created directly to Recycle Bin Manager, the dynamic
* icons used to show bin state will not work. Therefore, we need an intermediate
* program to create a shortcut to that will run silently and launch RBM. 
* This also allows us to set a static icon in the start menu in a manner that
* does not interfere with the dynamic icons in RBM.
* 
* Copyright (C) 2024 ERROR_SUCCESS Software
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _CRT_SECURE_NO_WARNINGS

#pragma once
#include "logger.h"
#include <Windows.h>
#include <stdio.h>
#include <PathCch.h>

#define RBM_EXE_NAME	L"RecycleBinManager.exe"

/// @brief entry point, starts RecycleBinManager and exits
/// @param hInstance 
/// @param hPrevInstance 
/// @param cmdLine 
/// @param cmdShow 
/// @return 0 on success, 
///			-1 if the path to the executable is too long,
///			GetLastError() result if the process fails to start
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ wchar_t* cmdLine,
					_In_ int cmdShow)
{
	static wchar_t rbmExePath[MAX_PATH + 1] = { 0 };

	// Get the current program's directory
	GetModuleFileNameW(NULL,
					   rbmExePath,
					   MAX_PATH);
	PathCchRemoveFileSpec(rbmExePath,
						  MAX_PATH);

	// Add a character for the \ we're going to add the path
	size_t pathLength = wcslen(rbmExePath) + wcslen(RBM_EXE_NAME) + 1;

	// Check if the resulting path will be too long
	if (pathLength > MAX_PATH)
	{
		LOG(L"The file path %s is too long",
			rbmExePath);
		return -1;
	}

	// If not, create the path
	_snwprintf(rbmExePath,
			   ARRAYSIZE(rbmExePath),
			   L"%s\\%s",
			   rbmExePath,
			   RBM_EXE_NAME);
	rbmExePath[MAX_PATH] = 0;

	// Start recycle bin manager
	STARTUPINFOW startupInfo = { 0 };
	PROCESS_INFORMATION procInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	BOOL result = CreateProcessW(rbmExePath,
								 NULL,
								 NULL,
								 NULL,
								 FALSE,
								 0,
								 NULL,
								 NULL,
								 &startupInfo,
								 &procInfo);
	if (result == FALSE)
	{
		int error = GetLastError();
		LOG(L"Failed to start recycle bin manager, error code %d\n",
			error);
		return error;
	}

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	return 0;
}
