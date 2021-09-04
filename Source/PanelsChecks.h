#pragma once
#include <vector>

template<typename T>
std::vector<T> merge(std::initializer_list<std::vector<T>*> vecs)
{
	size_t size = 0;
	for (auto v : vecs) { size += v->size(); }
	std::vector<T> ret;
	ret.reserve(size);
	for (auto v : vecs) { ret.insert(ret.end(), v->begin(), v->end()); }
	return ret;
}

std::vector<int> tutorial = {
	0x03629, // Tutorial Gate Open
	0x00061, // Outside Tutorial Dots Tutorial 5
	0x00021, // Outside Tutorial Stones Tutorial 9
	0x0C373, // Tutorial Patio floor
	0x032FF, // Orchard Apple Tree 5
};

std::vector<int> glassfactory = {
	0x00062, // Glass Factory Vertical Symmetry 4
	0x00083, // Glass Factory Rotational Symmetry 3
	0x0343A, // Glass Factory Melting 3

};

std::vector<int> symmetry = {
	0x00026, // Symmetry Island Black Dots 5
	0x00079, // Symmetry Island Colored Dots 6
	0x00076, // Symmetry Island Fading Lines 7
	0x00B8D, // Symmetry Island Transparent 5
	0x00A5B, // Symmetry Island Laser Yellow 3
	0x00A68, // Symmetry Island Laser Blue 3
	0x0360D, // Symmetry Laser
};

std::vector<int> desert = {
	0x09F94, // Desert Surface 8
	0x0A02D, // Desert Light 3
	0x18313, // Desert Pond 5
	0x17ECA, // Desert Flood 5
	0x03608, // Desert Laser
};

std::vector<int> quarry = {
	0x014E8, // Mill Lower Row 6
	0x014E9, // Mill Upper Row 8
	0x3C125, // Mill Control Room 2
	0x021AE, // Boathouse Erasers and Shapers 5
	0x3C124, // Boathouse Erasers and Stars 7
	0x0A3D0, // Boathouse Erasers Shapers and Stars 5
	0x03612, // Quarry Laser
};

std::vector<int> treehouse = {
	0x17DC4, // Treehouse Yellow 9
	0x17D6C, // Treehouse First Purple 5
	0x17DC6, // Treehouse Second Purple 7
	0x17DDB, // Treehouse Left Orange 15
	0x17DA2, // Treehouse Right Orange 12
	0x17E61, // Treehouse Green 7
	0x03613, // Treehouse Laser
};

std::vector<int> shadows = {
	0x0A8E0, // Shadows Tutorial 8
	0x1972F, // Shadows Avoid 8
	0x197E5, // Shadows Follow 5
	0x19650, // Shadows Laser
};

std::vector<int> monastery = {
	0x00037, // Monastery Exterior 3
	0x193A6, // Monastery Interior 4
	0x17CA4, // Monastery Laser
};

std::vector<int> keep = {
	0x01A0F, // Keep Hedges 4
	0x01D3F, // Keep Blue Pressure Plates
	0x0360E, // Keep Front Laser
	0x03317, // Keep Back Laser
};

std::vector<int> town = {
	0x28AD9, // Town Eraser
	0x28ACC, // Town Blue 5
	0x28B39, // Town Red Hexagonal
	0x28A69, // Town Lattice
	0x032F5, // Town Laser
};

std::vector<int> swamp = {
	0x0056F, // Swamp Tutorial 6
	0x181A9, // Swamp Tutorial 14
	0x00990, // Swamp Red 4
	0x009A1, // Swamp Discontinuous 4
	0x0000A, // Swamp Rotation Tutorial 4
	0x00E3A, // Swamp Rotation Advanced 4
	0x00006, // Swamp Blue Underwater 5
	0x00596, // Swamp Teal Underwater 5
	0x014D1, // Swamp Red Underwater 4
	0x009A6, // Swamp Purple Tetris
	0x03615, // Swamp Laser
};

std::vector<int> bunker = {
	0x09FF8, // Bunker Tutorial 5
	0x09DAF, // Bunker Advanced 4
	0x0A01F, // Bunker Glass 3
	0x17E67, // Bunker Ultraviolet 2
	0x09DE0, // Bunker Laser
};

std::vector<int> jungle = {
	0x002C6, // Jungle Waves 3
	0x002C7, // Jungle Waves 7
	0x014B2, // Jungle Dots 6
	0x03616, // Jungle Laser
};

std::vector<int> mountain_outside = {
	0x0042D, // Mountaintop River
};

std::vector<int> mountain_inside = {
	0x09E6B, // Mountain 1 Orange 7
	0x09EAF, // Mountain 1 Purple 2
	0x09E7B, // Mountain 1 Green 5
	0x09FD7, // Mountain 2 Rainbow 4
	0x17F93, // Mountain 2 Discard
};

std::vector<int> standardChecks = merge<int>({ &tutorial, &glassfactory, &swamp, &town, &treehouse, &symmetry, &desert, &quarry, &shadows, &jungle, &keep, &monastery, &bunker, &mountain_inside, &mountain_outside });


std::vector<int> discards = {
	0x17CFB, // Outside Tutorial Discard
	0x3C12B, // Glass Factory Discard
	0x17CE7, // Desert Discard
	0x17CF0, // Mill Discard
	0x17FA9, // Treehouse Green Bridge Discard
	0x17FA0, // Treehouse Laser Discard
	0x17D27, // Keep Discard
	0x17D28, // Shipwreck Discard
	0x17D01, // Town Orange Crate Discard
	0x17F9B, // Jungle Discard
	0x17C42, // Mountainside Discard
};

std::vector<int> vaults = {
	0x033D4, // Outside Tutorial Vault
	0x0CC7B, // Desert Vault
	0x00AFB, // Shipwreck Vault
	0x002A6, // Mountainside Vault
};