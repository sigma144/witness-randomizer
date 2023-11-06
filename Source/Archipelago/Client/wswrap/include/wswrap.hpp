// simple wrapper around different implementations for different targets
#ifndef _WSWRAP_HPP
#define _WSWRAP_HPP

#define WSWRAP_VERSION 10200  // 1.02.00

#ifdef __EMSCRIPTEN__
#include "wswrap_wsjs.hpp"
#else
#include "wswrap_websocketpp.hpp"
#endif

#endif // _WSWRAP_HPP
