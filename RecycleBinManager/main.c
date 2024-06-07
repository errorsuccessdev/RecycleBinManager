/*
* Recycle Bin Manager - Access and empty your recycle bin from anywhere.
*/

#pragma once
#include "ini.h"
#include "logger.h"
#include <Windows.h>
#include <windowsx.h>
#include <shlobj_core.h>
#include <stdio.h>
#include <assert.h>

// Setup for using Common Controls

#pragma comment(linker,										\
				"\"/manifestdependency:type='Win32' "		\
				"name='Microsoft.Windows.Common-Controls' " \
				"version='6.0.0.0' "						\
				"processorArchitecture='*' "				\
				"publicKeyToken='6595b64144ccf1df' "		\
				"language='*'\"")

// Constants

#define ALIGNMENT_DWORD	4
#define ALIGNMENT_WORD	2
#define TOOLTIP_TEXT	L"Determines whether the delete confirmation \
dialog is displayed"

// IDs

#define WM_CUSTOM_SHUPDATEIMAGE	(WM_USER + 100)
#define ID_BUTTON_OPEN_BIN		100
#define ID_BUTTON_EMPTY_BIN		200
#define ID_CHECKBOX_SHOW_DIALOG	300
#define ID_TOOLTIP_SHOW_DIALOG	400
#define ID_CHECKBOX_SUBCLASS	1
#define ID_ICON_FULL_BIN		32 // Part of Shell32, do not change
#define ID_ICON_EMPTY_BIN		31 // Part of Shell32, do not change

// Structs

typedef struct Tooltip
{
	HWND hWnd; // The HWND of the tooltip
	TTTOOLINFOW toolInfo; // The Toolnfo for the tooltip
} Tooltip;

// Dialog box helper functions

void* alignPointer(void* pointer, ULONG_PTR alignment);
void centerWindow(HWND hWnd);
size_t copyAndReturnLengthWithTerminator(const wchar_t* source, wchar_t* dest);
int createDialogBox(HINSTANCE hInstance, HWND hWndOwner);
void updateGui(HWND hWnd);
void testGuiState(HWND hWnd, unsigned long registrationId);

// Checkbox helper functions

Tooltip* createToolTip(HWND hWndTool);
void displayShowDeleteDialogWarning(HWND hWnd);
BOOL isShowDeleteDialogChecked(HWND hWndDialog);
void setCheckboxState(HWND hWndDialog, BOOL setCheck);

// Recycle Bin helpr functions

BOOL isBinFull(void);
unsigned long registerForShellNotifs(HWND hWnd);

// Window procedures

LRESULT CALLBACK checkboxProc(HWND hWndCheckbox, unsigned int msg,
							  WPARAM wParam, LPARAM lParam, UINT_PTR subclassId,
							  DWORD_PTR tooltipPointer);
INT_PTR CALLBACK dialogProc(HWND hWndDialog, unsigned int msg, WPARAM wParam,
							LPARAM lParam);

/// @brief aligns a pointer to a specified boundary
/// @param pointer the pointer to align
/// @param alignment the boundary to align the pointer to
/// @note the alignment is of type ULONG_PTR to avoid a cast
/// @return the aligned pointer
void* alignPointer(void* pointer,
				   ULONG_PTR alignment)
{
	ULONG_PTR pointerAsNumber = (ULONG_PTR) pointer;
	pointerAsNumber = (pointerAsNumber + (alignment - 1)) / alignment;
	return (void*) (pointerAsNumber * alignment);
}

/// @brief centers a window on the desktop
/// @param hWnd a handle to the window to center
void centerWindow(HWND hWnd)
{
	RECT rectDesktop, rectDialog, rectDesktopCopy;
	GetWindowRect(GetDesktopWindow(),
				  &rectDesktop);
	GetWindowRect(hWnd,
				  &rectDialog);
	CopyRect(&rectDesktopCopy,
			 &rectDesktop);

	// Moves rect into top left corner
	// Desktop is already in the top left corner so we don't need to do this
	OffsetRect(&rectDialog,
			   -rectDialog.left,
			   -rectDialog.top);

	// Moves left and above the top left corner by the size of the dialog box
	OffsetRect(&rectDesktopCopy,
			   -rectDialog.right,
			   -rectDialog.bottom);

	SetWindowPos(hWnd,
				 HWND_TOP,
				 rectDesktop.left + (rectDesktopCopy.right / 2),
				 rectDesktop.top + (rectDesktopCopy.bottom / 2),
				 0,
				 0,
				 SWP_NOSIZE);
}

/// @brief copies a wide string and returns the length with null terminator
/// @param source the wide string to be copied
/// @param dest the pointer to a wide string to copy into
/// @return the length of the copied string, INCLUDING the null terminator
size_t copyAndReturnLengthWithTerminator(const wchar_t* source,
										 wchar_t* dest)
{
	wcscpy(dest, source);
	return (wcslen(source) + 1); // Add 1 for null terminator
}

/// @brief create the main window's dialog box template in memory
/// @param hInstance a handle to the application instance
/// @param hWndOwner the owner window of the dialog box, this can be NULL
/// @return 0 if the dialog box was created successfully
///			1 if we cannot allocate memory for the template
///			-1 if creation fails for any other reason
int createDialogBox(HINSTANCE hInstance,
					HWND hWndOwner)
{
	WORD numControls = 3;
	short borderPadding = 3; // The amount of padding around the window border
	short buttonPadding = 2; // The amount of padding between buttons
	short buttonWidth = 80;
	short buttonHeight = 13;
	short checkboxPadding = 7; // The amount of padding to the left of the checkbox
	short checkboxHeight = 10;
	short checkboxWidth = buttonWidth;
	WORD fontSize = 11;
	const wchar_t* fontName = L"Segoe UI";
	const wchar_t* windowTitle = L"Recycle Bin Manager";
	const wchar_t* openButtonTitle = L"Open Recycle Bin";
	const wchar_t* emptyButtonTitle = L"Empty Recycle Bin";
	const wchar_t* showDialogCheckboxTitle = L"Show delete dialog";

	DLGITEMTEMPLATE* dialogItemTemplate;
	WORD* wordPointer;
	wchar_t* wideStringPointer;
	DLGTEMPLATE* dialogTemplate = (DLGTEMPLATE*) HeapAlloc(GetProcessHeap(),
														   HEAP_ZERO_MEMORY,
														   1024);
	if (dialogTemplate == NULL) // Memory allocation failed
	{
		return 1;
	}

	dialogTemplate = (DLGTEMPLATE*) alignPointer((void*) dialogTemplate,
												 ALIGNMENT_DWORD);
	dialogTemplate->style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION |
		DS_MODALFRAME | DS_SETFONT;
	dialogTemplate->cdit = numControls; // Number of controls
	dialogTemplate->x = 0;
	dialogTemplate->y = 0;
	dialogTemplate->cx = buttonWidth + (2 * borderPadding);
	dialogTemplate->cy = (buttonHeight * 2) +
		checkboxHeight +
		(borderPadding * 2) +
		(buttonPadding * (numControls - 1));

	wordPointer = (WORD*) alignPointer(dialogTemplate + 1,
									   ALIGNMENT_WORD);
	*wordPointer++ = 0; // Do not add a menu
	*wordPointer++ = 0; // Use the default dialog box class

	wideStringPointer = (wchar_t*) alignPointer(wordPointer,
												ALIGNMENT_WORD);
	wordPointer += copyAndReturnLengthWithTerminator(windowTitle,
													 wideStringPointer);

	// Set font size
	wordPointer = alignPointer(wordPointer,
							   ALIGNMENT_WORD);
	*wordPointer++ = fontSize;

	// Set font name
	wideStringPointer = (wchar_t*) alignPointer(wordPointer,
												ALIGNMENT_WORD);
	wordPointer += copyAndReturnLengthWithTerminator(fontName,
													 wideStringPointer);

	// First button
	wordPointer = alignPointer(wordPointer,
							   ALIGNMENT_DWORD);
	dialogItemTemplate = (DLGITEMTEMPLATE*) wordPointer;
	dialogItemTemplate->x = borderPadding;
	dialogItemTemplate->y = borderPadding;
	dialogItemTemplate->cx = buttonWidth;
	dialogItemTemplate->cy = buttonHeight;
	dialogItemTemplate->id = ID_BUTTON_OPEN_BIN;
	dialogItemTemplate->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON;
	wordPointer = (WORD*) alignPointer(dialogItemTemplate + 1,
									   ALIGNMENT_WORD);
	*wordPointer++ = 0xFFFF; // Use a system class
	*wordPointer++ = 0x0080; // Button class

	wideStringPointer = (wchar_t*) alignPointer(wordPointer,
												ALIGNMENT_WORD);
	wordPointer += copyAndReturnLengthWithTerminator(openButtonTitle,
													 wideStringPointer);
	*wordPointer++ = 0; // There is no additional data

	// Second button
	wordPointer = alignPointer(wordPointer,
							   ALIGNMENT_DWORD);
	dialogItemTemplate = (DLGITEMTEMPLATE*) wordPointer;
	dialogItemTemplate->x = borderPadding;
	dialogItemTemplate->y = borderPadding + buttonPadding + buttonHeight;
	dialogItemTemplate->cx = buttonWidth;
	dialogItemTemplate->cy = buttonHeight;
	dialogItemTemplate->id = ID_BUTTON_EMPTY_BIN;
	dialogItemTemplate->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

	wordPointer = (WORD*) alignPointer(dialogItemTemplate + 1,
									   ALIGNMENT_WORD);
	*wordPointer++ = 0xFFFF; // Use a system class
	*wordPointer++ = 0x0080; // Button class

	wideStringPointer = (wchar_t*) alignPointer(wordPointer,
												ALIGNMENT_WORD);
	wordPointer += copyAndReturnLengthWithTerminator(emptyButtonTitle,
													 wideStringPointer);
	*wordPointer++ = 0; // There is no additional data

	// Checkbox
	wordPointer = alignPointer(wordPointer,
							   ALIGNMENT_DWORD);
	dialogItemTemplate = (DLGITEMTEMPLATE*) wordPointer;
	dialogItemTemplate->x = checkboxPadding;
	dialogItemTemplate->y = borderPadding +
		(2 * buttonPadding) +
		(2 * buttonHeight);
	dialogItemTemplate->cx = checkboxWidth;
	dialogItemTemplate->cy = checkboxHeight;
	dialogItemTemplate->id = ID_CHECKBOX_SHOW_DIALOG;
	dialogItemTemplate->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX;

	wordPointer = (WORD*) alignPointer(dialogItemTemplate + 1,
									   ALIGNMENT_WORD);
	*wordPointer++ = 0xFFFF; // Use a system class
	*wordPointer++ = 0x0080; // Button class

	wideStringPointer = (wchar_t*) alignPointer(wordPointer,
												ALIGNMENT_WORD);
	wordPointer += copyAndReturnLengthWithTerminator(showDialogCheckboxTitle,
													 wideStringPointer);
	*wordPointer++ = 0; // There is no additional data

	// Create the dialog box
	INT_PTR result = DialogBoxIndirectParamW(hInstance,
											 dialogTemplate,
											 hWndOwner,
											 (DLGPROC) dialogProc,
											 0L);

	// Per https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxindirectparamw
	// the return value will be -1 if the call to DialogBoxIndirectParamW fails for 
	// any other reason than an invalid parent HWND. The parent HWND is invalid in 
	// our case anyway since the dialog box itself is the top level window. Therefore,
	// the only return value we care about is -1.
	HeapFree(GetProcessHeap(),
			 0,
			 dialogTemplate);
	return (result == -1) ? -1 : 0;
}

/// @brief update the dialog box controls to reflect the current state of the bin
/// @param hWndDialog a window handle to the dialog box
void updateGui(HWND hWndDialog)
{
	BOOL binIsFull = isBinFull();

	// Set the icon. We need to add one to the icon ID to get the correct 
	// icon from Shell32. I am unsure why this is the case.
	int icon = ((binIsFull) ? ID_ICON_FULL_BIN : ID_ICON_EMPTY_BIN) + 1;
	HICON hIcon = LoadIconW(LoadLibraryW(L"shell32.dll"),
							MAKEINTRESOURCEW(icon));
	SendMessageW(hWndDialog,
				 WM_SETICON,
				 ICON_SMALL,
				 (LPARAM) hIcon);
	SendMessageW(hWndDialog,
				 WM_SETICON,
				 ICON_BIG,
				 (LPARAM) hIcon);

	// Enable or disable the empty button
	EnableWindow(GetDlgItem(hWndDialog,
							ID_BUTTON_EMPTY_BIN),
				 binIsFull);
	SetFocus(GetDlgItem(hWndDialog,
						ID_BUTTON_OPEN_BIN));
}

/// @brief verifies that the dialog box controls are consistent with the bin state
/// in a debug build, returns immediately in a release build
/// @param hWndDialog a handle to the dialog window
/// @param registrationId the current registration ID for shell notifications.
/// If this is is 0 (registration failed), the GUI will not update
void testGuiState(HWND hWndDialog,
				  unsigned long registrationId)
{
	// If debugging is not enabled, we don't have assert, 
	// and these tests are counterproductive
#ifndef NDEBUG

	// If registration failed, the GUI will not update
	if (registrationId == 0)
	{
		return;
	}

	// Get the actual state of the recycle bin
	BOOL binIsFull = isBinFull();

	// There wasn't a problem with querying the recycle bin
	assert(binIsFull > -1);

	// Get the current icon
	HICON hIcon = (HICON) SendMessageW(hWndDialog,
									   WM_GETICON,
									   ICON_SMALL,
									   0);
	ICONINFOEXW info = { .cbSize = sizeof(ICONINFOEXW) };
	BOOL result = GetIconInfoExW(hIcon,
								 &info);
	assert(result);

	// The current icon ID is one greater than its ID in Shell32. 
	// Again, I am unsure why this is the case. 
	WORD iconId = info.wResID - 1;
	assert(iconId > 0);

	// Get the current state of the empty button
	BOOL btnEmptyEnabled = IsWindowEnabled(GetDlgItem(hWndDialog,
													  ID_BUTTON_EMPTY_BIN));

	// Check icon
	assert(iconId == ((binIsFull) ? ID_ICON_FULL_BIN : ID_ICON_EMPTY_BIN));

	// Check empty button 
	assert(btnEmptyEnabled == binIsFull);
#endif
}

/// @brief creates a tooltip
/// @param hWndTool a handle to the "tool" the tooltip should be created for
/// @note "tool" typically refers to a control, e.g. a checkbox in our case
/// @return a pointer to a Tooltip structure containing the created tooltip,
/// NULL if tooltip creation has failed
Tooltip* createToolTip(HWND hWndTool)
{
	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtrW(hWndTool,
														GWLP_HINSTANCE);
	HWND hWndTooltip = CreateWindowExW(0,
									   TOOLTIPS_CLASSW,
									   NULL,
									   WS_POPUP | TTS_ALWAYSTIP,
									   CW_USEDEFAULT,
									   CW_USEDEFAULT,
									   CW_USEDEFAULT,
									   CW_USEDEFAULT,
									   hWndTool,
									   NULL,
									   hInstance,
									   NULL);
	if (!hWndTooltip)
	{
		return NULL;
	}

	// Build the toolinfo structure
	TTTOOLINFOW toolInfo = { 0 };
	toolInfo.cbSize = sizeof(TTTOOLINFOW);
	toolInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	toolInfo.hwnd = hWndTool;
	toolInfo.hinst = hInstance;
	toolInfo.lpszText = TOOLTIP_TEXT;
	toolInfo.uId = (UINT_PTR) hWndTool;
	GetClientRect(hWndTool, &toolInfo.rect);

	// Associate the tooltip with the tool
	SendMessageW(hWndTooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTTTOOLINFOW) &toolInfo);
	Tooltip* tooltip = HeapAlloc(GetProcessHeap(),
								 HEAP_ZERO_MEMORY,
								 sizeof(Tooltip));
	if (tooltip == NULL) // Memory allocation failed
	{
		return NULL;
	}

	// Build the Tooltip structure
	tooltip->hWnd = hWndTooltip;
	tooltip->toolInfo = toolInfo;
	return tooltip;
}

/// @brief displays a warning messagebox explaining the risks of hiding the
/// confirmation dialog
/// @param hWnd a handle to the owner window of the messagebox
void displayShowDeleteDialogWarning(HWND hWnd)
{
	static BOOL warningShown = FALSE;
	if (warningShown == FALSE)
	{
		MessageBoxW(hWnd, L"This setting specifies that the Recycle Bin will be "
					L"emptied WITHOUT prompting the user beforehand. This will cause "
					L"data loss if the empty button is clicked accidentally.",
					L"Warning: Potential Data Loss",
					MB_ICONWARNING);
		warningShown = TRUE;
	}
}

/// @brief check if the checkbox to show the delete dialog is checked
/// @param hWndDialog a handle to the dialog box window
/// @return TRUE if checked, FALSE if unchecked
BOOL isShowDeleteDialogChecked(HWND hWndDialog)
{
	unsigned int isChecked = IsDlgButtonChecked(hWndDialog,
												ID_CHECKBOX_SHOW_DIALOG);
	return (isChecked == BST_CHECKED);
}

/// @brief set the checkbox to either checked or unchecked
/// @param hWndDialog a handle to the dialog box window
/// @param setCheck whether to set the state to checked (TRUE) or 
/// unchecked (FALSE)
void setCheckboxState(HWND hWndDialog,
					  BOOL setCheck)
{
	HWND hWndCheckbox = GetDlgItem(hWndDialog,
								   ID_CHECKBOX_SHOW_DIALOG);
	Button_SetCheck(hWndCheckbox,
					(setCheck) ? BST_CHECKED : BST_UNCHECKED);
}

/// @brief checks if the Recycle Bin is currently full
/// @param none
/// @return TRUE if bin is full, 
///         FALSE if bin is empty, 
///         -1 if querying the bin has failed
BOOL isBinFull(void)
{
	// Get recycle bin information
	SHQUERYRBINFO info = { sizeof(SHQUERYRBINFO) };
	int result = SHQueryRecycleBinW(L"",
									&info);
	if (result != S_OK)
	{
		LOG(L"Querying the recycle bin failed with HRESULT %d\n",
			GetLastError());
		return -1;
	}
	return ((info.i64NumItems == 0) ? FALSE : TRUE);
}

/// @brief register for notifications from the shell
/// @param hWnd the window handle that will receive the notifications  
/// @return the registration ID if registration succeeds,
///			0 if registation fails
unsigned long registerForShellNotifs(HWND hWnd)
{
	// Even though we can retrieve a PIDL for the recycle bin, it does not generate 
	// filesystem  events (create, delete, update, etc.) when files are "moved" there 
	// or deleted. There does appear to be a reliable way to track state changes of 
	// the recycle bin, and that is via the UPDATEIMAGE notification, which fires 
	// because the shell needs to update the recycle bin icon when it is 
	// emptied or becomes full. We do not need to track a specific PIDL since this 
	// is a global notification.
	SHChangeNotifyEntry const entries[] = { NULL, FALSE };
	int sources = SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery;
	long events = SHCNE_UPDATEIMAGE;
	unsigned long registrationId = SHChangeNotifyRegister(hWnd,
														  sources,
														  events,
														  WM_CUSTOM_SHUPDATEIMAGE,
														  ARRAYSIZE(entries),
														  entries);
	if (registrationId == 0)
	{
		LOG(L"Registration for shell event notifications failed!\n");
		MessageBoxW(hWnd,
					L"Registration for recycle bin notifications has failed."
					" Functionality may be limited and the program may not behave as"
					" expected.",
					L"Registration Failed",
					MB_ICONWARNING);
	}
	return registrationId;
}

/// @brief the window procedure for the checkbox control
/// @param hWndCheckbox a window handle to the checkbox control
/// @param msg the window message
/// @param wParam
/// @param lParam
/// @param subclassId required by subclass window proc template
/// @param tooltipPointer a pointer to the Tooltip structure used by the checkbox
/// @return TRUE if message was fully processed by this procedure, 
///         FALSE or the return value of DefSubclassProc() if not
LRESULT CALLBACK checkboxProc(HWND hWndCheckbox,
							  unsigned int msg,
							  WPARAM wParam,
							  LPARAM lParam,
							  UINT_PTR subclassId,
							  DWORD_PTR tooltipPointer)
{
	UNREFERENCED_PARAMETER(subclassId);

	// Track if the mouse is currently within the checkbox
	static BOOL mouseIsInsideCheckbox = FALSE;

	// Dereference the tooltip pointer for easier use
	Tooltip tooltip = *((Tooltip*) tooltipPointer);

	switch (msg)
	{
		// Hide the tooltip when the mouse pointer leaves the window
		case WM_MOUSELEAVE:
		{
			SendMessageW(tooltip.hWnd,
						 TTM_TRACKACTIVATE,
						 (WPARAM) FALSE,
						 (LPARAM) &tooltip.toolInfo);
			mouseIsInsideCheckbox = FALSE;
			return FALSE;
		}
		case WM_MOUSEMOVE:
		{
			// Request mouse hover notifications
			TRACKMOUSEEVENT tracker =
			{
				.cbSize = sizeof(TRACKMOUSEEVENT),
				.dwFlags = TME_HOVER,
				.dwHoverTime = HOVER_DEFAULT,
				.hwndTrack = hWndCheckbox
			};
			TrackMouseEvent(&tracker);
			return FALSE;
		}
		case WM_MOUSEHOVER:
		{
			// Track the mouse position across calls
			static int priorMouseX, priorMouseY;
			int currentMouseX, currentMouseY;

			// If the mouse was not previously inside the checkbox
			if (!mouseIsInsideCheckbox)
			{
				// Request notification for mouse leave
				TRACKMOUSEEVENT tracker =
				{
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.hwndTrack = hWndCheckbox,
					.dwFlags = TME_LEAVE
				};
				TrackMouseEvent(&tracker);

				// Activate tooltip
				SendMessageW(tooltip.hWnd,
							 TTM_TRACKACTIVATE,
							 (WPARAM) TRUE,
							 (LPARAM) &tooltip.toolInfo);
				mouseIsInsideCheckbox = TRUE;
			}

			// Check if mouse has moved and update the tooltip position if so
			currentMouseX = GET_X_LPARAM(lParam);
			currentMouseY = GET_Y_LPARAM(lParam);
			if ((currentMouseX != priorMouseX) || (currentMouseY != priorMouseY))
			{
				priorMouseX = currentMouseX;
				priorMouseY = currentMouseY;
				POINT point = { currentMouseX, currentMouseY };
				ClientToScreen(hWndCheckbox,
							   &point);

				// Offset checkbox so it is above the cursor, 
				// but still aligned with the left edge of the cursor
				SendMessageW(tooltip.hWnd,
							 TTM_TRACKPOSITION,
							 0,
							 (LPARAM) MAKELONG(point.x,
											   point.y - 20));
			}
			return FALSE;
		}
		case WM_NCDESTROY:
		{
			// Free the tooltip's memory. Doing this on WM_DESTROY results in a
			// use after free because WM_NCDESTROY is sent after WM_DESTROY
			HeapFree(GetProcessHeap(),
					 0,
					 (Tooltip*) tooltipPointer);
			return FALSE;
		}
	}
	return DefSubclassProc(hWndCheckbox,
						   msg,
						   wParam,
						   lParam);
}

/// @brief the dialog box window procedure
/// @param hWndDialog a window handle to the dialog box
/// @param msg the window message
/// @param wParam 
/// @param lParam 
/// @return TRUE if message was fully processed by this procedure, 
///         FALSE if not
INT_PTR CALLBACK dialogProc(HWND hWndDialog,
							unsigned int msg,
							WPARAM wParam,
							LPARAM lParam)
{
	// This holds the registration ID we receive after calling SHChangeNotifyRegister
	static unsigned long registrationId = 0;

	UNREFERENCED_PARAMETER(lParam);
	switch (msg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ID_BUTTON_OPEN_BIN:
				{
					// Open the recycle bin
					ShellExecuteW(hWndDialog,
								  L"explore",
								  L"shell:RecycleBinFolder",
								  NULL,
								  NULL,
								  SW_SHOWNORMAL);
					return TRUE;
				}
				case ID_BUTTON_EMPTY_BIN:
				{
					DWORD emptyOperationFlags = (isShowDeleteDialogChecked(hWndDialog)) ?
						0 : (SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI);
					SHEmptyRecycleBinW(hWndDialog,
									   NULL,
									   emptyOperationFlags);
					return TRUE;
				}
				case ID_CHECKBOX_SHOW_DIALOG:
				{
					// If they are checking the checkbox, warn them
					BOOL currentlyChecked = isShowDeleteDialogChecked(hWndDialog);
					if (currentlyChecked)
					{
						displayShowDeleteDialogWarning(hWndDialog);
					}
					setCheckboxState(hWndDialog, (currentlyChecked) ? FALSE : TRUE);
					return TRUE;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hWndDialog);
			return TRUE;
		}
		case WM_DESTROY:
		{
			saveSettingToIni(checkForIni(),
							 INI_KEY_SHOW_DELETE_DIALOG,
							 isShowDeleteDialogChecked(hWndDialog));
			if (registrationId > 0)
			{
				SHChangeNotifyDeregister(registrationId);
			}
			PostQuitMessage(0);
			return TRUE;
		}
		case WM_INITDIALOG:
		{
			// Configure GUI to reflect the current state of the bin
			updateGui(hWndDialog);
			testGuiState(hWndDialog,
						 registrationId);

			//Configure GUI to reflect current ini settings
			setCheckboxState(hWndDialog,
							 getIniSetting(INI_KEY_SHOW_DELETE_DIALOG));

			// Center the window
			centerWindow(hWndDialog);

			// Register for shell notificaitons
			registrationId = registerForShellNotifs(hWndDialog);

			// Add tooltip
			HWND hWndCheckbox = GetDlgItem(hWndDialog,
										   ID_CHECKBOX_SHOW_DIALOG);
			Tooltip* tooltip = createToolTip(hWndCheckbox);

			// Add checkbox subclass for hover notifications
			SetWindowSubclass(GetDlgItem(hWndDialog,
										 ID_CHECKBOX_SHOW_DIALOG),
							  (SUBCLASSPROC) checkboxProc,
							  ID_CHECKBOX_SUBCLASS,
							  (DWORD_PTR) tooltip);
			return TRUE;
		}
		case WM_CUSTOM_SHUPDATEIMAGE:
		{
			LOG(L"ShUpdateImage event fired.\n");
			updateGui(hWndDialog);
			testGuiState(hWndDialog,
						 registrationId);
			return TRUE;
		}
	}
	return FALSE;
}

/// @brief application entry point
/// @param hInstance 
/// @param hPrevInstance 
/// @param cmdLine 
/// @param cmdShow 
/// @return result of dialog box creation
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ wchar_t* cmdLine,
					_In_ int cmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	UNREFERENCED_PARAMETER(cmdShow);

	// Initialize common controls (needed to give our window a modern appearance)
	INITCOMMONCONTROLSEX initControls =
	{
		sizeof(initControls),
		ICC_STANDARD_CLASSES
	};
	InitCommonControlsEx(&initControls);

	BOOL creationResult = createIniIfNonexistent();
	assert(creationResult);
	return createDialogBox(hInstance,
						   NULL);
}