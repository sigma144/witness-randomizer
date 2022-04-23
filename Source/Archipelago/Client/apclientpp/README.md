# apclientpp

C++ Archipelago multiworld randomizer client library. See [archipelago.gg](https://archipelago.gg/).

## How to use

* add dependencies to your project
  * [nlohmann/json](https://github.com/nlohmann/json)
  * [tristanpenman/valijson](https://github.com/tristanpenman/valijson)
  * [black-sliver/wswrap](https://github.com/black-sliver/wswrap)
  * for desktop: [zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp)
  * for desktop: asio (and define ASIO_STANDALONE) or boost:asio
* add wswrap/src/wswrap.cpp as source file
* include apclient.hpp
* instantiate APClient and use its API
* see [ap-soeclient](https://github.com/black-sliver/ap-soeclient) for an example

## When using Visual Studio for building
* follow steps mentioned above
* Set up Additional Include Directories for the dependencies
  * project properties -> C/C++ -> General -> Additional Include Directories
    * Add subprojects\asio\include
    * Add subprojects\websocketpp
    * Add subprojects\wswrap\include
    * Add subprojects\json\include
    * Add subprojects\valijson\include 
* Add /Zc:__cplusplus to the command line
  * project properties -> C/C++ -> Command Line -> Additional Options
* Add _WIN32_WINNT=0x0600 (or higher) to Preprocessor Definitions
  * project properties -> C/C++ -> Preprocessor -> Preprocessor Definitions
* If your code includes <windows.h>, you must also define the WIN32_LEAN_AND_MEAN preprocessor
  * project properties -> C/C++ -> Preprocessor -> Preprocessor Definitions
  * if your code relies on features outside of the include scope of WIN32_LEAN_AND_MEAN, you can choose to include apclientpp.hpp before including windows.h, in this case you also need to define the ASIO_NO_WIN32_LEAN_AND_MEAN preprocessor
