#pragma once




#define WAVEHDR_ARR_SIZE 16
#define AUDIO_BUFFER_SIZE 4000
#define RIFF_HDR_SIZE 44
#define PORT 6500						// sent from BENS COMP
#define DATA_BUFSIZE 4000




void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam);
void arrToCArr(char*, std::array<char, AUDIO_BUFFER_SIZE>*);
void cArrToArr(std::array<char, AUDIO_BUFFER_SIZE>*, char*);
void CALLBACK WorkerRoutineOutput(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI WorkerThreadOutput(LPVOID lpParameter);
DWORD WINAPI setupOutputDevice(LPVOID voider);