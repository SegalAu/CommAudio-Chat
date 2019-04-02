/*---------------------------------------------------------------------------------------------------
-- SOURCE FILE: physical.cpp 
--
-- PROGRAM :    DUMB TERMINAL
-- 
-- FUNCTIONS:
-- DWORD WINAPI receiveMessages(LPVOID voider);
-- DWORD WINAPI create_thread_read(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nHandle);
-- static DWORD WINAPI displayMessage(HWND hwnd, char buffer);
-- VOID sendMessagesSimple(HANDLE hComm, WPARAM wParam);
--
-- DATE: October 3, 2018
--
-- DESIGNER: Segal Au, A01000835
--
-- PROGRAMMER: Segal Au, A01000835
--
-- NOTES:
-- Handles creation of thread for reading input from serial port, as well as writing to serial port from
-- application. 
---------------------------------------------------------------------------------------------------*/


#include <windows.h>
#include <stdio.h>
#include "physical.h"

static unsigned k = 0;
HANDLE hThreadRead;
DWORD readThreadId;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       create_thread_read
--
-- INTERFACE :	   DWORD WINAPI create_thread_read(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nRead)
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Creates thread to read inputs from serial port.
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI create_thread_read(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nRead) {
	threadStructReadPoint threader = new threadStructRead(hComm, hwnd, buffer, nRead); 
	hThreadRead = CreateThread(NULL, 0, setupInputDevice, (LPVOID)threader, 0, &readThreadId);
	if (!hThreadRead) {
		OutputDebugString("hThread for read wasn't initialized");
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       receiveMessages
--
-- INTERFACE :	   DWORD WINAPI receiveMessages(LPVOID voider)
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Continuously reads input from serial port and sends inputs received to display (char by char). 
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI setupInputDevice(LPVOID voider) {
	
	threadStructReadPoint threader = (threadStructReadPoint)voider;
	HANDLE hComm = (HANDLE)threader->hComm;
	const HWND hwnd = (HWND)threader->hwnd;
	/*char buffer[1] = threader->buffer;*/
	char buffer;
	DWORD nRead;

	
	
	HWAVEIN hwi;
	WAVEINCAPS   wic;
	WAVEFORMATEX wfx;
	UINT         nDevId;
	MMRESULT     rc;
	UINT         nMaxDevices = waveInGetNumDevs();

	hwi = NULL;
	//nPlaybackBufferPos = 0L;  // position in playback buffer
	//nPlaybackBufferLen = 0L;  // total data in playback buffer
	//eStatus = StatusOkay;     // reset status

	for (nDevId = 0; nDevId < nMaxDevices; nDevId++)
	{
		rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
		if (rc == MMSYSERR_NOERROR)
		{
			// attempt 44.1 kHz stereo if device is capable

			if (wic.dwFormats & WAVE_FORMAT_4S16)
			{
				wfx.nChannels = 2;      // stereo
				wfx.nSamplesPerSec = 44100;  // 44.1 kHz (44.1 * 1000)
			}
			else
			{
				wfx.nChannels = wic.wChannels;  // use DevCaps # channels
				wfx.nSamplesPerSec = 22050;  // 22.05 kHz (22.05 * 1000)
			}

			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.wBitsPerSample = 8;
			wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			wfx.cbSize = 0;

			// open waveform input device
			//...........................

			rc = waveInOpen(&hwi, nDevId, &wfx, 0l, (DWORD)hwnd,
				WAVE_FORMAT_DIRECT);

			if (rc == MMSYSERR_NOERROR)
				break;
			else
			{
				/*waveInGetErrorText(rc, msg, MSG_LEN),
					MessageBox(hwnd, msg, NULL, MB_OK);*/
				OutputDebugString("can't open input device!\n");
				return(FALSE);
			}



		}
	}

	// device not found, error condition
	//..................................

	if (hwi == NULL)
	{
		OutputDebugString("hwi is null!\n");
		return(FALSE);
	}

	//Successfully found input device
	OutputDebugString("Found input device!\n");

	return(TRUE);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       displayMessage
--
-- INTERFACE :	   static DWORD WINAPI displayMessage(HWND hwnd, char buffer)
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Displays character from buffer to window of application.
----------------------------------------------------------------------------------------------------------------------*/


static DWORD WINAPI displayMessage(HWND hwnd, char buffer) {

	HDC hdc;
	PAINTSTRUCT paintstruct;
	char str[80] = "";

	hdc = GetDC(hwnd);		 // get device context
	sprintf_s(str, "%c", (char)buffer); // Convert char to string
	TextOut(hdc, 13 * k, 0, str, strlen(str)); // output character	
	k++; // increment the screen x-coordinate
	ReleaseDC(hwnd, hdc); // Release device context
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:       sendMessagesSimple
--
-- INTERFACE :	   VOID sendMessagesSimple(HANDLE hComm, WPARAM wParam)
--
-- DATE:           October 3rd, 2018
--
-- DESIGNER:       Segal Au, A01000835
--
-- PROGRAMMER:     Segal Au, A01000835
--
-- NOTES:          Handles writing to serial port. Run when character is inputted into application.
----------------------------------------------------------------------------------------------------------------------*/


VOID sendMessagesSimple(HANDLE hComm, WPARAM wParam) {
	char str[80];
	LPDWORD nWrite = 0; 
	char charToSend = wParam;

	if (!WriteFile(hComm, &charToSend, 1, nWrite, NULL)) {
		OutputDebugString("Write file failed!");
	}
}
