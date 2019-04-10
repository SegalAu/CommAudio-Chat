#define PORT 7000
#define DATA_BUFSIZE 4000
#define NUMBUFS 16


typedef struct threadStructWrite {
	HANDLE hComm;
	HWND hwnd;
	char* buffer;
	LPDWORD nHandle;
	WPARAM wParam;
	threadStructWrite(HANDLE hComm, HWND hwnd, char* buffer, LPDWORD nHandle, WPARAM wParam) : hComm(hComm), hwnd(hwnd), buffer(buffer), nHandle(nHandle), wParam(wParam){}
} *threadStructWritePoint;

typedef struct threadStructRead {
	HANDLE hComm;
	HWND hwnd;
	char* buffer;
	LPDWORD nHandle;
	threadStructRead(HANDLE hComm, HWND hwnd, char* buffer, LPDWORD nHandle) : hComm(hComm), hwnd(hwnd), buffer(buffer), nHandle(nHandle) {};
} *threadStructReadPoint;



DWORD WINAPI WorkerThread(LPVOID lpParameter);

int setupSendSocket();

//Creating threads
DWORD WINAPI create_thread_read(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nHandle);
DWORD WINAPI create_thread_write(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nRead);

// Thread Functions
DWORD WINAPI setupInputDevice(LPVOID voider);


void CALLBACK waveInProc(
	HWAVEIN   hwi,
	UINT      uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
);


