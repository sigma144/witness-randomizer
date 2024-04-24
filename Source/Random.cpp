
#include "Random.h"
#include <time.h>

std::mt19937 Random::gen = std::mt19937((int)time(0));