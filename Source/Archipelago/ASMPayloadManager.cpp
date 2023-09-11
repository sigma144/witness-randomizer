#include "ASMPayloadManager.h"

#include "../Memory.h"

ASMPayloadManager* ASMPayloadManager::_singleton = nullptr;

void ASMPayloadManager::create() {
	if (_singleton == nullptr) {
		_singleton = new ASMPayloadManager();

		_singleton->findGameloopFunction();
		_singleton->setupPayload();
	}
}

ASMPayloadManager::ASMPayloadManager(){
	return;
}

ASMPayloadManager* ASMPayloadManager::get() {
	return _singleton;
}

void ASMPayloadManager::findGameloopFunction() {
	Memory* memory = Memory::get();

	memory->ReadAbsolute(reinterpret_cast<LPVOID>(memory->gameLoop3CallsInARow + 1), &call1AbsoluteAddress, 4);
	memory->ReadAbsolute(reinterpret_cast<LPVOID>(memory->gameLoop3CallsInARow + 6), &call2AbsoluteAddress, 4);
	memory->ReadAbsolute(reinterpret_cast<LPVOID>(memory->gameLoop3CallsInARow + 11), &call3AbsoluteAddress, 4);

	call1AbsoluteAddress += memory->gameLoop3CallsInARow + 5;
	call2AbsoluteAddress += memory->gameLoop3CallsInARow + 10;
	call3AbsoluteAddress += memory->gameLoop3CallsInARow + 15;
}

void ASMPayloadManager::setupPayload() {
	Memory* memory = Memory::get();

	payloadBlocked = reinterpret_cast<uint64_t>(VirtualAllocEx(memory->getHandle(), NULL, 0x4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	payloadStart = payloadBlocked + 8;

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(payloadBlocked), "\x00\x00\x00\x00\x00\x00\x00\x00", 8);

	char asmBuff[] =
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00" // call far to absolute address
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x83\x3D\xC1\xFF\xFF\xFF\x01"
		"\x74\x0E" // je 15
		"\xFF\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	uint64_t returnpoint = memory->gameLoop3CallsInARow + 15;

	asmBuff[8] = call1AbsoluteAddress & 0xff;
	asmBuff[9] = (call1AbsoluteAddress >> 8) & 0xff;
	asmBuff[10] = (call1AbsoluteAddress >> 16) & 0xff;
	asmBuff[11] = (call1AbsoluteAddress >> 24) & 0xff;
	asmBuff[12] = (call1AbsoluteAddress >> 32) & 0xff;
	asmBuff[13] = (call1AbsoluteAddress >> 40) & 0xff;
	asmBuff[14] = (call1AbsoluteAddress >> 48) & 0xff;
	asmBuff[15] = (call1AbsoluteAddress >> 56) & 0xff;
	asmBuff[24] = call2AbsoluteAddress & 0xff;
	asmBuff[25] = (call2AbsoluteAddress >> 8) & 0xff;
	asmBuff[26] = (call2AbsoluteAddress >> 16) & 0xff;
	asmBuff[27] = (call2AbsoluteAddress >> 24) & 0xff;
	asmBuff[28] = (call2AbsoluteAddress >> 32) & 0xff;
	asmBuff[29] = (call2AbsoluteAddress >> 40) & 0xff;
	asmBuff[30] = (call2AbsoluteAddress >> 48) & 0xff;
	asmBuff[31] = (call2AbsoluteAddress >> 56) & 0xff;
	asmBuff[40] = call3AbsoluteAddress & 0xff;
	asmBuff[41] = (call3AbsoluteAddress >> 8) & 0xff;
	asmBuff[42] = (call3AbsoluteAddress >> 16) & 0xff;
	asmBuff[43] = (call3AbsoluteAddress >> 24) & 0xff;
	asmBuff[44] = (call3AbsoluteAddress >> 32) & 0xff;
	asmBuff[45] = (call3AbsoluteAddress >> 40) & 0xff;
	asmBuff[46] = (call3AbsoluteAddress >> 48) & 0xff;
	asmBuff[47] = (call3AbsoluteAddress >> 56) & 0xff;
	asmBuff[63] = returnpoint & 0xff;
	asmBuff[64] = (returnpoint >> 8) & 0xff;
	asmBuff[65] = (returnpoint >> 16) & 0xff;
	asmBuff[66] = (returnpoint >> 24) & 0xff;
	asmBuff[67] = (returnpoint >> 32) & 0xff;
	asmBuff[68] = (returnpoint >> 40) & 0xff;
	asmBuff[69] = (returnpoint >> 48) & 0xff;
	asmBuff[70] = (returnpoint >> 56) & 0xff;

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(payloadStart), asmBuff, sizeof(asmBuff) - 1);

	char redirectAsmBuff[] =
		"\xFF\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //jmp to start of payload
		"\x90"; // nop;

	redirectAsmBuff[6] = payloadStart & 0xff;
	redirectAsmBuff[7] = (payloadStart >> 8) & 0xff;
	redirectAsmBuff[8] = (payloadStart >> 16) & 0xff;
	redirectAsmBuff[9] = (payloadStart >> 24) & 0xff;
	redirectAsmBuff[10] = (payloadStart >> 32) & 0xff;
	redirectAsmBuff[11] = (payloadStart >> 40) & 0xff;
	redirectAsmBuff[12] = (payloadStart >> 48) & 0xff;
	redirectAsmBuff[13] = (payloadStart >> 56) & 0xff;

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(memory->gameLoop3CallsInARow), redirectAsmBuff, sizeof(redirectAsmBuff) - 1);

	payloadASMStart = payloadStart + sizeof(asmBuff) - 1;

	return;
}

void ASMPayloadManager::CallVoidFunction(uint64_t functionAddress, int id) {
	Memory* memory = Memory::get();

	uint64_t offset = reinterpret_cast<uintptr_t>(memory->ComputeOffset({ memory->GLOBALS, 0x18, id * 8, 0 }));

	char buffer[] =
		"\x51" // push rcx
		"\x51" // push rcx
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x59" // pop rcx
		"\x59"; // pop rcx

	buffer[4] = offset & 0xff; //address of door
	buffer[5] = (offset >> 8) & 0xff;
	buffer[6] = (offset >> 16) & 0xff;
	buffer[7] = (offset >> 24) & 0xff;
	buffer[8] = (offset >> 32) & 0xff;
	buffer[9] = (offset >> 40) & 0xff;
	buffer[10] = (offset >> 48) & 0xff;
	buffer[11] = (offset >> 56) & 0xff;
	buffer[20] = functionAddress & 0xff;
	buffer[21] = (functionAddress >> 8) & 0xff;
	buffer[22] = (functionAddress >> 16) & 0xff;
	buffer[23] = (functionAddress >> 24) & 0xff;
	buffer[24] = (functionAddress >> 32) & 0xff;
	buffer[25] = (functionAddress >> 40) & 0xff;
	buffer[26] = (functionAddress >> 48) & 0xff;
	buffer[27] = (functionAddress >> 56) & 0xff;

	ExecuteASM(buffer, sizeof(buffer));
}

void ASMPayloadManager::OpenDoor(int id) {
	Memory* memory = Memory::get();
	
	CallVoidFunction(memory->openDoorFunction, id);
}

void ASMPayloadManager::CloseDoor(int id) {
	Memory* memory = Memory::get();

	CallVoidFunction(memory->closeDoorFunction, id);
}

void ASMPayloadManager::UpdateEntityPosition(int id) {
	Memory* memory = Memory::get();

	CallVoidFunction(memory->updateEntityPositionFunction, id);
}

void ASMPayloadManager::BridgeToggle(int associatedPanel, bool extend) {
	Memory* memory = Memory::get();

	uint64_t offset = reinterpret_cast<uintptr_t>(memory->ComputeOffset({ memory->GLOBALS, 0x18, associatedPanel * 8, 0 }));
	uint64_t functionAddress = memory->bridgeToggleFunction;

	char buffer[] =
		"\x51" // push rcx
		"\x51" // push rcx
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\xB2\x00" //mov dl, extend
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x59" // pop rcx
		"\x59"; // pop rcx

	buffer[4] = offset & 0xff; //address of door
	buffer[5] = (offset >> 8) & 0xff;
	buffer[6] = (offset >> 16) & 0xff;
	buffer[7] = (offset >> 24) & 0xff;
	buffer[8] = (offset >> 32) & 0xff;
	buffer[9] = (offset >> 40) & 0xff;
	buffer[10] = (offset >> 48) & 0xff;
	buffer[11] = (offset >> 56) & 0xff;
	buffer[13] = extend & 0xff;
	buffer[22] = functionAddress & 0xff;
	buffer[23] = (functionAddress >> 8) & 0xff;
	buffer[24] = (functionAddress >> 16) & 0xff;
	buffer[25] = (functionAddress >> 24) & 0xff;
	buffer[26] = (functionAddress >> 32) & 0xff;
	buffer[27] = (functionAddress >> 40) & 0xff;
	buffer[28] = (functionAddress >> 48) & 0xff;
	buffer[29] = (functionAddress >> 56) & 0xff;

	ExecuteASM(buffer, sizeof(buffer));
}

void ASMPayloadManager::SendBunkerElevatorToFloor(int floor, bool force) {
	Memory* memory = Memory::get();

	int id = 0x0A079;
	uint64_t offset = reinterpret_cast<uintptr_t>(memory->ComputeOffset({ memory->GLOBALS, 0x18, id * 8, 0 }));
	uint64_t functionAddress = memory->moveBunkerElevatorFunction;

	char buffer[] =
		"\x51" // push rcx
		"\x51" // push rcx
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\xBA\x00\x00\x00\x00" //mov edx floornumber
		"\x49\xB8\x00\x00\x00\x00\x00\x00\x00\x00" // mov r8, force
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x59" // pop rcx
		"\x59"; // pop rcx

	buffer[4] = offset & 0xff; //address of door
	buffer[5] = (offset >> 8) & 0xff;
	buffer[6] = (offset >> 16) & 0xff;
	buffer[7] = (offset >> 24) & 0xff;
	buffer[8] = (offset >> 32) & 0xff;
	buffer[9] = (offset >> 40) & 0xff;
	buffer[10] = (offset >> 48) & 0xff;
	buffer[11] = (offset >> 56) & 0xff;
	buffer[13] = floor & 0xff; // floor no
	buffer[19] = force & 0xff; // force
	buffer[35] = functionAddress & 0xff;
	buffer[36] = (functionAddress >> 8) & 0xff;
	buffer[37] = (functionAddress >> 16) & 0xff;
	buffer[38] = (functionAddress >> 24) & 0xff;
	buffer[39] = (functionAddress >> 32) & 0xff;
	buffer[40] = (functionAddress >> 40) & 0xff;
	buffer[41] = (functionAddress >> 48) & 0xff;
	buffer[42] = (functionAddress >> 56) & 0xff;

	ExecuteASM(buffer, sizeof(buffer));
}

void ASMPayloadManager::ToggleFloodgate(std::string name, bool disconnect) {
	Memory* memory = Memory::get();

	char namebuffer[128];

	memset(namebuffer, 0, sizeof(namebuffer));

	strcpy_s(namebuffer, name.c_str());

	auto nameAlloc = VirtualAllocEx(Memory::get()->getHandle(), NULL, sizeof(namebuffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	WriteProcessMemory(Memory::get()->getHandle(), nameAlloc, namebuffer, sizeof(namebuffer), NULL);

	uint64_t nameAllocPointer = reinterpret_cast<uint64_t>(nameAlloc); nameAllocPointer;

	uint64_t functionAddress = memory->floodgateToggleFunction;

	char buffer[] =
		"\x51" // push rcx
		"\x51" // push rcx
		"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00" //mov rcx [address]
		"\xB2\x00" //mov dl, connect
		"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x59" // pop rcx
		"\x59"; // pop rcx

	buffer[4] = nameAllocPointer & 0xff; //address of door
	buffer[5] = (nameAllocPointer >> 8) & 0xff;
	buffer[6] = (nameAllocPointer >> 16) & 0xff;
	buffer[7] = (nameAllocPointer >> 24) & 0xff;
	buffer[8] = (nameAllocPointer >> 32) & 0xff;
	buffer[9] = (nameAllocPointer >> 40) & 0xff;
	buffer[10] = (nameAllocPointer >> 48) & 0xff;
	buffer[11] = (nameAllocPointer >> 56) & 0xff;
	buffer[13] = disconnect & 0xff;
	buffer[22] = functionAddress & 0xff;
	buffer[23] = (functionAddress >> 8) & 0xff;
	buffer[24] = (functionAddress >> 16) & 0xff;
	buffer[25] = (functionAddress >> 24) & 0xff;
	buffer[26] = (functionAddress >> 32) & 0xff;
	buffer[27] = (functionAddress >> 40) & 0xff;
	buffer[28] = (functionAddress >> 48) & 0xff;
	buffer[29] = (functionAddress >> 56) & 0xff;

	ExecuteASM(buffer, sizeof(buffer));
}

void ASMPayloadManager::ExecuteASM(char* asmBuff, int buffersize) {
	Memory* memory = Memory::get();

	while (true) {
		uint64_t blockedState = 1;
		memory->ReadAbsolute(reinterpret_cast<LPVOID>(payloadBlocked), &blockedState, 8);
		if (blockedState == 0) break;
	}

	uint64_t returnpoint = memory->gameLoop3CallsInARow + 15;

	uint64_t endAsmPoint = payloadASMStart + buffersize - 1;
	uint64_t diff = payloadBlocked - endAsmPoint - 0xA;

	char endBuff[] = 
		"\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00" // mov payload-block 0
		"\xFF\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	endBuff[2] = diff & 0xff;
	endBuff[3] = (diff >> 8) & 0xff;
	endBuff[4] = (diff >> 16) & 0xff;
	endBuff[5] = (diff >> 24) & 0xff;

	endBuff[16] = returnpoint & 0xff;
	endBuff[17] = (returnpoint >> 8) & 0xff;
	endBuff[18] = (returnpoint >> 16) & 0xff;
	endBuff[19] = (returnpoint >> 24) & 0xff;
	endBuff[20] = (returnpoint >> 32) & 0xff;
	endBuff[21] = (returnpoint >> 40) & 0xff;
	endBuff[22] = (returnpoint >> 48) & 0xff;
	endBuff[23] = (returnpoint >> 56) & 0xff;

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(endAsmPoint), endBuff, sizeof(endBuff) - 1);
	memory->WriteAbsolute(reinterpret_cast<LPVOID>(payloadASMStart), asmBuff, buffersize - 1);

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(payloadBlocked), "\x01", 1);

	return;
}