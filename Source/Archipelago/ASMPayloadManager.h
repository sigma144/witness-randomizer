#include <cstdint>
#include <string>

class Memory;

class ASMPayloadManager
{
private:
	ASMPayloadManager();

	void findGameloopFunction();
	void setupPayload();

	uint64_t call1AbsoluteAddress = 0;
	uint64_t call2AbsoluteAddress = 0;
	uint64_t call3AbsoluteAddress = 0;

	uint64_t payloadBlocked = 0;
	uint64_t payloadStart = 0;
	uint64_t payloadASMStart = 0;

	static ASMPayloadManager* _singleton;

	void ExecuteASM(char* asmBuff, int buffersize);
	void CallVoidFunction(uint64_t functionAddress, int id);
public:
	static void create();
	static ASMPayloadManager* get();

	void ActivateMarker(int id);
	void OpenDoor(int id);
	void CloseDoor(int id);
	void UpdateEntityPosition(int id);
	uint64_t FindSoundByName(std::string name);
	int FindEntityByName(std::string name);
	void SendBunkerElevatorToFloor(int floor, bool force);
	void ToggleFloodgate(std::string name, bool connect);
	void BridgeToggle(int associatedPanel, bool disconnect);
	void ExitSolveMode();
};