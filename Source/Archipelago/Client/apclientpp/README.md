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
