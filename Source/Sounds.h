//This file contributed by nathanle1406 (https://github.com/n-elderbroom)

#pragma once
#include <sndfile.h>
#include <vector>
#include <unordered_map>

struct AudioData {
    std::vector<float> data;
    int framecount;
};

enum Note_Value {
    VeryLow,
    Low,
    Mid,
    High,
    VeryHigh,
};

enum Instrument {
    Bird,
    Drip,
    Test
};

struct Note {
    Instrument type;
    Note_Value note;
    bool is_long;
};

std::vector<uint8_t> build_sound(std::vector<std::string> files);
std::vector<uint8_t> build_sound(std::vector<Note> notes);

//maps puzzle ids to matching sound files
//all the bird and water drip noises are in the globals package, and are always loaded. they can be replaced at any time without issue.
//i dont think anything in-game actually ties the panels to the sounds. the sounds are just nearby to the puzzle.
//There are usually several sound files per puzzle, for variance
inline std::unordered_map<int32_t, std::vector<std::string>> panel_sound_map = {
    //wave puzzles:
    {0x002C4, {"bird_high_low1", "bird_high_low2", "bird_high_low3"}},
    //{0x002C4, {"bird_high_low1"}},
    {0x00767, {"bird_high_high_low_med1", "bird_high_high_low_med2", "bird_high_high_low_med3"}},
    {0x002c6, {"bird_med_med_high_med_low1", "bird4_med_med_high_med_low","bird_med_med_high_med_low3", "bird_med_med_high_med_low2"}},
    {0x0070e, {"bird2_high_high_low_med_low", "bird4_high_high_low_med_low"} },
    {0x0070f, {"bird3_highlong_low_med_med"} },
    {0x0087d, {"bird2_low_med_high_low"} },
    {0x002c7, {"bird3_low_med_low_highlong"} },
    //sound dot puzzles:
    {0x0026d, {"bird_high_low1", "bird_high_low2", "bird_high_low3"}},
    {0x0026e, {"bird_high_low1", "bird_high_low2", "bird_high_low3"}},
    {0x0026f, {"bird_med_med_high_med_low1", "bird4_med_med_high_med_low","bird_med_med_high_med_low3", "bird_med_med_high_med_low2"}},
    {0x00c3f, {"bird5_high_med_high_low"} },
    {0x00c41, {"bird2_high_high_low_med_low", "bird4_high_high_low_med_low"} },
    {0x014b2, {"bird6_low_med_high_low_high"} },
};
