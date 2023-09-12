#include <vector>

class PanelRestore {
public:
	static void RestoreOriginalPanelData();
	static std::vector<float> GetPositions(int id);
	static bool HasPositions(int id);
	static std::vector<float> GetEPColors(int startPointId);
};