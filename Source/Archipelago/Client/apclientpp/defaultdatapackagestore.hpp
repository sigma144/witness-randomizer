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
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#if defined WIN32 || defined _WIN32
#include <shlobj.h>
#endif


class DefaultDataPackageStore : public APDataPackageStore
{
private:
    typedef nlohmann::json json;
    typedef std::filesystem::path path;

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

    static path get_default_cache_dir(const std::string& fallbackPath, const std::string& app = "Archipelago")
    {
#if defined WIN32 || defined _WIN32
        WCHAR appData[MAX_PATH];
        HRESULT hres = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData);
        if (hres == S_OK && appData && *appData)
            return path(appData) / app / "Cache";
#elif defined __APPLE__
        const char* home = std::getenv("HOME");
        if (home)
            return path(home) / "Library" / "Caches" / app;
#else
        const char* xdg = std::getenv("XDG_CACHE_HOME");
        if (xdg)
            return path(xdg) / app;
        const char* home = std::getenv("HOME");
        if (home)
            return path(home) / ".cache" / app;
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
            std::ifstream f(p, std::ios::binary);
            if (f.fail() || f.eof())
                return false;
            data = json::parse(f);
            return true;
        } catch (const std::exception& ex) {
            log(("Failed to load " + p.string() + ":").c_str());
            log(ex.what());
            return false;
        }
    }

    virtual bool save(const std::string& game, const json& data) override
    {
        using std::filesystem::create_directories;

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
            std::ofstream f(p, std::ios::binary);
            f << data.dump();
            return true;
        } catch (const std::exception& ex) {
            log(ex.what());
            return false;
        }
    }
};


#endif // _DEFAULTDATAPACKAGESTORE
