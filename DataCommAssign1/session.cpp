/*---------------------------------------------------------------------------------------------------
-- SOURCE FILE: session.cpp
--
-- PROGRAM :    DUMB TERMINAL
--
-- FUNCTIONS:
-- BOOL connectSerialPort(HANDLE* hComm, LPCSTR lpszCommName)
--
-- DATE: October 3, 2018
--
-- DESIGNER: Segal Au, A01000835
--
-- PROGRAMMER: Segal Au, A01000835
--
-- NOTES:
-- Handles initialization and opening of serial port.
---------------------------------------------------------------------------------------------------*/

#include <windows.h>
#include "session.h"


int bytesBuffer = 16;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       connectSerialPort
--
-- INTERFACE :	   BOOL connectSerialPort(HANDLE* hComm, LPCSTR lpszCommName)
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Handles initialization and opening of serial port. 
----------------------------------------------------------------------------------------------------------------------*/

BOOL connectSerialPort(HANDLE* hComm, LPCSTR lpszCommName) {
	if ((*hComm = CreateFile(lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
				NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Error opening COM port:", "", MB_OK);
		return FALSE;
	}
	else {
		SetupComm(hComm, bytesBuffer, bytesBuffer);
		return TRUE;
	}
}
