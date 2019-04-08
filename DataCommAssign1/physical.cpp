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

#include <queue>
#include <array>
#include "physical.h"

using namespace std; 

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


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

SOCKET AcceptSocket;
SOCKET sock;
SOCKADDR_IN InternetAddr;

// zeroed buffer for empty dQueue
BYTE* zeroBuffer; 

// 2 buffers to handle simultaneously reading from mic and sending data to socket
BYTE* waveInBuffer;
BYTE* waveInBuffer2; 


// 2 buffers to handle simultaneously playing audio data to speaker and reading data from socket
BYTE* waveOutBuffer;
BYTE* waveOutBuffer2; 

// Queue to hold input audio data from mic (parsed through by completion routine to send to socket)
typedef queue<BYTE *> socketDataQueue; 
socketDataQueue dQueue; 

// SET UP INPUT DEVICE 
// i/o input device variables
HWAVEIN hwi;
HWAVEOUT hwo; 

// WaveIn Headers (for the two buffers)
WAVEHDR      WaveInHdr;
WAVEHDR      WaveInHdr2;

WAVEINCAPS   wic;
WAVEFORMATEX wfx;  //input
WAVEFORMATEX wfx2; //output
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


DWORD WINAPI create_thread_write(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nRead) {
	threadStructReadPoint threader = new threadStructRead(hComm, hwnd, buffer, nRead);
	hThreadRead = CreateThread(NULL, 0, setupOutputDevice, (LPVOID)threader, 0, &readThreadId);
	if (!hThreadRead) {
		OutputDebugString("hThread for write wasn't initialized");
	}

	return 0;
}


DWORD WINAPI setupOutputDevice(LPVOID voider) {

	// Malloc memory for waveout buffers
	waveInBuffer = (BYTE*)malloc(DATA_BUFSIZE);
	waveInBuffer2 = (BYTE*)malloc(DATA_BUFSIZE);
	hwo = NULL; 

	// Waveformatex for output (same as input)
	wfx2.wFormatTag = WAVE_FORMAT_PCM;
	wfx2.nChannels = 1;
	wfx2.nSamplesPerSec = 20000;
	wfx2.wBitsPerSample = 8;
	wfx2.nAvgBytesPerSec = 20000;
	wfx2.nBlockAlign = 1;
	wfx2.cbSize = 0;

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


	// Malloc memory for wavein buffers
	waveInBuffer = (BYTE*)malloc(DATA_BUFSIZE);
	waveInBuffer2 = (BYTE*)malloc(DATA_BUFSIZE); 
	zeroBuffer = (BYTE*)malloc(DATA_BUFSIZE); 

	memset(zeroBuffer, 0, DATA_BUFSIZE); 

	MMRESULT     rc;

	// Set up waveformat for input (and output)
	hwi = NULL;

	wfx.wFormatTag =
		WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 20000;
	wfx.wBitsPerSample = 8;
	wfx.nAvgBytesPerSec = 20000;
	wfx.nBlockAlign = 1;
	wfx.cbSize = 0;

	

	// open waveform input device
	//...........................

	rc = waveInOpen(&hwi, WAVE_MAPPER, &wfx, (ULONG)waveInProc, (DWORD)hwnd,
		CALLBACK_FUNCTION);


	// device not found, error condition
	//..................................

	if (hwi == NULL)
	{
		OutputDebugString("device not found!\n");
		return(FALSE);
	}

	//Successfully found input device
	OutputDebugString("Found input device!\n");
	

	// Set up and prepare headers for input
	// First WAVEHDR header (with first wave in buffer)
	WaveInHdr.lpData = (LPSTR)waveInBuffer;
	WaveInHdr.dwBufferLength = DATA_BUFSIZE;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;


	WaveInHdr2.lpData = (LPSTR)waveInBuffer2;
	WaveInHdr2.dwBufferLength = DATA_BUFSIZE;
	WaveInHdr2.dwBytesRecorded = 0;
	WaveInHdr2.dwUser = 0L;
	WaveInHdr2.dwFlags = 0L;
	WaveInHdr2.dwLoops = 0L;


	// header preparation and buffer adding for wavein will occur during completion routine

	result = waveInPrepareHeader(hwi, &WaveInHdr, sizeof(WAVEHDR));

	if (result) {
		OutputDebugString("Error: cannot prepare wave in header 1!\n");
		return 1;
	}

	result = waveInPrepareHeader(hwi, &WaveInHdr2, sizeof(WAVEHDR));

	if (result) {
		OutputDebugString("Error: cannot prepare wave in header 2!\n");
		return 1;
	}

	// Insert a wave input buffers
	result = waveInAddBuffer(hwi, &WaveInHdr, sizeof(WAVEHDR));
	if (result)
	{
		OutputDebugString("Failed to add wave input buffer 1!\n");
		return 1;
	}

	result = waveInAddBuffer(hwi, &WaveInHdr2, sizeof(WAVEHDR)); 
	if (result)
	{
		OutputDebugString("Failed to add wave input buffer 2!\n");
		return 1;
	}

	result = waveInStart(hwi);



	if (result != 0) {
		OutputDebugString("Error starting input device!\n");
	}



	// Handle closing
	/*do {} while (waveInUnprepareHeader(hwi, &WaveInHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING); 

	waveInClose(hwi); */



/*	// Open output device
	result = waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, (DWORD)hwnd, CALLBACK_WINDOW); 

	if (result) {
		OutputDebugString("Error opening output device!\n");
	}

	result = waveOutPrepareHeader(hwo, &WaveInHdr, sizeof(WAVEHDR)); 

	if (result) {
		OutputDebugString("Error: cannot prepare wave out header!\n");
	}

	

	result = waveOutWrite(hwo, &WaveInHdr, sizeof(WAVEHDR)); 

	if (result) {
		OutputDebugString("Error: cannot play to output device!\n");
	}
*/


	// Set up Socket

	
	rc = setupSendSocket(); 

	if (rc != 0) {
		OutputDebugString("Error setting up socket!\n");
	}
	
	




	//return(0);
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

	/*
		1) Pop off next BYTE stream buffer to be sent to socket
		2) Copy it to socket struct buffer
		3) free BYTE stream buffer
	
	*/

	OutputDebugString("Reached completion routine\n");
	
	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	if (!dQueue.empty()) {
		memcpy(SI->DataBuf.buf, dQueue.front(), DATA_BUFSIZE);			// Copy next BYTE stream buffer to be sent to socket (from dQueue)
		dQueue.pop();													// Pop off BYTE stream buffer that was just copied (from dQueue)
		SI->DataBuf.len = DATA_BUFSIZE;									// Designated buffer size
	}
	else {																// if dQueue is empty
		OutputDebugString("dQueue is empty!\n");
		memcpy(SI->DataBuf.buf, zeroBuffer, DATA_BUFSIZE); 
		SI->DataBuf.len = DATA_BUFSIZE;
	}

	

	if (WSASendTo(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, (SOCKADDR *)&InternetAddr,
		sizeof(InternetAddr), &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSASend() failed with error %d\n", WSAGetLastError());
			return;
		}
		else {
			OutputDebugString("output pending (Completion Routine)!\n");
		}
	}
	else {
		OutputDebugString("sending data!\n");
	}
}


void CALLBACK waveInProc(
	HWAVEIN   hwi,
	UINT      uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2) {

	switch (uMsg) {
		case WIM_CLOSE:
		{
			// Exit! Do clean up operation
			OutputDebugString("input device closed");
			break;
		}			
		case WIM_DATA:
		{
			OutputDebugString("RECEIVED DATA FROM INPUT DEVICE!\n");
			/*
			Data being entered into input device
			 1) Retrieve full buffer and transfer data to new temp buffer
			 2) Append temp buffer to queue of buffers
			 3) Empty full buffer and add it back to input device wave header
			*/
			// 1)
			WAVEHDR completedWaveInHeader = *(WAVEHDR*)dwParam1;
			BYTE* inputSocketBuffer = (BYTE*)malloc(DATA_BUFSIZE);						// alloc for buffer to be sent to queue of buffers
			inputSocketBuffer = (BYTE*)completedWaveInHeader.lpData;					// temp buffer is filled with recently filled buffer

			// 2)
			dQueue.push(inputSocketBuffer);												// pushed temp buffer to queue of buffers to be sent to socket


			// 3)

			memset(completedWaveInHeader.lpData, 0, DATA_BUFSIZE); 						// clear filled buffer
			result = waveInAddBuffer(hwi, &completedWaveInHeader, sizeof(WAVEHDR));		// Add cleared buffer back to input device wave header
			if (result)
			{
				OutputDebugString("Failed to add wave in input buffer (from WIM_DATA)!\n");
				exit(1);
			}
			break;
		}
		case WIM_OPEN:
		{
			// Device is being opened
			OutputDebugString("Input Device found in waveInProc\n!");
			break;
		}
	}
}