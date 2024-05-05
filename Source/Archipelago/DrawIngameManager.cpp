#include "DrawIngameManager.h"
#include "../Memory.h"

DrawIngameManager* DrawIngameManager::_singleton = nullptr;

DrawIngameManager::DrawIngameManager() {
	findRelevantFunctions();
	allocatePayloads();
	initialPatch();
}

void DrawIngameManager::create() {
	if (_singleton == nullptr) {
		_singleton = new DrawIngameManager();
	}
}

DrawIngameManager* DrawIngameManager::get() {
	return _singleton;
}

void DrawIngameManager::findRelevantFunctions() {
	Memory* memory = Memory::get();

	drawSphereCheap = memory->getBaseAddress() + memory->executeSigScan({ 0x48, 0x81, 0xEC, 0xB8, 0x05, 0x00, 0x00, 0xF3, 0x0F, 0x10, 0x11, 0xF3, 0x0F, 0x10, 0x59, 0x04, 0xF3, 0x0F, 0x10, 0x61, 0x08, 0x33 });
	drawEntityRenderListFunction = memory->getBaseAddress() + memory->executeSigScan({ 0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x8B, 0x42 });
}

void DrawIngameManager::allocatePayloads() {
	Memory* memory = Memory::get();

	address_drawSpherePayload_1 = (uint64_t)VirtualAllocEx(memory->getHandle(), NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	address_drawSpherePayload_2 = address_drawSpherePayload_1 + 0x2000;
	spheres_1 = (uint64_t)VirtualAllocEx(memory->getHandle(), NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	spheres_2 = spheres_1 + 0x2000;
}

void DrawIngameManager::initialPatch() {
	Memory* memory = Memory::get();
	auto handle = memory->getHandle();

	char asmBuff[] = // Original Function Bytes
		"\x48\x89\x5C\x24\x18\x48\x89\x74\x24\x20\x57\x48\x83\xEC\x20"
		"\x53"
		"\x66\x0F\x7E\xCB"
		"\x53"
		"\x51\x41\x50\x52\x53\x53\x53\x53\x53";

	WriteProcessMemory(handle, reinterpret_cast<LPVOID>(address_drawSpherePayload_1), asmBuff, sizeof(asmBuff) - 1, NULL);
	WriteProcessMemory(handle, reinterpret_cast<LPVOID>(address_drawSpherePayload_2), asmBuff, sizeof(asmBuff) - 1, NULL);

	address_drawSpherePayload_1_postpatch = address_drawSpherePayload_1 + 0x1E;
	address_drawSpherePayload_2_postpatch = address_drawSpherePayload_2 + 0x1E;

	writeCapBytes(address_drawSpherePayload_1_postpatch);
	writeCapBytes(address_drawSpherePayload_2_postpatch);

	switchBuffers();
}

void DrawIngameManager::writeCapBytes(uint64_t capStart) {
	Memory* memory = Memory::get();
	auto handle = memory->getHandle();

	char capBytes[] =
		"\x5B\x5B\x5B\x5B\x5B\x5A\x41\x58\x59\x5B"
		"\x66\x0F\x6E\xCB"
		"\x5B"
		"\xFF\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	uint64_t backEntry = drawEntityRenderListFunction + 0xF;

	capBytes[21] = backEntry & 0xff;
	capBytes[22] = (backEntry >> 8) & 0xff;
	capBytes[23] = (backEntry >> 16) & 0xff;
	capBytes[24] = (backEntry >> 24) & 0xff;
	capBytes[25] = (backEntry >> 32) & 0xff;
	capBytes[26] = (backEntry >> 40) & 0xff;
	capBytes[27] = (backEntry >> 48) & 0xff;
	capBytes[28] = (backEntry >> 56) & 0xff;

	WriteProcessMemory(handle, reinterpret_cast<LPVOID>(capStart), capBytes, sizeof(capBytes) - 1, NULL);
}

void DrawIngameManager::switchBuffers() {
	Memory* memory = Memory::get();
	auto handle = memory->getHandle();

	currentBufferIs1 = !currentBufferIs1;
	uint64_t correctBuffer = currentBufferIs1 ? address_drawSpherePayload_1 : address_drawSpherePayload_2;

	char jmpBytes[] =
		"\xFF\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	jmpBytes[6] = correctBuffer & 0xff;
	jmpBytes[7] = (correctBuffer >> 8) & 0xff;
	jmpBytes[8] = (correctBuffer >> 16) & 0xff;
	jmpBytes[9] = (correctBuffer >> 24) & 0xff;
	jmpBytes[10] = (correctBuffer >> 32) & 0xff;
	jmpBytes[11] = (correctBuffer >> 40) & 0xff;
	jmpBytes[12] = (correctBuffer >> 48) & 0xff;
	jmpBytes[13] = (correctBuffer >> 56) & 0xff;

	WriteProcessMemory(handle, reinterpret_cast<LPVOID>(drawEntityRenderListFunction), jmpBytes, sizeof(jmpBytes) - 1, NULL);
}

void DrawIngameManager::drawSpheres(std::vector<WitnessDrawnSphere> spheres) {
	Memory* memory = Memory::get();
	auto handle = memory->getHandle();

	uint64_t current_sphere = currentBufferIs1 ? spheres_2 : spheres_1;
	uint64_t current_asm = currentBufferIs1 ? address_drawSpherePayload_2_postpatch : address_drawSpherePayload_1_postpatch;

	for (WitnessDrawnSphere sphere : spheres) {
		float sphereFloats[] = {sphere.position.X, sphere.position.Y, sphere.position.Z, sphere.color.R, sphere.color.G, sphere.color.B, sphere.color.A };

		WriteProcessMemory(handle, reinterpret_cast<LPVOID>(current_sphere), sphereFloats, sizeof(sphereFloats), NULL);

		char asmbuff[] =
			"\x48\xB9\x00\x00\x00\x00\x00\x00\x00\x00"
			"\x49\xB8\x00\x00\x00\x00\x00\x00\x00\x00"
			"\xBB\x00\x00\x00\x00"
			"\x66\x0F\x6E\xCB"
			"\xFF\x15\x02\x00\x00\x00\xEB\x08\x00\x00\x00\x00\x00\x00\x00\x00"; //call to faraway function address

		asmbuff[2] = current_sphere & 0xff;
		asmbuff[3] = (current_sphere >> 8) & 0xff;
		asmbuff[4] = (current_sphere >> 16) & 0xff;
		asmbuff[5] = (current_sphere >> 24) & 0xff;
		asmbuff[6] = (current_sphere >> 32) & 0xff;
		asmbuff[7] = (current_sphere >> 40) & 0xff;
		asmbuff[8] = (current_sphere >> 48) & 0xff;
		asmbuff[9] = (current_sphere >> 56) & 0xff;

		current_sphere += 12;

		asmbuff[12] = current_sphere & 0xff;
		asmbuff[13] = (current_sphere >> 8) & 0xff;
		asmbuff[14] = (current_sphere >> 16) & 0xff;
		asmbuff[15] = (current_sphere >> 24) & 0xff;
		asmbuff[16] = (current_sphere >> 32) & 0xff;
		asmbuff[17] = (current_sphere >> 40) & 0xff;
		asmbuff[18] = (current_sphere >> 48) & 0xff;
		asmbuff[19] = (current_sphere >> 56) & 0xff;

		current_sphere += 16;

		int * radius = reinterpret_cast<int *>(&sphere.radius);

		asmbuff[21] = *radius & 0xff;
		asmbuff[22] = (*radius >> 8) & 0xff;
		asmbuff[23] = (*radius >> 16) & 0xff;
		asmbuff[24] = (*radius >> 24) & 0xff;

		asmbuff[37] = drawSphereCheap & 0xff;
		asmbuff[38] = (drawSphereCheap >> 8) & 0xff;
		asmbuff[39] = (drawSphereCheap >> 16) & 0xff;
		asmbuff[40] = (drawSphereCheap >> 24) & 0xff;
		asmbuff[41] = (drawSphereCheap >> 32) & 0xff;
		asmbuff[42] = (drawSphereCheap >> 40) & 0xff;
		asmbuff[43] = (drawSphereCheap >> 48) & 0xff;
		asmbuff[44] = (drawSphereCheap >> 56) & 0xff;

		WriteProcessMemory(handle, reinterpret_cast<LPVOID>(current_asm), asmbuff, sizeof(asmbuff) - 1, NULL);
		
		current_asm += sizeof(asmbuff) - 1;
	}

	writeCapBytes(current_asm);
	switchBuffers();
}