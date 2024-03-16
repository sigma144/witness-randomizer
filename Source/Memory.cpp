// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "Memoryapi.h"
#include "Utilities.h"

#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include "ClientWindow.h"
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

	// First, attempt to find the process.
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

void Memory::create() {
	if (_singleton == nullptr) {
		_singleton = new Memory();

		_singleton->findGlobals();
		_singleton->findGamelibRenderer();
		_singleton->findMovementSpeed();
		_singleton->findActivePanel();
		_singleton->findPlayerPosition();
		_singleton->findImportantFunctionAddresses();
	}
}

Memory* Memory::get() {
	return _singleton;
}

DWORD Memory::getProcessID() {
	return GetProcessId(_handle);
}

// Copied from Witness Trainer https://github.com/jbzdarkid/witness-trainer/blob/master/Source/Memory.cpp#L218
void Memory::findGlobals() {
	if (!GLOBALS) {
		bool showMsgTemp = _singleton->showMsg;
		_singleton->showMsg = false;
		// Check to see if this is a version with a known globals pointer.
		for (int g : globalsTests) {
			GLOBALS = g;
			try {
				if (_singleton->ReadPanelData<int>(0x17E52, STYLE_FLAGS) == 0xA040) {
					_singleton->showMsg = showMsgTemp;
					return;
				}
			}
			catch (std::exception e) {
			}
		}

		_singleton->showMsg = showMsgTemp;

		if (!ClientWindow::get()->showDialogPrompt("This version of The Witness is not known to the randomizer. Proceed anyway? (May cause issues.)")) {
			return;
		}

		// Checked for a cached value.
		std::ifstream file("WRPGglobals.txt");
		if (file.is_open()) {
			file >> std::hex >> GLOBALS;
			file.close();
		}
		else {
			// We had no cached value; scan for it in the process instead.
			const std::vector<byte> scanBytes = { 0x74, 0x41, 0x48, 0x85, 0xC0, 0x74, 0x04, 0x48, 0x8B, 0x48, 0x10 };
			std::vector<byte> buff;
			buff.resize(SIGSCAN_STRIDE + 0x100); // padding in case the sigscan is past the end of the buffer

			GLOBALS = 0;
			for (uintptr_t i = 0; i < PROGRAM_SIZE; i += SIGSCAN_STRIDE) {
				SIZE_T numBytesWritten;
				if (!ReadProcessMemory(_handle, reinterpret_cast<void*>(_baseAddress + i), &buff[0], buff.size(), &numBytesWritten)) continue;
				buff.resize(numBytesWritten);
				int index = Utilities::findSequence(buff, scanBytes);
				if (index == -1) continue;

				index = index + 0x14; // This scan targets a line slightly before the key instruction
				// (address of next line) + (index interpreted as 4byte int)
				GLOBALS = (int)(i + index + 4) + *(int*)&buff[index];
				break;
			}

			if (GLOBALS) {
				// Store the pointer to disk for faster lookup next time.
				std::ofstream ofile("WRPGglobals.txt", std::ofstream::app);
				ofile << std::hex << GLOBALS << std::endl;
				ofile.close();
			}
			else {
				ThrowError("Unable to find globals pointer.");
			}
		}
	}
}

void Memory::findGamelibRenderer()
{
	// Find the pointer to gamelib_renderer. A reliable access point to this is in Overlay::begin().
	const std::vector<byte> gamelibSearchBytes = {
		0x45, 0x33, 0xC9,
		0x89, 0x41, 0x40,
		0x48, 0x8B, 0x0D  // <- MOV RCX, qword ptr [next 4 bytes]
	};

	uint64_t pointerLocation = executeSigScan(gamelibSearchBytes) + gamelibSearchBytes.size();

	// Read the literal value passed to the MOV operation.
	if (!ReadRelative(reinterpret_cast<void*>(pointerLocation), &GAMELIB_RENDERER, sizeof(int))) {
		throw "Unable to dereference gamelib_render.";
		GAMELIB_RENDERER = 0;
		return;
	}
	
	// Since referencing global values is a relative operation, our final address is equal to the offset passed to the MOV operation, plus the address
	//   of the instruction immediately after the call. Note that since pointerLocation is an offset relative to the program's base address, this final
	//   value is itself relative to the base address.
	GAMELIB_RENDERER += pointerLocation + 0x4;
}

void Memory::findPlayerPosition() {
	executeSigScan({ 0x84, 0xC0, 0x75, 0x59, 0xBA, 0x20, 0x00, 0x00, 0x00 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		// This int is actually desired_movement_direction, which immediately preceeds camera_position
		this->CAMERAPOSITION = ReadStaticInt(offset, index + 0x19, data) + 0x10;

		return true;
	});
}

void Memory::StopDesertLaserPropagation() {
	executeSigScan({ 0x8B, 0x40, 0x2C, 0xF2, 0x0F, 0x10, 0x43, 0x24, 0x89, 0x44 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (;; index--) {
			if (data[index - 3] == 0x48 && data[index - 2] == 0x8B && data[index - 1] == 0xC8 && data[index - 0] == 0xE8) {
				char asmBuff[] = "\x90\x90\x90\x90\x90";
				
				LPVOID addressPointer = reinterpret_cast<LPVOID>(_baseAddress + offset + index);

				WriteProcessMemory(_handle, addressPointer, asmBuff, sizeof(asmBuff) - 1, NULL);

				return true;
			}
		}

		return false;
	});
}

void Memory::SetInfiniteChallenge(bool enable) {
	if (_bytesLengthChallenge == 0) { //first time, find the music file in memory
		char buffer[128];
		std::string name = "peer_gynt";

		memset(buffer, 0, sizeof(buffer));

		strcpy_s(buffer, name.c_str());

		auto challengeStuff = VirtualAllocEx(_handle, NULL, sizeof(buffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		__int64 soundName = reinterpret_cast<__int64>(challengeStuff);
		__int64 returnAddress = soundName + 0x20;

		WriteProcessMemory(_handle, challengeStuff, buffer, sizeof(buffer), NULL);

		uint64_t offset = reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, 0x00BFF * 8, 0 }));

		char asmBuff[] =
			"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax, function address
			"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx, address of record player
			"\x48\xBA\x00\x00\x00\x00\x00\x00\x00\x00" //mov rdx, string name of sound
			"\x49\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov r8, 0 (necessary for call)
			"\x48\x83\xEC\x48" // sub rsp, 48
			"\xFF\xD0" // call rax
			"\x48\x83\xC4\x48" // add rsp, 48
			"\x48\xBB\x00\x00\x00\x00\x00\x00\x00\x00" //mov rbx, return address
			"\x48\x89\x03" // mov [rbx],rax
			"\xC3"; // ret

		asmBuff[2] = _getSoundFunction & 0xff;
		asmBuff[3] = (_getSoundFunction >> 8) & 0xff;
		asmBuff[4] = (_getSoundFunction >> 16) & 0xff;
		asmBuff[5] = (_getSoundFunction >> 24) & 0xff;
		asmBuff[6] = (_getSoundFunction >> 32) & 0xff;
		asmBuff[7] = (_getSoundFunction >> 40) & 0xff;
		asmBuff[8] = (_getSoundFunction >> 48) & 0xff;
		asmBuff[9] = (_getSoundFunction >> 56) & 0xff;
		asmBuff[12] = offset & 0xff;
		asmBuff[13] = (offset >> 8) & 0xff;
		asmBuff[14] = (offset >> 16) & 0xff;
		asmBuff[15] = (offset >> 24) & 0xff;
		asmBuff[16] = (offset >> 32) & 0xff;
		asmBuff[17] = (offset >> 40) & 0xff;
		asmBuff[18] = (offset >> 48) & 0xff;
		asmBuff[19] = (offset >> 56) & 0xff;
		asmBuff[22] = soundName & 0xff;
		asmBuff[23] = (soundName >> 8) & 0xff;
		asmBuff[24] = (soundName >> 16) & 0xff;
		asmBuff[25] = (soundName >> 24) & 0xff;
		asmBuff[26] = (soundName >> 32) & 0xff;
		asmBuff[27] = (soundName >> 40) & 0xff;
		asmBuff[28] = (soundName >> 48) & 0xff;
		asmBuff[29] = (soundName >> 56) & 0xff;

		asmBuff[52] = returnAddress & 0xff;
		asmBuff[53] = (returnAddress >> 8) & 0xff;
		asmBuff[54] = (returnAddress >> 16) & 0xff;
		asmBuff[55] = (returnAddress >> 24) & 0xff;
		asmBuff[56] = (returnAddress >> 32) & 0xff;
		asmBuff[57] = (returnAddress >> 40) & 0xff;
		asmBuff[58] = (returnAddress >> 48) & 0xff;
		asmBuff[59] = (returnAddress >> 56) & 0xff;

		SIZE_T allocation_size = sizeof(asmBuff);

		LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		WriteProcessMemory(_handle, allocation_start, asmBuff, allocation_size, NULL);
		HANDLE thread = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);

		WaitForSingleObject(thread, INFINITE);

		__int64 sound_object[1];
		SIZE_T numBytesWritten;
		ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(returnAddress), sound_object, 8, &numBytesWritten);

		_bytesLengthChallenge = sound_object[0] + 0x28;
	}

	if (enable) {
		char asmBuff[] = "\xEB\x07\x66\x90";

		LPVOID addressPointer = reinterpret_cast<LPVOID>(_recordPlayerUpdate);

		WriteProcessMemory(_handle, addressPointer, asmBuff, sizeof(asmBuff) - 1, NULL);

		char asmBuff2[] = "\x00\x00\x00"; // Length of song to 0

		LPVOID addressPointer2 = reinterpret_cast<LPVOID>(_bytesLengthChallenge);

		WriteProcessMemory(_handle, addressPointer2, asmBuff2, sizeof(asmBuff2) - 1, NULL);


	}
	else {
		char asmBuff[] = "\x48\x8B\x4B\x18";

		LPVOID addressPointer = reinterpret_cast<LPVOID>(_recordPlayerUpdate);

		WriteProcessMemory(_handle, addressPointer, asmBuff, sizeof(asmBuff) - 1, NULL);

		char asmBuff2[] = "\x67\xB1\x26"; // Length of song to original length

		LPVOID addressPointer2 = reinterpret_cast<LPVOID>(_bytesLengthChallenge);

		WriteProcessMemory(_handle, addressPointer2, asmBuff2, sizeof(asmBuff2) - 1, NULL);
	}
}

void Memory::applyDestructivePatches() {
	//Cursor size
	char asmBuff[] = "\xBB\x00\x00\x80\x3F\x66\x44\x0F\x6E\xCB\x90\x90\x90";
	/*mov ebx, 3F800000
	movd xmm9, ebx
	nop
	nop
	nop*/

	LPVOID addressPointer = reinterpret_cast<LPVOID>(cursorSize);

	WriteProcessMemory(_handle, addressPointer, asmBuff, sizeof(asmBuff) - 1, NULL);
}

void Memory::doSecretThing() {
	// If you find this, please don't talk about it publicly. DM Violet and they'll tell you what it does. :)

	executeSigScan({ 0x48, 0x89, 0x5C, 0x24, 0x30, 0x48, 0x89, 0x74, 0x24, 0x38, 0x48, 0x8B, 0x35 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		uint64_t to_read = _baseAddress + offset + index - 11;
		uint64_t localisation_ptr = 0;
		uint64_t localisation_ptr2 = 0;

		ReadAbsolute(reinterpret_cast<void*>(to_read), &localisation_ptr, sizeof(int));

		localisation_ptr += to_read + 5;

		ReadAbsolute(reinterpret_cast<void*>(localisation_ptr), &localisation_ptr2, sizeof(uint64_t));

		ReadAbsolute(reinterpret_cast<void*>(localisation_ptr2), &localisation, sizeof(uint64_t));

		char buf[13];

		for (uint64_t i = localisation; i < localisation + 0x10000; i++) {
			if (!ReadAbsolute(reinterpret_cast<void*>(i), &buf, sizeof(buf))) continue;

			if (buf[0] == 0x00 && buf[1] == 0x54 && buf[2] == 0x68 && buf[3] == 0x65 && buf[4] == 0x20 && buf[5] == 0x57 && buf[6] == 0x69 && buf[12] == 0x00) {
				the_witness_string = i + 1;

				char writebuf[] = "\x54\x68\x65\x20\x57\x69\x6E\x74\x65\x73\x73";

				WriteProcessMemory(_handle, reinterpret_cast<void*>(the_witness_string), writebuf, sizeof(writebuf) - 1, NULL);
				return true; // This return call makes it not work for e.g. the French version where the title is the *second* occurance of the string.
				// But if I just remove it it might cause bugs. The real answer is to somehow work out the end of the "localisation_text" allocation. No clue how to do that
			}
		}

		return true;
	});
}

void Memory::findImportantFunctionAddresses(){
	if (Utilities::isAprilFools()) {
		doSecretThing();
		// If you find this, please don't talk about it publicly. DM Violet and they'll tell you what it does. :)
	}

	executeSigScan({ 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x57, 0x48, 0x81, 0xEC, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index] == 0xF3 && data[index + 1] == 0x44 && data[index + 2] == 0x0F && data[index + 3] == 0x10 && data[index + 4] == 0x35) {
				index += 5;
				uint64_t addressOfRelativePointer = _baseAddress + offset + index;
				int relativePointer;
				ReadAbsolute(reinterpret_cast<LPCVOID>(addressOfRelativePointer), &relativePointer, sizeof(int));

				this->windmillMaxTurnSpeed = addressOfRelativePointer + relativePointer + 4;

				break;
			}
		}

		for (; index < data.size(); index++) {
			if (data[index - 2] == 0x80 && data[index - 1] == 0x3D && data[index + 4] == 0x00) {
				uint64_t addressOfRelativePointer = _baseAddress + offset + index;
				int relativePointer;
				ReadAbsolute(reinterpret_cast<LPCVOID>(addressOfRelativePointer), &relativePointer, sizeof(int));

				this->windmillCurrentlyTurning = addressOfRelativePointer + relativePointer + 4;

				break;
			}
		}

		for (; index < data.size(); index++) {
			if (data[index - 4] == 0xF3 && data[index - 3] == 0x0F && data[index - 2] == 0x10 && data[index - 1] == 0x05 && data[index - 9] == 0x0D) {
				uint64_t addressOfRelativePointer = _baseAddress + offset + index;
				int relativePointer;
				ReadAbsolute(reinterpret_cast<LPCVOID>(addressOfRelativePointer), &relativePointer, sizeof(int));

				this->windmillCurrentTurnSpeed = addressOfRelativePointer + relativePointer + 4;

				break;
			}
		}

		return true;
	});

	executeSigScan({ 0x0F, 0x29, 0x74, 0x24, 0x20, 0x0F, 0x28, 0xF0, 0x48, 0x8B, 0x01, 0xF3, 0x0F }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (;; index--) {
			if ((data[index] == 0x48 || data[index] == 0xC3) && data[index + 1] == 0x83 && data[index + 2] == 0xEC) { // This is the close door function. Could only find a sigscan for the middle of it, so we need to go backwards.
				this->setUserBrightnessFunction = _baseAddress + offset + index;
				break;
			}
		}

		for (; index < data.size(); index++) {
			if (data[index - 3] == 0x48 && data[index - 2] == 0x8B && data[index - 1] == 0x05) { // This is the close door function. Could only find a sigscan for the middle of it, so we need to go backwards.
				uint64_t addressOfRelativePointer = _baseAddress + offset + index;
				int relativePointer;
				ReadAbsolute(reinterpret_cast<LPCVOID>(addressOfRelativePointer), &relativePointer, sizeof(int));

				this->deviceDisplay = addressOfRelativePointer + relativePointer + 4;

				return true;
			}
		}

		return false;
	});

	// Exit solve mode is a pain to find. We'll find enter_solve_mode, which is easier to find, and then find the start of the next function.
	// We also need to do some work to capture both the old and the new versions' versions of this function
	executeSigScan({ 0x66, 0x0F, 0x5A, 0xC0, 0xF3, 0x0F, 0x5C, 0x05 }, [this](__int64 offset, int index, const std::vector<byte>& data) { // This will be inside Enter Solve Mode
		int backIndex = index;

		for (;; backIndex--) {
			if ((data[backIndex - 1] == 0xCC || data[backIndex - 1] == 0xC3) && (data[backIndex] == 0x48 || data[backIndex] == 0xC3) && data[backIndex + 1] == 0x89 && (data[backIndex + 2] == 0x6C || data[backIndex + 2] == 0x74 && data[backIndex + 3] == 0x24 && data[backIndex + 4] == 0x10)) {
				this->enterSolveModeFunction = _baseAddress + backIndex + offset;
				break;
			}
		}

		for (; index < data.size(); index++) {
			if (data[index] == 0x48 && data[index + 1] == 0x83 && data[index + 2] == 0xC4) { // This will be the end of Enter Solve Mode
				for (; index < data.size(); index++) {
					if (data[index] == 0x0F && data[index + 1] == 0x2F && data[index + 2] == 0xC1 && data[index + 3] == 0x76 && data[index + 4] == 0x03) { // This will be inside Exit Solve Mode
						for (; index > 0; index--) {
							if (data[index] == 0x48 && data[index + 1] == 0x83 && data[index + 2] == 0xEC) { // This will be the start of Exit Solve Mode, except ...
								for (; index > 0; index--) { // ... the start of Exit Solve Mode has an extra instruction depending on version
									if (data[index] == 0xCC || data[index] == 0xC3) { // So we go backwards until CC (int) or CE (ret)
										this->exitSolveModeFunction = _baseAddress + offset + index + 1;

										return true;
									}
								}

								return false;
							}
						}
						return false;
					}
				}
			}
		}

		return false;
	});

	executeSigScan({ 0xF3, 0x41, 0x0F, 0x58, 0xF0, 0x41, 0x0F, 0x2E, 0xF0, 0x0F, 0x84, 0x9A }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index - 3] == 0x4C && data[index - 2] == 0x8D && data[index - 1] == 0x05) {
				uint64_t ptypeIssuedSoundRelativePointer = _baseAddress + offset + index;
				int ptypeIssuedSoundRelative;
				ReadAbsolute(reinterpret_cast<LPCVOID>(ptypeIssuedSoundRelativePointer), &ptypeIssuedSoundRelative, sizeof(int));
				ptypeIssuedSound = ptypeIssuedSoundRelativePointer + ptypeIssuedSoundRelative + 4;
				break;
			}
		}

		for (; index < data.size(); index++) {
			if (data[index - 3] == 0x49 && data[index - 2] == 0x8B && data[index - 1] == 0xD0) {
				uint64_t getPortablesOfTypeRelativePointer = _baseAddress + offset + index + 14;
				int getPortablesOfTypeRelative;
				ReadAbsolute(reinterpret_cast<LPCVOID>(getPortablesOfTypeRelativePointer), &getPortablesOfTypeRelative, sizeof(int));
				getPortablesOfType = getPortablesOfTypeRelativePointer + getPortablesOfTypeRelative + 4;
				break;
			}
		}

		return true;
	});

	executeSigScan({ 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x41, 0x56, 0x48, 0x63 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->getEntityByName = _baseAddress + offset + index;
		
		return true;
	});

	executeSigScan({ 0x45, 0x0F, 0x28, 0xC8, 0xF3, 0x44 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		cursorSize = _baseAddress + offset + index;

		for (; index < data.size(); index++) {
			if (data[index - 3] == 0xC7 && data[index - 2] == 0x45 && data[index - 1] == 0x40) {
				cursorR = _baseAddress + offset + index;
				break;
			}
		}
		index++;
		for (; index < data.size(); index++) {
			if (data[index - 3] == 0xC7 && data[index - 2] == 0x45 && data[index - 1] == 0x44) {
				cursorG = _baseAddress + offset + index;
				break;
			}
		}
		index++;
		for (; index < data.size(); index++) {
			if (data[index - 3] == 0xC7 && data[index - 2] == 0x45 && data[index - 1] == 0x48) {
				cursorB = _baseAddress + offset + index;
				break;
			}
		}

		return true;
	});

	executeSigScan ({0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x20, 0x48, 0x89, 0x48, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54}, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index - 2] == 0x78 && data[index - 1] == 0x10 && data[index] == 0xE8) { // need to find actual function, which I could not get a sigscan to work for, so I did the function right before it
				uint64_t pointerLocation = _baseAddress + offset + index + 1;

				int function;

				ReadAbsolute(reinterpret_cast<void*>(pointerLocation), &function, sizeof(int));

				removeFromPatternMapFunction = pointerLocation + function + 4;
				
				// stop game from trying to delete previous pattern map

				uint64_t testInstruction = pointerLocation + 0x10;

				LPVOID testInstructionPointer = reinterpret_cast<LPVOID>(testInstruction);

				char buf[] = "\x90\x90\x90\x75"; //Change to nop nop nop jne instead of test je. Will never be equal due to the preceding code, so will always jump.
				//This will cause memory leaks because the object is now not deleted. But that's preferrable to the game crashing.
				//The memory leaks are about on the order of 0.1kB per "Resolving an already solved EP", an action the player shouldn't do too often.

				WriteAbsolute(testInstructionPointer, buf, sizeof(buf) -1); // Write the new relative address into the "movss xmm0 [address]" statement.


				break;
			}
		}

		index++;

		for (; index < data.size(); index++) {
			if (data[index - 2] == 0x4B && data[index - 1] == 0x30 && data[index] == 0xE8) { // need to find actual function, which I could not get a sigscan to work for, so I did the function right before it
				uint64_t pointerLocation = _baseAddress + offset + index + 1;

				int function;

				ReadAbsolute(reinterpret_cast<void*>(pointerLocation), &function, sizeof(int));

				addToPatternMapFunction = pointerLocation + function + 4;

				break;
			}
		}

		for (; index < data.size(); index--) {
			if (data[index] == 0x48 && data[index + 1] == 0x8D && data[index + 2] == 0x0D) { // need to find actual function, which I could not get a sigscan to work for, so I did the function right before it
				uint64_t pointerLocation = _baseAddress + offset + index + 3;

				int function;

				ReadAbsolute(reinterpret_cast<void*>(pointerLocation), &function, sizeof(int));

				patternMap = pointerLocation + function + 4;

				break;
			}
		}

		return true;
	});

	executeSigScan({ 0x48, 0x89, 0x6C, 0x24, 0x18, 0x57, 0x8B, 0x81 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->updateJunctionsFunction = _baseAddress + offset + index;

		return true;
	});

	executeSigScan({ 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x70 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->powerGaugeFunction = _baseAddress + offset + index;

		return true;
	});

	executeSigScan({ 0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x49, 0x8B, 0xF8, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xD9 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->_getSoundFunction = _baseAddress + offset + index;

		return true;
	});

	

	executeSigScan({ 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xE9, 0xB3 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->_recordPlayerUpdate = _baseAddress + offset + index - 0x0C;

		return true;
	});

	//open door
	executeSigScan({ 0x0F, 0x57, 0xC9, 0x48, 0x8B, 0xCB, 0x48, 0x83, 0xC4, 0x20 }, [this](__int64 offset, int index, const std::vector<byte>& data) {	
		for (; index < data.size(); index++) {
			if (data[index - 2] == 0xF3 && data[index - 1] == 0x0F) {
				this->openDoorFunction = _baseAddress + offset + index - 2;
				break;
			}
		}

		return true;
	});

	//close door
	executeSigScan({ 0x8B, 0x7B, 0x30, 0x4D, 0x8B, 0x63, 0x38, 0x49, 0x8B, 0xE3, 0x41, 0x5F, 0x41, 0x5E, 0x5D, 0xC3 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index - 2] == 0x40 && data[index - 1] == 0x53 && data[index] == 0x48) { // The open door function comes after.
				this->closeDoorFunction = _baseAddress + offset + index - 2;
				break;
			}
		}

		return true;
	});

	//extend bridge
	executeSigScan({ 0x48, 0x89, 0x4C, 0x24, 0x08, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->bridgeToggleFunction = _baseAddress + offset + index;

		return true;
	});

	executeSigScan({ 0x40, 0x56, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x44, 0x0F, 0xB6, 0xF2 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->floodgateToggleFunction = _baseAddress + offset + index;

		return true;
	});

	//bunker elevator
	executeSigScan({ 0x48, 0x8B, 0xC4, 0x44, 0x88, 0x40, 0x18, 0x53, 0x55, 0x56, 0x57 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->moveBunkerElevatorFunction = _baseAddress + offset + index;

		return true;
	});

	//init panel
	executeSigScan({ 0x48, 0x89, 0x74, 0x24, 0x48, 0xFF, 0xC9, 0x8D, 0x70, 0xFF, 0x48, 0x89, 0x7C }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->initPanelFunction = _baseAddress + offset + index - 0x32;

		return true;
	});

	//find a spot to inject payload
	executeSigScan({0x0F, 0x5B, 0xC0, 0x0F, 0x2E, 0xC1, 0x74}, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index - 5] == 0xE8 && data[index] == 0xE8 && data[index + 5] == 0xE8 && data[index + 5] == 0xE8 && data[index + 10] == 0xE8) {
				this->gameLoop3CallsInARow = _baseAddress + offset + index;
				break;
			}
		}

		return true;
	});

	////display subtitles
	//executeSigScan({ 0x48, 0x89, 0xB4, 0x24, 0xC8, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x4B, 0x08, 0x4C, 0x8D, 0x84, 0x24 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
	//	this->displaySubtitlesFunction = _baseAddress + offset + index - 9;

	//	for (; index < data.size(); index--) { // find rax statement at start of function (Subtitles setting)
	//		if (data[index] == 0x48 && data[index + 1] == 0x8B && data[index+2] == 0x05) { 
	//			uint64_t raxstatement = _baseAddress + offset + index + 3;
	//			
	//			int buff[1];

	//			ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(raxstatement), buff, sizeof(buff), NULL);

	//			this->subtitlesOnOrOff = raxstatement + buff[0] + 4;
	//			
	//			break;
	//		}
	//	}

	//	for (; index < data.size(); index++) { // find rax statement at start of function (Subtitles setting)
	//		if (data[index] == 0x48 && data[index + 1] == 0x8B && data[index + 2] == 0x1D) {
	//			uint64_t rbxstatement = _baseAddress + offset + index + 3;

	//			int buff[1];

	//			ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(rbxstatement), buff, sizeof(buff), NULL);

	//			this->subtitlesHashTable = rbxstatement + buff[0] + 4;

	//			break;
	//		}
	//	}

	//	return true;
	//});

	////display subtitles2
	//executeSigScan({ 0xF3, 0x0F, 0x10, 0x8C, 0x24, 0xE0, 0x00, 0x00, 0x00, 0x8B, 0xCE, 0x4C, 0x8B, 0xC0, 0x48, 0x89, 0xBC }, [this](__int64 offset, int index, const std::vector<byte>& data) {
	//	this->displaySubtitlesFunction2 = _baseAddress + offset + index;
	//	
	//	return true;
	//});

	////display subtitles3
	//executeSigScan({0xC4, 0xD0, 0x00, 0x00, 0x00, 0x41, 0x5E, 0x5F, 0x5D, 0xC3 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
	//	for (; index < data.size(); index++) {
	//		if (data[index] == 0x48  && data[index + 1] == 0x89 && data[index + 2] == 0x5C) { // need to find actual function, which I could not get a sigscan to work for, so I did the function right before it
	//			this->displaySubtitlesFunction3 = _baseAddress + offset + index;
	//			break;
	//		}
	//	}

	//	return true;
	//});

	//Get Sound Stream for Entity Issued Sound
	executeSigScan({ 0x4C, 0x63, 0x51, 0x08, 0x45, 0x8B, 0xD8, 0x33, 0xC0, 0x0F }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->getSoundStreamFunction = _baseAddress + offset + index;

		return true;
	});

	//Update Entity Position
	executeSigScan({ 0x44, 0x0F, 0xB6, 0xCA, 0x4C, 0x8D, 0x41, 0x34 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->updateEntityPositionFunction = _baseAddress + offset + index;

		return true;
	});

	//Power Next Item
	executeSigScan({ 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x42, 0x18, 0x48, 0x8B, 0xFA, 0x48, 0x85, 0xC0, 0x0F, 0x84 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->powerNextFunction = _baseAddress + offset + index - 8;

		return true;
	});

	//Activate Laser
	executeSigScan({ 0x40, 0x53, 0x48, 0x83, 0xEC, 0x60, 0x83, 0xB9 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->activateLaserFunction = _baseAddress + offset + index;

		return true;
	});

	//Display Hud + change hudTime
	executeSigScan({ 0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x83, 0x3D }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->displayHudFunction = _baseAddress + offset + index;

		for (; index < data.size(); index++) {
			if (data[index - 2] == 0xF3 && data[index - 1] == 0x0F) { // find the movss statement
				this->hudTimePointer = _baseAddress + offset + index + 2;

				int buff[1];

				ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->hudTimePointer), buff, sizeof(buff), NULL);

				this->relativeAddressOf6 = buff[0];

				uintptr_t addressOf1 = this->hudTimePointer + 4 + buff[0];

				executeSigScan({ 0x00, 0x00, 0xC0, 0x40 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
					this->relativeAddressOf6 += index;

					return true;
				}, addressOf1);

				break;
			}
		}

		return true;
	});

	// Find hud_draw_headline and its three hardcoded float values for R, G, and B.
	executeSigScan({ 0x48, 0x83, 0xEC, 0x68, 0xF2, 0x0F, 0x10, 0x05 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		// hud_draw_headline draws text twice: once for black text to use as a drop shadow and once using values that are assigned at runtime from hardcoded values. We need to
		//   find the addresses of the three hardcoded values (which are all 1.0f) and store them off to be overwritten in the future.
		uint64_t functionAddress = _baseAddress + offset + index;

		// Read the function into memory to ensure that we have the entire function call. This is necessary in case the executeSigScan call found the function very, very close
		//   to the end of its buffer.
		const int functionSize = 0x200;
		std::vector<byte> functionBody;
		functionBody.resize(functionSize);

		SIZE_T numBytesWritten;
		ReadProcessMemory(_handle, reinterpret_cast<void*>(functionAddress), &functionBody[0], functionSize, &numBytesWritten);

		// Find all three instances of 1.0f. (0x3f800000)
		std::vector<int> foundIndices = Utilities::findAllSequences(functionBody, { 0x00, 0x00, 0x80, 0x3f });
		if (foundIndices.size() != 3) {
			return false;
		}

		// Assign values relative to the base function address.
		for (int colorIndex = 0; colorIndex < 3; colorIndex++)
		{
			hudMessageColorAddresses[colorIndex] = functionAddress + foundIndices[colorIndex];
		}

		return true;
	});

	//Boat speed
	//Find Entity_Boat::set_speed
	executeSigScan({0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x0F, 0x29, 0x74, 0x24, 0x30, 0x8B, 0xFA }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->setBoatSpeed = _baseAddress + offset + index;

		for (; index < data.size(); index++) { // We now need to look for "cmp edi 04", "cmp edi 03", "cmp edi 02", and "cmp edi 01"
			  
			if (data[index - 2] == 0x83 && data[index - 1] == 0xFF && data[index] == 0x04) { // find cmp edi 04 (Will only comment the rest once as it's the same for the other 3)
				this->boatSpeed4 = _baseAddress + offset + index + 7; // movss xmm0 ->register<- is 7 bytes later

				int buff[1];

				ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->boatSpeed4), buff, sizeof(buff), NULL); // Read the current address of the constant loaded into xmm0 relative to the instruction

				this->relativeBoatSpeed4Address = buff[0]; // This is now the address of the constant !!relative to the movss instruction!!

				uintptr_t boatSpeed4Address = this->boatSpeed4 + 4 + buff[0]; // This is the *absolute* address.

				executeSigScan({ 0x00, 0x00, 0xC0, 0x40 }, [this](__int64 offset, int index, const std::vector<byte>& data) { // Find a 0x04C00000 (6.0f) near the memory location of the original constant.
					this->relativeBoatSpeed4Address += index; // Use this 6.0f as the new address to load into xmm0

					__int32 urelativeBoatSpeed4Address = this->relativeBoatSpeed4Address;
					__int64 address = this->boatSpeed4;
					LPVOID addressPointer = reinterpret_cast<LPVOID>(address);


					WriteAbsolute(addressPointer, &urelativeBoatSpeed4Address, sizeof(urelativeBoatSpeed4Address)); // Write the new relative address into the "movss xmm0 [address]" statement.

					return true;
				}, boatSpeed4Address); // Start this Sigscan for a new constant at the location of the original constant to find one "nearby"
			}
			if (data[index - 2] == 0x83 && data[index - 1] == 0xFF && data[index] == 0x03) { // find cmp edi 03
				this->boatSpeed3 = _baseAddress + offset + index + 7;

				int buff[1];

				ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->boatSpeed3), buff, sizeof(buff), NULL);

				this->relativeBoatSpeed3Address = buff[0];

				uintptr_t boatSpeed3Address = this->boatSpeed3 + 4 + buff[0];

				executeSigScan({ 0x00, 0x00, 0x40, 0x40 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
					this->relativeBoatSpeed3Address += index;

					__int32 urelativeBoatSpeed3Address = this->relativeBoatSpeed3Address;
					__int64 address = this->boatSpeed3;
					LPVOID addressPointer = reinterpret_cast<LPVOID>(address);


					WriteAbsolute(addressPointer, &urelativeBoatSpeed3Address, sizeof(urelativeBoatSpeed3Address));

					return true;
					}, boatSpeed3Address);
			}
			if (data[index - 2] == 0x83 && data[index - 1] == 0xFF && data[index] == 0x02) { // find cmp edi 02
				this->boatSpeed2 = _baseAddress + offset + index + 7;

				int buff[1];

				ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->boatSpeed2), buff, sizeof(buff), NULL);

				this->relativeBoatSpeed2Address = buff[0];

				uintptr_t boatSpeed2Address = this->boatSpeed2 + 4 + buff[0];

				executeSigScan({ 0x00, 0x00, 0xA0, 0x3f }, [this](__int64 offset, int index, const std::vector<byte>& data) {
					this->relativeBoatSpeed2Address += index;

					__int32 urelativeBoatSpeed2Address = this->relativeBoatSpeed2Address;
					__int64 address = this->boatSpeed2;
					LPVOID addressPointer = reinterpret_cast<LPVOID>(address);


					WriteAbsolute(addressPointer, &urelativeBoatSpeed2Address, sizeof(urelativeBoatSpeed2Address));

					return true;
					}, boatSpeed2Address);
			}
			if (data[index - 2] == 0x83 && data[index - 1] == 0xFF && data[index] == 0x01) { // find cmp edi 01
				this->boatSpeed1 = _baseAddress + offset + index + 7;

				int buff[1];

				ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->boatSpeed1), buff, sizeof(buff), NULL);

				this->relativeBoatSpeed1Address = buff[0];

				uintptr_t boatSpeed1Address = this->boatSpeed1 + 4 + buff[0];

				executeSigScan({ 0x00, 0x00, 0x00, 0x3f }, [this](__int64 offset, int index, const std::vector<byte>& data) {
					this->relativeBoatSpeed1Address += index;

					__int32 urelativeBoatSpeed1Address = this->relativeBoatSpeed1Address;
					__int64 address = this->boatSpeed1;
					LPVOID addressPointer = reinterpret_cast<LPVOID>(address);


					WriteAbsolute(addressPointer, &urelativeBoatSpeed1Address, sizeof(urelativeBoatSpeed1Address));

					return true;
					}, boatSpeed1Address);
				break;
			}
		}

		return true;
	});

	//Boat turn, in Entity_Boat::update()
	executeSigScan({ 0x41, 0x0F, 0x28, 0xC9, 0xF3, 0x0F, 0x5C, 0xCE, 0x0F, 0x28, 0xD8, 0xF3, 0x0F, 0x11, 0x9B }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		int index2 = index;

		//Boat max turn speed
		for (; index < data.size(); index++) { // Find next mov instruction
			if (data[index - 6] == 0xC7 && data[index - 5] == 0x83) {
				float newSpeed = 3.0f; // 1.0f originally

				WriteAbsolute(reinterpret_cast<LPVOID>(_baseAddress + index), &newSpeed, sizeof(newSpeed));

				break;
			}
		}

		//Boat turn accell
		for (; index2 >= 4; index2--) { // Find next mov instruction
			if (data[index2 - 4] == 0xf3 && data[index2 - 3] == 0x0f && data[index2 - 2] == 0x59) {
				this->boatTurnAccel = _baseAddress + index2;

				executeSigScan({ 0x00, 0x00, 0x00, 0x40 }, [this](__int64 offset, int index, const std::vector<byte>& data) { // 0.5f originally
					index -= 3;

					WriteAbsolute(reinterpret_cast<LPVOID>(this->boatTurnAccel), &index, sizeof(index));

					return true;
				}, this->boatTurnAccel);

				return true;
			}
		}

		//There is a second stage to turning that lowers the boat turn speed back to 0.5f. Not sure if this can be changed or how

		return true;
	});

	executeSigScan({ 0xF3, 0x0F, 0x59, 0xF0, 0xF3, 0x41, 0x0F, 0x58, 0xF0, 0x0F, 0x2F, 0xFA }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		__int64 comissStatement = _baseAddress + offset + index + 0xE;

		char asmBuff[] = // Bypass the upper bound check (comiss) for the boat speed
			"\x38\xC0\x90\x90\x90\x90\x90"; //cmp al,al

		LPVOID addressPointer = reinterpret_cast<LPVOID>(comissStatement);

		WriteProcessMemory(_handle, addressPointer, asmBuff, sizeof(asmBuff) - 1, NULL);

		return true;
	});

	executeSigScan({ 0x45, 0x8B, 0xF7, 0x48, 0x8B, 0x4D }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		byte newByte = 0xEB;

		WriteAbsolute(reinterpret_cast<LPVOID>(_baseAddress + offset + index + 0x15), &newByte, sizeof(newByte));

		return true;
	});

	executeSigScan({ 0x48, 0x8B, 0x51, 0x18, 0x2B, 0x42, 0x08, 0x78, 0x37 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index--) {
			if (data[index] == 0x40 && data[index + 1] == 0x53 && data[index + 2] == 0x48) { // need to find function start (backwards)
				this->completeEPFunction = _baseAddress + offset + index;
				break;
			}
		}

		return true;
	});

	executeSigScan({ 0x66, 0x0F, 0x6E, 0xD9, 0x0F, 0x5B, 0xDB }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		for (; index < data.size(); index++) {
			if (data[index] == 0x48 && data[index - 5] == 0xE8 && data[index - 10] == 0xE8 && data[index + 7] == 0xE8 && (data[index + 12] == 0x39 || data[index + 12] == 0x83)) {
				this->GESTURE_MANAGER = _baseAddress + offset + index + 3;
				
				int addOffset = 0;
				ReadAbsolute(reinterpret_cast<LPVOID>(this->GESTURE_MANAGER), &addOffset, 0x4);

				this->GESTURE_MANAGER += addOffset + 0x4;
			}
		}

		return true;
	});
}

void Memory::findMovementSpeed() {
	executeSigScan({ 0xF3, 0x0F, 0x59, 0xFD, 0xF3, 0x0F, 0x5C, 0xC8 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		int found = 0;

		int back_index = index;
		for (; back_index < data.size(); back_index--) {
			if (data[back_index + 4] == 0x44 && data[back_index + 5] == 0x0F && data[back_index + 6] == 0x28 && data[back_index + 7] == 0xF8) {
				for (; back_index < data.size(); back_index--) {
					if (data[back_index - 4] == 0xF3 && data[back_index - 3] == 0x0F && data[back_index - 2] == 0x10) {
						found++;

						this->baseMovementSpeedAddress = _baseAddress + offset + back_index;

						uint64_t normalSpeedAddressAbsolute = _baseAddress + ReadStaticInt(offset, back_index, data);

						ReadProcessMemory(_handle, reinterpret_cast<LPCVOID>(this->baseMovementSpeedAddress), &this->normalSpeedRelativeAddress, sizeof(this->normalSpeedRelativeAddress), NULL); // Read the current address of the constant loaded into xmm0 relative to the instruction

						this->zeroSpeedRelativeAddress = this->normalSpeedRelativeAddress;

						executeSigScan({ 0x00, 0x00, 0x00, 0x00 }, [this](__int64 offset, int index, const std::vector<byte>& data) { // Find a 0x00000000 (0.0f) near the memory location of the original constant.
							this->zeroSpeedRelativeAddress += index;

							return true;
						}, normalSpeedAddressAbsolute); // Start this Sigscan for a new constant at the location of the original constant to find one "nearby"


						break;
					}
				}
				break;
			}
		}

		// This doesn't have a consistent offset from the scan, so search until we find "jmp +08"
		for (; index < data.size(); index++) {
			if (data[index - 2] == 0xEB && data[index - 1] == 0x08) {
				this->ACCELERATION = ReadStaticInt(offset, index - 0x06, data);
				this->DECELERATION = ReadStaticInt(offset, index + 0x04, data);
				found++;
				break;
			}
		}

		// Once again, there's no consistent offset, so we read until "movss xmm1, [addr]"
		for (; index < data.size(); index++) {
			if (data[index - 4] == 0xF3 && data[index - 3] == 0x0F && data[index - 2] == 0x10 && data[index - 1] == 0x0D) {
				this->RUNSPEED = ReadStaticInt(offset, index, data);
				found++;
				break;
			}
		}

		this->DEFAULTSPRINTSPEED = this->ReadData<float>({ RUNSPEED }, 1)[0];
		DEFAULTACCEL = this->ReadData<float>({ ACCELERATION }, 1)[0];
		DEFAULTDECEL = this->ReadData<float>({ DECELERATION }, 1)[0];

		return (found == 3);
	});
}

void Memory::findActivePanel() {
	executeSigScan({ 0xF2, 0x0F, 0x58, 0xC8, 0x66, 0x0F, 0x5A, 0xC1, 0xF2 }, [this](__int64 offset, int index, const std::vector<byte>& data) {
		this->ACTIVEPANELOFFSETS = {};
		this->ACTIVEPANELOFFSETS.push_back(ReadStaticInt(offset, index + 0x36, data, 5));
		this->ACTIVEPANELOFFSETS.push_back(data[index + 0x5A]); // This is 0x10 in both versions I have, but who knows.

		this->ACTIVEPANELOFFSETS.push_back(*(int*)&data[index + 0x54]);

		return true;
	});
}

int Memory::GetActivePanel() {
	return this->ReadData<int>(this->ACTIVEPANELOFFSETS, 1)[0] - 1;
}

__int64 Memory::ReadStaticInt(__int64 offset, int index, const std::vector<byte>& data, size_t bytesToEOL) {
	// (address of next line) + (index interpreted as 4byte int)
	return offset + index + bytesToEOL + *(int*)&data[index];
}

uint64_t Memory::executeSigScan(const std::vector<byte>& signatureBytes, const SigScanDelegate& scanFunc, uint64_t startAddress) {
	std::vector<byte> scanBuffer;
	scanBuffer.resize(SIGSCAN_STRIDE + SIGSCAN_PADDING); // padding in case the sigscan is past the end of the buffer

	SIZE_T numBytesWritten;
	for (uint64_t scanAddress = 0; scanAddress < PROGRAM_SIZE; scanAddress += SIGSCAN_STRIDE) {
		if (!ReadProcessMemory(_handle, reinterpret_cast<void*>(scanAddress + startAddress), &scanBuffer[0], scanBuffer.size(), &numBytesWritten)) continue;

		// Search for our target signature, testing our scan function against every match until we find a match or reach the end of our stride.
		int searchLength = std::min(numBytesWritten, SIGSCAN_STRIDE + signatureBytes.size());
		for (int matchIndex = 0; matchIndex < searchLength;) {
			// Find the next instance of the signature.
			matchIndex = Utilities::findSequence(scanBuffer, signatureBytes, matchIndex, searchLength);

			if (matchIndex == -1) {
				// We didn't find the signature in the remainder of this scan. Move to the next segment.
				break;
			}
			// Test the scan function for a match. Note that the function is expecting an address relative to the starting address.
			else if (scanFunc(scanAddress, matchIndex, scanBuffer)) {
				return scanAddress + matchIndex;
			}
		}
	}

	return UINT64_MAX;
}

uint64_t Memory::executeSigScan(const std::vector<byte>& signatureBytes, const SigScanDelegate& scanFunc) {
	return executeSigScan(signatureBytes, scanFunc, _baseAddress);
}

uint64_t Memory::executeSigScan(const std::vector<byte>& signatureBytes, uint64_t startAddress) {
	return executeSigScan(signatureBytes, [](uint64_t offset, int index, const std::vector<byte>& data) { return true; }, startAddress);
}

uint64_t Memory::executeSigScan(const std::vector<byte>& signatureBytes) {
	return executeSigScan(signatureBytes, [](uint64_t offset, int index, const std::vector<byte>& data) { return true; });
}

uint32_t Memory::scanForRelativeAddress(const std::vector<byte>& signatureBytes, uint32_t scanOffset, uint32_t valueOffset) {
	uint64_t scanResult = executeSigScan(signatureBytes, scanOffset + _baseAddress);
	if (scanResult == UINT64_MAX) {
		throw std::exception("Sigscan found no result when scanning for relative address.");
		return UINT32_MAX;
	}

	scanResult += signatureBytes.size() + valueOffset + scanOffset;
	uint32_t foundValue;
	if (!Memory::get()->ReadAbsolute(reinterpret_cast<void*>(scanResult + _baseAddress), &foundValue, 0x4)) {
		throw std::exception("Unable to read memory at found address when scanning for relative address.");
		return UINT32_MAX;
	}

	return foundValue + scanResult;
}

uint64_t Memory::getBaseAddress() const
{
	return _baseAddress;
}

void Memory::ThrowError(std::string message) {
	if (!showMsg) {
		ClientWindow* clientWindow = ClientWindow::get();
		if (clientWindow != nullptr) {
			// clientWindow->setErrorMessage("Most recent error: " + message);
		}
		throw std::exception(message.c_str());
	}
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

void* Memory::ComputeOffset(std::vector<int> offsets, bool forceRecalculatePointer)
{
	// Leave off the last offset, since it will be either read/write, and may not be of type unitptr_t.
	int final_offset = offsets.back();
	offsets.pop_back();

	uintptr_t cumulativeAddress = _baseAddress;
	for (const int offset : offsets) {
		cumulativeAddress += offset;

		const auto search = _computedAddresses.find(cumulativeAddress);
		if (search == std::end(_computedAddresses) || (forceRecalculatePointer && cumulativeAddress != _baseAddress + GLOBALS)) { // GLOBALS pointer can change temporarily for a split second.
			// If the address is not yet computed, then compute it.
			uintptr_t computedAddress = 0;
			if (!ReadAbsolute(reinterpret_cast<LPVOID>(cumulativeAddress), &computedAddress, sizeof(uintptr_t))) {
				ThrowError(offsets, false);
			}
			_computedAddresses[cumulativeAddress] = computedAddress;
		}

		cumulativeAddress = _computedAddresses[cumulativeAddress];
	}
	return reinterpret_cast<void*>(cumulativeAddress + final_offset);
}

uint64_t Memory::getSoundStream(uint64_t issuedSoundPointer) {
	char resultsBuffer[16];

	auto resultsPointer = VirtualAllocEx(_handle, NULL, sizeof(resultsBuffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	auto resultsAddress = reinterpret_cast<uint64_t>(resultsPointer);

	WriteProcessMemory(_handle, resultsPointer, resultsBuffer, sizeof(resultsBuffer), NULL);

	uint64_t entityManager;
	ReadAbsolute(reinterpret_cast<LPCVOID>(_baseAddress + GLOBALS), &entityManager, sizeof(uint64_t));

	int managerId;
	ReadAbsolute(reinterpret_cast<LPCVOID>(entityManager + 0x1A0), &managerId, sizeof(int));

	int portableIdOfIssuedSound;
	ReadAbsolute(reinterpret_cast<LPCVOID>(issuedSoundPointer + 0x10), &portableIdOfIssuedSound, sizeof(int));

	uint64_t soundPlayer;
	ReadAbsolute(reinterpret_cast<LPCVOID>(_baseAddress + GLOBALS + 0x478), &soundPlayer, sizeof(uint64_t)); // THIS WOULDN'T WORK IN OLD VERSION! Probably a better way to do this.

	unsigned char buffer[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax functionaddress
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx soundplayer
		"\x41\xB8\x00\x00\x00\x00" //mov r8d entity_manager_id
		"\xBA\x00\x00\x00\x00" //mov edx portableId
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\x48\xBA\x00\x00\x00\x00\x00\x00\x00\x00" //mov rdx result alloc
		"\x48\x89\x02" // mov [rdx], rax
		"\xC3"; //ret

	buffer[2] = getSoundStreamFunction & 0xff;
	buffer[3] = (getSoundStreamFunction >> 8) & 0xff;
	buffer[4] = (getSoundStreamFunction >> 16) & 0xff;
	buffer[5] = (getSoundStreamFunction >> 24) & 0xff;
	buffer[6] = (getSoundStreamFunction >> 32) & 0xff;
	buffer[7] = (getSoundStreamFunction >> 40) & 0xff;
	buffer[8] = (getSoundStreamFunction >> 48) & 0xff;
	buffer[9] = (getSoundStreamFunction >> 56) & 0xff;

	buffer[12] = soundPlayer & 0xff;
	buffer[13] = (soundPlayer >> 8) & 0xff;
	buffer[14] = (soundPlayer >> 16) & 0xff;
	buffer[15] = (soundPlayer >> 24) & 0xff;
	buffer[16] = (soundPlayer >> 32) & 0xff;
	buffer[17] = (soundPlayer >> 40) & 0xff;
	buffer[18] = (soundPlayer >> 48) & 0xff;
	buffer[19] = (soundPlayer >> 56) & 0xff;

	buffer[22] = managerId & 0xff;
	buffer[23] = (managerId >> 8) & 0xff;
	buffer[24] = (managerId >> 16) & 0xff;
	buffer[25] = (managerId >> 24) & 0xff;

	buffer[27] = portableIdOfIssuedSound & 0xff;
	buffer[28] = (portableIdOfIssuedSound >> 8) & 0xff;
	buffer[29] = (portableIdOfIssuedSound >> 16) & 0xff;
	buffer[30] = (portableIdOfIssuedSound >> 24) & 0xff;

	buffer[43] = resultsAddress & 0xff;
	buffer[44] = (resultsAddress >> 8) & 0xff;
	buffer[45] = (resultsAddress >> 16) & 0xff;
	buffer[46] = (resultsAddress >> 24) & 0xff;
	buffer[47] = (resultsAddress >> 32) & 0xff;
	buffer[48] = (resultsAddress >> 40) & 0xff;
	buffer[49] = (resultsAddress >> 48) & 0xff;
	buffer[50] = (resultsAddress >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(buffer);
	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, buffer, allocation_size, NULL);
	auto thread = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);

	WaitForSingleObject(thread, INFINITE);

	uint64_t result;
	ReadAbsolute(resultsPointer, &result, sizeof(uint64_t));

	return result;
}

void Memory::PowerNext(int source, int target) {
	uint64_t offset = reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, source * 8, 0 }));
	target += 1;

	unsigned char buffer[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\xB9\x00\x00\x00\x00" //mov ecx [address]
		"\x48\xBA\x00\x00\x00\x00\x00\x00\x00\x00" //mov rdx [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	buffer[2] = powerNextFunction & 0xff; //address of laser activation function
	buffer[3] = (powerNextFunction >> 8) & 0xff;
	buffer[4] = (powerNextFunction >> 16) & 0xff;
	buffer[5] = (powerNextFunction >> 24) & 0xff;
	buffer[6] = (powerNextFunction >> 32) & 0xff;
	buffer[7] = (powerNextFunction >> 40) & 0xff;
	buffer[8] = (powerNextFunction >> 48) & 0xff;
	buffer[9] = (powerNextFunction >> 56) & 0xff;
	buffer[11] = target & 0xff; //address of target
	buffer[12] = (target >> 8) & 0xff;
	buffer[13] = (target >> 16) & 0xff;
	buffer[14] = (target >> 24) & 0xff;
	buffer[17] = offset & 0xff; //address of source
	buffer[18] = (offset >> 8) & 0xff;
	buffer[19] = (offset >> 16) & 0xff;
	buffer[20] = (offset >> 24) & 0xff;
	buffer[21] = (offset >> 32) & 0xff;
	buffer[22] = (offset >> 40) & 0xff;
	buffer[23] = (offset >> 48) & 0xff;
	buffer[24] = (offset >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(buffer);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, buffer, allocation_size, NULL);
	CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);
}

void Memory::PowerGauge(int id, int sourceId, int slot) {
	uint64_t offset = reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, id * 8, 0 }));
	sourceId += 1;

	unsigned char buffer[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" // mov rax, function address
		"\xBA\x00\x00\x00\x00" // mov edx, source_id
		"\x41\xB8\x00\x00\x00\x00" // mov r8d, slot
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" // mov rcx, address of Gauge
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	buffer[2] = powerGaugeFunction & 0xff;
	buffer[3] = (powerGaugeFunction >> 8) & 0xff;
	buffer[4] = (powerGaugeFunction >> 16) & 0xff;
	buffer[5] = (powerGaugeFunction >> 24) & 0xff;
	buffer[6] = (powerGaugeFunction >> 32) & 0xff;
	buffer[7] = (powerGaugeFunction >> 40) & 0xff;
	buffer[8] = (powerGaugeFunction >> 48) & 0xff;
	buffer[9] = (powerGaugeFunction >> 56) & 0xff;
	buffer[11] = sourceId & 0xff; //source
	buffer[12] = (sourceId >> 8) & 0xff;
	buffer[13] = (sourceId >> 16) & 0xff;
	buffer[14] = (sourceId >> 24) & 0xff;
	buffer[17] = slot & 0xff; //slot
	buffer[18] = (slot >> 8) & 0xff;
	buffer[19] = (slot >> 16) & 0xff;
	buffer[20] = (slot >> 24) & 0xff;
	buffer[23] = offset & 0xff;
	buffer[24] = (offset >> 8) & 0xff;
	buffer[25] = (offset >> 16) & 0xff;
	buffer[26] = (offset >> 24) & 0xff;
	buffer[27] = (offset >> 32) & 0xff;
	buffer[28] = (offset >> 40) & 0xff;
	buffer[29] = (offset >> 48) & 0xff;
	buffer[30] = (offset >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(buffer);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, buffer, allocation_size, NULL);
	CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);
}

void Memory::CallVoidFunction(int id, uint64_t functionAdress) {
	uint64_t offset = reinterpret_cast<uintptr_t>(ComputeOffset({ GLOBALS, 0x18, id * 8, 0 }));

	unsigned char buffer[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	buffer[2] = functionAdress & 0xff; //address of laser activation function
	buffer[3] = (functionAdress >> 8) & 0xff;
	buffer[4] = (functionAdress >> 16) & 0xff;
	buffer[5] = (functionAdress >> 24) & 0xff;
	buffer[6] = (functionAdress >> 32) & 0xff;
	buffer[7] = (functionAdress >> 40) & 0xff;
	buffer[8] = (functionAdress >> 48) & 0xff;
	buffer[9] = (functionAdress >> 56) & 0xff;
	buffer[12] = offset & 0xff; //address of laser
	buffer[13] = (offset >> 8) & 0xff;
	buffer[14] = (offset >> 16) & 0xff;
	buffer[15] = (offset >> 24) & 0xff;
	buffer[16] = (offset >> 32) & 0xff;
	buffer[17] = (offset >> 40) & 0xff;
	buffer[18] = (offset >> 48) & 0xff;
	buffer[19] = (offset >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(buffer);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, buffer, allocation_size, NULL);
	HANDLE thread = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);

	WaitForSingleObject(thread, INFINITE);
}

void Memory::EnableMovement(bool enable) {
	if (enable) {
		WriteProcessMemory(_handle, reinterpret_cast<LPVOID>(baseMovementSpeedAddress), &normalSpeedRelativeAddress, sizeof(normalSpeedRelativeAddress), NULL);
		this->WriteData<float>({ ACCELERATION }, { DEFAULTACCEL * 10.0f });
		this->WriteData<float>({ DECELERATION }, { DEFAULTDECEL * 10.0f });
	}
	else {
		WriteProcessMemory(_handle, reinterpret_cast<LPVOID>(baseMovementSpeedAddress), &zeroSpeedRelativeAddress, sizeof(zeroSpeedRelativeAddress), NULL);
		this->WriteData<float>({ ACCELERATION }, { DEFAULTACCEL * 1.0f });
		this->WriteData<float>({ DECELERATION }, { DEFAULTDECEL * 1.0f });
	}
}

void Memory::EnableVision(bool enable) {
	LPVOID addressPointer = reinterpret_cast<LPVOID>(this->setUserBrightnessFunction);

	char currentByte = 0x00;
	char disabledByte = 0xC3;
	char enabledByte = 0x48;
	ReadProcessMemory(_handle, addressPointer, &currentByte, sizeof(currentByte), NULL);

	if (enable && currentByte == disabledByte) {
		WriteProcessMemory(_handle, addressPointer, &enabledByte, sizeof(enabledByte), NULL);
	}
	else if (!enable && currentByte == enabledByte) {
		WriteProcessMemory(_handle, addressPointer, &disabledByte, sizeof(disabledByte), NULL);
	}
}

std::pair<float, float> Memory::MoveVisionTowards(float target, float deltaAbs) {
	uint64_t deviceDisplayActual;
	ReadProcessMemory(_handle, reinterpret_cast<LPVOID>(this->deviceDisplay), &deviceDisplayActual, sizeof(uint64_t), NULL);

	float brightnessBefore;
	float brightness;

	ReadProcessMemory(_handle, reinterpret_cast<LPVOID>(deviceDisplayActual + 0x2C), &brightnessBefore, sizeof(float), NULL);
	brightness = brightnessBefore;

	if (brightness == target) return { brightnessBefore, brightness };

	if (brightness < target) {
		brightness += deltaAbs;
		if (brightness > target) brightness = target;
	}

	if (brightness > target) {
		brightness -= deltaAbs;
		if (brightness < target) brightness = target;
	}

	WriteProcessMemory(_handle, reinterpret_cast<LPVOID>(deviceDisplayActual + 0x2C), &brightness, sizeof(float), NULL);

	return { brightnessBefore, brightness };
}

void Memory::EnableSolveMode(bool enable) {
	LPVOID addressPointer = reinterpret_cast<LPVOID>(this->enterSolveModeFunction);

	char currentByte = 0x00;
	char disabledByte = 0xC3;
	char enabledByte = 0x48;
	ReadProcessMemory(_handle, addressPointer, &currentByte, sizeof(currentByte), NULL);

	if (enable && currentByte == disabledByte) {
		WriteProcessMemory(_handle, addressPointer, &enabledByte, sizeof(enabledByte), NULL);
	}
	else if (!enable && currentByte == enabledByte) {
		WriteProcessMemory(_handle, addressPointer, &disabledByte, sizeof(disabledByte), NULL);
	}
}

void Memory::ExitSolveMode() {
	unsigned char buffer[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	buffer[2] = this->exitSolveModeFunction & 0xff; //address of laser activation function
	buffer[3] = (this->exitSolveModeFunction >> 8) & 0xff;
	buffer[4] = (this->exitSolveModeFunction >> 16) & 0xff;
	buffer[5] = (this->exitSolveModeFunction >> 24) & 0xff;
	buffer[6] = (this->exitSolveModeFunction >> 32) & 0xff;
	buffer[7] = (this->exitSolveModeFunction >> 40) & 0xff;
	buffer[8] = (this->exitSolveModeFunction >> 48) & 0xff;
	buffer[9] = (this->exitSolveModeFunction >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(buffer);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, buffer, allocation_size, NULL);
	HANDLE thread = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);

	WaitForSingleObject(thread, INFINITE);
}

void Memory::WriteMovementSpeed(float speed) {
	std::lock_guard<std::recursive_mutex> lock(mtx);
	this->WriteData<float>({ RUNSPEED }, { DEFAULTSPRINTSPEED * speed });
	this->WriteData<float>({ ACCELERATION }, { DEFAULTACCEL * speed });
	this->WriteData<float>({ DECELERATION }, { DEFAULTDECEL * speed });
}

void Memory::DisplayHudMessage(std::string message, std::array<float, 3> rgbColor) {
	char buffer[1024];

	if (!_messageAddress) {
		_messageAddress = VirtualAllocEx(_handle, NULL, sizeof(buffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		__int64 address = hudTimePointer;
		LPVOID addressPointer = reinterpret_cast<LPVOID>(address);
		__int32 addressOf6 = relativeAddressOf6;

		WriteAbsolute(addressPointer, &addressOf6, sizeof(addressOf6));
	}

	strcpy_s(buffer, message.c_str());

	WriteProcessMemory(_handle, _messageAddress, buffer, sizeof(buffer), NULL);

	// Write the message's color values to the addresses of the constants we previously found.
	const SIZE_T colorSize = sizeof(float);
	for (int colorIndex = 0; colorIndex < 3; colorIndex++)
	{
		void* writeAddress = reinterpret_cast<void*>(hudMessageColorAddresses[colorIndex]);
		void* readAddress = reinterpret_cast<void*>(&rgbColor[colorIndex]);

		WriteProcessMemory(_handle, writeAddress, readAddress, colorSize, NULL);
	}

	__int64 funcAdress = displayHudFunction;
	__int64 messageAddress = reinterpret_cast<__int64>(_messageAddress);

	unsigned char asmBuff[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	asmBuff[2] = funcAdress & 0xff;
	asmBuff[3] = (funcAdress >> 8) & 0xff;
	asmBuff[4] = (funcAdress >> 16) & 0xff;
	asmBuff[5] = (funcAdress >> 24) & 0xff;
	asmBuff[6] = (funcAdress >> 32) & 0xff;
	asmBuff[7] = (funcAdress >> 40) & 0xff;
	asmBuff[8] = (funcAdress >> 48) & 0xff;
	asmBuff[9] = (funcAdress >> 56) & 0xff;
	asmBuff[12] = messageAddress & 0xff;
	asmBuff[13] = (messageAddress >> 8) & 0xff;
	asmBuff[14] = (messageAddress >> 16) & 0xff;
	asmBuff[15] = (messageAddress >> 24) & 0xff;
	asmBuff[16] = (messageAddress >> 32) & 0xff;
	asmBuff[17] = (messageAddress >> 40) & 0xff;
	asmBuff[18] = (messageAddress >> 48) & 0xff;
	asmBuff[19] = (messageAddress >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(asmBuff);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, asmBuff, allocation_size, NULL);
	CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);
}

void Memory::RemoveMesh(int id) {
	__int64 meshPointer = ReadPanelData<__int64>(id, 0x60); //Mesh
	
	__int64 buffer[1];
	buffer[0] = 0;

	__int64 collisionMesh = WriteAbsolute(reinterpret_cast<LPVOID>(meshPointer + 0x98), buffer, sizeof(buffer)); //Collision Mesh
}

void Memory::DoFullPositionUpdate() {
	this->WriteData<byte>({ GLOBALS, 0x278 + 0x39 }, { 1 });
	return;
}

void Memory::MakeEPGlow(std::string name, std::vector<byte> patternPointBytes) {
	char buffer[2048];

	memset(buffer, 0, sizeof(buffer));

	for (int i = 0; i < name.size(); i++) {
		buffer[0x200 + i] = name[i];
	}

	auto alloc = VirtualAllocEx(_handle, NULL, sizeof(buffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	uint64_t allocStart = reinterpret_cast<uint64_t>(alloc);
	uint64_t patternSolvedPart = allocStart + 0x100;
	uint64_t namePointer = allocStart + 0x200;
	uint64_t patternPointStart = allocStart + 0x300;

	for (int i = 0; i < patternPointBytes.size(); i++) {
		buffer[0x300 + i] = patternPointBytes[i];
	}

	byte elements = patternPointBytes.size() / 4;

	buffer[0x100] = elements;

	buffer[0x104] = elements;

	buffer[0x108] = (patternPointStart) & 0xff;
	buffer[0x109] = (patternPointStart >> 8) & 0xff;
	buffer[0x10A] = (patternPointStart >> 16) & 0xff;
	buffer[0x10B] = (patternPointStart >> 24) & 0xff;
	buffer[0x10C] = (patternPointStart >> 32) & 0xff;
	buffer[0x10D] = (patternPointStart >> 40) & 0xff;
	buffer[0x10E] = (patternPointStart >> 48) & 0xff;
	buffer[0x10F] = (patternPointStart >> 56) & 0xff;

	WriteProcessMemory(_handle, alloc, buffer, sizeof(buffer), NULL);

	/*unsigned char removeBuff[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\x48\xBA\x00\x00\x00\x00\x00\x00\x00\x00" //mov rdx [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	removeBuff[2] = removeFromPatternMapFunction & 0xff;
	removeBuff[3] = (removeFromPatternMapFunction >> 8) & 0xff;
	removeBuff[4] = (removeFromPatternMapFunction >> 16) & 0xff;
	removeBuff[5] = (removeFromPatternMapFunction >> 24) & 0xff;
	removeBuff[6] = (removeFromPatternMapFunction >> 32) & 0xff;
	removeBuff[7] = (removeFromPatternMapFunction >> 40) & 0xff;
	removeBuff[8] = (removeFromPatternMapFunction >> 48) & 0xff;
	removeBuff[9] = (removeFromPatternMapFunction >> 56) & 0xff;
	removeBuff[12] = patternMap & 0xff;
	removeBuff[13] = (patternMap >> 8) & 0xff;
	removeBuff[14] = (patternMap >> 16) & 0xff;
	removeBuff[15] = (patternMap >> 24) & 0xff;
	removeBuff[16] = (patternMap >> 32) & 0xff;
	removeBuff[17] = (patternMap >> 40) & 0xff;
	removeBuff[18] = (patternMap >> 48) & 0xff;
	removeBuff[19] = (patternMap >> 56) & 0xff;
	removeBuff[22] = namePointer & 0xff;
	removeBuff[23] = (namePointer >> 8) & 0xff;
	removeBuff[24] = (namePointer >> 16) & 0xff;
	removeBuff[25] = (namePointer >> 24) & 0xff;
	removeBuff[26] = (namePointer >> 32) & 0xff;
	removeBuff[27] = (namePointer >> 40) & 0xff;
	removeBuff[28] = (namePointer >> 48) & 0xff;
	removeBuff[29] = (namePointer >> 56) & 0xff;

	SIZE_T allocation_size = sizeof(removeBuff);

	LPVOID allocation_start = VirtualAllocEx(_handle, NULL, allocation_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start, removeBuff, allocation_size, NULL);
	HANDLE thread = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start, NULL, 0, 0);

	WaitForSingleObject(thread, INFINITE);*/



	unsigned char addBuff[] =
		"\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov rax [address]
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\x48\xBA\x00\x00\x00\x00\x00\x00\x00\x00" //mov rdx [address]
		"\x49\xB8\x00\x00\x00\x00\x00\x00\x00\x00" //mov r8 [address]
		"\x48\x83\xEC\x48" // sub rsp,48
		"\xFF\xD0" //call rax
		"\x48\x83\xC4\x48" // add rsp,48
		"\xC3"; //ret

	addBuff[2] = addToPatternMapFunction & 0xff;
	addBuff[3] = (addToPatternMapFunction >> 8) & 0xff;
	addBuff[4] = (addToPatternMapFunction >> 16) & 0xff;
	addBuff[5] = (addToPatternMapFunction >> 24) & 0xff;
	addBuff[6] = (addToPatternMapFunction >> 32) & 0xff;
	addBuff[7] = (addToPatternMapFunction >> 40) & 0xff;
	addBuff[8] = (addToPatternMapFunction >> 48) & 0xff;
	addBuff[9] = (addToPatternMapFunction >> 56) & 0xff;
	addBuff[12] = patternMap & 0xff;
	addBuff[13] = (patternMap >> 8) & 0xff;
	addBuff[14] = (patternMap >> 16) & 0xff;
	addBuff[15] = (patternMap >> 24) & 0xff;
	addBuff[16] = (patternMap >> 32) & 0xff;
	addBuff[17] = (patternMap >> 40) & 0xff;
	addBuff[18] = (patternMap >> 48) & 0xff;
	addBuff[19] = (patternMap >> 56) & 0xff;
	addBuff[22] = namePointer & 0xff;
	addBuff[23] = (namePointer >> 8) & 0xff;
	addBuff[24] = (namePointer >> 16) & 0xff;
	addBuff[25] = (namePointer >> 24) & 0xff;
	addBuff[26] = (namePointer >> 32) & 0xff;
	addBuff[27] = (namePointer >> 40) & 0xff;
	addBuff[28] = (namePointer >> 48) & 0xff;
	addBuff[29] = (namePointer >> 56) & 0xff;
	addBuff[32] = patternSolvedPart & 0xff;
	addBuff[33] = (patternSolvedPart >> 8) & 0xff;
	addBuff[34] = (patternSolvedPart >> 16) & 0xff;
	addBuff[35] = (patternSolvedPart >> 24) & 0xff;
	addBuff[36] = (patternSolvedPart >> 32) & 0xff;
	addBuff[37] = (patternSolvedPart >> 40) & 0xff;
	addBuff[38] = (patternSolvedPart >> 48) & 0xff;
	addBuff[39] = (patternSolvedPart >> 56) & 0xff;

	SIZE_T allocation_size2 = sizeof(addBuff);

	auto allocation_start2 = VirtualAllocEx(_handle, NULL, allocation_size2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(_handle, allocation_start2, addBuff, allocation_size2, NULL);
	auto thread2 = CreateRemoteThread(_handle, NULL, 0, (LPTHREAD_START_ROUTINE)allocation_start2, NULL, 0, 0);

	WaitForSingleObject(thread2, INFINITE);
}