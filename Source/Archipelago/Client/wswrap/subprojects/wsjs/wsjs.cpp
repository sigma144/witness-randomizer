#include <stdio.h>
#include "wsjs.hpp"
#include <emscripten/bind.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

static uint32_t _wsjs_nextid = 0;
static std::map<unsigned, WSJS*> _wsjs_objects;

static void warn(const char* fmt, ...)
{
    fprintf(stderr, "WSJS: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}
static void log(const char* fmt, ...)
{
    fprintf(stdout, "WSJS: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

#ifdef DEBUG_WSJS
#define debug log
#else
#define debug(...)
#endif

void WSJS::onopen(uint32_t id)
{
    auto it = _wsjs_objects.find(id);
    if (it == _wsjs_objects.end()) {
        warn("onopen(%u): no such id\n", (unsigned)id);
        return;
    }
    if (!it->second->_hopen) return;
    debug("onopen(%u)\n", id);
    it->second->_hopen();
}

void WSJS::onclose(uint32_t id)
{
    auto it = _wsjs_objects.find(id);
    if (it == _wsjs_objects.end()) {
        warn("onclose(%u): no such id\n", (unsigned)id);
        return;
    }
    if (!it->second->_hclose) return;
    debug("onclose(%u)\n", id);
    it->second->_hclose();
}

void WSJS::onmessage(uint32_t id, std::string data)
{
    auto it = _wsjs_objects.find(id);
    if (it == _wsjs_objects.end()) {
        warn("onmessage(%u): no such id\n", (unsigned)id);
        return;
    }
    if (!it->second->_hmessage) return;
    std::string s = data;
    debug("onmessage(%u)\n", id);
    it->second->_hmessage(s);
}

void WSJS::onerror(uint32_t id)
{
    auto it = _wsjs_objects.find(id);
    if (it == _wsjs_objects.end()) {
        warn("onerror(%u): no such id\n", (unsigned)id);
        return;
    }
    if (!it->second->_herror) return;
    debug("onerror(%u)\n", id);
    it->second->_herror();
}

uint32_t WSJS::nextid()
{
    while (_wsjs_objects.find(_wsjs_nextid) != _wsjs_objects.end()) _wsjs_nextid++; // required if we wrap around
    auto res = _wsjs_nextid;
    _wsjs_nextid++; // post-increment to avoid reuse of ID
    return res;
}

void WSJS::create(WSJS* wsjs, const std::string& uri)
{
    wsjs->_id = WSJS::nextid();
    _wsjs_objects[wsjs->_id] = wsjs;
    debug("create(%u)\n", (unsigned)wsjs->_id);
    EM_ASM({
        var uri = UTF8ToString($0);
        console.log('Connecting to ' + uri);
        if (typeof Module.websockets === 'undefined') Module.websockets = {};
        var ws = new WebSocket(uri);
        ws.binaryType = "arraybuffer";
        ws.id = $1;
        Module.websockets[$1] = ws;
        ws.onopen = function() { if (this.id !== null) Module.wsjs_onopen(this.id); };
        ws.onerror = function() { if (this.id !== null) Module.wsjs_onerror(this.id); };
        ws.onclose = function() { if (this.id !== null) Module.wsjs_onclose(this.id); };
        ws.onmessage = function(e) { if (this.id !== null) Module.wsjs_onmessage(this.id, e.data); };
    }, uri.c_str(), wsjs->_id);
}

void WSJS::destroy(WSJS* wsjs)
{
    debug("destroy(%u)\n", (unsigned)wsjs->_id);
    _wsjs_objects.erase(wsjs->_id);
    EM_ASM({
        // TODO: check if websockets[$0] exists
        Module.websockets[$0].id = null;
        Module.websockets[$0].close();
        delete Module.websockets[$0];
    }, wsjs->_id);
}

void WSJS::send(const std::string& data)
{
    bool isBinary = false; // TODO: detect if data is valid UTF8
    if (isBinary)
        send_binary(data);
    else
        send_text(data);
}

void WSJS::send_binary(const std::string& data)
{
    // binary
    EM_ASM({
        Module.websockets[$0].send(Module.HEAPU8.subarray($1, $1 + $2));
    }, _id, data.c_str(), data.length());
}

void WSJS::send_text(const std::string& data)
{
    // utf8 string -> text
    EM_ASM({
        Module.websockets[$0].send(UTF8ToString($1));
    }, _id, data.c_str());
}

unsigned long WSJS::get_ok_connect_interval() const
{
    auto n = _wsjs_objects.size();
    unsigned long time_per_socket = 30000; // TODO: detect browser
    return n>0 ? time_per_socket * n : time_per_socket; 
}


EMSCRIPTEN_BINDINGS(wsjs) {
    emscripten::function("wsjs_onopen", &WSJS::onopen);
    emscripten::function("wsjs_onclose", &WSJS::onclose);
    emscripten::function("wsjs_onmessage", &WSJS::onmessage);
    emscripten::function("wsjs_onerror", &WSJS::onerror);
}
