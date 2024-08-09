#include <cstdint>
#include "../DataTypes.h"
class Memory;

struct WitnessDrawnSphere {
	Vector3 position;
	float radius;
	RgbColor color;
	bool cull;
};

class DrawIngameManager {
private:

	DrawIngameManager();

	static DrawIngameManager* _singleton;

public:
	static void create();
	static DrawIngameManager* get();
	void drawSpheres(std::vector<WitnessDrawnSphere> spheres);

private:
	void findRelevantFunctions();
	void allocatePayloads();
	void initialPatch();
	void writeCapBytes(uint64_t capStart);
	void switchBuffers();
	
	uint64_t drawEntityRenderListFunction;
	uint64_t drawSphereCheap;

	uint64_t allocation;

	uint64_t address_drawSpherePayload_1 = 0;
	uint64_t address_drawSpherePayload_1_postpatch = 0;
	uint64_t address_drawSpherePayload_2 = 0;
	uint64_t address_drawSpherePayload_2_postpatch = 0;
	uint64_t spheres_1;
	uint64_t spheres_2;

	bool currentBufferIs1 = false;
};