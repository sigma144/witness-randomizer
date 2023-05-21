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

void ASMPayloadManager::OpenDoor(int id) {
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
	buffer[20] = memory->openDoorFunction & 0xff;
	buffer[21] = (memory->openDoorFunction >> 8) & 0xff;
	buffer[22] = (memory->openDoorFunction >> 16) & 0xff;
	buffer[23] = (memory->openDoorFunction >> 24) & 0xff;
	buffer[24] = (memory->openDoorFunction >> 32) & 0xff;
	buffer[25] = (memory->openDoorFunction >> 40) & 0xff;
	buffer[26] = (memory->openDoorFunction >> 48) & 0xff;
	buffer[27] = (memory->openDoorFunction >> 56) & 0xff;

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