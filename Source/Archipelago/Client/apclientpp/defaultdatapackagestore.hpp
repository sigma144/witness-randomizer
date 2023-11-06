/* Copyright (c) 2022 black-sliver, FelicitusNeko

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DEFAULTDATAPACKAGESTORE_HPP
#define _DEFAULTDATAPACKAGESTORE_HPP

#include "apclient.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <time.h>

#if defined WIN32 || defined _WIN32
#include <shlobj.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101500
#define NO_STD_FILESYSTEM
#endif
#endif

#if (defined(HAS_STD_FILESYSTEM) || (__cplusplus >= 201703L)) && !defined(NO_STD_FILESYSTEM)
#include <filesystem>
#else
#ifndef NO_STD_FILESYSTEM
#define NO_STD_FILESYSTEM
#endif
#if defined WIN32 || defined _WIN32
// Windows uses UC2 filenames, which is not implemented below.
#error "Dummy filesystem not supported on Windows. Please use -std=c++17, write a custom DataPackageStore, or open a PR"
#endif
#include <string>
#include <errno.h>
#endif


class DefaultDataPackageStore : public APDataPackageStore
{
private:
    typedef nlohmann::json json;
#ifndef NO_STD_FILESYSTEM
    typedef std::filesystem::path path;
#else
    class path final
    {
        std::string s;

    public:
        path() = default;

        path(const std::string& s)
            : s(s)
        {
        }

        path(const char* s)
            : s(s)
        {
        }

        const std::string& string() const
        {
            return s;
        }

        const char* c_str() const
        {
            return s.c_str();
        }

        path parent_path() const
        {
            if (s == "/")
                return "/";
            auto p = s.rfind("/");
            if (p == s.npos)
                return "";
            return s.substr(0, p);
        }

        path operator/(const std::string& other) const
        {
            return s + "/" + other;
        }

        bool operator==(const std::string& other) const
        {
            return s == other;
        }
    };

    bool create_directories(const path& d, std::error_code& ec) noexcept
    {
        size_t len = d.string().length();
        if (len >= 256) {
            ec = {ENOMEM, std::generic_category()};
            return false;
        }
        char tmp[256];
        char *p = NULL;
        memcpy(tmp, d.c_str(), len + 1);

        if (tmp[len - 1] == '/')
            tmp[len - 1] = 0;
        for (p = tmp + 1; *p; p++) {
            if (*p == '/') {
                *p = 0;
                mkdir(tmp, S_IRWXU);
                *p = '/';
            }
        }
        mkdir(tmp, S_IRWXU);
        if (errno != EEXIST)
            ec = {errno, std::generic_category()};
        return !!errno;
    }
#endif

    path _path;


    void log(const char* msg)
    {
        printf("APClient: %s\n", msg);
    }

    path get_path(const std::string& game, const std::string& checksum) const
    {
        std::string safe_game, safe_checksum;
        const std::string exclude = "<>:\"/\\|?*";
        auto sanitize = [&](char c) { return exclude.find(c) == std::string::npos; };
        std::copy_if(game.begin(), game.end(), std::back_inserter(safe_game), sanitize);
        std::copy_if(checksum.begin(), checksum.end(), std::back_inserter(safe_checksum), sanitize);

        if (safe_game.empty() || safe_checksum != checksum)
            return ""; // invalid
        if (checksum.empty())
            return _path / (safe_game + ".json");
        return (_path / safe_game) / (safe_checksum + ".json");
    }

    static void touch(const path& filename)
    {
#if defined WIN32 || defined _WIN32
        struct __utimbuf64 ut;
        ut.actime = _time64(NULL);
        ut.modtime = ut.actime;
        _wutime64(filename.c_str(), &ut);
#else
        struct utimbuf ut;
        ut.actime = time(NULL);
        ut.modtime = ut.actime;
        utime(filename.c_str(), &ut);
#endif
    }

    static path get_default_cache_dir(const std::string& fallbackPath, const std::string& app = "Archipelago")
    {
#if defined WIN32 || defined _WIN32
        WCHAR appData[MAX_PATH];
        HRESULT hres = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData);
        if (hres == S_OK && *appData)
            return path(appData) / app / "Cache";
#elif defined __APPLE__
        const char* home = std::getenv("HOME");
        if (home)
            return path(home) / "Library" / "Caches" / app;
#else
#   if defined __EMSCRIPTEN__
        if (fallbackPath.empty()) { // HOME might not be persistent on emscripten
#   endif
        const char* xdg = std::getenv("XDG_CACHE_HOME");
        if (xdg)
            return path(xdg) / app;
        const char* home = std::getenv("HOME");
        if (home)
            return path(home) / ".cache" / app;
#   if defined __EMSCRIPTEN__
        }
#   endif
#endif
        if (!fallbackPath.empty())
            return path(fallbackPath) / "cache";
        return path("cache");
    }

public:
    DefaultDataPackageStore(const std::string fallbackPath = "")
        : _path(get_default_cache_dir(fallbackPath) / "datapackage")
    {
    }

    virtual bool load(const std::string& game, const std::string& checksum, json& data) override
    {
        auto p = get_path(game, checksum);
        if (p == "")
            return false;
        try {
#ifdef NO_STD_FILESYSTEM
            std::ifstream f(p.string(), std::ios::binary);
#else
            std::ifstream f(p, std::ios::binary);
#endif
            if (f.fail() || f.eof())
                return false;
            data = json::parse(f);
            touch(p); // update file time to keep it in cache
            return true;
        } catch (const std::exception& ex) {
            log(("Failed to load " + p.string() + ":").c_str());
            log(ex.what());
            return false;
        }
    }

    virtual bool save(const std::string& game, const json& data) override
    {
#ifndef NO_STD_FILESYSTEM
        using std::filesystem::create_directories;
#endif

        if (!data.is_object())
            return false;
        auto it = data.find("checksum");
        path p;
        if (it == data.end())
            p = get_path(game, "");
        else
            p = get_path(game, *it);
        if (p == "")
            return false;

        std::error_code ec;
        create_directories(p.parent_path(), ec);
        if (ec)
            return false;

        try {
#ifdef NO_STD_FILESYSTEM
            std::ofstream f(p.string(), std::ios::binary);
#else
            std::ofstream f(p, std::ios::binary);
#endif
            f << data.dump();
            return true;
        } catch (const std::exception& ex) {
            log(ex.what());
            return false;
        }
    }
};


#endif // _DEFAULTDATAPACKAGESTORE
