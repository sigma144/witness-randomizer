# apclientpp

C++ Archipelago multiworld randomizer client library. See [archipelago.gg](https://archipelago.gg/).


## Prerequisites

* gcc6 or msvc toolset v14.1 or newer on Windows
* gcc5 or clang3.3 or newer on other platforms
* std c++14 or newer (c++11 support needs a bit of rework)


## How to use

* add dependencies to your project
  * [nlohmann/json](https://github.com/nlohmann/json)
  * [tristanpenman/valijson](https://github.com/tristanpenman/valijson)
  * [black-sliver/wswrap](https://github.com/black-sliver/wswrap)
  * for desktop: [zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp)
  * for desktop: asio (and define ASIO_STANDALONE) or boost::asio
  * not all websocketpp versions are compatible to all asio versions
    * [try those](https://github.com/black-sliver/ap-soeclient/tree/master/subprojects) (download repo as zip and extract)
  * make sure to set up include paths correctly, the dependencies are all header-only
* include apclient.hpp
* instantiate APClient and use its API
  * you can use `ap_get_uuid` from `apuuid.hpp` helper to generate a UUID
  * use `set_data_package` and `set_data_package_changed_handler` to load and save data package
  * use `set_*_handler` to set event callbacks
  * call `poll` repeatedly (e.g. once per frame) for it to connect and callbacks to fire
  * use `ConnectSlot` to connect to a slot after RoomInfo
  * use `StatusUpdate`, `LocationChecks` and `LocationScouts` to send status, checks and scouts
  * use `Say` to send a (chat) message
  * use `Bounce` to send a bounce (deathlink, ...)
  * use `Get`, `Set` and `SetNotify` to access data storage api,
    see [Archipelago network protocol](https://github.com/ArchipelagoMW/Archipelago/blob/main/docs/network%20protocol.md#get)
* see [ap-soeclient](https://github.com/black-sliver/ap-soeclient) for an example
* see [Gotchas](#gotchas)


## When using Visual Studio for building

* follow steps mentioned above
* Set up Additional Include Directories for the dependencies
  * project properties -> C/C++ -> General -> Additional Include Directories
    * Add subprojects\asio\include
    * Add subprojects\websocketpp
    * Add subprojects\wswrap\include
    * Add subprojects\json\include
    * Add subprojects\valijson\include 
* Add `/Zc:__cplusplus` to the command line
  * project properties -> C/C++ -> Command Line -> Additional Options
* Add `_WIN32_WINNT=0x0600` (or higher) to Preprocessor Definitions
  * project properties -> C/C++ -> Preprocessor -> Preprocessor Definitions
* If your code includes <windows.h>, you must also define the `WIN32_LEAN_AND_MEAN` preprocessor
  * project properties -> C/C++ -> Preprocessor -> Preprocessor Definitions
  * if your code relies on features outside the include scope of WIN32_LEAN_AND_MEAN, you can choose to include
    apclientpp.hpp before including windows.h, in this case you also need to define the `ASIO_NO_WIN32_LEAN_AND_MEAN`
    preprocessor


## Implementations

* [ap-soeclient](https://github.com/black-sliver/ap-soeclient) \
  shell build scripts, gcc/mingw/emscripten, Linux/Windows/Webbrowser
* [Meritous AP: apclient branch](https://github.com/FelicitusNeko/meritous-ap/tree/apclient) \
  Makefile, gcc, Windows; wraps apclientpp in a C file
* [The Witness Randomizer for Archipelago](https://github.com/Jarno458/The-Witness-Randomizer-for-Archipelago) \
  VS2019, msvc, Windows
* [Dark Souls III Archipelago client](https://github.com/Marechal-L/Dark-Souls-III-Archipelago-client) \
  VS2022, msvc, Windows


## Gotchas

* `poll()` handles the socket operations, so it has to be called repeatedly while APClient exists.
* If desired, multi-threading has to be done by the caller. Events fire from within `poll()`. While inside poll other threads may not access the instance.
* Some versions of mingw may not define a compatible std::err
  * Commits from [this PR](https://github.com/zaphoyd/websocketpp/pull/479/files) can be cherry-picked in.
  * Forks of subprojects to be used directly as submodules are on the todo list.
