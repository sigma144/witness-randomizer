#pragma once
#include <map>

template<typename T, typename T2>
std::map<T, T2> merge(std::initializer_list<std::map<T, T2>*> vecs)
{
	size_t size = 0;
	for (auto v : vecs) { size += v->size(); }
	std::map<T, T2> ret;
	ret.reserve(size);
	for (auto v : vecs) { ret.insert(ret.end(), v->begin(), v->end()); }
	return ret;
}

std::map<int, long> tutorial = {
	{ 0x03629, 158000 }, // Tutorial Gate Open
	{ 0x00061, 158001 }, // Outside Tutorial Dots Tutorial 5
	{ 0x00021, 158002 }, // Outside Tutorial Stones Tutorial 9
	{ 0x0C373, 158003 }, // Tutorial Patio floor
	{ 0x032FF, 158004 }, // Orchard Apple Tree 5
};

std::map<int, long> glassfactory = {
	{ 0x00062, 158005 }, // Glass Factory Vertical Symmetry 4
	{ 0x00083, 158006 }, // Glass Factory Rotational Symmetry 3
	{ 0x0343A, 158007 }, // Glass Factory Melting 3
};

std::map<int, long> symmetry = {
	{ 0x00026, 158008 }, // Symmetry Island Black Dots 5
	{ 0x00079, 158009 }, // Symmetry Island Colored Dots 6
	{ 0x00076, 158010 }, // Symmetry Island Fading Lines 7
	{ 0x00B8D, 158011 }, // Symmetry Island Transparent 5
	{ 0x00A5B, 158012 }, // Symmetry Island Laser Yellow 3
	{ 0x00A68, 158013 }, // Symmetry Island Laser Blue 3
	{ 0x0360D, 158014 }, // Symmetry Laser
};

std::map<int, long> desert = {
	{ 0x09F94, 158015 }, // Desert Surface 8
	{ 0x0A02D, 158016 }, // Desert Light 3
	{ 0x18313, 158017 }, // Desert Pond 5
	{ 0x17ECA, 158018 }, // Desert Flood 5
	{ 0x03608, 158019 }, // Desert Laser
};

std::map<int, long> quarry = {
	{ 0x014E8, 158020 }, // Mill Lower Row 6
	{ 0x014E9, 158021 }, // Mill Upper Row 8
	{ 0x3C125, 158022 }, // Mill Control Room 2
	{ 0x021AE, 158023 }, // Boathouse Erasers and Shapers 5
	{ 0x3C124, 158024 }, // Boathouse Erasers and Stars 7
	{ 0x0A3D0, 158025 }, // Boathouse Erasers Shapers and Stars 5
	{ 0x03612, 158026 }, // Quarry Laser
};

std::map<int, long> treehouse = {
	{ 0x17DC4, 158027 }, // Treehouse Yellow 9
	{ 0x17D6C, 158028 }, // Treehouse First Purple 5
	{ 0x17DC6, 158029 }, // Treehouse Second Purple 7
	{ 0x17DDB, 158030 }, // Treehouse Left Orange 15
	{ 0x17DA2, 158031 }, // Treehouse Right Orange 12
	{ 0x17E61, 158032 }, // Treehouse Green 7
	{ 0x03613, 158033 }, // Treehouse Laser
};

std::map<int, long> shadows = {
	{ 0x0A8E0, 158034 }, // Shadows Tutorial 8
	{ 0x1972F, 158035 }, // Shadows Avoid 8
	{ 0x197E5, 158036 }, // Shadows Follow 5
	{ 0x19650, 158037 }, // Shadows Laser
};

std::map<int, long> monastery = {
	{ 0x00037, 158038 }, // Monastery Exterior 3
	{ 0x193A6, 158039 }, // Monastery Interior 4
	{ 0x17CA4, 158040 }, // Monastery Laser
};

std::map<int, long> keep = {
	{ 0x01A0F, 158041 }, // Keep Hedges 4
	{ 0x01D3F, 158042 }, // Keep Blue Pressure Plates
	{ 0x0360E, 158043 }, // Keep Front Laser
	{ 0x03317, 158044 }, // Keep Back Laser
};

std::map<int, long> town = {
	{ 0x28AD9, 158045 }, // Town Eraser
	{ 0x28ACC, 158046 }, // Town Blue 5
	{ 0x28B39, 158047 }, // Town Red Hexagonal
	{ 0x28A69, 158048 }, // Town Lattice
	{ 0x032F5, 158049 }, // Town Laser
};

std::map<int, long> swamp = {
	{ 0x0056F, 158050 }, // Swamp Tutorial 6
	{ 0x181A9, 158051 }, // Swamp Tutorial 14
	{ 0x00990, 158052 }, // Swamp Red 4
	{ 0x009A1, 158053 }, // Swamp Discontinuous 4
	{ 0x0000A, 158054 }, // Swamp Rotation Tutorial 4
	{ 0x00E3A, 158055 }, // Swamp Rotation Advanced 4
	{ 0x00006, 158056 }, // Swamp Blue Underwater 5
	{ 0x00596, 158057 }, // Swamp Teal Underwater 5
	{ 0x014D1, 158058 }, // Swamp Red Underwater 4
	{ 0x009A6, 158059 }, // Swamp Purple Tetris
	{ 0x03615, 158060 }, // Swamp Laser
};

std::map<int, long> bunker = {
	{ 0x09FF8, 158061 }, // Bunker Tutorial 5
	{ 0x09DAF, 158062 }, // Bunker Advanced 4
	{ 0x0A01F, 158063 }, // Bunker Glass 3
	{ 0x17E67, 158064 }, // Bunker Ultraviolet 2
	{ 0x09DE0, 158065 }, // Bunker Laser
};

std::map<int, long> jungle = {
	{ 0x002C6, 158066 }, // Jungle Waves 3
	{ 0x002C7, 158067 }, // Jungle Waves 7
	{ 0x014B2, 158068 }, // Jungle Dots 6
	{ 0x03616, 158069 }, // Jungle Laser
};

std::map<int, long> mountain_outside = {
	{ 0x0042D, 158070 }, // Mountaintop River
};

std::map<int, long> mountain_inside = {
	{ 0x09E6B, 158071 }, // Mountain 1 Orange 7
	{ 0x09EAF, 158072 }, // Mountain 1 Purple 2
	{ 0x09E7B, 158073 }, // Mountain 1 Green 5
	{ 0x09FD7, 158074 }, // Mountain 2 Rainbow 4
};

std::map<int, long> standardChecks = merge<int, long>({ &tutorial, &glassfactory, &swamp, &town, &treehouse, &symmetry, &desert, &quarry, &shadows, &jungle, &keep, &monastery, &bunker, &mountain_inside, &mountain_outside });

std::map<int, long> discards = {
	{ 0x17F93, 158075 }, // Mountain 2 Discard
	{ 0x17CFB, 158076 }, // Outside Tutorial Discard
	{ 0x3C12B, 158077 }, // Glass Factory Discard
	{ 0x17CE7, 158078 }, // Desert Discard
	{ 0x17CF0, 158079 }, // Mill Discard
	{ 0x17FA9, 158080 }, // Treehouse Green Bridge Discard
	{ 0x17FA0, 158081 }, // Treehouse Laser Discard
	{ 0x17D27, 158082 }, // Keep Discard
	{ 0x17D28, 158083 }, // Shipwreck Discard
	{ 0x17D01, 158084 }, // Town Orange Crate Discard
	{ 0x17F9B, 158085 }, // Jungle Discard
	{ 0x17C42, 158086 }, // Mountainside Discard
};

std::map<int, long> vaults = {
	{ 0x033D4, 158087 }, // Outside Tutorial Vault
	{ 0x0CC7B, 158088 }, // Desert Vault
	{ 0x00AFB, 158089 }, // Shipwreck Vault
	{ 0x002A6, 158090 }, // Mountainside Vault
};