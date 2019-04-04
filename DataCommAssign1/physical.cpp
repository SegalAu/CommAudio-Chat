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
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "physical.h"




static unsigned k = 0;
HANDLE hThreadRead;
DWORD readThreadId;

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;


typedef struct INPUT_DEVICE_INFO {

} input_device_info;


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);



SOCKET AcceptSocket;
SOCKET sock;
SOCKADDR_IN InternetAddr;

//buffer
BYTE* waveInBuffer;

// SET UP INPUT DEVICE 

// Wave in input device variables
HWAVEIN hwi;
WAVEHDR      WaveInHdr;

WAVEINCAPS   wic;
WAVEFORMATEX wfx;
UINT         nDevId;
UINT         nMaxDevices = waveInGetNumDevs();

MMRESULT result = 0;


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

	waveInBuffer = (BYTE*)malloc(DATA_BUFSIZE);
	
	
	//HWAVEIN hwi;
	//WAVEHDR      WaveInHdr;

	//WAVEINCAPS   wic;
	//WAVEFORMATEX wfx;
	//UINT         nDevId;
	//MMRESULT     rc;
	//UINT         nMaxDevices = waveInGetNumDevs();

	//MMRESULT result = 0; 

	//hwi = NULL;
	////nPlaybackBufferPos = 0L;  // position in playback buffer
	////nPlaybackBufferLen = 0L;  // total data in playback buffer
	////eStatus = StatusOkay;     // reset status

	//for (nDevId = 0; nDevId < nMaxDevices; nDevId++)
	//{
	//	rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
	//	if (rc == MMSYSERR_NOERROR)
	//	{
	//		// attempt 44.1 kHz stereo if device is capable

	//		if (wic.dwFormats & WAVE_FORMAT_4S16)
	//		{
	//			wfx.nChannels = 2;      // stereo
	//			wfx.nSamplesPerSec = 44100;  // 44.1 kHz (44.1 * 1000)
	//		}
	//		else
	//		{
	//			wfx.nChannels = wic.wChannels;  // use DevCaps # channels
	//			wfx.nSamplesPerSec = 22050;  // 22.05 kHz (22.05 * 1000)
	//		}

	//		wfx.wFormatTag = WAVE_FORMAT_PCM;
	//		wfx.wBitsPerSample = 8;
	//		wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	//		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	//		wfx.cbSize = 0;

	//		// open waveform input device
	//		//...........................

	//		rc = waveInOpen(&hwi, nDevId, &wfx, 0l, (DWORD)hwnd,
	//			WAVE_FORMAT_DIRECT);

	//		if (rc == MMSYSERR_NOERROR)
	//			break;
	//		else
	//		{
	//			/*waveInGetErrorText(rc, msg, MSG_LEN),
	//				MessageBox(hwnd, msg, NULL, MB_OK);*/
	//			OutputDebugString("can't open input device!\n");
	//			return(FALSE);
	//		}



	//	}
	//}

	//// device not found, error condition
	////..................................

	//if (hwi == NULL)
	//{
	//	OutputDebugString("device not found!\n");
	//	return(FALSE);
	//}

	////Successfully found input device
	//OutputDebugString("Found input device!\n");

	MMRESULT     rc;

	// Set up input device
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
		OutputDebugString("device not found!\n");
		return(FALSE);
	}

	//Successfully found input device
	OutputDebugString("Found input device!\n");
	



	// Set up Socket

	rc = setupSendSocket(); 

	if (rc != 0) {
		OutputDebugString("Error setting up socket!\n");
	}



	return(0);
}



int setupSendSocket() {
	OutputDebugString("Reached Set up Socket!\n");


	int i, nBufSize, err, Ret;
	HANDLE ThreadHandle;
	DWORD ThreadId;
	
	char *buf, *buf_ptr;
	/*struct sockaddr_in sin;*/

	WSADATA stWSAData;
	WSAEVENT AcceptEvent;
	WORD wVersionRequested = MAKEWORD(2, 2);



	int portNum;
	char * ipAddr; 

	//HARDCODED VALUES
	nBufSize = DATA_BUFSIZE;
	portNum = 7000; 
	ipAddr = "127.0.0.1";


	buf = (char*)malloc(nBufSize);

	for (i = 0; i < nBufSize; i++)
		buf[i] = 'a';

	

	// Initialize the DLL with version Winsock 2.2
	/*if (WSAStartup(wVersionRequested, &stWSAData) == 0){
		OutputDebugString("WSAStartup failed!\n");
		WSACleanup(); 
		return 1;
	}*/
	if ((Ret = WSAStartup(0x0202, &stWSAData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return 1;
	}

	// UDP Socket
	if ((sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		OutputDebugString("Failed to get socket!\n");
		exit(2);
	}
		

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = inet_addr(ipAddr);
	InternetAddr.sin_port = htons(PORT);				//7000


	if (bind(sock, (PSOCKADDR)&InternetAddr,
		sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		
		OutputDebugString("bind() failed with error %d\n");
		/*printf(, WSAGetLastError());*/
		return 1;
	}

	/*if (listen(sock, 2) == SOCKET_ERROR)
	{
		char msgBuffery[20];
		OutputDebugString(_itoa(WSAGetLastError(),msgBuffery, 10));
		OutputDebugString("\n listen failed with error!\n");
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 2;
	}*/
	
	OutputDebugString("IP Address & Socket Okay\n");
	printf("Socket is %d\n", sock);
	OutputDebugString("We have ignition!\n");


	// Worker thread to service completed I/O Requests


	if ((ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID)sock, 0, &ThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 1;
	}

	OutputDebugString("setupSendSocket thread func ended!\n");
	return 0; 

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


DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT EventArray[1];
	DWORD Index;
	DWORD RecvBytes;
	WSAEVENT dummyEvent;

	if ((dummyEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
		OutputDebugString("ERROR: INVALID EVENT!");
	}

	// Save the accept event in the event array.

	EventArray[0] = dummyEvent;
	WSASetEvent(EventArray[0]); 

	// Fill in the details of our accepted socket.		

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		OutputDebugString("global alloc failed!\n");
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}




	// Set up and prepare header for input
	WaveInHdr.lpData = (LPSTR)waveInBuffer;
	WaveInHdr.dwBufferLength = DATA_BUFSIZE;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;
	result = waveInPrepareHeader(hwi, &WaveInHdr, sizeof(WAVEHDR));

	if (result) {
		OutputDebugString("Error: cannot prepare wave in header!\n");
		return 1;
	}

	// Insert a wave input buffer
	result = waveInAddBuffer(hwi, &WaveInHdr, sizeof(WAVEHDR));
	if (result)
	{
		OutputDebugString("Failed to add wave input buffer!\n");
		return 1;
	}

	result = waveInStart(hwi);



	if (result != 0) {
		OutputDebugString("Error starting input device!\n");
	}
	

	
		
		


	while (TRUE)
	{
		
			

		OutputDebugString("checkpoint B!\n");


		Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

		if (Index == WSA_WAIT_FAILED)
		{
			OutputDebugString("WSA wait failed!\n");
			printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
			return FALSE;
		}

		else if (Index == WAIT_IO_COMPLETION) {
			OutputDebugString("index = wait io completion \n");
		}
		else if (Index == 0) {
			//reset dummy event
			WSAResetEvent(EventArray[0]);
		}
		else {
			OutputDebugString("COULD NOT RESET EVENT!\n");
		}

		//if (Index != WAIT_IO_COMPLETION)
		//{				
		//	// An accept() call event is ready - break the wait loop

		//}




		SocketInfo->Socket = sock;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->BytesSEND = 0;
		SocketInfo->BytesRECV = 0;

		/*memcpy(SocketInfo->DataBuf.buf, waveInBuffer, DATA_BUFSIZE); */

		SocketInfo->DataBuf.len = DATA_BUFSIZE;
		
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;
	

		Flags = 0;


		if (WSASendTo(SocketInfo->Socket,
			&(SocketInfo->DataBuf),
			1,
			&RecvBytes,
			Flags,
			(SOCKADDR *)&InternetAddr,
			sizeof(InternetAddr),
			&(SocketInfo->Overlapped),
			WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				char msgBuffy[20];
				OutputDebugString("WSASendTo Error: ");
				OutputDebugString(_itoa(WSAGetLastError(), msgBuffy, 10));
				OutputDebugString("\n");
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return FALSE;
			}
		}
		
		
		//clear waveInBuffer
		memset(waveInBuffer, 0, DATA_BUFSIZE);

		//// Insert a wave input buffer
		//result = waveInAddBuffer(hwi, &WaveInHdr, sizeof(WAVEHDR));
		//if (result)
		//{
		//	OutputDebugString("Failed to add wave input buffer!\n");
		//	return 1;
		//}
		//
		
		
	}
	

	return TRUE;
}



// COMPLETION ROUTINE 



void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	if (BytesTransferred == 0)
	{
		printf("Closing socket %d\n", SI->Socket);
	}

	if (Error != 0 || BytesTransferred == 0)
	{
		closesocket(SI->Socket);
		GlobalFree(SI);
		return;
	}

	// Check to see if the BytesRECV field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the BytesRECV field
	// with the BytesTransferred value from the completed WSARecv() call.

	//if (SI->BytesRECV == 0)
	//{
	//	SI->BytesRECV = BytesTransferred;
	//	SI->BytesSEND = 0;
	//}
	//else
	//{
	//	SI->BytesSEND += BytesTransferred;
	//}


	// TODO: TAKE INPUT AUDIO DATA FROM MIC AND FILL BUFFER

	OutputDebugString("Reached completion routine\n");



	// Take buffer with audio data and send to socket

	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	memcpy(SI->DataBuf.buf, waveInAddBuffer, DATA_BUFSIZE);
	SI->DataBuf.len = DATA_BUFSIZE;

	if (WSASendTo(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, (SOCKADDR *)&InternetAddr,
		sizeof(InternetAddr), &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSASend() failed with error %d\n", WSAGetLastError());
			return;
		}
	}


	
}