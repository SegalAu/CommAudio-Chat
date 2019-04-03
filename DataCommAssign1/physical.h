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

DWORD WINAPI setupInputDevice(LPVOID voider);
DWORD WINAPI create_thread_read(HANDLE hComm, HWND hwnd, char buffer[1], LPDWORD nHandle);
static DWORD WINAPI displayMessage(HWND hwnd, char buffer);
VOID sendMessagesSimple(HANDLE hComm, WPARAM wParam);


int setupSendSocket();
