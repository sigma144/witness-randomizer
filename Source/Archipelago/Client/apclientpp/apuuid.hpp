/* Copyright (c) 2022 black-sliver

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _APUUID_HPP
#define _APUUID_HPP


#include <stdint.h>
#include <stdio.h>
#include <string>


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <stdlib.h>
#include <time.h>
#endif


static void apuuid_init_rng()
{
    #ifdef __EMSCRIPTEN__
    /* js crypto needs no init from C */
    #else
    srand ((unsigned int) time (NULL));
    #endif
}

static uint8_t apuuid_rand_byte()
{
    #ifdef __EMSCRIPTEN__
    return (uint8_t)EM_ASM_INT({
        var buf = new Uint8Array(1);
        crypto.getRandomValues(buf);
        return buf[0];
    });
    #else
    return rand();
    #endif
}

static void apuuid_generate(char* out)
{
    for (uint8_t i=0; i<16; i++) {
        sprintf(out + 2*i, "%02hhx", apuuid_rand_byte());
    }
}

static std::string ap_get_uuid(const std::string& uuidFile)
{
    char uuid[33]; uuid[32] = 0;
#if defined USE_IDBFS || !defined __EMSCRIPTEN__
    FILE* f = uuidFile.empty() ? nullptr : fopen(uuidFile.c_str(), "rb");
    size_t n = 0;
    if (f) {
        n = fread(uuid, 1, 32, f);
        fclose(f);
    }
    if (!f || n < 32) {
        apuuid_init_rng();
        apuuid_generate(uuid);
        f = uuidFile.empty() ? nullptr : fopen(uuidFile.c_str(), "wb");
        if (f) {
            n = fwrite(uuid, 1, 32, f);
            fclose(f);
            #ifdef __EMSCRIPTEN__
            EM_ASM(
                FS.syncfs(function (err) {}); // cppcheck-suppress unknownMacro
            );
            #endif
        }
        if (!uuidFile.empty() && (!f || n < 32)) {
            fprintf(stderr, "Could not write persistant UUID!\n");
        }
    }
    f = nullptr;
#else
    #error TODO: implement localStorage API
#endif
    return uuid;
}

#endif // _APUUID_HPP
