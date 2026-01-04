// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "Memoryapi.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include "Randomizer.h"

#undef PROCESSENTRY32
#undef Process32Next

Memory* Memory::_singleton = nullptr;

#define SIGSCAN_STRIDE   0x100000 // 100 KiB. Note that larger reads are not significantly slower than small reads, but have an increased chance of failing, since ReadProcessMemory fails if ANY of the memory is inaccessible.
#define SIGSCAN_PADDING  0x000800 // The additional amount to scan in order to ensure that a useful amount of data is returned if the found signature is at the end of the buffer.
#define PROGRAM_SIZE    0x5000000 // 5 MiB. (The application itself is only 4.7 MiB large.)

Memory::Memory() {
	std::string process32 = "witness_d3d11.exe";
	std::string process64 = "witness64_d3d11.exe";

	// First, get the handle of the process
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	while (Process32Next(snapshot, &entry)) {
		if (entry.szExeFile == process64) {
			_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
			break;
		}
	}

	// If we didn't find the process, terminate.
	if (!_handle) {
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		while (Process32Next(snapshot, &entry)) {
			if (process32 == entry.szExeFile) {
				MessageBox(GetActiveWindow(), L"You appear to be running the 32 bit version of The Witness. Please run the 64 bit version instead.", NULL, MB_OK);
				throw std::exception("Unable to find process!");
			}
		}

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
		if (name == process64) {
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
		for (size_t j = 0; j < maxJ; j++) {
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

void Memory::create()
{
	if (_singleton == nullptr) {
		_singleton = new Memory();

		_singleton->findGlobals();
		_singleton->fixTriangleNegation();
		_singleton->setupCustomSymbols();
	}
}

Memory* Memory::get()
{
	return _singleton;
}

void Memory::invalidateCache() {
	_computedAddresses = std::map<uintptr_t, uintptr_t>();
}

void Memory::findGlobals() {
	if (!GLOBALS) {
		// Check to see if this is a version with a known globals pointer.
		for (int g : globalsTests) {
			GLOBALS = g;
			try {
				if (_singleton->ReadPanelData<int>(0x17E52, STYLE_FLAGS) == 0xA040) {
					return;
				}
			}
			catch (const std::exception&) {
				// Globals not found
			}
		}
	}

	// Otherwise, checked for a cached value.
	std::ifstream file("WRPGglobals.txt");
	if (file.is_open()) {
		file >> std::hex >> GLOBALS;
		file.close();
		return;
	}
	
	// Finally, fall back to sigscanning.
	ScanForBytes({ 0x74, 0x41, 0x48, 0x85, 0xC0, 0x74, 0x04, 0x48, 0x8B, 0x48, 0x10 }, [&](__int64 offset, int index, const std::vector<byte>& data) {
		// This scan targets a line slightly before the key instruction
		GLOBALS = (int)Memory::ReadStaticInt(offset, index + 0x14, data);
	});

	if (!GLOBALS) ThrowError("Unable to find globals pointer.");

	// Store the pointer to disk for faster lookup next time.
	std::ofstream ofile("WRPGglobals.txt", std::ofstream::app);
	ofile << std::hex << GLOBALS << std::endl;
	ofile.close();
}

void Memory::fixTriangleNegation() {
	Memory* memory = Memory::get();

	__int64 drawDecorationHelper = 0;
	memory->ScanForBytes({ 0x41, 0x81, 0xFE, 0x00, 0x06, 0x00, 0x00 }, [&](__int64 offset, int index, const std::vector<byte>& data) {
		drawDecorationHelper = offset + index;
	});

	// Replace the line which is reading from &color with &result (the eraser-modified color)
	memory->WriteData<byte>({ (int)(drawDecorationHelper + 0x41) }, { 0x97 });
}

void Memory::setupCustomSymbols() {
	Memory* memory = Memory::get();

	__int64 drawCounter = 0;
	memory->ScanForBytes({ 0x83, 0xF8, 0x05, 0x0F, 0x87, 0x17, 0x02, 0x00, 0x00 }, [&](__int64 offset, int index, const std::vector<byte>& data) {
		drawCounter = offset + index;
	});

	if (drawCounter == 0) return; // Already injected

	// Remove the cases for 5 and 6 triangles from the switch statement so we have space for our code
	memory->WriteData<byte>({ (int)(drawCounter) }, {
		0x83, 0xF8, 0x03,					// cmp eax, 03   ; If the number of triangles is greater than 4
		0x0F, 0x87, 0x13, 0x01, 0x00, 0x00, // ja 0x00000113 ; jump 0x113 bytes forward (case label 5)
	});

	constexpr int MAX_INSTRUCTIONS = 260;

	// Here is our assembly code.
	const byte instructions[] = {
		0x01, 0x02, 0x03, 0x04,
	};

	static_assert(sizeof(instructions) < MAX_INSTRUCTIONS, "There are only 260 bytes available in this code cave");

	// Ensure that all of the non-instruction bytes are NOPs
	std::vector<byte> bytes(MAX_INSTRUCTIONS, 0x90);
	for (int i = 0; i < sizeof(instructions); i++) bytes[i] = instructions[i];

	// Write the actual instructions into the program.
	memory->WriteData<byte>({ (int)(drawCounter + 0x11C) }, bytes);
}

bool Memory::ScanForBytes(const std::vector<byte>& scanBytes, const ScanFunc& scanFunc) {
	std::vector<byte> buff;
	buff.resize(SIGSCAN_STRIDE + 0x100); // padding in case the sigscan is past the end of the buffer
	int result = -1;
	for (uintptr_t i = 0; i < PROGRAM_SIZE; i += SIGSCAN_STRIDE) {
		SIZE_T numBytesWritten;
		if (!ReadProcessMemory(_handle, reinterpret_cast<void*>(_baseAddress + i), &buff[0], buff.size(), &numBytesWritten)) continue;
		buff.resize(numBytesWritten);
		int index = find(buff, scanBytes);
		if (index != -1) {
			scanFunc(i, index, buff);
			return true;
		}
	}

	return false; // Not found
}

__int64 Memory::ReadStaticInt(__int64 offset, int index, const std::vector<byte>& data, size_t bytesToEOL) {
	// (address of next line) + (index interpreted as 4byte int)
	return offset + index + bytesToEOL + *(int*)&data[index];
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
};