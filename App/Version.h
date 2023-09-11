#pragma once

#define TO_STRING2(s) #s
#define TO_STRING(s) TO_STRING2(s)

#define MAJOR 4
#define MINOR 0
#define PATCH 0

#define VERSION_STR     TO_STRING(MAJOR) "." TO_STRING(MINOR) "." TO_STRING(PATCH)
#define VERSION			MAJOR, MINOR, PATCH

#define AP_VERSION_STR	"0.4.3 Violet Beta (Doors Rework)"

#define PRODUCT_NAME L"Witness Random Puzzle Generator for Archipelago.gg"
