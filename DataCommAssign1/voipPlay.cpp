#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <queue>
#include <array>

#include "voipPlay.h"

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

SOCKET ListenSocket;
HANDLE MY_WOM_DONE;
HANDLE ADDED_TO_QUEUE;
//std::queue<std::array<char, AUDIO_BUFFER_SIZE>> testQueue;
std::queue<BYTE *> testQueue;
bool waitFirst8;



DWORD WINAPI setupOutputDevice(LPVOID voider) {

	OutputDebugString("Entered setupOutputDevice!\n");

	char        szFileName[128];    // filename of file to open 
	HMMIO       hmmio;              // file handle for open file 
	MMCKINFO    mmckinfoParent;     // parent chunk information 
	MMCKINFO    mmckinfoSubchunk;   // subchunk information structure 
	DWORD       dwFmtSize;          // size of "FMT" chunk 
	DWORD       dwDataSize;         // size of "DATA" chunk 
	LPWAVEFORMATEX  pFormat;           // address of "FMT" chunk 
	HPSTR       lpData;             // address of "DATA" chunk 

	WAVEFORMATEX wfx;  //input


	HWAVEOUT outputDevice;
	DWORD_PTR callbackInstance;
	WAVEHDR waveHdr;
	HANDLE dummyEvent;

	int bytesRead;
	int totalBytesRead;
	int bytesReadPerSecond;

	WAVEHDR waveHeaderArr[WAVEHDR_ARR_SIZE];
	for (int i = 0; i < WAVEHDR_ARR_SIZE; i++) {
		waveHeaderArr[i].dwFlags = WHDR_DONE;
		waveHeaderArr[i].lpData = (LPSTR)malloc(AUDIO_BUFFER_SIZE);
	}

	dummyEvent = CreateEvent(NULL, true, false, "dummyEvent");
	MY_WOM_DONE = WSACreateEvent();
	ADDED_TO_QUEUE = WSACreateEvent();
	HANDLE mainEvents[2];
	mainEvents[0] = ADDED_TO_QUEUE;
	mainEvents[1] = MY_WOM_DONE;

	/*char buffer[AUDIO_BUFFER_SIZE];
	int numBuffersPerSecond = (bytesReadPerSecond + AUDIO_BUFFER_SIZE - 1) / AUDIO_BUFFER_SIZE;
	printf("numbufferpersec: %d\n", numBuffersPerSecond);
	char* bufferOneSec = (char*)malloc(AUDIO_BUFFER_SIZE * numBuffersPerSecond);*/


	//load up queue
	bytesRead = 0;
	totalBytesRead = 0;

	//}

	// Open a waveform-audio output device. 
	//if (waveOutOpen(&outputDevice, WAVE_MAPPER, pFormat, (ULONG)waveOutProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
	//	fprintf(stderr, "failed to open output device");
	//}

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 8;
	wfx.nAvgBytesPerSec = 44100;
	wfx.nBlockAlign = 1;
	wfx.cbSize = 0;
	pFormat = &wfx;
	if (waveOutOpen(&outputDevice, WAVE_MAPPER, pFormat, (ULONG)waveOutProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		fprintf(stderr, "failed to open output device");
		return 0;
	}
	else {
		OutputDebugString("Opened output device!\n");
	}

	//initialize and fill up first 8 buffers
	//then start playing 
	char outBuffers[WAVEHDR_ARR_SIZE][AUDIO_BUFFER_SIZE];
	//for (int i = 0; i < WAVEHDR_ARR_SIZE; i++) {
	//	waveHeaderArr[i].dwBufferLength = AUDIO_BUFFER_SIZE;
	//	//waveHeaderArr[i].lpData = testQueue.front().data();
	//	waveHeaderArr[i].lpData = outBuffers[i];
	//	cArrToArr(&(testQueue.front()), outBuffers[i]);
	//	testQueue.pop();
	//	waveHeaderArr[i].dwLoops = 0;
	//	waveHeaderArr[i].dwFlags = 0;
	//}

	//for (int i = 0; i < WAVEHDR_ARR_SIZE; i++) {
	//	waveOutPrepareHeader(outputDevice, &waveHeaderArr[i], sizeof(waveHeaderArr[i]));
	//	waveOutWrite(outputDevice, &waveHeaderArr[i], sizeof(waveHeaderArr[i]));
	//}

	WSADATA wsaData;
	SOCKADDR_IN InternetAddr;
	INT Ret;
	HANDLE ThreadHandle;
	DWORD ThreadId;

	//initialize socket lib
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return 0;
	}

	if ((ListenSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 0;
	}

	//Set params for internet connection 
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	//bind socket to a port
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr,
		sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 0;
	}

	OutputDebugString("Socket Opening / Setup okay!\n");

	// Create a worker thread to service completed I/O requests. 
	// Pass in thread functuon Worker thread with the created event object as the parameter
	if ((ThreadHandle = CreateThread(NULL, 0, WorkerThreadOutput, (LPVOID)ListenSocket, 0, &ThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 0;
	}



	int waitResult;
	while (1) {
		waitResult = WSAWaitForMultipleEvents(2, mainEvents, false, WSA_INFINITE, true);
		if (waitResult == WAIT_OBJECT_0 || waitResult == 1) {
			printf("wait result = %d\n, waitResult");
			for (int i = 0; i < WAVEHDR_ARR_SIZE; i++) {
				if (waveHeaderArr[i].dwFlags & WHDR_DONE) {
					//waveOutUnprepareHeader(outputDevice, &waveHeaderArr[i], sizeof(waveHeaderArr[i]));
					waveHeaderArr[i].dwBufferLength = AUDIO_BUFFER_SIZE;
					//waveHeaderArr[i].lpData = testQueue.front().data();
					if (testQueue.size() > 0) {
						//cArrToArr(&(testQueue.front()), outBuffers[i]);
						memcpy(waveHeaderArr[i].lpData, testQueue.front(), AUDIO_BUFFER_SIZE);
						//waveHeaderArr[i].lpData = LPSTR(outBuffers[i]);
						testQueue.pop();
						waveHeaderArr[i].dwLoops = 0;
						waveHeaderArr[i].dwFlags = 0;
						waveOutPrepareHeader(outputDevice, &waveHeaderArr[i], sizeof(waveHeaderArr[i]));
						waveOutWrite(outputDevice, &waveHeaderArr[i], sizeof(waveHeaderArr[i]));
						break;
					}
				}
			}
		}
	}
	return 0;

}


void arrToCArr(char* src, std::array<char, AUDIO_BUFFER_SIZE>* dest) {
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
		(*dest)[i] = src[i];
	}
}

void cArrToArr(std::array<char, AUDIO_BUFFER_SIZE>* src, char* dest) {
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
		dest[i] = (*src)[i];
	}
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	switch (uMsg)
	{
	case WOM_OPEN:
		break;

	case WOM_DONE:
		SetEvent(MY_WOM_DONE);
		break;

	case WOM_CLOSE:
		break;
	}
}

DWORD WINAPI WorkerThreadOutput(LPVOID lpParameter)
{
	printf("STARTED worker thread\n");
	LPSOCKET_INFORMATION SocketInfo;
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, //what is global alloc?
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return 0;
	}
	DWORD Flags;
	DWORD RecvBytes;
	DWORD Index = 0;
	WSAEVENT eventArr[2];
	WSAEVENT workerThreadDummyEvent;
	struct sockaddr_in fromSock;
	INT fromSockLen = sizeof(fromSock);
	DWORD bytesSent;

	//SOCKET* ListenSocket = (SOCKET*)lpParameter;

	//create a new event object
	if ((workerThreadDummyEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 0;
	}
	eventArr[0] = workerThreadDummyEvent;
	//eventArr[1] = ADDED_TO_QUEUE;
	WSASetEvent(eventArr[0]);

	while (TRUE)
	{
		//return when an event in the event array is signaled, or an IOcompletionRoutine returns
		//last parameter set true so that completion routines can be run.
		//return WSA_WAIT_EVENT_0 (or 1 or whatever) when event at that index is signaled
		//return WSA_WAIT_IO_COMPLETION when completionRoutine finishes

		Index = WSAWaitForMultipleEvents(1, eventArr, FALSE, WSA_INFINITE, TRUE);

		if (Index == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
			//return 0;
		}

		if (Index == 0) {
			printf("dummyevebt\n");
			WSAResetEvent(eventArr[Index - WSA_WAIT_EVENT_0]);

			SocketInfo->Socket = ListenSocket;
			ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
			SocketInfo->BytesSEND = 0;
			SocketInfo->BytesRECV = 0;
			SocketInfo->DataBuf.len = DATA_BUFSIZE;
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;
			Flags = 0;

			//call WSARecv with the workerRoutine
			//Here we are just using the overlapped parameter to instead store our SOCKET_INFORMATION struct
			if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR *)&fromSock, &fromSockLen,
				&(SocketInfo->Overlapped), WorkerRoutineOutput) == SOCKET_ERROR)
			{
				//ignore when error = WSA_IO_PENDING
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("LINE HERE WSARecv() failed with error %d\n", WSAGetLastError());
					//return 0;
				}
			}

		}

		if (Index == WAIT_IO_COMPLETION) {
			SocketInfo->Socket = ListenSocket;
			ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
			SocketInfo->BytesSEND = 0;
			SocketInfo->BytesRECV = 0;
			SocketInfo->DataBuf.len = DATA_BUFSIZE;
			//SocketInfo->DataBuf.buf = SocketInfo->Buffer;
			Flags = 0;

			//call WSARecv with the workerRoutine
			//Here we are just using the overlapped parameter to instead store our SOCKET_INFORMATION struct
			if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, (struct sockaddr *)&fromSock, &fromSockLen,
				&(SocketInfo->Overlapped), WorkerRoutineOutput) == SOCKET_ERROR)
			{
				//ignore when error = WSA_IO_PENDING
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("LINE HERE WSARecv() failed with error %d\n", WSAGetLastError());
					//return 0;
				}
			}

		}



	}

	return TRUE;
}

void CALLBACK WorkerRoutineOutput(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	printf("ENTERED WORKER ROUTINE");
	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	printf("DATA RECEIVED: %s\n", SI->Buffer);

	BYTE tempArr[AUDIO_BUFFER_SIZE]; //so the queue can hold it
	//sender should send packets of length AUDIO_BUFFER_SIZE
	//arrToCArr(SI->Buffer, &tempArr);
	memcpy(tempArr, SI->Buffer, AUDIO_BUFFER_SIZE);
	testQueue.push(tempArr);
	SetEvent(ADDED_TO_QUEUE);
}
