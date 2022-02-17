// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "Memoryapi.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>

#undef PROCESSENTRY32
#undef Process32Next

Memory::Memory(const std::string& processName) {
	// First, get the handle of the process
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	while (Process32Next(snapshot, &entry)) {
		if (processName == entry.szExeFile) {
			_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
			break;
		}
	}
	if (!_handle) {
		MessageBox(GetActiveWindow(), L"Process not found in RAM. Please open The Witness and then try again.", NULL, MB_OK);
		throw std::exception("Unable to find process!");
	}

	// Next, get the process base address
	DWORD numModules;
	std::vector<HMODULE> moduleList(1024);
	EnumProcessModulesEx(_handle, &moduleList[0], static_cast<DWORD>(moduleList.size()), &numModules, 3);

	std::string name(64, '\0');
	for (DWORD i = 0; i < numModules / sizeof(HMODULE); i++) {
		int length = GetModuleBaseNameA(_handle, moduleList[i], &name[0], static_cast<DWORD>(name.size()));
		name.resize(length);
		if (processName == name) {
			_baseAddress = (uintptr_t)moduleList[i];
			break;
		}
	}
	if (_baseAddress == 0) {
		throw std::exception("Couldn't find the base process address!");
	}
}

Memory::~Memory() {
	CloseHandle(_handle);
}

// Copied from Witness Trainer https://github.com/jbzdarkid/witness-trainer/blob/master/Source/Memory.cpp#L218
int find(const std::vector<byte> &data, const std::vector<byte> &search) {
	const byte* dataBegin = &data[0];
	const byte* searchBegin = &search[0];
	size_t maxI = data.size() - search.size();
	size_t maxJ = search.size();

	for (int i=0; i<maxI; i++) {
		bool match = true;
		for (size_t j=0; j<maxJ; j++) {
			if (*(dataBegin + i + j) == *(searchBegin + j)) {
				continue;
			}
			match = false;
			break;
		}
		if (match) return i;
	}
	return -1;
}

int Memory::findGlobals() {
	const std::vector<byte> scanBytes = {0x74, 0x41, 0x48, 0x85, 0xC0, 0x74, 0x04, 0x48, 0x8B, 0x48, 0x10};
	#define BUFFER_SIZE 0x10000 // 10 KB
	std::vector<byte> buff;
	buff.resize(BUFFER_SIZE + 0x100); // padding in case the sigscan is past the end of the buffer

	for (uintptr_t i = 0; i < 0x500000; i += BUFFER_SIZE) {
		SIZE_T numBytesWritten;
		if (!ReadProcessMemory(_handle, reinterpret_cast<void*>(_baseAddress + i), &buff[0], buff.size(), &numBytesWritten)) continue;
		buff.resize(numBytesWritten);
		int index = find(buff, scanBytes);
		if (index == -1) continue;

		index = index + 0x14; // This scan targets a line slightly before the key instruction
		// (address of next line) + (index interpreted as 4byte int)
		Memory::GLOBALS = (int)(i + index + 4) + *(int*)&buff[index];
		break;
	}

	return Memory::GLOBALS;
}

void Memory::findMovementSpeed() {
	const std::vector<byte> scanBytes = { 0xF3, 0x0F, 0x59, 0xFD, 0xF3, 0x0F, 0x5C, 0xC8 };
#define BUFFER_SIZE 0x10000 // 10 KB
	std::vector<byte> buff;
	buff.resize(BUFFER_SIZE + 0x100); // padding in case the sigscan is past the end of the buffer

	for (uintptr_t i = 0; i < 0x500000; i += BUFFER_SIZE) {
		SIZE_T numBytesWritten;
		if (!ReadProcessMemory(_handle, reinterpret_cast<void*>(_baseAddress + i), &buff[0], buff.size(), &numBytesWritten)) continue;
		buff.resize(numBytesWritten);
		int index = find(buff, scanBytes);
		if (index == -1) continue;

		for (; index < buff.size(); index++) {
			if (buff[index - 4] == 0xF3 && buff[index - 3] == 0x0F && buff[index - 2] == 0x10 && buff[index - 1] == 0x0D) {
				int speedAddr = (int)(i + index + 4) + *(int*)&buff[index];

				Memory::RUNSPEED = speedAddr;

				break;
			}
		}

		for (; index < buff.size(); index++) {
			if (buff[index - 2] == 0xEB && buff[index - 1] == 0x08) {
				index -= 0x06;
				
				int accelAddr = (int)(i + index + 4) + *(int*)&buff[index];

				Memory::ACCELERATION = accelAddr;

				return;
			}
		}


		break;
	}

	return;
}

void Memory::ThrowError(std::string message) {
	if (!showMsg) throw std::exception(message.c_str());
	DWORD exitCode;
	GetExitCodeProcess(_handle, &exitCode);
	if (exitCode != STILL_ACTIVE) throw std::exception(message.c_str());
	message += "\nPlease close The Witness and try again. If the error persists, please report the issue on the Github Issues page.";
	MessageBoxA(GetActiveWindow(), message.c_str(), NULL, MB_OK);
	throw std::exception(message.c_str());
}

void Memory::ThrowError(const std::vector<int>& offsets, bool rw_flag) {
	std::stringstream ss; ss << std::hex;
	if (offsets.size() == 4) {
		ss << "Error " << (rw_flag ? "writing" : "reading") << " 0x" << offsets[3] << " in panel 0x" << offsets[2] / 8;
		ThrowError(ss.str());
	}
	else if (offsets.size() == 3) {
		for (int i : offsets) ss << "0x" << i << " ";
		ThrowError("Error computing offsets: " + ss.str());
	}
	else {
		for (int i : offsets) ss << "0x" << i << " ";
		ThrowError("Unknown error: " + ss.str());
	}
}

void Memory::ThrowError() {
	std::string message(256, '\0');
	int length = FormatMessageA(4096, nullptr, GetLastError(), 1024, &message[0], static_cast<DWORD>(message.size()), nullptr);
	message.resize(length);
	ThrowError(message);
}

void* Memory::ComputeOffset(std::vector<int> offsets)
{
	// Leave off the last offset, since it will be either read/write, and may not be of type unitptr_t.
	int final_offset = offsets.back();
	offsets.pop_back();

	uintptr_t cumulativeAddress =  _baseAddress;
	for (const int offset : offsets) {
		cumulativeAddress += offset;

		const auto search = _computedAddresses.find(cumulativeAddress);
		if (search == std::end(_computedAddresses)) {
			// If the address is not yet computed, then compute it.
			uintptr_t computedAddress = 0;
			if (!Read(reinterpret_cast<LPVOID>(cumulativeAddress), &computedAddress, sizeof(uintptr_t))) {
				if (!showMsg) throw std::exception();
				ThrowError(offsets, false);
			}
			_computedAddresses[cumulativeAddress] = computedAddress;
		}

		cumulativeAddress = _computedAddresses[cumulativeAddress];
	}
	return reinterpret_cast<void*>(cumulativeAddress + final_offset);
}

int Memory::GLOBALS = 0;
int Memory::RUNSPEED = 0;
int Memory::ACCELERATION = 0;
bool Memory::showMsg = false;
int Memory::globalsTests[3] = {
	0x62D0A0, //Steam and Epic Games
	0x62B0A0, //Good Old Games
	0x5B28C0 //Older Versions
};
