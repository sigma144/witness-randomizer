#include "../Memory.h">
#include <memory>

class PanelRestore {
public:
	static void RestoreOriginalPanelData(std::shared_ptr<Memory> memory);
};