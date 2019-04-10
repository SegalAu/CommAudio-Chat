/*---------------------------------------------------------------------------------------------------
-- SOURCE FILE: application.cpp
--
-- PROGRAM :    DUMB TERMINAL
--
-- FUNCTIONS:
-- int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
--
-- DATE: October 3, 2018
--
-- DESIGNER: Segal Au, A01000835
--
-- PROGRAMMER: Segal Au, A01000835
--
-- NOTES:
-- Handles entry point of program and any input sent to application from user.
---------------------------------------------------------------------------------------------------*/

#define STRICT

#include <windows.h>
#include <stdio.h>
#include "Header.h"
#include "physical.h"
#include "session.h"
#include "DataCommAssign1.h"

char Name[] = "Comm Shell";
char* buffer;
LPDWORD nRead, nWrite;
char str[80];
static unsigned k = 0;
boolean connected;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       WndMain
--
-- INTERFACE :	   LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Entry point for application. Provides highest level of control and executes loop for retrieving messages.
----------------------------------------------------------------------------------------------------------------------*/



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#pragma warning (disable: 4096)

LPCSTR	lpszCommName;
COMMCONFIG	cc;
HANDLE hComm;
HANDLE hThread;
DWORD threadId;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;
	connected = false;
	

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = "MYMENU"; // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);


	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	

	//Loop to receive message
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       WndProc
--
-- INTERFACE :	   LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Handles all input sent to window.
----------------------------------------------------------------------------------------------------------------------*/


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	static unsigned k = 0;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			lpszCommName = "COM1";
			EnableMenuItem(GetMenu(hwnd), IDM_CONNECT, MF_ENABLED);
			CloseHandle(hComm);
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			if (!CommConfigDialog(lpszCommName, hwnd, &cc))
				break;
			SetCommConfig(hComm, &cc, sizeof(COMMCONFIG));
			
			break;
		case IDM_COM2:
			lpszCommName = "COM2";
			EnableMenuItem(GetMenu(hwnd), IDM_CONNECT, MF_ENABLED);
			CloseHandle(hComm);
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			if (!CommConfigDialog(lpszCommName, hwnd, &cc))
				break;
			SetCommConfig(hComm, &cc, sizeof(COMMCONFIG));			
			break;
		case IDM_COM3:
			lpszCommName = "COM3";
			EnableMenuItem(GetMenu(hwnd), IDM_CONNECT, MF_ENABLED);
			CloseHandle(hComm);
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			if (!CommConfigDialog(lpszCommName, hwnd, &cc))
				break;
			SetCommConfig(hComm, &cc, sizeof(COMMCONFIG));			
			break;
		case IDM_COM4:
			lpszCommName = "COM4";
			EnableMenuItem(GetMenu(hwnd), IDM_CONNECT, MF_ENABLED);
			CloseHandle(hComm);
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			if (!CommConfigDialog(lpszCommName, hwnd, &cc))
				break;
			SetCommConfig(hComm, &cc, sizeof(COMMCONFIG));			
			break;

		case IDM_EXIT_APP:
			MessageBox(NULL, "Exiting Application", "", MB_OK);
			PostQuitMessage(0);
			break;
		case IDM_CONNECT:
			
			//Create thread for reading
			create_thread_read(hComm, hwnd, buffer, nRead);			

			//Create thread for writing 
			create_thread_write(hComm, hwnd, buffer, nWrite);

			break;
		case IDM_HELP:
			const char* instructionMessage =
				"Steps to Use: \n\n"
				"1) Choose Settings -> Port \n"
				"2) Choose COM port to connect to \n"
				"3) Choose settings in pop-up dialog \n"
				"4) Click 'Connect' \n"
				"5) press <ESC> to return to command mode or 'EXIT' to close application";

			MessageBox(NULL, instructionMessage, "Usage Help", MB_ICONINFORMATION);
			break;
		}
		break;	

	case WM_CHAR:	// Process keystroke
		
		if (hComm == NULL) {
			OutputDebugString("hComm is null");
		}

		if (wParam == 27) {
			MessageBox(NULL, "Returning to Command Mode", "", MB_OK);
			CloseHandle(hComm);
			connected = false;
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			if (!CommConfigDialog(lpszCommName, hwnd, &cc))
				break;
			if (!SetCommConfig(hComm, &cc, sizeof(COMMCONFIG))) {
				OutputDebugString("this setting comm config failed");
			}
			EnableMenuItem(GetMenu(hwnd), IDM_CONNECT, MF_ENABLED);
			break;
		}
		else {
			/*sendMessagesSimple(hComm, wParam);*/
		}		
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}


