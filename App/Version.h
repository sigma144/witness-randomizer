#pragma once

#define TO_STRING2(s) #s
#define TO_STRING(s) TO_STRING2(s)

#define MAJOR 5
#define MINOR 0
#define PATCH 5

#define VERSION_STR     "5.0.5"
#define VERSION			MAJOR, MINOR, PATCH

#define AP_VERSION_STR	"0.4.6"
#define AP_VERSION_STR_BACKCOMPAT  "0.4.4"

#define PRODUCT_NAME L"Witness Random Puzzle Generator for Archipelago.gg"
