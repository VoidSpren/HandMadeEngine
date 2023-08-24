#pragma once
#include <string>
#include <Windows.h>
#include "utilDefs.h"

class SerialPort {
	HANDLE COMHandle;
	bool connected;
	COMSTAT status;
	DWORD errors;
	std::string errorMsg;

public:
	SerialPort() {
		errors = 0;
		status = { 0 };
		connected = false;
	}
	~SerialPort() {
		if (connected == true) {
			connected = false;
			CloseHandle(COMHandle);
		}
	}

	void Connect(const char* portName, ui64 waitTime = 0) {
		errors = 0;
		status = { 0 };
		connected = false;

		COMHandle = CreateFileA(
			portName, GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
		);

		DWORD CreationError = GetLastError();
		if (CreationError == ERROR_FILE_NOT_FOUND) errorMsg = "|COM port not connected|";
		else if (CreationError == ERROR_ACCESS_DENIED) errorMsg = "|access to the COM port denied|";
		else if (CreationError == ERROR_SUCCESS) {
			errorMsg = "success";

			DCB DCBSerialParam = { 0 };

			if (!GetCommState(COMHandle, &DCBSerialParam))
				errorMsg.append("|failed to Get current serial parameters|");
			else {
				DCBSerialParam.BaudRate = CBR_115200;
				DCBSerialParam.ByteSize = 8;
				DCBSerialParam.StopBits = ONESTOPBIT;
				DCBSerialParam.Parity = NOPARITY;
				DCBSerialParam.fDtrControl = DTR_CONTROL_ENABLE;

				if (!SetCommState(COMHandle, &DCBSerialParam))
					errorMsg.append("|failed to Set current serial parameters|");
				else {
					connected = true;
					PurgeComm(COMHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
					Sleep(waitTime);
				}
			}
		}
	}

	void Disconnect() {
		if (connected == true) {
			connected = false;
			CloseHandle(COMHandle);
		}
	}

	int ReadSerialPort(char* buff, ui32 size) {
		if (connected) {
			DWORD bytesRead;
			ui32 toRead = 0;

			ClearCommError(COMHandle, &errors, &status);

			if (status.cbInQue > 0) {
				if (status.cbInQue > size) {
					toRead = size;
				}
				else
					toRead = status.cbInQue;
			}

			if (ReadFile(COMHandle, buff, toRead, &bytesRead, NULL)) return bytesRead;
		}

		return 0;
	}
	int WriteSerialPort(const char* buff, ui32 size) {

	}
	bool isConnected() { return connected; }
	std::string ErrorMsg() { return errorMsg; }
};
