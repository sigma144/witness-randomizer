#include "../Memory.h">
#include <memory>

class PanelRestore {
public:
	static void RestoreOriginalPanelData(std::shared_ptr<Memory> memory);
	static std::vector<float> GetPositions(int id);
	static bool HasPositions(int id);
};