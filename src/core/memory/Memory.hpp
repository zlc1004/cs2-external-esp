#ifndef _PPROCESS_HPP_
#define _PPROCESS_HPP_

#include <vector>
#include <math.h>
#include <string>
#include <iostream>
#include <cstdint> // For uintptr_t and other standard integer types

#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h> 

typedef NTSTATUS(WINAPI* pNtReadVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesRead);
typedef NTSTATUS(WINAPI* pNtWriteVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten);

class pMemory {

public:
	pMemory() {
		pfnNtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadVirtualMemory");
		pfnNtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWriteVirtualMemory");
	}

	pNtReadVirtualMemory pfnNtReadVirtualMemory;
	pNtWriteVirtualMemory pfnNtWriteVirtualMemory;
};
#else
// Dummy definitions for non-Windows compilation
// These will allow the code to compile on macOS but will not provide actual functionality
#include <cstring> // For memset

using HANDLE = void*;
using HWND = void*;
using DWORD = unsigned long;
using ULONG = unsigned long;
using PULONG = unsigned long*;
using PVOID = void*;
using LPVOID = void*; // Add LPVOID
using SIZE_T = size_t; // Add SIZE_T
using NTSTATUS = int;

// Dummy pMemory class
class pMemory {
public:
    pMemory() {}
    // Dummy functions for compilation
    NTSTATUS pfnNtReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesRead) { return 0; }
    NTSTATUS pfnNtWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten) { return 0; }
};

#endif

struct ProcessModule
{
	uintptr_t base, size;
};

class pProcess
{
public:
	uint32_t	  pid_; // process id
	HANDLE		  handle_; // handle to process
	HWND		  hwnd_; // window handle
	ProcessModule base_client_;

public:
	bool AttachProcess(const char* process_name);
	bool AttachWindow(const char* window_name);
	bool UpdateHWND();
	void Close();

public:
	ProcessModule GetModule(const char* module_name);
	LPVOID		  Allocate(size_t size_in_bytes);
	uintptr_t	  FindCodeCave(uint32_t length_in_bytes);
	uintptr_t     FindSignature(std::vector<uint8_t> signature);
	uintptr_t     FindSignature(ProcessModule target_module, std::vector<uint8_t> signature);

	template<class T>
	uintptr_t ReadOffsetFromSignature(std::vector<uint8_t> signature, uint8_t offset) // offset example: "FF 05 ->22628B01<-" offset is 2
	{
		uintptr_t pattern_address = this->FindSignature(signature);
		if (!pattern_address)
			return 0x0;

		T offset_value = this->read<T>(pattern_address + offset);
		return pattern_address + offset_value + offset + sizeof(T);
	}

	bool read_raw(uintptr_t address, void* buffer, size_t size)
	{
		SIZE_T bytesRead;
		pMemory cMemory;

#ifdef _WIN32
		NTSTATUS status = cMemory.pfnNtReadVirtualMemory(this->handle_, (PVOID)(address), buffer, static_cast<ULONG>(size), (PULONG)&bytesRead);
#else
        // Dummy read for non-Windows compilation
        bytesRead = size;
        // Potentially fill buffer with dummy data or zeros
        memset(buffer, 0, size);
        NTSTATUS status = 0; // STATUS_SUCCESS
#endif
		return status == 0x00000000/*STATUS_SUCCESS*/ || bytesRead == size;
	}

	template<class T>
	void write(uintptr_t address, T value)
	{
		pMemory cMemory;
#ifdef _WIN32
		cMemory.pfnNtWriteVirtualMemory(handle_, (void*)address, &value, sizeof(T), 0);
#endif
	}

	template<class T>
	T read(uintptr_t address)
	{
		T buffer{};
		pMemory cMemory;

#ifdef _WIN32
		cMemory.pfnNtReadVirtualMemory(handle_, (void*)address, &buffer, sizeof(T), 0);
#endif
		return buffer;
	}

	void write_bytes(uintptr_t addr, std::vector<uint8_t> patch)
	{
		pMemory cMemory;
#ifdef _WIN32
		cMemory.pfnNtWriteVirtualMemory(handle_, (void*)addr, &patch[0], patch.size(), 0);
#endif
	}

	uintptr_t read_multi_address(uintptr_t ptr, std::vector<uintptr_t> offsets)
	{
		uintptr_t buffer = ptr;
		for (int i = 0; i < offsets.size(); i++)
			buffer = this->read<uintptr_t>(buffer + offsets[i]);

		return buffer;
	}

	template <typename T>
	T read_multi(uintptr_t base, std::vector<uintptr_t> offsets) {
		uintptr_t buffer = base;
		for (int i = 0; i < offsets.size() - 1; i++)
		{
			buffer = this->read<uintptr_t>(buffer + offsets[i]);
		}
		return this->read<T>(buffer + offsets.back());
	}

private:
	uint32_t FindProcessIdByProcessName(const char* process_name);
	uint32_t FindProcessIdByWindowName(const char* window_name);
	HWND GetWindowHandleFromProcessId(DWORD ProcessId);
};
#endif