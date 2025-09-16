//This file contributed by nathanle1406 (https://github.com/n-elderbroom)

#include <sndfile.h>
#include <vector>
#include <algorithm>
#include <random>
#include "Sounds.h"
#include "Memory.h"

std::vector<uint8_t> build_sound(std::vector<std::string> files) {
	std::vector<AudioData> sounds;
	SF_INFO info;
	for (std::string& filename : files) {
		std::vector<float> buffer;
		SNDFILE* file = sf_open(("sounds/" + filename).c_str(), SFM_READ, &info);
		if (!file)
			throw std::runtime_error("File missing: sounds/" + filename);
		int total_frames = info.frames * info.channels;
		buffer.resize(total_frames);
		auto frames_written = sf_read_float(file, buffer.data(), total_frames);
		sf_close(file);

		AudioData sound;
		sound.data = buffer;
		sound.framecount = total_frames;
		sounds.emplace_back(sound);
	}
	SF_INFO out_info = {};
	out_info.samplerate = info.samplerate;
	out_info.channels = info.channels;
	out_info.format = info.format;
	SNDFILE* outfile = sf_open("sounds/output.wav", SFM_WRITE, &out_info); //TODO its better to do this in memory with sf_open_virtual? Can avoid disk I/O that way.
	for (const auto& s : sounds) {
		sf_write_float(outfile, s.data.data(), s.framecount);
	}
	sf_close(outfile);
	auto m = Memory::get();
	return m->ReadFileToVector("sounds/output.wav");
}

std::vector<uint8_t> build_sound(std::vector<Note> notes) {
	std::vector<std::string> files;
	if (notes[0].type == Bird) {
		for (Note& n : notes) {
			if (n.note == Low && !n.is_long) files.emplace_back("lowshort.wav");
			else if (n.note == Mid && !n.is_long) files.emplace_back("midshort.wav");
			else if (n.note == High && !n.is_long) files.emplace_back("highshort.wav");
			else if (n.note == Low && n.is_long) files.emplace_back("lowlong.wav");
			else if (n.note == Mid && n.is_long) files.emplace_back("midlong.wav");
			else if (n.note == High && n.is_long) files.emplace_back("highlong.wav");
		}
	}
	return build_sound(files);
}